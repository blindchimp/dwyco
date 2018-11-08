
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.9
import QtQuick.Controls 2.2

AppDrawerForm {
    //property bool dwy_invis
    //property bool dwy_quiet
    signal close()
    browse_hidden_button.onClicked: {
        simp_tag_browse.to_tag = "_hid"
        stack.push(simp_tag_browse)
        close()
}
    browse_hidden_button.visible: show_hidden
    browse_tags_button.onClicked: {
        simp_tag_browse.to_tag = "_fav"
        stack.push(simp_tag_browse)
        close()
    }
    anchors.fill: parent

//    vid_preview_button.onClicked: {
//        stack.push(vid_cam_preview)
//        close()
//    }
//    about_button.onClicked: {
//        stack.push(about_dialog)
//        close()
//    }
    settings_button.onClicked: {
        stack.push(settings_dialog)
        close()
    }
//    pin_lock_button.onClicked: {
//        stack.push(pwchange_dialog)
//        close()
//    }
//    block_list_button.onClicked: {
//        stack.push(iglist_dialog)
//        close()
//    }

    profile_button.onClicked: {
        profile_update_dialog.preview_existing = true
        stack.push(profile_update_dialog)
        close()
    }
    lock_and_exit_button.onClicked: {
        expire_immediate = true
        core.power_clean()
        if(Qt.platform.os === "android") {
            notificationClient.start_background()
        }
        Qt.quit()
    }

    quiet_switch.checked: dwy_quiet
    invisible_switch.checked: dwy_invis

    quiet_switch.onClicked: {
        dwy_quiet = quiet_switch.checked

        if(Qt.platform.os === "android") {
            notificationClient.set_quiet(dwy_quiet ? 1 : 0)
        }
        core.set_local_setting("quiet", dwy_quiet ? "true" : "false")
        close()
    }
    invisible_switch.onClicked: {
        dwy_invis = invisible_switch.checked
        core.set_local_setting("invis", dwy_invis ? "true" : "false")
        core.set_invisible_state(dwy_invis ? 1 : 0)
        close()
    }

}
