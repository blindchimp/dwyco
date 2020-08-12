
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.3

AppDrawerForm {

    signal close()

    delete_all.onClicked: {
        confirm_delete.visible = true
        close()

    }
    MessageDialog {
        id: confirm_delete
        title: "Delete all"
        icon: StandardIcon.Question
        text: "Delete ALL pictures?"
        informativeText: "This REMOVES FAVORITE pictures too."
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            ConvListModel.set_all_selected(true)
            ConvListModel.delete_all_selected()
            close()
        }
        onNo: {
            close()
        }
    }

    clear_nonfav.onClicked: {
        confirm_delete2.visible = true
        close()
    }

    MessageDialog {
        id: confirm_delete2
        title: "Delete Non-favorites?"
        icon: StandardIcon.Question
        text: "Delete Non-favorite pictures?"
        informativeText: "This KEEPS FAVORITE pictures"
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            var i
            var u
            for(i = 0; i < ConvListModel.count; i++) {
                u = ConvListModel.get(i).uid
                core.clear_messages_unfav(u)
            }

            close()
        }
        onNo: {
            close()
        }
    }


    anchors.fill: parent


    about_button.onClicked: {
        stack.push(about_dialog)
        close()
    }

    help_button.onClicked: {
        stack.push(help_dialog)
        close()
    }

    freebies_switch.checked: dwy_freebies

    freebies_switch.onClicked: {
        dwy_freebies = freebies_switch.checked
        core.set_local_setting("send_freebies", dwy_freebies ? "true" : "false")
        rando_status.refresh()
        //close()
    }



}

