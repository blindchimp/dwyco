
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import dwyco

Page {
    anchors.fill: parent
    header: SimpleToolbar {

    }
    background: Rectangle {
        color: amber_light
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(3)
        //spacing: mm(3)
        //width: parent.width
        RowLayout {
            spacing: mm(3)
            Label {
                text: "Dwyco Rando (open source): " + core.client_name + " (Qt " + core.qt_version_string + ")"
                Layout.fillWidth: true
                fontSizeMode: Text.Fit
            }

        }
        Label {
            text: "(C) 1995-present, Dwyco, Inc."
            Layout.fillWidth: true
        }
        Label {
            text: "Source code is subject to terms of MPL 2.0."
            Layout.fillWidth: true
        }
        Label {
            text: "https://mozilla.org/MPL/2.0/"
            Layout.fillWidth: true
        }
        Label {
            text: "Web: www.dwyco.com"
            Layout.fillWidth: true
        }
        Label {
            text: "Tech support: cdchelp@dwyco.com"
            Layout.fillWidth: true
        }
        Label {
            text: "This software based in part on the work of the Independent JPEG Group."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Button {
            text: "Privacy policy"
            onClicked: {

                Qt.openUrlExternally("https://www.dwyco.net/privacy-policy")
            }
            Layout.fillWidth: true
        }
        Button {
            text: "Terms of service"
            onClicked: {

                Qt.openUrlExternally("https://www.dwyco.net/terms-of-service")
            }
            Layout.fillWidth: true
        }
        Button {
            text: "End User License Agreement"
            onClicked: {

                Qt.openUrlExternally("http://www.dwyco.com/license.txt")
            }
            Layout.fillWidth: true
        }
        Button {
            visible: dwyco_debug
            text: "test"
            onClicked: {

                core.send_debug("a33bbcb888e2e0c11099")
            }
            Layout.fillWidth: true
        }
        Item {
            Layout.fillHeight: true
        }

    }

}
