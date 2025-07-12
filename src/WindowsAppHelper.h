// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#pragma once

#include <QObject>
#include <QFile>
#include <QQmlEngine>
#include <KIO/ApplicationLauncherJob>

using namespace Qt::Literals::StringLiterals;

#define BOTTLES_ID u"com.usebottles.bottles"_s

class WindowsAppHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    // Indicates if Bottles is installed on the system.
    // If this is not the case, the button to open Bottles will instead prompt the user to install Bottles.
    Q_PROPERTY(bool isBottlesInstalled READ isBottlesInstalled CONSTANT)
    // The name of the application that the user is trying to run.
    // This will be the executable name if no app is matched in the database.
    Q_PROPERTY(QString appName READ appName CONSTANT)
    // The icon of the application that the user is trying to run.
    // This will be the icon of the native application if one is available, or a generic icon if not.
    Q_PROPERTY(QString appIcon READ appIcon CONSTANT)
    // Indicates if any native replacement is available.
    // If this is not the case, a generic message will be shown advising the user to find an alternative, or to continue with Bottles.
    Q_PROPERTY(bool hasNativeApp READ hasNativeApp CONSTANT)
    // Indicates if an alternative application is needed, that isn't an exact match for the executable.
    // e.g. if the user is trying to run Internet Explorer 11, Microsoft Edge would be suggested as the alternative.
    Q_PROPERTY(bool needsAlternativeApp READ needsAlternativeApp CONSTANT)
    // Indicates if the application is already installed on the system, whether that is the exact application or an alternative.
    Q_PROPERTY(bool nativeAppAlreadyInstalled READ nativeAppAlreadyInstalled CONSTANT)
    // The name of the alternative application, if one is available.
    // This will be the same as appName if no alternative is available.
    Q_PROPERTY(QString alternativeAppName READ alternativeAppName CONSTANT)
    // The name of the default system app store to use for installing the native application.
    Q_PROPERTY(QString defaultAppStoreName READ defaultAppStoreName CONSTANT)
    // The icon of the default system app store to use for installing the native application.
    Q_PROPERTY(QString defaultAppStoreIcon READ defaultAppStoreIcon CONSTANT)

public:
    explicit WindowsAppHelper(const QUrl &databaseFilePath, const QUrl &openedExePath, QObject *parent = nullptr);
    
    // Static method for QML singleton creation.
    // See https://doc.qt.io/qt-6/qml-singleton.html
    static WindowsAppHelper *create(QQmlEngine *engine, QJSEngine *scriptEngine);

    bool isBottlesInstalled() const {
        return m_isBottlesInstalled;
    };
    QString appName() const {
        return m_appName;
    };
    QString appIcon() const;
    
    bool hasNativeApp() const {
        return m_hasNativeApp;
    };
    bool needsAlternativeApp() const {
        return m_needsAlternativeApp;
    };
    bool nativeAppAlreadyInstalled() const {
        return m_nativeAppAlreadyInstalled;
    };
    QString alternativeAppName() const {
        return m_alternativeAppName;
    };
    QString defaultAppStoreName() const;
    QString defaultAppStoreIcon() const;

    // Opens the software store to install the native application, or opens the native application if it is already installed.
    Q_INVOKABLE void installOrOpenNativeApp() const;
    // Opens the exe with the Bottles application, or prompts the user to install Bottles if it is not installed.
    Q_INVOKABLE void openExeWithOrInstallBottles() const;
    // Opens the "Open With" dialog to select an alternative application to launch with.
    Q_INVOKABLE void openWith() const;
    
    // Set the executable path for singleton creation.
    static void setExecutablePath(const QUrl &path);

private:
    static QUrl s_executablePath;
    
    KService::Ptr defaultAppStore() const;

    QFile m_databaseFile;
    QUrl m_openedExe;
    
    QString m_appName;
    bool m_isBottlesInstalled = false;
    bool m_hasNativeApp = false;
    bool m_needsAlternativeApp = false;
    bool m_nativeAppAlreadyInstalled = false;
    QString m_alternativeAppName;

    // The Flatpak reference for the native app or alternative.
    QString m_flatpakRef;

    // Helper to open the Flatpak reference in the default app store.
    void openRefInAppStore(const QString &ref) const;
};
