
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml
import QtQuick
import dwyco
import QtQuick.Controls
import QtQuick.Layouts

Page {
    anchors.fill: parent
    header: SimpleToolbar {

    }
    background: Rectangle {
        color: amber_light
    }

    Component.onCompleted: {
        var a = core.get_local_setting("tox_enabled")
        if(a === "" || a === "0") {
            enable_tox_cb.checked = false
        } else {
            enable_tox_cb.checked = true
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(2)
        spacing: mm(1)

        CheckBox {
            id: enable_tox_cb
            text: "Enable Tox"
            onCheckedChanged: {
                core.set_local_setting("tox_enabled", checked ? "1" : "0")
                if(checked) {
                    core.enable_tox()
                } else {
                    core.disable_tox()
                }
            }
            Layout.fillWidth: true
        }

        RowLayout {
            enabled: core.tox_enabled
            spacing: mm(1)

            Rectangle {
                id: statusIndicator
                width: 16
                height: 16
                radius: 8
                color: core.tox_connected ? "green" : "red"
            }

            Label {
                text: core.tox_connected ? "Connected" : "Not connected"
                Layout.fillWidth: true
            }
        }

        RowLayout {
            enabled: core.tox_enabled
            spacing: mm(1)

            Button {
                text: "Copy"
                onClicked: core.copy_to_clipboard(toxIdField.text)
            }

            Label {
                id: toxIdField
                text: core.tox_self_address
                font.family: "monospace"
                font.pixelSize: 10
                Layout.fillWidth: true
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
        }

        Label {
            text: "Add Friend"
            enabled: core.tox_enabled
            font.bold: true
            Layout.topMargin: mm(2)
        }

        TextFieldX {
            id: toxIdInput
            enabled: core.tox_enabled
            placeholder_text: "Paste Tox ID here..."
            Layout.fillWidth: true
        }

        Button {
            id: addFriendButton
            text: "Add Friend"
            enabled: core.tox_enabled && toxIdInput.text_input.length > 0
            onClicked: {
                core.tox_add_friend(toxIdInput.text_input, "Hello from Phoo!")
                toxIdInput.text_input = ""
            }
            Layout.fillWidth: true
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
