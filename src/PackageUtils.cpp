// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

#include <QDomDocument>
#include <QProcess>
#include <QRegularExpression>

#include "PackageUtils.h"

void matchFlatpakFromMetainfo(QStringList metainfoFilesContent, QString &nativeAppRef, QString &nativeAppName, bool &hasFlatpakApp, bool &isAnApp)
{
    // Read and parse the extracted metainfo files.
    for (const QString &metainfoContent : metainfoFilesContent) {
        if (metainfoContent.isEmpty()) {
            continue; // Skip empty contents.
        }

        QDomDocument doc;

        auto parseResult = doc.setContent(metainfoContent);
        if (!parseResult) {
            qWarning() << "Failed to parse XML on line" << parseResult.errorLine << "column" << parseResult.errorColumn << ":" << parseResult.errorMessage;
            continue; // Skip this file if parsing fails.
        }

        QDomElement root = doc.documentElement();
        if (!((root.tagName() == u"component"_s && (root.attribute(u"type"_s) == u"desktop"_s || root.attribute(u"type"_s) == u"desktop-application"_s))
              || root.tagName() == u"application"_s)) {
            // This isn't an app or is malformed somehow, so skip it.
            continue;
        }

        QDomElement idElement = root.firstChildElement(u"id"_s);
        QDomElement nameElement = root.firstChildElement(u"name"_s);

        if (!idElement.isNull()) {
            nativeAppRef = idElement.text();
            // Remove the ".desktop" suffix if it exists
            QString suffix = u".desktop"_s;
            if (nativeAppRef.endsWith(suffix)) {
                nativeAppRef.chop(suffix.length());
            }
        }

        if (!nameElement.isNull()) {
            isAnApp = true; // If we have a name (and a metainfo file), we consider this an application.
            nativeAppName = nameElement.text();
        }
    }

    // Perform search only if we have a name and an initial ref to search for.
    // Prioritize searching by m_nativeAppRef as a direct ID match first,
    // then fallback to m_nativeAppName for a name match.
    if (!nativeAppRef.isEmpty() && !nativeAppName.isEmpty()) {
        QProcess flatpakSearchProcess;
        QStringList flatpakArgs;

        // Request both Application ID and Name columns.
        flatpakArgs << u"search"_s << u"--columns=application:f,name:f"_s;

        flatpakArgs << nativeAppRef;

        flatpakSearchProcess.start(u"flatpak"_s, flatpakArgs);
        flatpakSearchProcess.waitForFinished(-1);

        if (flatpakSearchProcess.exitCode() != 0) {
            qWarning() << "Error executing 'flatpak search' for" << nativeAppRef << ":" << flatpakSearchProcess.readAllStandardError();
            qWarning() << "An alternative native application will not be matched for this package.";
            return;
        } else {
            QString searchOutput = QString::fromLocal8Bit(flatpakSearchProcess.readAllStandardOutput()).trimmed();
            QStringList lines = searchOutput.split(u"\n"_s, Qt::SkipEmptyParts);

            // Output format with --columns=application:f,name:f will be:
            // Application ID           Name
            // org.kde.kdenlive         Kdenlive
            // com.spotify.Client       Spotify
            // ...

            // Remove the header line.
            if (!lines.isEmpty() && lines.first().startsWith(u"Application ID"_s)) {
                lines.removeFirst();
            }

            for (const QString &line : lines) {
                QStringList parts = line.split(QRegularExpression(u"\t"_s), Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    QString currentAppId = parts[0].trimmed();
                    QString currentName = parts[1].trimmed();

                    // Strategy:
                    // 1. Try to find an exact match where the Flatpak App ID is identical to m_nativeAppRef
                    if (!nativeAppRef.isEmpty() && currentAppId == nativeAppRef) {
                        // If we find an exact ID match this is the strongest candidate.
                        hasFlatpakApp = true;
                        nativeAppRef = currentAppId;
                        break;
                    }

                    // 2. If no exact ID match, try to find a match where the Flatpak's name matches m_nativeAppName exactly.
                    //    This is a fallback or alternative primary match if the ID isn't directly transferable.
                    //    e.g. the Discord RPM has ID "discord.desktop" but the Flatpak has "com.discordapp.Discord".
                    if (!nativeAppName.isEmpty() && currentName.compare(nativeAppName, Qt::CaseInsensitive) == 0) {
                        // If we already found an exact ID match, prefer it. Otherwise, consider this.
                        hasFlatpakApp = true;
                        nativeAppRef = currentAppId;
                        break;
                    }
                }
            }
        }
    }
}
