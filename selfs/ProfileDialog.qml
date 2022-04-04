
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import dwyco 1.0

Pane {
    anchors.fill: parent
    // this just makes sure mouse events don't go down to other
    // components
//    MouseArea {
//        anchors.fill: parent
//    }

    background: Rectangle {
        gradient: Gradient {
        GradientStop { position: 0.0; color: primary_light }
        GradientStop { position: 1.0; color: primary_dark}
        }
    }
    focusPolicy: Qt.StrongFocus

    ColumnLayout {

        anchors.fill:parent
        anchors.margins: mm(3)

        TextField {
            id: textInput1
            text: fname.fname()
            placeholderText: "Enter camera name (you can change it later)"
            Layout.fillWidth: true
            //activeFocusOnTab: true
        }

        Label {
            id: label1
            text: qsTr("Enter a short name that you use to connect to this camera. You can change it later.")
            font.italic: true
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        TextFieldX {
            id: textInput2
            placeholder_text: qsTr("Email (optional)")
            Layout.fillWidth: true
            visible: false

        }
        Label {
            id: label2
            text: qsTr("Your Email is NOT visible to other users, and we do not sell it or send you spam. If you enter an email address, you can use the email-based features of this app.")
            font.italic: true
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            visible: false
        }
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Label {
            id: label3
            text: qsTr("By clicking OK, you agree to our Terms of Service and Privacy Policy. A quick summary: Your communications are encrypted so Dwyco cannot access them. Dwyco doesn't track you, data-mine your phone, or send you spam.")
            font.italic: true
            color: amber_light
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Button {
            id: done_button
            text: qsTr("OK")
            Layout.fillWidth: true
            focus: true
            Component.onCompleted: {
                forceActiveFocus()
            }

            KeyNavigation.up: textInput1

            onClicked: {
                Qt.inputMethod.commit()
                var name;
                if(textInput1.text.length === 0) {
                    name = "ShinyHappyRock"
                } else {
                    name = textInput1.text
                }

                core.bootstrap(name, textInput2.text_input)
                core.set_local_setting("first-run", "done")
                profile_bootstrapped = 1
                stack.pop()
            }
        }
    }

}
