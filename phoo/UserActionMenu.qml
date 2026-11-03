/*
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import dwyco

Menu {
    title: "Action"
    property string uid
    x: parent.width / 2
    y: parent.height / 2

    // cached state for the current uid. the core calls we need to make the
    // labels below are plain functions with no notifier, so bindings on them
    // would never re-evaluate on their own. instead we snapshot them
    // whenever the uid changes and whenever the menu is about to be shown,
    // which covers pal/block changes made from anywhere else in the ui.
    // note: don't assign to the item text properties from a handler, that
    // would remove their bindings and freeze them.
    property bool is_pal: false
    property bool is_blocked: false

    function refresh_state() {
        if(uid === "") {
            is_pal = false
            is_blocked = false
            return
        }
        is_pal = core.get_pal(uid) !== 0
        is_blocked = core.get_ignore(uid) !== 0
    }

    // names come from the network, so they can be missing entirely for
    // someone we never resolved.
    function who() {
        var n = uid === "" ? "" : core.uid_to_name(uid)
        return n === "" ? "this person" : n
    }

    onUidChanged: refresh_state()
    onAboutToShow: refresh_state()

    MenuItem {
        text: "View profile"
        onTriggered: {
            stack.push(theprofileview)
        }
    }

    MenuItem {
        text: is_pal ? "Unfavorite" : "Favorite"
        onTriggered: {
            ConvListModel.set_pal_uid(uid, !is_pal)
            is_pal = !is_pal
        }
    }

    MenuItem {
        text: "Trash messages"
        onTriggered: {
            confirm_trash.text = "Trash ALL messages from " + who() + "?"
            confirm_trash.visible = true
        }
        MessageYN {
            id: confirm_trash
            title: "Trash messages?"
            text: "Trash ALL messages from user?"
            informativeText: "This KEEPS FAVORITE messages."
            onYesClicked: {
                // tags the messages _trash, so SimpleTrashBrowse can bring
                // them back. (the "_unfav" api would delete them outright,
                // which is not what "trash" here promises)
                core.trash_messages(uid)
                // the unread marker is a uid tag, not a mid tag, so it
                // survives the trash. this also refreshes the conv list item
                // via decorate_user().
                core.reset_unviewed_msgs(uid)
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
        text: is_blocked ? "Blocked" : "Block user"
        enabled: !is_blocked
        onTriggered: {
            // set_ignore, not set_ignore_now: a tox contact has to come off
            // the tox friend list before they can be blocked, which is the
            // ui's job (it offers to take the messages with them).
            core.set_ignore(uid, 1)
        }
    }

    MenuSeparator {}

    MenuItem {
        text: "Block and delete"
        onTriggered: {
            if(!core.is_tox_friend(uid)) {
                confirm_block_delete.text = "Delete ALL messages from " + who() + " and BLOCK them?"
                confirm_block_delete.visible = true
            } else {
                // the tox block dialog covers the whole thing -- it already
                // offers removing the messages, so asking here first would
                // double up the confirms.
                core.set_ignore(uid, 1)
            }
        }
        MessageYN {
            id: confirm_block_delete
            title: "Block and delete?"
            text: "Delete ALL messages from user and BLOCK them?"
            informativeText: "This removes FAVORITE and HIDDEN messages too. (NO UNDO)"
            onYesClicked: {
                // set_ignore_now, not set_ignore: the prompt for this very
                // dialog has already happened, and going back through
                // set_ignore would raise it again.
                core.set_ignore_now(uid, 1)
                // delete_user() reloads the conv list itself.
                core.delete_user(uid)
                themsglist.reload_model()
                close()
            }
            onNoClicked: {
                close()
            }
        }
    }

    MenuItem {
        text: "Delete conversation"
        onTriggered: {
            confirm_delete.text = "Delete ALL messages from " + who() + " permanently?"
            confirm_delete.visible = true
        }
        MessageYN {
            id: confirm_delete
            title: "Delete conversation?"
            text: "Delete ALL messages from user?"
            informativeText: "This removes FAVORITE and HIDDEN messages too. (NO UNDO)"
            onYesClicked: {
                core.delete_user(uid)
                themsglist.reload_model()
                close()
            }
            onNoClicked: {
                close()
            }
        }
    }

    MenuItem {
        text: "Export last 30 days"
        onTriggered: {
            var thirtyDaysAgo = Math.floor(Date.now() / 1000) - (30 * 24 * 60 * 60)
            var result = core.export_conversation(uid, thirtyDaysAgo)
            export_result.text = (result !== "") ? "Conversation exported to: " + result : "Export failed"
            export_result.visible = true
        }
        MessageDialog {
            id: export_result
            title: "Export conversation"
            buttons: MessageDialog.Ok
        }
    }
}
