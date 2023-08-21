
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: textfieldx

    property alias text_input: textInput1.text
    property alias placeholder_text: textInput1.placeholderText
    property alias inputMethodHints: textInput1.inputMethodHints
    property alias inputMask: textInput1.inputMask
    property alias acceptableInput: textInput1.acceptableInput

    property alias readOnly: textInput1.readOnly
    signal accepted()

    TextField {
        id: textInput1
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        onAccepted: {
            textfieldx.accepted()
        }
    }
    Button {
        enabled: !textInput1.readOnly
        text: "x"
        onClicked: {
            textInput1.text = ""
            textInput1.focus = true
        }
        Layout.maximumHeight: textInput1.height
        Layout.maximumWidth: textInput1.height
        Layout.alignment: Qt.AlignVCenter
        activeFocusOnTab: false
    }

    //Layout.fillWidth: true
}
