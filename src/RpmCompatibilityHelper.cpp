// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#include "RpmCompatibilityHelper.h"
#include "PackageUtils.h"

#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KIO/JobUiDelegateFactory>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QProcess>

RpmCompatibilityHelper::RpmCompatibilityHelper(const QUrl &filePath, QObject *parent)
    : ICompatibilityHelper(filePath, parent)
{
    // Initialize the native app name to the file name of the RPM package.
    m_nativeAppName = m_filePath.fileName();
    QProcess findProcess;

    const QString findCommand = u"rpm2cpio %1 | cpio -t --quiet \"%2\" \"%3\""_s.arg(m_filePath.toLocalFile())
                                    .arg(u"./usr/share/metainfo/*.xml"_s)
                                    .arg(u"./usr/local/share/metainfo/*.xml"_s);

    findProcess.start(u"/bin/sh"_s, QStringList() << u"-c"_s << findCommand);
    findProcess.waitForFinished(-1);

    if (findProcess.exitCode() != 0) {
        qWarning() << "Error during metainfo file search:" << findProcess.readAllStandardError();
        qWarning() << "An alternative native application will not be matched for this RPM package.";
        return;
    }

    // Read the filename and trim whitespace (like the trailing newline)
    QStringList specificFilesToExtract = QString::fromLocal8Bit(findProcess.readAllStandardOutput()).trimmed().split(u"\n"_s, Qt::SkipEmptyParts);

    if (specificFilesToExtract.isEmpty()) {
        m_isAnApp = false; // No metainfo files found, so this is not an application.
        return;
    }

    // Extract the metainfo files from the RPM package
    QStringList metainfoFilesContent;
    for (QString &file : specificFilesToExtract) {
        QProcess extractProcess;

        const QString extractCommand = u"rpm2cpio %1 | cpio -i --to-stdout --no-absolute-filenames \"%2\""_s.arg(m_filePath.toLocalFile()).arg(file);

        extractProcess.start(u"/bin/sh"_s, QStringList() << u"-c"_s << extractCommand);
        extractProcess.waitForFinished(-1);

        if (extractProcess.exitCode() != 0) {
            qWarning() << "Error extracting metainfo file:" << extractProcess.readAllStandardError();
            continue;
        }

        metainfoFilesContent.append(QString::fromLocal8Bit(extractProcess.readAllStandardOutput()).trimmed());
    }

    if (metainfoFilesContent.isEmpty()) {
        qWarning() << "An alternative native application will not be matched for this RPM package.";
        m_isAnApp = false; // No metainfo files found, so this is not an application.
        return;
    }

    // See if it exists on Flatpak.
    matchFlatpakFromMetainfo(metainfoFilesContent, m_nativeAppRef, m_nativeAppName, m_hasFlatpakApp, m_isAnApp);
}

QString RpmCompatibilityHelper::windowTitle() const
{
    return nativeAppName();
}

QString RpmCompatibilityHelper::heading() const
{
    if (hasNativeApp()) {
        if (isNativeAppInstalled()) {
            return i18n("Open the native version of %1 instead", nativeAppName());
        } else if (m_hasFlatpakApp) {
            return i18n("Install %1 from %2 instead", nativeAppName(), appStoreName());
        } else if (m_isAnApp) {
            return i18n("Search for %1 in %2 instead", nativeAppName(), appStoreName());
        }
    } else {
        return i18n("RPM packages are not natively supported on %1", distroName());
    }

    return QString();
}

QString RpmCompatibilityHelper::icon() const
{
    if (hasNativeApp() && hasIcon(nativeAppRef())) {
        return nativeAppRef();
    }
    return u"application-x-rpm"_s;
}

QString RpmCompatibilityHelper::description() const
{
    QString desc;

    if (hasNativeApp()) {
        if (isNativeAppInstalled()) {
            desc = i18n("A native %1 version of %2 is already installed on your system. ", distroName(), nativeAppName());
            desc += i18n("It's recommended to use the native version for better system integration.");
        } else if (m_hasFlatpakApp) {
            desc = i18n("A native %1 version of %2 is available for installation. ", distroName(), nativeAppName());
            desc += i18n("Installing the native version is recommended for better system integration.");
        } else if (m_isAnApp) {
            desc += i18n("A native %1 version of %2 may be available for installation from %3. ", distroName(), nativeAppName(), appStoreName());
            desc += i18n("Installing the native version is recommended for better system integration.");
        }
    } else {
        desc = i18n("You can search for alternatives online or in %1.", appStoreName());
    }

    desc += u"<br><br>"_s;
    desc += i18n("Alternatively, you may be able to create a Distrobox to run this RPM package in a containerized environment. ");
    desc += i18n("This is not recommended for most users, as it requires additional advanced setup.");

    return desc;
}

bool RpmCompatibilityHelper::hasNativeApp() const
{
    // The native app action will be to open/install the native app if it exists in Flatpak,
    // or to search for the name in the app store if it doesn't.
    return m_hasFlatpakApp || m_isAnApp;
}

QString RpmCompatibilityHelper::nativeAppName() const
{
    return m_nativeAppName;
}

QString RpmCompatibilityHelper::nativeAppRef() const
{
    return m_nativeAppRef;
}

bool RpmCompatibilityHelper::isNativeAppInstalled() const
{
    return isAppInstalled(nativeAppRef());
}

QString RpmCompatibilityHelper::nativeAppActionText() const
{
    if (isNativeAppInstalled()) {
        return i18n("Open %1", nativeAppName());
    } else if (m_hasFlatpakApp) {
        return i18n("Install %1", nativeAppName());
    } else if (m_isAnApp) {
        return i18n("Search for %1 in %2", nativeAppName(), appStoreName());
    }

    return QString();
}

QString RpmCompatibilityHelper::nativeAppActionIcon() const
{
    if (isNativeAppInstalled()) {
        return nativeAppRef();
    } else {
        return appStoreIcon();
    }
}

void RpmCompatibilityHelper::nativeAppAction() const
{
    if (!hasNativeApp()) {
        qWarning() << "Invalid operation: No native application was found for the provided RPM package.";
        return;
    }

    if (isNativeAppInstalled()) {
        openApp(nativeAppRef());
    } else if (m_hasFlatpakApp) {
        openAppInAppStore(nativeAppRef());
    } else if (m_isAnApp) {
        openAppInAppStore(nativeAppName());
    }
}
