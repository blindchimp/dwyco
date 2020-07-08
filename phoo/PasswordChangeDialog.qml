
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12

Pane {
    id: rectangle1
    //anchors.fill: parent
    //width: 100
    //height: 62


    PasswordDialog {
        id: confirm_old_pw
        visible: {password_ok == 0}
        place_holder_text: "Confirm old password..."
        onExit_clickedChanged: {
            stack.pop()
        }

    }

    Rectangle {
        id: rectangle2
        anchors.fill: parent
        visible: confirm_old_pw.password_ok
        // this just makes sure mouse events don't go down to other
        // components
        MouseArea {
            anchors.fill: parent
        }

        TextField {
            id: pw1
            //height: 50
            anchors.top: parent.top
            anchors.topMargin: 23
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 10
            placeholderText: qsTr("New password...")
            inputMethodHints: Qt.ImhNoAutoUppercase|Qt.ImhNoPredictiveText|Qt.ImhSensitiveData|Qt.ImhLowercaseOnly

        }

        TextField {
            id: pw2
            //height: 50
            anchors.top: pw1.bottom
            anchors.topMargin: 50
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 10
            placeholderText: qsTr("Confirm new password...")
            inputMethodHints: Qt.ImhNoAutoUppercase|Qt.ImhNoPredictiveText|Qt.ImhSensitiveData|Qt.ImhLowercaseOnly

        }

        Button {
            id: ok_button
            //x: 227
            //y: 285
            text: {(pw1.text == pw2.text && pw1.text.length > 0) ? qsTr("Set password") :
                                                                   ((pw1.text == pw2.text && pw1.length == 0) ? qsTr("Remove Password") : qsTr("Cancel"))}
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            onClicked: {
                if(pw1.text == pw2.text) {
                    if(pw1.text.length > 0) {
                        var salt = core.random_string(10);
                        var total = salt + pw1.text;
                        var hash = Qt.md5(total);
                        core.set_local_setting("salt", salt);
                        core.set_local_setting("pw", hash);
                    } else {
                        core.set_local_setting("salt", "");
                        core.set_local_setting("pw", "");
                    }
                }

                pw1.text = ""
                pw2.text = ""
                confirm_old_pw.pw_text = ""
                stack.pop()

            }
        }

    }
}

