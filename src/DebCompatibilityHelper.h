// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#pragma once

#include "ICompatibilityHelper.h"

using namespace Qt::Literals::StringLiterals;

class DebCompatibilityHelper : public ICompatibilityHelper
{
    Q_OBJECT

public:
    explicit DebCompatibilityHelper(const QUrl &filePath, QObject *parent = nullptr);
    ~DebCompatibilityHelper() override = default;

    QString windowTitle() const override;
    QString heading() const override;
    QString icon() const override;
    QString description() const override;
    bool hasNativeApp() const override;
    QString nativeAppActionText() const override;
    QString nativeAppActionIcon() const override;
    bool hasCompatibilityTool() const override
    {
        // Always false for this helper.
        // Maybe at some point work out how to create a Distrobox for DEBs.
        // However, this is probably a bit too complex for people who would be using this helper.
        return false;
    };
    QString compatibilityToolActionText() const override
    {
        // Not implemented.
        return QString();
    }
    QString compatibilityToolActionIcon() const override
    {
        // Not implemented.
        return QString();
    }

    Q_INVOKABLE void nativeAppAction() const override;
    Q_INVOKABLE void compatibilityToolAction() const override
    {
        // Not implemented.
        qWarning() << "Invalid operation: No compatibility tool action is available for RPM files.";
    }

private:
    QString m_nativeAppName;
    QString m_nativeAppRef;

    // Whether a corresponding Flatpak application was found.
    bool m_hasFlatpakApp = false;

    // Whether the DEB package being opened is an actual application.
    // This is used to determine if the helper should offer to search Discover or not.
    // This is set to true if the DEB package has a metainfo file with an application name.
    bool m_isAnApp = false;

    QString nativeAppName() const override;
    QString nativeAppRef() const override;
    bool isCompatibilityToolInstalled() const override
    {
        // Always false for this helper.
        return false;
    }
    bool isNativeAppInstalled() const override;
};
