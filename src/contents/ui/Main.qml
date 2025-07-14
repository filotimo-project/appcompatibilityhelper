// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Thomas Duckworth <tduck@filotimoproject.org>

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.filotimoproject.appcompatibilityhelper

Kirigami.ApplicationWindow {
    id: root

    title: AppCompatibilityHelper.windowTitle

    // This is uniquely moronic, but so is QML.
    // FIXME: In some rare cases, this may clip the content.
    // This looks "good enough", but it would be better to have a proper minimum height determined. Unfortunately, I tried, but QML layouting sucks.
    minimumHeight: pageContent.Layout.minimumHeight + Kirigami.Units.largeSpacing * 10
    height: minimumHeight
    maximumHeight: height

    minimumWidth: Math.max(Kirigami.Units.gridUnit * 30, icon.width + actionButtons.width + Kirigami.Units.largeSpacing * 4 + Kirigami.Units.smallSpacing * 2)
    width: minimumWidth
    maximumWidth: width

    controlsVisible: false
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

    header: Kirigami.Separator {
        Layout.fillWidth: true
    }

    pageStack.initialPage: Kirigami.Page {
        padding: Kirigami.Units.largeSpacing

        ColumnLayout {
            id: pageContent

            spacing: Kirigami.Units.smallSpacing
            anchors.fill: parent

            RowLayout {
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.largeSpacing

                Kirigami.Icon {
                    id: icon
                    Layout.rightMargin: Kirigami.Units.largeSpacing * 2
                    Layout.preferredWidth: Kirigami.Units.iconSizes.large * 2
                    Layout.preferredHeight: Kirigami.Units.iconSizes.large * 2
                    Layout.alignment: Qt.AlignCenter
                    source: AppCompatibilityHelper.icon
                }

                ColumnLayout {
                    spacing: Kirigami.Units.largeSpacing
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true

                    Kirigami.Heading {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: AppCompatibilityHelper.heading
                    }

                    QQC2.Label {
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: AppCompatibilityHelper.description
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
                        AppCompatibilityHelper.openWithAction()
                        root.close()
                    }
                }

                QQC2.Button {
                    id: compatibilityToolActionButton

                    highlighted: !nativeAppActionButton.visible
                    visible: AppCompatibilityHelper.hasCompatibilityTool

                    icon.name: AppCompatibilityHelper.compatibilityToolActionIcon
                    text: AppCompatibilityHelper.compatibilityToolActionText

                    onClicked: {
                        AppCompatibilityHelper.compatibilityToolAction()
                        root.close()
                    }
                }

                QQC2.Button {
                    id: nativeAppActionButton

                    highlighted: true
                    visible: AppCompatibilityHelper.hasNativeApp

                    icon.name: AppCompatibilityHelper.nativeAppActionIcon
                    text: AppCompatibilityHelper.nativeAppActionText

                    onClicked: {
                        AppCompatibilityHelper.nativeAppAction()
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
