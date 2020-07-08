
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import dwyco 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQml 2.2
//import QtGraphicalEffects 1.0

Rectangle {
    id: popupid
    property string warning
    property string inhibit_key
    property bool oops: false
    property alias oops_text : oops_button.text

    anchors.margins: mm(1)
    anchors.fill: parent
    color: "white"
    border.width: mm(1)
    gradient: Gradient {
        GradientStop { position: 0.0; color: primary_light }
        GradientStop { position: 1.0; color: primary_dark}
    }
    MouseArea {
        anchors.fill: parent
    }


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(2)
        spacing: mm(1)

        Label {
            text: warning
            wrapMode: Text.WordWrap
            font.italic: true
            Layout.fillWidth: true
        }
        Item {
            Layout.fillHeight: true
        }

        Button {
            id: oops_button

            onClicked: {
                popupid.visible = false
                oops = true
            }
            visible: {text.length > 0}

            Layout.fillWidth: true
            Layout.bottomMargin: mm(2)
        }

        Button {
            id: gotit
            text: "Got it"
            onClicked: {
                popupid.visible = false
            }

            Layout.fillWidth: true
        }

        Button {
            id: gotit_forever
            text: "Got it, and don't show again"
            visible: {inhibit_key.length > 0}
            onClicked: {
                core.set_local_setting(inhibit_key, "1")
                popupid.visible = false
            }
            Layout.fillWidth: true
        }
    }

}
