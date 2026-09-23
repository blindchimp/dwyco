/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Controls
import QtQml

Menu {
    x: parent.width - width
    transformOrigin: Menu.TopRight

    property string to_uid
    property bool isGroup: false
    signal sendPicture()
    signal sendVideo()

    MenuItem {
        text: "View profile"
        onTriggered: {
            stack.push(theprofileview)
        }
    }

    MenuItem {
        text: "Send picture"
        onTriggered: {
            sendPicture()
        }
    }

    MenuItem {
        text: "Send video message"
        onTriggered: {
            sendVideo()
        }
    }

    MenuItem {
        text: "Browse Msgs"
        onTriggered: {
            stack.push(simp_msg_browse)
        }
    }

    MenuSeparator {
    }

    MenuItem {
        text: isGroup ? "Clear msgs" : "Trash msgs..."
        onTriggered: {
            if(isGroup) {
                core.clear_messages_unfav(to_uid)
                themsglist.reload_model()
            } else {
                confirm_trash.visible = true
            }
        }
        MessageYN {
            id: confirm_trash
            title: "Trash all msgs?"
            text: "Trash ALL (including HIDDEN) msgs from this user?"
            informativeText: "This KEEPS FAVORITE messages."
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
        text: "Delete user"
        visible: isGroup
        onTriggered: {
            confirm_delete.visible = true
        }
        MessageYN {
            id: confirm_delete
            title: "Bulk delete?"
            text: "Delete ALL messages from user?"
            informativeText: "This removes FAVORITE and HIDDEN messages too. (NO UNDO)"
            onYesClicked: {
                core.delete_user(to_uid)
                themsglist.reload_model()
                close()
                stack.pop()
            }
            onNoClicked: {
                close()
            }
        }
    }

    MenuItem {
        text: "More..."
        onTriggered: {
            moremenu.open()
        }
    }
}