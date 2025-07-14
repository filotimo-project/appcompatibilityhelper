// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#pragma once

#include "ICompatibilityHelper.h"

using namespace Qt::Literals::StringLiterals;

class RpmCompatibilityHelper : public ICompatibilityHelper
{
    Q_OBJECT

public:
    explicit RpmCompatibilityHelper(const QUrl &filePath, QObject *parent = nullptr);
    ~RpmCompatibilityHelper() override = default;

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
        // Maybe at some point work out how to create a Distrobox for RPMs.
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

    QString nativeAppName() const override;
    QString nativeAppRef() const override;
    bool isCompatibilityToolInstalled() const override
    {
        // Always false for this helper.
        return false;
    }
    bool isNativeAppInstalled() const override;
};
