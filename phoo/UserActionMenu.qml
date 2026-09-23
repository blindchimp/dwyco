/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Controls

Menu {
    title: "Action"
    property string uid
    x: parent.width / 2
    y: parent.height / 2

    MenuItem {
        text: "View profile"
        onTriggered: {
            stack.push(theprofileview)
        }
    }

    MenuItem {
        text: "Trash msgs"
        onTriggered: {
            confirm_trash.visible = true
        }
        MessageYN {
            id: confirm_trash
            title: "Trash all msgs?"
            text: "Trash ALL (non-favorite) messages from user?"
            informativeText: "This KEEPS FAVORITE messages. You can restore them from the Trash."
            onYesClicked: {
                themsglist.set_all_selected()
                themsglist.trash_all_selected()
                themsglist.invalidate_model_filter()
                themsglist.reload_model()
                close()
            }
            onNoClicked: {
                close()
            }
        }
    }

    MenuItem {
        text: "Clear msgs"
        onTriggered: {
            confirm_clear.visible = true
        }
        MessageYN {
            id: confirm_clear
            title: "Clear all msgs?"
            text: "Delete ALL (non-favorite) messages from user permanently?"
            informativeText: "This KEEPS FAVORITE messages. (NO UNDO)"
            onYesClicked: {
                core.clear_messages(uid)
                themsglist.reload_model()
                close()
            }
            onNoClicked: {
                close()
            }
        }
    }

    MenuSeparator {

    }

    MenuItem {
        text: "Block user"
        onTriggered: {
            core.set_ignore(uid, 1)
        }
    }

    MenuItem {
        text: "Block and Delete user"
        onTriggered: {
            confirm_block_delete.visible = true
        }
        MessageYN {
            id: confirm_block_delete
            title: "Block and delete?"
            text: "Delete ALL messages from user and BLOCK them?"
            informativeText: "This removes FAVORITE and HIDDEN messages too. (NO UNDO)"
            onYesClicked: {
                core.set_ignore(uid, 1)
                core.delete_user(uid)
                themsglist.reload_model()
                stack.pop()
                close()
            }
            onNoClicked: {
                close()
            }
        }
    }

}