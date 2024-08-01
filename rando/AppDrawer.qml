
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Controls.Material

AppDrawerForm {

    signal close()

    delete_all.onClicked: {
        confirm_delete.open()
        close()

    }

    clear_nonfav.onClicked: {
        confirm_delete2.open()
        close()
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

    load_backup_button.onClicked: {
        stack.push(restore_auto_backup)
        close()
    }



}

