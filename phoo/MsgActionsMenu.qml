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
    id: msg_actions_menu

    x: parent.width - width
    transformOrigin: Menu.TopRight

    property string uid
    property string mid
    property bool fav: false
    property bool hid: false
    property bool is_trash: false
    property var msgTextObj: null
    property bool popAfterAction: false

    function finish() {
        themsglist.invalidate_model_filter()
        if(popAfterAction)
            stack.pop()
    }

    function undoPrep() {
        themsglist.invalidate_model_filter()
        if(popAfterAction)
            stack.pop()
    }

    function trashIt() {
        var m = mid
        core.set_tag_message(mid, "_trash")
        undoPrep()
        undo_banner.show("Message trashed", ()=> {
            core.unset_tag_message(m, "_trash")
            themsglist.invalidate_model_filter()
        })
    }

    function untrashIt() {
        var m = mid
        core.unset_tag_message(mid, "_trash")
        undoPrep()
        undo_banner.show("Message restored", ()=> {
            core.set_tag_message(m, "_trash")
            themsglist.invalidate_model_filter()
        })
    }

    function favIt() {
        var m = mid
        var f = fav
        core.set_fav_message(mid, !fav)
        undo_banner.show(fav ? "Removed favorite" : "Favorited message", ()=> {
            core.set_fav_message(m, f)
            themsglist.invalidate_model_filter()
        })
    }

    function hideIt() {
        var m = mid
        core.set_tag_message(mid, "_hid")
        undoPrep()
        undo_banner.show("Message hidden", ()=> {
            core.unset_tag_message(m, "_hid")
            themsglist.invalidate_model_filter()
        })
    }

    function unhideIt() {
        var m = mid
        core.unset_tag_message(mid, "_hid")
        undoPrep()
        undo_banner.show("Message unhidden", ()=> {
            core.set_tag_message(m, "_hid")
            themsglist.invalidate_model_filter()
        })
    }

    MenuItem {
        text: is_trash ? "Untrash msg" : "Trash msg"
        onTriggered: {
            if(is_trash)
                msg_actions_menu.untrashIt()
            else
                msg_actions_menu.trashIt()
        }
    }

    MenuItem {
        text: "Forward msg"
        onTriggered: {
            forward_dialog.mid_to_forward = mid
            stack.push(forward_dialog)
        }
    }

    MenuItem {
        text: fav ? "Unfavorite" : "Favorite"
        onTriggered: msg_actions_menu.favIt()
    }

    MenuItem {
        text: hid ? "Unhide" : "Hide"
        onTriggered: {
            if(hid)
                msg_actions_menu.unhideIt()
            else
                msg_actions_menu.hideIt()
        }
    }

    MenuSeparator {
    }

    MenuItem {
        text: "Copy Text"
        visible: msgTextObj !== null
        onTriggered: {
            msgTextObj.selectAll()
            msgTextObj.copy()
        }
    }

    MenuItem {
        text: "Report"
        onTriggered: {
            stack.push(msg_report)
        }
    }

    MenuItem {
        text: "Review"
        visible: core.this_uid === applicationWindow1.the_man
        onTriggered: {
            stack.push(msg_review)
        }
    }

    MenuItem {
        text: "Delete forever"
        visible: is_trash
        onTriggered: {
            confirm_delete_forever.visible = true
        }
        MessageYN {
            id: confirm_delete_forever
            title: "Delete forever?"
            text: "Permanently delete this message?"
            informativeText: "(NO UNDO)"
            onYesClicked: {
                core.delete_message(uid, mid)
                themsglist.invalidate_model_filter()
                if(popAfterAction)
                    stack.pop()
                close()
            }
            onNoClicked: {
                close()
            }
        }
    }
}