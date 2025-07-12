/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>
*/

#include <QtGlobal>
#include <QApplication>

#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include "version-windowsapphelper.h"
#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>

#include "WindowsAppHelper.h"

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Ensure there's actually something to run.
    if (argc < 2) {
        qWarning() << "No executable file provided.";
        qWarning() << "Usage: windowsapphelper <path to Windows executable>";
        return -1;
    }

    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(u"org.kde.desktop"_s);
    }

    KLocalizedString::setApplicationDomain("windowsapphelper");
    QCoreApplication::setOrganizationName(u"Filotimo Project"_s);

    KAboutData aboutData(
        // The program name used internally.
        u"windowsapphelper"_s,
        // A displayable program name string.
        i18nc("@title", "Windows App Support"),
        // The program version string.
        QStringLiteral(WINDOWSAPPHELPER_VERSION_STRING),
        // Short description of what the app does.
        i18n("Provides support for running or finding alternatives to Windows applications on Linux."),
        // The license this code is released under.
        KAboutLicense::GPL,
        // Copyright Statement.
        i18n("(c) 2025"));
    aboutData.addAuthor(i18nc("@info:credit", "Thomas Duckworth"),
                        i18nc("@info:credit", "Maintainer"),
                        u"tduck@filotimoproject.org"_s,
                        u"https://filotimoproject.org/"_s);
    aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    KAboutData::setApplicationData(aboutData);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"application-x-ms-dos-executable"_s));

    QQmlApplicationEngine engine;

    // Set the executable path for the singleton
    WindowsAppHelper::setExecutablePath(QUrl::fromLocalFile(QString::fromLatin1(argv[1])));

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.loadFromModule("org.filotimoproject.windowsapphelper", u"Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
