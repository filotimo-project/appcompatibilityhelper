// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#include "WindowsCompatibilityHelper.h"

#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KIO/JobUiDelegateFactory>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KService>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>

WindowsCompatibilityHelper::WindowsCompatibilityHelper(const QUrl &databaseFilePath, const QUrl &openedExePath, QObject *parent)
    : ICompatibilityHelper(openedExePath, parent)
{
    m_nativeAppName = m_filePath.fileName();
    m_hasNativeApp = false;
    m_needsAlternativeApp = false;

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
    const QString exeFileName = m_filePath.fileName();

    // Search for matching application in database
    for (const QJsonValue &value : appDb) {
        QJsonObject appEntry = value.toObject();

        // Ignore any entry without a Flatpak reference
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
            m_nativeAppName = appEntry[u"name"_s].toString(exeFileName);

            QJsonObject flatpakObject = appEntry[u"flatpak"_s].toObject();
            if (flatpakObject.contains(u"id"_s) && flatpakObject[u"id"_s].isString()) {
                m_nativeAppRef = flatpakObject[u"id"_s].toString();
            }

            if (appEntry.contains(u"alternative"_s) && appEntry[u"alternative"_s].isObject()) {
                m_needsAlternativeApp = true;
                QJsonObject alternativeObject = appEntry[u"alternative"_s].toObject();
                m_alternativeAppName = alternativeObject[u"name"_s].toString(m_nativeAppName);
            } else {
                m_needsAlternativeApp = false;
                m_alternativeAppName = m_nativeAppName;
            }

            if (appEntry.contains(u"desktopLauncher"_s) && appEntry[u"desktopLauncher"_s].isString()) {
                QString desktopFile = appEntry[u"desktopLauncher"_s].toString();
                if (desktopFile.endsWith(u".desktop"_s)) {
                    desktopFile.chop(8);
                }
            }
            break;
        }
    }
}

QString WindowsCompatibilityHelper::windowTitle() const
{
    return nativeAppName();
}

QString WindowsCompatibilityHelper::heading() const
{
    if (hasNativeApp()) {
        if (isNativeAppInstalled()) {
            return i18n("Open %1 instead", m_alternativeAppName);
        } else {
            return i18n("Install %1 from %2 instead", m_alternativeAppName, appStoreName());
        }
    } else {
        return i18n("Windows applications are not natively supported on %1", distroName());
    }
}

QString WindowsCompatibilityHelper::icon() const
{
    if (hasNativeApp() && hasIcon(nativeAppRef())) {
        return nativeAppRef();
    }
    return u"application-x-ms-dos-executable"_s;
}

QString WindowsCompatibilityHelper::description() const
{
    QString desc;

    if (hasNativeApp()) {
        if (isNativeAppInstalled() && !m_needsAlternativeApp) {
            desc = i18n("A native %1 version of %2 is already installed on your system. ", distroName(), m_alternativeAppName);
            desc += i18n("It's recommended to use the native version for better performance and system integration.");
        } else if (!isNativeAppInstalled() && m_needsAlternativeApp) {
            desc = i18n("%1, a native %2 alternative to %3, is available for installation. ", m_alternativeAppName, distroName(), nativeAppName());
            desc += i18n("Installing the native version is recommended for better performance and system integration.");
        } else {
            desc = i18n("A native %1 version of %2 is available for installation. ", distroName(), m_alternativeAppName);
            desc += i18n("Installing the native version is recommended for better performance and system integration.");
        }
    } else {
        desc = i18n("No native %1 alternative was found for this Windows application. ", distroName());
        desc += i18n("You can search for alternatives online or in %1.", appStoreName());
    }

    if (hasCompatibilityTool()) {
        desc += u"<br><br>"_s;
        if (isCompatibilityToolInstalled()) {
            desc += i18n("Alternatively, you can run the Windows version using Bottles.");
        } else {
            desc += i18n("Alternatively, you can install Bottles to run Windows applications.");
        }
    }

    return desc;
}

bool WindowsCompatibilityHelper::isNativeAppInstalled() const
{
    return isAppInstalled(nativeAppRef());
}

QString WindowsCompatibilityHelper::nativeAppActionText() const
{
    if (isNativeAppInstalled()) {
        return i18n("Open %1", m_alternativeAppName);
    } else {
        return i18n("Install %1", m_alternativeAppName);
    }
}

QString WindowsCompatibilityHelper::nativeAppActionIcon() const
{
    if (isNativeAppInstalled()) {
        return nativeAppRef();
    } else {
        return appStoreIcon();
    }
}

void WindowsCompatibilityHelper::nativeAppAction() const
{
    if (!hasNativeApp()) {
        qWarning() << "Invalid operation: No native application was found for the provided Windows executable.";
        return;
    }

    if (isNativeAppInstalled()) {
        openApp(nativeAppRef());
    } else {
        openAppInAppStore(nativeAppRef());
    }
}

bool WindowsCompatibilityHelper::isCompatibilityToolInstalled() const
{
    return isAppInstalled(BOTTLES_ID);
}

QString WindowsCompatibilityHelper::compatibilityToolActionText() const
{
    // TODO: Make this compatibility tool agnostic.
    if (isCompatibilityToolInstalled()) {
        return i18n("Run with Bottles");
    } else {
        return i18n("Install Bottles");
    }
}

QString WindowsCompatibilityHelper::compatibilityToolActionIcon() const
{
    // TODO: Make this compatibility tool agnostic.
    if (isCompatibilityToolInstalled() && hasIcon(BOTTLES_ID)) {
        return BOTTLES_ID;
    } else {
        return u"plasmadiscover"_s;
    }
}

void WindowsCompatibilityHelper::compatibilityToolAction() const
{
    // TODO: Make this compatibility tool agnostic.
    if (isCompatibilityToolInstalled()) {
        openApp(BOTTLES_ID, {m_filePath});
    } else {
        openAppInAppStore(BOTTLES_ID);
    }
}
