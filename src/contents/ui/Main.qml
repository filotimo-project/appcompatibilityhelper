// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.filotimoproject.windowsapphelper

Kirigami.ApplicationWindow {
    id: root

    title: i18n("%1 — Windows App Support", WindowsAppHelper.appName)

    minimumWidth: appIcon.width + actionButtons.width + Kirigami.Units.largeSpacing * 8
    width: appIcon.width + actionButtons.width + Kirigami.Units.largeSpacing * 8
    minimumHeight: pageStack.initialPage.implicitHeight
    height: minimumHeight

    controlsVisible: false
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

    header: Kirigami.Separator {
        Layout.fillWidth: true
    }

    pageStack.initialPage: Kirigami.Page {
        padding: Kirigami.Units.largeSpacing

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            anchors.fill: parent

            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                Layout.fillHeight: true

                Kirigami.Icon {
                    id: appIcon
                    Layout.margins: Kirigami.Units.largeSpacing * 2
                    Layout.preferredWidth: Kirigami.Units.iconSizes.large * 2
                    Layout.preferredHeight: Kirigami.Units.iconSizes.large * 2
                    Layout.alignment: Qt.AlignCenter
                    source: WindowsAppHelper.appIcon
                }

                ColumnLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.margins: Kirigami.Units.largeSpacing
                    Layout.fillWidth: true

                    Kirigami.Heading {
                        Layout.margins: Kirigami.Units.smallSpacing
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true

                        wrapMode: Text.WordWrap
                        visible: WindowsAppHelper.hasNativeApp && !WindowsAppHelper.needsAlternativeApp && !WindowsAppHelper.nativeAppAlreadyInstalled
                        text: i18n("%1 can be installed from %2", WindowsAppHelper.appName, WindowsAppHelper.defaultAppStoreName)
                    }

                    Kirigami.Heading {
                        Layout.margins: Kirigami.Units.smallSpacing
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true

                        wrapMode: Text.WordWrap
                        visible: WindowsAppHelper.nativeAppAlreadyInstalled
                        text: i18n("%1 is already installed", WindowsAppHelper.appName)
                    }

                    Kirigami.Heading {
                        Layout.margins: Kirigami.Units.smallSpacing
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true

                        wrapMode: Text.WordWrap
                        visible: WindowsAppHelper.hasNativeApp && WindowsAppHelper.needsAlternativeApp && !WindowsAppHelper.nativeAppAlreadyInstalled
                        text: i18n("%1, an alternative for %2, can be installed from %3", WindowsAppHelper.alternativeAppName, WindowsAppHelper.appName, WindowsAppHelper.defaultAppStoreName)
                    }

                    Kirigami.Heading {
                        Layout.margins: Kirigami.Units.smallSpacing
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true

                        wrapMode: Text.WordWrap
                        visible: !WindowsAppHelper.hasNativeApp && !WindowsAppHelper.nativeAppAlreadyInstalled
                        text: WindowsAppHelper.isBottlesInstalled ? i18n("Run %1 with Bottles?", WindowsAppHelper.appName) : i18n("Install Bottles to run %1?", WindowsAppHelper.appName)
                    }

                    QQC2.Label {
                        Layout.margins: Kirigami.Units.smallSpacing
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        wrapMode: Text.WordWrap
                        text: i18n("The file you are trying to open is a Windows executable file, and is not natively supported on Linux-based operating systems.<br><br>Applications built for Windows are not designed for this operating system, but they can be run through compatibility layers such as Bottles. Preferably, install the native version if available.")
                    }
                }
            }

            RowLayout {
                id: actionButtons
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                Layout.fillWidth: true

                QQC2.Button {
                    icon.name: "system-run-symbolic"
                    text: i18n("Open With…")
                    onClicked: {
                        WindowsAppHelper.openWith()
                        root.close()
                    }
                }

                QQC2.Button {
                    highlighted: !WindowsAppHelper.hasNativeApp
                    icon.name: WindowsAppHelper.isBottlesInstalled ? "com.usebottles.bottles" : WindowsAppHelper.defaultAppStoreIcon
                    text: WindowsAppHelper.isBottlesInstalled ? i18n("Run With Bottles") : i18n("Install Bottles From %1", WindowsAppHelper.defaultAppStoreName)
                    onClicked: {
                        WindowsAppHelper.openExeWithOrInstallBottles()
                        root.close()
                    }
                }

                QQC2.Button {
                    highlighted: true
                    visible: WindowsAppHelper.hasNativeApp
                    icon.name: WindowsAppHelper.nativeAppAlreadyInstalled ? WindowsAppHelper.appIcon : WindowsAppHelper.defaultAppStoreIcon
                    text: WindowsAppHelper.nativeAppAlreadyInstalled ? i18n("Open %1", WindowsAppHelper.appName) : i18n("Install From %1", WindowsAppHelper.defaultAppStoreName)
                    onClicked: {
                        WindowsAppHelper.installOrOpenNativeApp()
                        root.close()
                    }
                }

                QQC2.Button {
                    icon.name: "dialog-cancel"
                    text: i18n("Cancel")
                    onClicked: {
                        root.close()
                    }
                }
            }
        }
    }
}
