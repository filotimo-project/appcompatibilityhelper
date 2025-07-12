// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#include "WindowsAppHelper.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDebug>
#include <QCoreApplication>
#include <QIcon>
#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KIO/JobUiDelegateFactory>

#define WINDOWSAPPHELPER_DB_PATH u"/usr/share/windowsapphelper/app_db.json"_s

// Static member definition.
QUrl WindowsAppHelper::s_executablePath;

// Static methods for singleton support.
WindowsAppHelper *WindowsAppHelper::create(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine)
    return new WindowsAppHelper(QUrl::fromLocalFile(WINDOWSAPPHELPER_DB_PATH), s_executablePath, engine);
}

void WindowsAppHelper::setExecutablePath(const QUrl &path)
{
    s_executablePath = path;
}

WindowsAppHelper::WindowsAppHelper(const QUrl &databaseFilePath, const QUrl &openedExePath, QObject *parent)
    : QObject(parent),
      m_openedExe(openedExePath)
{
    // By default, the app name is the name of the executable file itself.
    m_appName = m_openedExe.fileName();

    QFile databaseFile(databaseFilePath.toLocalFile());

    if (!databaseFile.open(QIODevice::ReadOnly | QIODevice::Text) || !databaseFile.exists()) {
        qWarning() << "Failed to open database file:" << databaseFilePath.toLocalFile();
        qWarning() << "The application database is required for matching Windows applications to their native alternatives.";
        return;
    }

    QByteArray databaseData = databaseFile.readAll();
    databaseFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(databaseData);
    
    QJsonArray appDb = doc.array();
    const QString exeFileName = m_openedExe.fileName();

    for (const QJsonValue &value : appDb) {
        QJsonObject appEntry = value.toObject();

        // Ignore any entry without a Flatpak reference.
        // You aren't really supposed to install native packages on ublue-based distros.
        if (!appEntry.contains(u"flatpak"_s) || !appEntry[u"flatpak"_s].isObject()) {
            continue;
        }

        if (!appEntry.contains(u"regex"_s) || !appEntry[u"regex"_s].isObject()) {
            continue;
        }
        QJsonObject regexObject = appEntry[u"regex"_s].toObject();
        if (!regexObject.contains(u"windows"_s) || !regexObject[u"windows"_s].isString()) {
            continue;
        }

        QString regexPattern = regexObject[u"windows"_s].toString();
        QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = regex.match(exeFileName);

        if (match.hasMatch()) {
            m_hasNativeApp = true;
            m_appName = appEntry[u"name"_s].toString(exeFileName);

            QJsonObject flatpakObject = appEntry[u"flatpak"_s].toObject();
            if (flatpakObject.contains(u"id"_s) && flatpakObject[u"id"_s].isString()) {
                m_flatpakRef = flatpakObject[u"id"_s].toString();
            }

            if (appEntry.contains(u"alternative"_s) && appEntry[u"alternative"_s].isObject()) {
                m_needsAlternativeApp = true;
                QJsonObject alternativeObject = appEntry[u"alternative"_s].toObject();
                m_alternativeAppName = alternativeObject[u"name"_s].toString(m_appName);
            } else {
                m_needsAlternativeApp = false;
                m_alternativeAppName = m_appName; // The "alternative" is itself. This shouldn't be necessary, but it ensures we have a value.
            }

            if (appEntry.contains(u"desktopLauncher"_s) && appEntry[u"desktopLauncher"_s].isString()) {
                QString desktopFile = appEntry[u"desktopLauncher"_s].toString();
                if (desktopFile.endsWith(u".desktop"_s)) {
                    // Remove the ".desktop" suffix if it exists for KService to find the app.
                    desktopFile.chop(8);
                }
                m_nativeAppAlreadyInstalled = KService::serviceByDesktopName(desktopFile);
            }
            break;
        }
    }

    m_isBottlesInstalled = KService::serviceByDesktopName(BOTTLES_ID);
}

QString WindowsAppHelper::appIcon() const {
    if (QIcon::hasThemeIcon(m_flatpakRef) && !m_flatpakRef.isEmpty()) {
        return m_flatpakRef;
    }
    return u"application-x-ms-dos-executable"_s;
};    

void WindowsAppHelper::installOrOpenNativeApp() const
{
    if (!m_hasNativeApp) {
        qWarning() << "Invalid operation: No native application was found for the provided executable.";
        return;
    }

    if (m_nativeAppAlreadyInstalled) {
        KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopName(m_alternativeAppName));
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
        job->start();
    } else {
        openRefInAppStore(m_flatpakRef);
    }
}

void WindowsAppHelper::openExeWithOrInstallBottles() const
{
    if (m_isBottlesInstalled) {
        KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopName(BOTTLES_ID));
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
        job->setUrls({m_openedExe});
        job->start();
    } else {
        openRefInAppStore(BOTTLES_ID);
    }
}

void WindowsAppHelper::openWith() const
{
    // Running with no KService will invoke the "Open With" dialog.
    // See https://api.kde.org/frameworks/kio/html/classKIO_1_1ApplicationLauncherJob.html
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob();
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
    job->setUrls({m_openedExe});
    job->start();
}

KService::Ptr WindowsAppHelper::defaultAppStore() const
{
    // TODO: Add actual logic to determine the default app store.
    return KService::serviceByDesktopName(u"org.kde.discover"_s);
}

QString WindowsAppHelper::defaultAppStoreName() const
{
    KService::Ptr store = defaultAppStore();
    return store ? store->name() : QString();
}

QString WindowsAppHelper::defaultAppStoreIcon() const
{
    KService::Ptr store = defaultAppStore();
    return store ? store->icon() : u"plasmadiscover"_s; // Default to Discover icon if it doesn't have one.
}

void WindowsAppHelper::openRefInAppStore(const QString &ref) const
{
    // TODO: Add actual logic to determine the default app store.
    KIO::CommandLauncherJob *job = new KIO::CommandLauncherJob(u"plasma-discover"_s, QStringList() << u"--search"_s << ref );
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
    job->start();
}
