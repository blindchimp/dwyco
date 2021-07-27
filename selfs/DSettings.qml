
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml 2.12
import QtQuick 2.12
import dwyco 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Page {
    property int inh_content_warning: 1
    property bool show_warning : false

    header: SimpleToolbar {

    }
    background: Rectangle {
        color: amber_light
    }

    Component.onCompleted: {
        var duration
        duration = core.get_local_setting("pin_duration")
        if(duration === "") {
            core.set_local_setting("pin_duration", "0")
            duration = "0"
        }
        if(duration !== "0") {
            cb_pin_expire.checked = true

        } else {
            cb_pin_expire.checked = false
        }

        var a
        a = core.get_local_setting("show_unreviewed")
        if(a === "" || a === "0") {
            unreviewed.checked = false
        } else {
            unreviewed.checked = true
        }
        if(core.get_local_setting("inh_content_warning") === "")
            inh_content_warning = 0
        else
            inh_content_warning = 1

        a = core.get_local_setting("show_hidden");
        if(a === "" || a === "1") {
            //themsglist.set_show_hidden(1)
            show_hidden_msgs.checked = true
            show_hidden = true
        } else {
            //themsglist.set_show_hidden(0)
            show_hidden_msgs.checked = false
            show_hidden = false
        }


    }

    Warning {
        id: warn
        visible: false
        z: 3
        warning: "Dwyco reviews all profiles. Normally this app will not display unreviewed or explicit profiles. By checking this box, you agree that YOU MAY RECEIVE UNREVIEWED, EXPLICIT, or OBJECTIONABLE profile content. You can use the BLOCK feature of this app to filter out unwanted profiles."
        inhibit_key: "inh_content_warning"
        oops_text: "Oops, no I don't want that"
        property bool reset_unreviewed
        reset_unreviewed: {!visible && oops}
        onReset_unreviewedChanged: {
            if(reset_unreviewed) {
                unreviewed.checked = false
            }
        }
        onVisibleChanged: {
            if(visible) {
                oops = false
            } else {
                if(core.get_local_setting("inh_content_warning") === "")
                    inh_content_warning = 0
                else
                    inh_content_warning = 1
            }
        }

    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(2)
        spacing: mm(1)

        CheckBox {
            id: cb_pin_expire
            text: "Ask for PIN 30 minutes after close"
            onCheckedChanged: {
                var d
                if(checked) {
                    d = 30 * 60
                } else {
                    d = 0
                }

                core.set_local_setting("pin_duration", d.toString())
                if(d === 0)
                    core.set_local_setting("pin_expire", "")
            }
            Layout.fillWidth: true
        }

        CheckBox {
            id: unreviewed
            text: "Show all profiles\n(WARNING: shows explicit content)"
            onCheckedChanged: {
                show_unreviewed = checked
                core.set_local_setting("show_unreviewed", checked ? "1" : "0")
                if(Qt.platform.os == "android") {
                    if(show_unreviewed)
                        notificationClient.set_user_property("content", "unrev")
                    else
                        notificationClient.set_user_property("content", "rev")
                }
                SimpleDirectoryList.clear()
            }
            onClicked: {
                if(checked) {
                    if(inh_content_warning === 0) {
                        warn.visible = true
                    }
                }
            }
            Layout.fillWidth: true
        }
        CheckBox {
            id: show_hidden_msgs
            text: "Show hidden messages"
            onCheckedChanged: {
                core.set_local_setting("show_hidden", checked ? "1" : "0")
                themsglist.set_show_hidden(checked ? 1 : 0)
                show_hidden = checked
            }
            Layout.fillWidth: true
        }

//        CheckBox {
//            id: show_archived
//            text: { "Show archived users (" + core.total_users.toString() + ")" }
//            onCheckedChanged: {
//                core.use_archived = checked
//                show_archived_users = checked
//            }
//            Layout.fillWidth: true
//        }


        ItemDelegate {
            id: block_list_button
            text: qsTr("Block List")
            onClicked: {
                    stack.push(iglist_dialog)
            }
            Layout.fillWidth: true
        }

        ItemDelegate {
            id: pin_lock_button
            text: qsTr("PIN Lock Setup")
            onClicked: {
                stack.push(pwchange_dialog)
            }

            Layout.fillWidth: true
        }
        ItemDelegate {
            id: about_button
            text: qsTr("About")
            onClicked: {
                    stack.push(about_dialog)
            }
            Layout.fillWidth: true
        }


        Item {
            Layout.fillHeight: true
        }


    }

}
