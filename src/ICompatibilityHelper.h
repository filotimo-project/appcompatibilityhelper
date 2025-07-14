// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#pragma once

#include <KIO/ApplicationLauncherJob>
#include <QFile>
#include <QObject>
#include <QQmlEngine>

using namespace Qt::Literals::StringLiterals;

class ICompatibilityHelper : public QObject
{
    Q_OBJECT

    // The window title to show the user.
    // e.g. "Mozilla Firefox — Windows App Support" if they're trying to install the Windows version.
    Q_PROPERTY(QString windowTitle READ windowTitle CONSTANT)
    // The heading to show the user -- this should be short description of what actions they should take.
    // e.g. "Mozilla Firefox can be installed from Discover" if they're trying to install the Windows version.
    Q_PROPERTY(QString heading READ heading CONSTANT)
    // The icon to show the user -- this should be the icon of the native application if one is available, or a generic icon if not.
    Q_PROPERTY(QString icon READ icon CONSTANT)
    // A useful description of the pathways the user has available to them.
    // e.g. describe how they can install Bottles for their .exe, or how they can find a native alternative.
    Q_PROPERTY(QString description READ description CONSTANT)

    // Indicates if any native (Flatpak) replacement is available.
    // If this is not the case, a generic message should be shown advising the user to find an alternative.
    // It would also indicate to continue with the compatibility tool if one exists.
    Q_PROPERTY(bool hasNativeApp READ hasNativeApp CONSTANT)
    // The text to show the user for the action to install or open the native application.
    Q_PROPERTY(QString nativeAppActionText READ nativeAppActionText CONSTANT)
    // The icon to show the user for the action to install or open the native application.
    Q_PROPERTY(QString nativeAppActionIcon READ nativeAppActionIcon CONSTANT)

    // Indicates if a compatibility tool exists for the executable.
    // e.g. Bottles for running .exe files, or Gear Lever for running AppImages.
    // This would usually be set with a simple `return true/false` in the subclass.
    Q_PROPERTY(bool hasCompatibilityTool READ hasCompatibilityTool CONSTANT)
    // The text to show the user for the action to install or open the compatibility tool.
    Q_PROPERTY(QString compatibilityToolActionText READ compatibilityToolActionText CONSTANT)
    // The icon to show the user for the action to install or open the compatibility tool.
    Q_PROPERTY(QString compatibilityToolActionIcon READ compatibilityToolActionIcon CONSTANT)

public:
    explicit ICompatibilityHelper(QUrl filePath, QObject *parent = nullptr)
        : QObject(parent)
        , m_filePath(filePath)
    {
    }
    virtual ~ICompatibilityHelper() = default;

    virtual QString windowTitle() const = 0;
    virtual QString heading() const = 0;
    virtual QString icon() const = 0;
    virtual QString description() const = 0;
    virtual bool hasNativeApp() const = 0;
    virtual QString nativeAppActionText() const = 0;
    virtual QString nativeAppActionIcon() const = 0;
    virtual bool hasCompatibilityTool() const = 0;
    virtual QString compatibilityToolActionText() const = 0;
    virtual QString compatibilityToolActionIcon() const = 0;

    // Opens the software store to install the native application, or opens the native application if it is already installed.
    Q_INVOKABLE virtual void nativeAppAction() const;
    // Opens the exe with the chosen compatibility tool, or prompts the user to install the compatibility tool if it is not installed.
    Q_INVOKABLE virtual void compatibilityToolAction() const;
    // Opens the "Open With" dialog to select an alternative application to launch the file with.
    // This is a generic action, so it doesn't need to be overridden in subclasses.
    Q_INVOKABLE void openWithAction() const;

protected:
    // Indicates if the application is already installed on the system, whether the exact application or an alternative.
    // This doesn't need to be exposed to the QML interface, as it is only used internally to determine how to display the native app action.
    virtual bool isNativeAppInstalled() const = 0;

    // Provides the name of the native application, e.g. "Microsoft Edge" or "Mozilla Firefox".
    // This doesn't need to be exposed to the QML interface, as it is only used internally to determine how to display the native app action.
    virtual QString nativeAppName() const = 0;

    // Provides the reference to the native application, e.g. "org.mozilla.firefox".
    // This doesn't need to be exposed to the QML interface, as it is only used internally for the native app action.
    virtual QString nativeAppRef() const = 0;

    // Indicates if the compatibility tool is already installed on the system.
    // This doesn't need to be exposed to the QML interface, as it is only used internally to determine how to display the compatibility tool action.
    virtual bool isCompatibilityToolInstalled() const = 0;

    // Helper to open a reference to an app in the default app store.
    void openAppInAppStore(const QString &ref) const;

    // Helper that returns the icon for the default app store, e.g. "plasmadiscover" or "io.github.kolunmi.Bazaar".
    QString appStoreIcon() const;

    // Helper that returns the name of the default app store, e.g. "Discover" or "Bazaar".
    QString appStoreName() const;

    // Helper to open an app.
    void openApp(const QString &ref, const QList<QUrl> &urls = {}) const;

    // Helper to check if an app is installed.
    bool isAppInstalled(const QString &ref) const;

    // Helper to check if an icon exists for the given application reference.
    bool hasIcon(const QString &ref) const;

    // Helper to return the distro name.
    QString distroName() const;

    // The file path of the executable/package being opened.
    QUrl m_filePath;
};
