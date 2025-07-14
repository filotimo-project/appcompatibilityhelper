// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#include "DebCompatibilityHelper.h"

#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KIO/JobUiDelegateFactory>
#include <KLocalizedContext>
#include <KLocalizedString>

DebCompatibilityHelper::DebCompatibilityHelper(const QUrl &filePath, QObject *parent)
    : ICompatibilityHelper(filePath, parent)
{
    m_nativeAppName = m_filePath.fileName();
    // TODO
}

QString DebCompatibilityHelper::windowTitle() const
{
    return nativeAppName();
}

QString DebCompatibilityHelper::heading() const
{
    if (hasNativeApp()) {
        return i18n("Open %1 instead", nativeAppName());
    } else {
        return i18n("DEB packages are not natively supported on %1", distroName());
    }
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
        } else {
            desc = i18n("A native %1 version of %2 is available for installation. ", distroName(), nativeAppName());
            desc += i18n("Installing the native version is recommended for better system integration.");
        }
    } else {
        desc = i18n("No native %1 alternative was found for this DEB package. ", distroName());
        desc += i18n("You can search for alternatives online or in %1.", appStoreName());
    }

    desc += u"<br><br>"_s;
    desc += i18n("Alternatively, you may be able to create a Distrobox to run this DEB package in a containerized environment. ");
    desc += i18n("This is not recommended for most users, as it requires additional advanced setup.");

    return desc;
}

bool DebCompatibilityHelper::hasNativeApp() const
{
    // TODO
    return false;
}

QString DebCompatibilityHelper::nativeAppName() const
{
    return m_nativeAppName;
}

QString DebCompatibilityHelper::nativeAppRef() const
{
    // TODO
    return QString();
}

bool DebCompatibilityHelper::isNativeAppInstalled() const
{
    return isAppInstalled(nativeAppRef());
}

QString DebCompatibilityHelper::nativeAppActionText() const
{
    if (isNativeAppInstalled()) {
        return i18n("Open %1", nativeAppName());
    } else {
        return i18n("Install %1", nativeAppName());
    }
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
    } else {
        openAppInAppStore(nativeAppRef());
    }
}
