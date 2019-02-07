
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
    property int password_ok
    property int exit_clicked
    property alias place_holder_text : pw.placeholderText
    property alias pw_text : pw.text

    anchors.fill: parent

    password_ok : {
        var salt = core.get_local_setting("salt")
        if(salt.length === 0)
            return 1
        var target = core.get_local_setting("pw");
        var total = salt + pw.text
        if(target === Qt.md5(total))
            return 1
        return 0
    }
    // this just makes sure mouse events don't go down to other
    // components
    MouseArea {
        anchors.fill: parent
    }
    TextField {
        id: pw
        //y: 97
        text: ""
        inputMask: ""
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        //echoMode: 2
        placeholderText: qsTr("Password...")
        focus: visible
        inputMethodHints: Qt.ImhNoAutoUppercase|Qt.ImhNoPredictiveText|Qt.ImhSensitiveData|Qt.ImhLowercaseOnly
    }

    Button {
        id: exit_button
        //y: 273
        text: qsTr("Cancel")
        anchors.left: parent.left
        anchors.leftMargin: 23
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        onClicked: {
            exit_clicked = 1
        }
    }



}

