
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1
import dwyco 1.0

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

        TextFieldX {
            id: textInput1
            text_input: fname.fname()
            placeholder_text: "Enter nickname (you can change it later)"
            Layout.fillWidth: true
        }

        Label {
            id: label1
            text: qsTr("Enter a short nick name that is displayed to other users. You can change it later.")
            font.italic: true
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        TextFieldX {
            id: textInput2
            placeholder_text: qsTr("Email (optional)")
            Layout.fillWidth: true

        }
        Label {
            id: label2
            text: qsTr("If you enter an email address, you can use email-based features of this app. Your Email is NOT visible to other users, and we do not sell it or send you spam.")
            font.italic: true
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Label {
            id: label3
            text: qsTr("By clicking OK, you agree to our Terms of Service and Privacy Policy. A quick summary: Your communications are encrypted so Dwyco cannot read them. Dwyco doesn't track you, data-mine your phone, or send you spam.")
            font.italic: true
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
                var name;
                if(textInput1.text_input.length === 0) {
                    name = "ShinyHappyRock"
                } else {
                    name = textInput1.text_input
                }

                core.bootstrap(name, textInput2.text_input)
                core.set_local_setting("first-run", "done")
                profile_bootstrapped = 1
                stack.pop()
            }
        }
    }

}
