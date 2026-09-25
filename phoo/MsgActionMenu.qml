
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// The one per-message menu. Used everywhere a message can be right-clicked
// (chat box, group chat, and all the browse grids) and as the overflow menu in
// the message viewer.
//
// It replaces the old MsgViewMenu / MsgViewMenuTrash pair, which were 90%
// identical and differed only in whether the message happened to be in the
// trash. Here that is a runtime question: the menu asks the core about the
// message when it opens, so the same component is correct on every page and
// there is no way to end up with the wrong variant.
//
// x/y are left unset so a caller can position it: right-click sites set them
// from the mouse, the viewer toolbar anchors it to the button.

import QtQuick
import QtQuick.Controls
import dwyco

Menu {
    id: menu

    property string mid: ""
    property string uid: ""
    property string msg_text: ""
    // a message the core still has queued for download has nothing to save yet
    property bool has_attachment: false
    // the message viewer is a page in the stack, so tagging the message out
    // from under it means leaving the page
    property bool pop_on_change: false
    property bool show_save: true
    property bool show_report: true
    property bool show_review: true

    // refreshed from the core every time the menu opens, rather than bound:
    // there is no notifier to re-evaluate a binding against, and the tags can
    // change from the footer buttons or a bulk operation while the menu is up
    property bool is_trashed: false
    property bool is_fav: false
    property bool is_hid: false

    readonly property bool has_msg: mid !== ""
    readonly property bool is_moderator: core.this_uid === applicationWindow1.the_man

    // Open at a point given in Overlay.overlay coordinates.
    //
    // Deliberately not implemented as "menu.x = px; menu.popup()":
    //  * mapToItem() cannot take a Popup as its argument at all -- the engine
    //    refuses to convert a Menu to const QQuickItem* -- so the caller has to
    //    map against some other item anyway. The overlay is the obvious one.
    //  * reparenting into the overlay also gets the menu out of whatever
    //    delegate/ListView declared it, which is clipped and recycles.
    //  * popup(item, x, y) positions relative to `item` and lets Qt clamp the
    //    menu so it stays on screen, which is what a context menu should do.
    function showAt(px, py) {
        parent = Overlay.overlay
        popup(Overlay.overlay, px, py)
    }

    onVisibleChanged: {
        if(!visible)
            return
        is_trashed = core.has_tag_message(mid, "_trash") === 1
        is_fav = core.has_tag_message(mid, "_fav") === 1
        is_hid = core.has_tag_message(mid, "_hid") === 1
    }

    MenuItem {
        text: menu.is_trashed ? qsTr("Restore Message") : qsTr("Trash Message")
        visible: menu.has_msg
        onTriggered: {
            if(menu.is_trashed)
                ops.untrash_msg(menu.mid)
            else
                ops.trash_msg(menu.mid)
            if(menu.pop_on_change)
                stack.pop()
        }
    }

    MenuItem {
        text: qsTr("Favorite Message")
        visible: menu.has_msg && !menu.is_fav
        onTriggered: ops.fav_msg(menu.mid, true)
    }

    MenuItem {
        text: qsTr("Remove Favorite")
        visible: menu.has_msg && menu.is_fav
        onTriggered: ops.fav_msg(menu.mid, false)
    }

    MenuItem {
        text: menu.is_hid ? qsTr("Unhide Message") : qsTr("Hide Message")
        visible: menu.has_msg
        onTriggered: ops.hide_msg(menu.mid, !menu.is_hid)
    }

    MenuItem {
        text: qsTr("Forward Message")
        visible: menu.has_msg
        onTriggered: ops.forward_msg(menu.mid)
    }

    MenuSeparator {
        visible: menu.has_msg
    }

    MenuItem {
        text: qsTr("Copy Text")
        enabled: menu.msg_text !== ""
        onTriggered: {
            if(!ops.copy_text(menu.msg_text))
                ops.notify(qsTr("Nothing to copy"))
        }
    }

    MenuItem {
        text: qsTr("Save Attachment")
        visible: menu.has_msg && menu.show_save && menu.has_attachment
        onTriggered: {
            var name = ops.export_msg(menu.mid)
            if(name === "")
                ops.notify(qsTr("Could not save attachment"))
            else
                ops.notify(qsTr("Saved to ") + name)
        }
    }

    MenuItem {
        text: qsTr("Report Message")
        visible: menu.has_msg && menu.show_report
        onTriggered: ops.report_msg(menu.mid, menu.uid, menu.msg_text)
    }

    MenuItem {
        text: qsTr("Review Message")
        visible: menu.has_msg && menu.show_review && menu.is_moderator
        onTriggered: ops.review_msg(menu.mid, menu.uid, menu.msg_text)
    }

    MenuSeparator {
        visible: menu.has_msg && menu.is_trashed
    }

    MenuItem {
        text: qsTr("Delete Forever")
        visible: menu.has_msg && menu.is_trashed
        onTriggered: {
            ops.delete_msg_forever(menu.mid, menu.uid)
            if(menu.pop_on_change)
                stack.pop()
        }
    }
}
