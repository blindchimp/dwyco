
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// The one per-user menu. Every list of users in the app (conversations,
// the chat sidebar, the room roster, the public directory, contact query
// results) right-clicks into this, so the same user offers the same actions
// wherever you find them.
//
// The caller primes uid and kind, then calls showAt() with a point in
// Overlay.overlay coordinates so the menu lands under the cursor instead of in
// the middle of the window.
//
// Everything destructive-ish goes through ops.qml, so Trashing is undoable and
// Blocking is undoable, and the banner says so.

import QtQuick
import QtQuick.Controls
import dwyco

Menu {
    id: menu

    property string uid: ""
    // which list this was opened from. "conversation" is the conversation list
    // and chat sidebars, "stranger" is the public directory and contact-query
    // results (people you have probably never written to, so there is no
    // conversation to trash or delete), "roster" is the chat room member list.
    property string kind: "conversation"

    readonly property bool is_stranger: kind === "stranger"
    readonly property bool is_roster: kind === "roster"
    readonly property bool show_trash_msgs: !is_stranger
    readonly property bool show_delete_conv: !is_stranger && !is_roster

    // Open at a point given in Overlay.overlay coordinates. See the comment on
    // MsgActionMenu.showAt() for why this is not a plain x/y assignment.
    function showAt(px, py) {
        parent = Overlay.overlay
        popup(Overlay.overlay, px, py)
    }

    // read fresh on each open: the block state and favorite state can change
    // from the multi-select toolbar or from another page while we were closed
    property bool is_blocked: false
    property bool is_fav: false

    onVisibleChanged: {
        if(!visible)
            return
        is_blocked = core.get_ignore(uid) !== 0
        is_fav = core.get_pal(uid) !== 0
    }

    MenuItem {
        text: qsTr("Open Conversation")
        onTriggered: ops.open_conv(menu.uid)
    }

    MenuItem {
        text: qsTr("View Profile")
        onTriggered: {
            top_dispatch.last_uid_selected = menu.uid
            stack.push(theprofileview)
        }
    }

    MenuSeparator {
    }

    MenuItem {
        text: menu.is_fav ? qsTr("Remove from Favorites") : qsTr("Add to Favorites")
        onTriggered: ops.fav_user(menu.uid, !menu.is_fav)
    }

    MenuItem {
        text: qsTr("Trash Messages...")
        visible: menu.show_trash_msgs
        onTriggered: confirm_trash_msgs.open()
        MessageYN {
            id: confirm_trash_msgs
            title: qsTr("Trash all messages?")
            text: qsTr("Trash all messages from this user?")
            informativeText: qsTr("This KEEPS FAVORITE messages. You can undo this.")
            onYesClicked: {
                ops.trash_user_msgs(menu.uid)
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
        text: menu.is_blocked ? qsTr("Unblock User") : qsTr("Block User")
        onTriggered: ops.block_user(menu.uid, !menu.is_blocked)
    }

    MenuItem {
        text: qsTr("Delete Conversation")
        visible: menu.show_delete_conv
        onTriggered: confirm_delete_conv.open()
        MessageYN {
            id: confirm_delete_conv
            title: qsTr("Delete conversation?")
            text: qsTr("Delete ALL messages from this user?")
            informativeText: qsTr("This removes FAVORITE messages too, and cannot be undone.")
            onYesClicked: {
                ops.delete_user(menu.uid)
                close()
            }
            onNoClicked: {
                close()
            }
        }
    }
}
