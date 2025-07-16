// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#include "ICompatibilityHelper.h"

#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KIO/JobUiDelegateFactory>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QIcon>

void ICompatibilityHelper::openAppInAppStore(const QString &ref) const
{
    // TODO: Add actual logic to determine the default app store.
    // ...or, get a specific app store which is configurable by the vendor.
    // This can be done with a KConfig object as a dependency to ICompatibilityHelper.
    // This can also be used for subclass-specific configuration, i.e. for setting specific compatibility tools.
    KIO::CommandLauncherJob *job = new KIO::CommandLauncherJob(u"plasma-discover"_s, QStringList() << u"--search"_s << ref);
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
    job->start();
}

void ICompatibilityHelper::openApp(const QString &ref, const QList<QUrl> &urls) const
{
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopName(ref));
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
    job->setUrls(urls);
    job->start();
}

bool ICompatibilityHelper::isAppInstalled(const QString &ref) const
{
    KService::Ptr service = KService::serviceByDesktopName(ref);
    return service && service->isValid() && service->isApplication();
}

QString ICompatibilityHelper::appStoreIcon() const
{
    // TODO: See openAppInAppStore.
    return u"plasmadiscover"_s; // Defaulting to Discover for now.
}

QString ICompatibilityHelper::appStoreName() const
{
    // TODO: See openAppInAppStore.
    return i18n("Discover"); // Defaulting to Discover for now.
}

void ICompatibilityHelper::openWithAction() const
{
    // Running with no KService will invoke the "Open With" dialog.
    // See https://api.kde.org/frameworks/kio/html/classKIO_1_1ApplicationLauncherJob.html
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob();
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
    job->setUrls({m_filePath});
    job->start();
}

QString ICompatibilityHelper::distroName() const
{
    static QString distroName = []() {
        QFile file(u"/etc/os-release"_s);

        // Check if the file can be opened for reading
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Could not open /etc/os-release.";
            qWarning() << "The distro name could not be determined from /etc/os-release. Defaulting to \"Linux\".";
            return i18n("Linux"); // Default to "Linux".
        }

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith(u"NAME="_s)) {
                QString distroName = line.section(u"="_s, 1, 1); // Get the part after the '='

                // Remove surrounding quotes if they exist
                if (distroName.startsWith(u"\""_s) && distroName.endsWith(u"\""_s)) {
                    distroName = distroName.mid(1, distroName.length() - 2);
                }

                file.close();
                return distroName;
            }
        }

        file.close();

        qWarning() << "The distro name could not be determined from /etc/os-release. Defaulting to \"Linux\".";
        return i18n("Linux"); // Default to "Linux".
    }();

    return distroName;
}

bool ICompatibilityHelper::hasIcon(const QString &ref) const
{
    return QIcon::hasThemeIcon(ref);
}

// Default implementations for the pure virtual Q_INVOKABLEs in ICompatibilityHelper.
// These should be overridden in subclasses to provide specific functionality.
// This is to avoid linker errors as the MOC is not able to resolve these without default implementations.
void ICompatibilityHelper::nativeAppAction() const
{
    qWarning() << "nativeAppAction() was called on base class - this should always be overridden in subclasses.";
    qWarning() << "Please file a bug report.";
}

void ICompatibilityHelper::compatibilityToolAction() const
{
    qWarning() << "compatibilityToolAction() was called on base class - this should always be overridden in subclasses.";
    qWarning() << "Please file a bug report.";
}
