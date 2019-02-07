
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
import QtQuick.Controls 2.1

Rectangle {
    id: rectangle1
    //width: 100
    //height: 62
    anchors.fill: parent

    // this just makes sure mouse events don't go down to other
    // components
    MouseArea {
        anchors.fill: parent
    }

    TextField {
        id: pw1
        x: 49
        y: 23
        width: 188
        height: 25
        anchors.horizontalCenter: parent.horizontalCenter
        placeholderText: qsTr("Type password...")
        inputMethodHints: Qt.ImhNoAutoUppercase|Qt.ImhNoPredictiveText|Qt.ImhSensitiveData|Qt.ImhLowercaseOnly
    }

    TextField {
        id: pw2
        y: 68
        anchors.right: pw1.right
        anchors.rightMargin: 0
        anchors.left: pw1.left
        anchors.leftMargin: 0
        placeholderText: qsTr("Confirm password...")
        inputMethodHints: Qt.ImhNoAutoUppercase|Qt.ImhNoPredictiveText|Qt.ImhSensitiveData|Qt.ImhLowercaseOnly
    }

    Button {
        id: ok_button
        x: 227
        y: 285
        text: {(pw1.text == pw2.text && pw1.text.length > 0) ? qsTr("Set password") : qsTr("Cancel")}
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        onClicked: {
            if(pw1.text == pw2.text && pw1.text.length > 0) {
                var salt = core.random_string(10);
                var total = salt + pw1.text;
                var hash = Qt.md5(total);
                core.set_local_setting("salt", salt);
                core.set_local_setting("pw", hash);
            }
            pw1.text = ""
            pw2.text = ""
            rectangle1.visible = false

        }
    }

}

