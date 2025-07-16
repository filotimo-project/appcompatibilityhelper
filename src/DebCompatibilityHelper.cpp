// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#include "DebCompatibilityHelper.h"
#include "PackageUtils.h"

#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KIO/JobUiDelegateFactory>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QProcess>

DebCompatibilityHelper::DebCompatibilityHelper(const QUrl &filePath, QObject *parent)
    : ICompatibilityHelper(filePath, parent)
{
    m_nativeAppName = m_filePath.fileName();

    // Find the name of the data archive (e.g., data.tar.xz, data.tar.zst)
    QProcess findDataProcess;
    const QString findDataCommand = u"ar t %1 | grep '^data.tar'"_s.arg(filePath.toLocalFile());
    findDataProcess.start(u"/bin/sh"_s, QStringList() << u"-c"_s << findDataCommand);
    findDataProcess.waitForFinished(-1);

    const QString dataArchiveName = QString::fromLocal8Bit(findDataProcess.readAllStandardOutput()).trimmed();
    if (dataArchiveName.isEmpty()) {
        qWarning() << "Could not find a data.tar.* archive in the .deb package.";
        qWarning() << "An alternative native application will not be matched for this RPM package.";
        return;
    }

    QProcess findFileProcess;
    const QString findFileCommand = u"ar p %1 %2 | tar -Jt --wildcards \"%3\" \"%4\""_s.arg(filePath.toLocalFile())
                                        .arg(dataArchiveName)
                                        .arg("./usr/share/metainfo/*.xml")
                                        .arg("./usr/local/share/metainfo/*.xml");

    findFileProcess.start(u"/bin/sh"_s, QStringList() << u"-c"_s << findFileCommand);
    findFileProcess.waitForFinished(-1);

    const QStringList specificFilesToExtract = QString::fromLocal8Bit(findFileProcess.readAllStandardOutput()).trimmed().split(u"\n"_s, Qt::SkipEmptyParts);

    if (specificFilesToExtract.isEmpty()) {
        m_isAnApp = false; // No metainfo files found, so this is not an application.
        return;
    }

    // Extract each metainfo file found in the previous step
    QStringList metainfoFilesContent;
    for (const QString &file : specificFilesToExtract) {
        QProcess extractProcess;

        // Note the -O flag to extract to stdout
        const QString extractCommand = u"ar p %1 %2 | tar -xJOf - \"%3\""_s.arg(filePath.toLocalFile()).arg(dataArchiveName).arg(file);

        extractProcess.start(u"/bin/sh"_s, QStringList() << u"-c"_s << extractCommand);
        extractProcess.waitForFinished(-1);

        if (extractProcess.exitCode() != 0) {
            qWarning() << "Error extracting file:" << extractProcess.readAllStandardError();
            continue;
        }

        metainfoFilesContent.append(QString::fromUtf8(extractProcess.readAllStandardOutput()).trimmed());
    }

    if (metainfoFilesContent.isEmpty()) {
        qWarning() << "An alternative native application will not be matched for this DEB package.";
        m_isAnApp = false; // No metainfo files found, so this is not an application.
        return;
    }

    matchFlatpakFromMetainfo(metainfoFilesContent, m_nativeAppRef, m_nativeAppName, m_hasFlatpakApp, m_isAnApp);
}

QString DebCompatibilityHelper::windowTitle() const
{
    return nativeAppName();
}

QString DebCompatibilityHelper::heading() const
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
        return i18n("DEB packages are not natively supported on %1", distroName());
    }

    return QString();
}

QString DebCompatibilityHelper::icon() const
{
    if (hasNativeApp() && hasIcon(nativeAppRef())) {
        return nativeAppRef();
    }
    return u"application-vnd.debian.binary-package"_s;
}

QString DebCompatibilityHelper::description() const
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
    desc += i18n("Alternatively, you may be able to create a Distrobox to run this DEB package in a containerized environment. ");
    desc += i18n("This is not recommended for most users, as it requires additional advanced setup.");

    return desc;
}

bool DebCompatibilityHelper::hasNativeApp() const
{
    // The native app action will be to open/install the native app if it exists in Flatpak,
    // or to search for the name in the app store if it doesn't.
    return m_hasFlatpakApp || m_isAnApp;
}

QString DebCompatibilityHelper::nativeAppName() const
{
    return m_nativeAppName;
}

QString DebCompatibilityHelper::nativeAppRef() const
{
    return m_nativeAppRef;
}

bool DebCompatibilityHelper::isNativeAppInstalled() const
{
    return isAppInstalled(nativeAppRef());
}

QString DebCompatibilityHelper::nativeAppActionText() const
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

QString DebCompatibilityHelper::nativeAppActionIcon() const
{
    if (isNativeAppInstalled()) {
        return nativeAppRef();
    } else {
        return appStoreIcon();
    }
}

void DebCompatibilityHelper::nativeAppAction() const
{
    if (!hasNativeApp()) {
        qWarning() << "Invalid operation: No native application was found for the provided DEB package.";
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
