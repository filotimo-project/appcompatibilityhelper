// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#include "CompatibilityHelperFactory.h"
#include "ICompatibilityHelper.h"
#include "WindowsCompatibilityHelper.h"
#include "directories.h"

#include <QMimeDatabase>

ICompatibilityHelper *CompatibilityHelperFactory::create(const QUrl &filePath)
{
    if (!filePath.isValid() || !filePath.isLocalFile()) {
        return nullptr;
    }

    QMimeDatabase mimeDb;
    QString mimeTypeName = mimeDb.mimeTypeForFile(filePath.toLocalFile()).name();

    if (mimeTypeName == u"application/x-ms-dos-executable"_s || mimeTypeName == u"application/x-msi"_s || mimeTypeName == u"application/x-ms-shortcut"_s) {
        return createWindowsCompatibilityHelper(QUrl::fromLocalFile(WINDOWSCOMPATIBILITYHELPER_DB_PATH), filePath);
    }

    // This returns when no compatible helper was found for the given file type.
    // At this point, the program should exit.
    return nullptr;
}

ICompatibilityHelper *CompatibilityHelperFactory::createWindowsCompatibilityHelper(const QUrl &databaseFilePath, const QUrl &openedExePath)
{
    return new WindowsCompatibilityHelper(databaseFilePath, openedExePath);
}
