
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

    property string origName: ""
    property string origStatus: ""

    Component.onCompleted: {
        var a = core.get_local_setting("tox_enabled")
        if(a === "" || a === "0") {
            enable_tox_cb.checked = false
        } else {
            enable_tox_cb.checked = true
        }
        ToxFriendModel.load_friends()
        toxNameInput.text_input = core.tox_get_name()
        toxStatusInput.text_input = core.tox_get_status_message()
        origName = toxNameInput.text_input
        origStatus = toxStatusInput.text_input
        var curStatus = core.tox_get_user_status()
        var statusIdx = ["none", "away", "busy"].indexOf(curStatus)
        if(statusIdx >= 0)
            userStatusCombo.currentIndex = statusIdx
    }

    Timer {
        interval: 5000
        running: core.tox_enabled
        repeat: true
        onTriggered: ToxFriendModel.load_friends()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(2)
        spacing: mm(1)

            RowLayout {
                spacing: mm(1)

                CheckBox {
                    id: enable_tox_cb
                    text: "Enable Tox"
                    onCheckedChanged: {
                        core.set_local_setting("tox_enabled", checked ? "1" : "0")
                        if(checked) {
                            core.enable_tox()
                            toxNameInput.text_input = core.tox_get_name()
                            toxStatusInput.text_input = core.tox_get_status_message()
                            origName = toxNameInput.text_input
                            origStatus = toxStatusInput.text_input
                        } else {
                            core.disable_tox()
                        }
                    }
                }

                Rectangle {
                    id: statusIndicator
                    width: 16
                    height: 16
                    radius: 8
                    color: core.tox_connected ? "green" : "red"
                    enabled: core.tox_enabled
                }

                Label {
                    text: core.tox_connected ? "Connected" : "Not connected"
                    enabled: core.tox_enabled
                }

                Label {
                    text: "Status:"
                    enabled: core.tox_enabled
                }

                ComboBox {
                    id: userStatusCombo
                    model: ["Available", "Away", "Busy"]
                    enabled: core.tox_enabled
                    onActivated: {
                        var map = ["none", "away", "busy"]
                        core.tox_set_user_status(map[currentIndex])
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            RowLayout {
                enabled: core.tox_enabled
                spacing: mm(1)

                TextFieldX {
                    id: toxNameInput
                    placeholder_text: "Enter client name..."
                    Layout.fillWidth: true
                }

                TextFieldX {
                    id: toxStatusInput
                    placeholder_text: "Enter status message..."
                    Layout.fillWidth: true
                }

                Button {
                    text: "Update"
                    enabled: core.tox_enabled && (toxNameInput.text_input !== origName || toxStatusInput.text_input !== origStatus)
                    onClicked: {
                        core.tox_set_name(toxNameInput.text_input)
                        core.tox_set_status_message(toxStatusInput.text_input)
                        origName = toxNameInput.text_input
                        origStatus = toxStatusInput.text_input
                    }
                }
            }

            RowLayout {
                enabled: core.tox_enabled
                spacing: mm(1)

                Button {
                    text: "Copy Tox ID"
                    onClicked: core.copy_to_clipboard(core.tox_self_address)
                }

                Label {
                    id: toxIdField
                    text: core.tox_self_address.substring(0, 8)
                    font.family: "monospace"
                    font.pixelSize: 10
                    Layout.fillWidth: true
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

            Label {
                text: "Friends"
                enabled: core.tox_enabled
                font.bold: true
                Layout.topMargin: mm(2)
                visible: core.tox_enabled
            }

            Item {
                enabled: core.tox_enabled
                visible: core.tox_enabled
                Layout.fillWidth: true
                Layout.fillHeight: true
                implicitHeight: mm(40)

                ListView {
                    id: friendList
                    anchors.fill: parent
                    clip: true
                    spacing: mm(1)
                    model: ToxFriendModel
                    currentIndex: -1
                    highlight: Rectangle {
                        color: amber_accent
                        opacity: 0.3
                    }
                    highlightMoveDuration: 200
                    ScrollBar.vertical: ScrollBar { }

                    delegate: Item {
                        width: ListView.view.width
                        height: mm(9)

                        MouseArea {
                            anchors.fill: parent
                            onClicked: friendList.currentIndex = index
                            onDoubleClicked: {
                                var pseudo_uid = pubkey.substring(0, 20)
                                top_dispatch.uid_selected(pseudo_uid, "clicked")
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            color: "transparent"

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: mm(1)
                                spacing: mm(0.5)

                                RowLayout {
                                    spacing: mm(1)
                                    Layout.fillWidth: true

                                    Text {
                                        text: name
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }

                                    Rectangle {
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: status === "offline" ? "gray" : user_status === "busy" ? "red" : user_status === "away" ? "orange" : "green"
                                        Layout.alignment: Qt.AlignVCenter
                                    }

                                    Text {
                                        text: status === "offline" ? "offline" : user_status === "none" ? "online" : user_status
                                        font.pixelSize: 10
                                        color: status === "offline" ? "gray" : user_status === "busy" ? "red" : user_status === "away" ? "orange" : "green"
                                    }
                                }

                                Text {
                                    text: pubkey.substring(0, 8)
                                    font.family: "monospace"
                                    font.pixelSize: 9
                                    color: "#666"
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: ToxFriendModel.count === 0
                    text: qsTr("(No friends yet. Add one above.)")
                }
            }

            Button {
                text: "Delete Friend"
                enabled: friendList.currentIndex >= 0
                Layout.fillWidth: true
                onClicked: deleteFriendDialog.open()
            }
        }

    Dialog {
        id: deleteFriendDialog
        title: "Delete Friend"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        anchors.centerIn: Overlay.overlay

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "Delete this friend and remove them from your contact list?"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            CheckBox {
                id: trashMessagesCb
                text: "Also trash all messages with this friend"
                checked: false
            }
        }

        onAccepted: {
            var f = ToxFriendModel.get(friendList.currentIndex)
            if(trashMessagesCb.checked)
                core.trash_messages(f.pubkey.substring(0, 20))
            core.tox_delete_friend(f.pubkey)
            friendList.currentIndex = -1
            ToxFriendModel.load_friends()
            trashMessagesCb.checked = false
        }

        onRejected: {
            trashMessagesCb.checked = false
        }
    }
}
