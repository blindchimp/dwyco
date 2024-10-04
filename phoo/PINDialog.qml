
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
//import QtQuick.Window 2.2
import QtQuick.Layouts
import QtQuick.Controls
//import QtQuick.Controls.Material 2.1

Rectangle {
    property int password_ok
    property string pw
    property string correct_pw : "feeblenumps"
    property string salt: ""
    property string title: "Enter PIN"
    property bool show_set_pin_button : false
    property int xhit
    property alias exit_button : exit_button
    property alias set_pin_button: set_pin_button
    property alias set_pin_button_text: set_pin_button.text

    anchors.fill: parent
    color: primary_dark
    gradient: Gradient {
        GradientStop { position: 0.0; color: primary_light }
        GradientStop { position: 1.0; color: primary_dark}
    }

    function reset() {
        pw = ""
        salt = ""
        correct_pw = calc_pw(pw)
        xhit = 0
    }

    function load_cpw() {
        correct_pw = core.get_local_setting("pw")
        salt = core.get_local_setting("salt")
    }

    function calc_pw(str) {
        if(salt.length === 0)
            return ""
        var h
        h = salt + str
        return Qt.md5(h)
    }

    password_ok : {
        if(calc_pw(pw) === correct_pw || xhit === 5) {
            if(xhit === 5) {
                corporate_censorship = false
            }

            return 1
        }
        else
            return 0
    }

    MouseArea {
        anchors.fill: parent
    }

    ColumnLayout {
        id: topcol
        //anchors.fill: parent
        anchors.left: parent.left
        anchors.top:parent.top
        anchors.right: parent.right
        anchors.bottom: l3.top
        anchors.margins: 3
        //anchors.margins: 10
        Label {
            id: l1
            Layout.fillWidth: true
            text: title
            horizontalAlignment: Text.AlignHCenter
        }
        RowLayout {
            id: l2
            Layout.fillWidth: true

            TextInput {
                //Layout.columnSpan: 2
                Layout.fillWidth: true
                text: pw
                horizontalAlignment: TextInput.AlignHCenter
                readOnly: true
                //inputMask: "DDDD"
                echoMode: {show.checked ? TextInput.Normal : TextInput.Password}

            CheckBox {
                id: show
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: "Show"
                checked: true
                onClicked: {
                    xhit = xhit + 1
                }
            }
            }

        }
        GridLayout {
            rows: 3
            columns: 3
            property int sz
            sz: Math.min(parent.height - l1.height - l2.height /*- l3.height*/, parent.width)
            Layout.minimumHeight: sz
            Layout.minimumWidth: sz
            Layout.maximumHeight: sz
            Layout.maximumWidth: sz
            Layout.alignment: Qt.AlignCenter

            Repeater {
                model: 8

                RoundButton {
                    contentItem: Text {
                        anchors.centerIn: parent
                        text: index + 1
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                    }
//                    background: Rectangle {
//                        //radius: parent.width / 2
//                        border.color: "blue"
//                        border.width: 3

//                    }

                    Layout.fillWidth: true; Layout.fillHeight: true

                    enabled: { pw.length < 4 }
                    onClicked: {
                        pw += index + 1
                        xhit = 0
                    }
                }
            }
            RoundButton {
                //text: "x"
                enabled: { pw.length > 0 }
                contentItem: Text {
                    anchors.centerIn: parent
                    text: "<"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true

                }
//                background: Rectangle {
//                    radius: width / 2
//                    border.color: "red"
//                    border.width: 4

//                }
                onClicked: {
                    pw = pw.substring(0, pw.length - 1)
                    xhit = 0
                }
                Layout.fillWidth: true; Layout.fillHeight: true
            }
        }
    }

        RowLayout {
            id: l3
            Layout.fillWidth: true
            height: exit_button.height
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 3

            Button {
                id: exit_button
                text: qsTr("Cancel")

            }
            Item {Layout.fillWidth: true}
            Button {
                id: set_pin_button
                visible: show_set_pin_button
                text: qsTr("Set PIN")
            }
        }

}
