
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
        ToxFriendModel.load_friends()
        toxNameInput.text_input = core.tox_get_name()
        toxStatusInput.text_input = core.tox_get_status_message()
    }

    Timer {
        interval: 5000
        running: core.tox_enabled
        repeat: true
        onTriggered: ToxFriendModel.load_friends()
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: mm(2)
        contentHeight: col.implicitHeight
        clip: true
        ScrollBar.vertical: ScrollBar { }

        ColumnLayout {
            id: col
            width: parent.width
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
                    onClicked: {
                        if(toxNameInput.text_input.length > 0)
                            core.tox_set_name(toxNameInput.text_input)
                        if(toxStatusInput.text_input.length > 0)
                            core.tox_set_status_message(toxStatusInput.text_input)
                    }
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
                                    text: pubkey
                                    font.family: "monospace"
                                    font.pixelSize: 9
                                    elide: Text.ElideRight
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
                onClicked: {
                    var f = ToxFriendModel.get(friendList.currentIndex)
                    core.tox_delete_friend(f.pubkey)
                    friendList.currentIndex = -1
                    ToxFriendModel.load_friends()
                }
            }
        }
    }
}
