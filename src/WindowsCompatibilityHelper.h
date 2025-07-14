// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#pragma once

#include "ICompatibilityHelper.h"
#include <KIO/ApplicationLauncherJob>
#include <QFile>
#include <QObject>
#include <QQmlEngine>
#include <qobject.h>

using namespace Qt::Literals::StringLiterals;

#define BOTTLES_ID u"com.usebottles.bottles"_s

class WindowsCompatibilityHelper : public ICompatibilityHelper
{
    Q_OBJECT

public:
    explicit WindowsCompatibilityHelper(const QUrl &databaseFilePath, const QUrl &openedExePath, QObject *parent = nullptr);
    ~WindowsCompatibilityHelper() override = default;

    QString windowTitle() const override;
    QString heading() const override;
    QString icon() const override;
    QString description() const override;
    bool hasNativeApp() const override
    {
        // If it's in the database, it has a native app.
        // The database is read in the constructor, which is where this member is set.
        return m_hasNativeApp;
    };
    QString nativeAppActionText() const override;
    QString nativeAppActionIcon() const override;
    bool hasCompatibilityTool() const override
    {
        // Always true for this helper.
        return true;
    };

    QString compatibilityToolActionText() const override;
    QString compatibilityToolActionIcon() const override;

    Q_INVOKABLE void nativeAppAction() const override;
    Q_INVOKABLE void compatibilityToolAction() const override;

private:
    QString m_nativeAppName;
    QString m_alternativeAppName;
    QString m_nativeAppRef;

    QString nativeAppName() const override
    {
        return m_nativeAppName;
    }
    QString nativeAppRef() const override
    {
        return m_nativeAppRef;
    }
    bool isCompatibilityToolInstalled() const override;
    bool isNativeAppInstalled() const override;

    bool m_hasNativeApp = false;
    // e.g. if the user opens ie11.exe, this will be true as the Flatpak alternative is Microsoft Edge.
    bool m_needsAlternativeApp = false;
};
