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

#include "ICompatibilityHelper.h"
#include "version-appcompatibilityhelper.h"
#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <qcoreapplication.h>

#include "CompatibilityHelperFactory.h"

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Ensure there's actually something to run.
    if (argc < 2) {
        qWarning() << "No executable file provided.";
        qWarning() << "Usage: appcompatibilityhelper <path to file>";
        return -1;
    }

    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(u"org.kde.desktop"_s);
    }

    KLocalizedString::setApplicationDomain("appcompatibilityhelper");
    QCoreApplication::setOrganizationName(u"Filotimo Project"_s);

    KAboutData aboutData(
        // The program name used internally.
        u"appcompatibilityhelper"_s,
        // A displayable program name string.
        i18nc("@title", "App Compatibility Support"),
        // The program version string.
        QStringLiteral(APPCOMPATIBILITYHELPER_VERSION_STRING),
        // Short description of what the app does.
        i18n("Provides support for running or finding alternatives to certain package types on ublue-based distributions."),
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

    // Register the correct compatibility helper as a QML singleton.
    QUrl filePath = QUrl::fromLocalFile(QString::fromLatin1(argv[1]));
    qmlRegisterSingletonType<ICompatibilityHelper>("org.filotimoproject.appcompatibilityhelper",
                                                   1,
                                                   0,
                                                   "AppCompatibilityHelper",
                                                   [filePath](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
                                                       Q_UNUSED(engine)
                                                       Q_UNUSED(scriptEngine)

                                                       ICompatibilityHelper *helper = CompatibilityHelperFactory::create(filePath);
                                                       if (!helper) {
                                                           qWarning() << "No compatible helper found for the provided file type.";
                                                           qWarning() << "The application will now exit.";
                                                           QCoreApplication::exit(-1);
                                                       }
                                                       return helper;
                                                   });

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.loadFromModule("org.filotimoproject.appcompatibilityhelper", u"Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
