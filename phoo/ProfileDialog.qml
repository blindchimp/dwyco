
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

Rectangle {
    anchors.fill: parent
    // this just makes sure mouse events don't go down to other
    // components
    MouseArea {
        anchors.fill: parent
    }
    ColumnLayout {

        anchors.fill:parent
        anchors.margins: mm(3)
//        RowLayout {
//            TextField {
//                id: textInput1
//                placeholderText:  qsTr("Enter nickname (you can change it later)")
//                text: fname.fname()
//                Layout.fillWidth: true
//                Layout.alignment: Qt.AlignVCenter
//            }
//            Button {
//                text: "x"
//                onClicked: {
//                    textInput1.text = ""
//                }
//                Layout.maximumHeight: textInput1.height
//                Layout.maximumWidth: textInput1.height
//                Layout.alignment: Qt.AlignVCenter
//            }

//            Layout.fillWidth: true
//        }
        TextFieldX {
            id: textInput1
            text_input: fname.fname()
            placeholder_text: "Enter nickname (you can change it later)"
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
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Button {
            id: done_button
            text: qsTr("OK")
            Layout.fillWidth: true
            onClicked: {
                Qt.inputMethod.commit()
                core.bootstrap(textInput1.text_input, textInput2.text_input)
                core.set_local_setting("first-run", "done")
                //parent.visible = false
                profile_bootstrapped = 1
                stack.pop()
            }
        }
    }

}
