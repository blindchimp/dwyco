
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.0
import QtQuick.Controls 1.4

Menu {
    title: "Action"
    property string uid
    property string mid

    MenuItem {
        text: "Delete msg"
        onTriggered: {
            core.delete_message(uid, mid)
            themsglist.reload_model()
        }
    }


    MenuItem {
        text: "Forward msg"
        onTriggered: {
            forward_dialog.mid_to_forward = mid
            forward_dialog.uid_folder = uid
            stack.push(forward_dialog)
        }
    }

}
