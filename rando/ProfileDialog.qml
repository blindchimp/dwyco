
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material
import dwyco

Rectangle {
    anchors.fill: parent
    // this just makes sure mouse events don't go down to other
    // components
    MouseArea {
        anchors.fill: parent
    }
    gradient: Gradient {
        GradientStop { position: 0.0; color: primary_light }
        GradientStop { position: 1.0; color: primary_dark}
    }
    ColumnLayout {

        anchors.fill:parent
        anchors.margins: mm(3)

        Label {
            id: textInput1
            text: fname.fname()

            Layout.fillWidth: true

        }

        Label {
            id: label1
            text: qsTr("(visible only to tech support, you can ignore this)")
            font.italic: true
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

//        TextFieldX {
//            id: textInput2
//            placeholder_text: qsTr("Email (optional)")
//            Layout.fillWidth: true

//        }
//        Label {
//            id: label2
//            text: qsTr("Your Email is NOT visible to other users, and we do not sell it or send you spam. If you enter an email address, you can use the email-based features of this app.")
//            font.italic: true
//            wrapMode: Text.WordWrap
//            Layout.fillWidth: true
//        }
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Label {
            id: label3
            text: qsTr("By clicking OK, you agree to our Terms of Service and Privacy Policy. A quick summary: Dwyco doesn't track you, data-mine your phone, or send you spam.")
            font.italic: true
            color: amber_light
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Label {
            id: label4
            text: qsTr("Dwyco REVIEWS pictures sent using this app. Please send only APPROPRIATE PICTURES. Thanks.")
            font.italic: true
            font.bold: true
            color: amber_light
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Button {
            id: done_button
            text: qsTr("OK")
            Layout.fillWidth: true
            onClicked: {
                Qt.inputMethod.commit()
                enabled = false
                var name;
                name = "R:"
                if(textInput1.text.length === 0) {
                    name += "ShinyHappyRock"
                } else {
                    name += textInput1.text
                }
                core.init()
                core.bootstrap(name, "rando")
                core.set_local_setting("first-run", "done")
                profile_bootstrapped = 1
                busy.running = true
                if(Qt.platform.os == "android") {
                    notificationClient.log_event2("TOSOK", "regular")
                }
            }
        }
    }

    BusyIndicator {
        id: busy
        running: false
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        z: 5
    }

}
