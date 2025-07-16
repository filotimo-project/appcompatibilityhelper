// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#include <QDebug>
#include <QString>

using namespace Qt::Literals::StringLiterals;

// Match a Flatpak application based on an app's metainfo file.
// This is used to find a corresponding Flatpak application for an RPM/DEB package.
void matchFlatpakFromMetainfo(QStringList metainfoFilesContent, QString &nativeAppRef, QString &nativeAppName, bool &hasFlatpakApp, bool &isAnApp);
