
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// Every mutating operation the user can trigger lives here, instantiated once
// in main.qml as "ops". Context menus (UserActionMenu, MsgActionMenu), the
// menu bar (AppMenuBar) and the older per-page overflow menus all call these
// same functions, so the same action behaves the same way no matter where it
// was invoked from -- and, more importantly, so that every reversible action
// goes through exactly one place that knows how to undo itself.
//
// Rules of the house:
//   * reversible  -> do the thing, then undo_hub.push(label, closure)
//   * irreversible -> do the thing, push nothing, keep the confirm dialog
// Never hand-roll a set_tag_message/unset_tag_message pair at a call site;
// add a function here instead.

import QtQuick
import dwyco

Item {
    id: ops
    visible: false
    enabled: false

    // the message the user last left-clicked or right-clicked. the menu bar's
    // Message menu binds to these so it always acts on something concrete.
    property string current_mid: ""
    property string current_uid: ""
    property bool current_is_trash: false
    property bool current_is_fav: false
    property bool current_is_hid: false

    // re-read the tags of the current message. done after every mutation so
    // menu items that label themselves off these stay truthful.
    function sync_current() {
        if(current_mid === "") {
            current_is_trash = false
            current_is_fav = false
            current_is_hid = false
            return
        }
        current_is_trash = core.has_tag_message(current_mid, "_trash") === 1
        current_is_fav = core.has_tag_message(current_mid, "_fav") === 1
        current_is_hid = core.has_tag_message(current_mid, "_hid") === 1
    }

    function n_label(n, singular, plural) {
        if(n === 1)
            return singular
        return plural.replace("%1", n)
    }

    // after a tag change the message lists have to re-filter, and the
    // conversation list has to re-decorate (it draws the hidden/unread dots)
    function refresh() {
        themsglist.invalidate_model_filter()
        ConvListModel.invalidate_model_filter()
    }

    function set_mid(mid, uid, is_trash) {
        current_mid = mid === undefined ? "" : mid
        current_uid = uid === undefined ? "" : uid
        if(is_trash !== undefined)
            current_is_trash = is_trash
        sync_current()
    }

    // ------------------------------------------------------------------
    // single message
    // ------------------------------------------------------------------

    function set_trash(mid, on) {
        if(on)
            core.set_tag_message(mid, "_trash")
        else
            core.unset_tag_message(mid, "_trash")
        refresh()
    }

    function trash_msg(mid) {
        if(!mid)
            return
        set_trash(mid, true)
        sync_current()
        undo_hub.push(qsTr("Message trashed"), function() { ops.set_trash(mid, false) })
    }

    function untrash_msg(mid) {
        if(!mid)
            return
        set_trash(mid, false)
        sync_current()
        undo_hub.push(qsTr("Message restored"), function() { ops.set_trash(mid, true) })
    }

    function set_fav(mid, on) {
        core.set_fav_message(mid, on ? 1 : 0)
        refresh()
    }

    function fav_msg(mid, on) {
        if(!mid)
            return
        set_fav(mid, on)
        sync_current()
        undo_hub.push(on ? qsTr("Message favorited") : qsTr("Message removed from favorites"),
                      function() { ops.set_fav(mid, !on) })
    }

    function set_hid(mid, on) {
        if(on)
            core.set_tag_message(mid, "_hid")
        else
            core.unset_tag_message(mid, "_hid")
        refresh()
    }

    function hide_msg(mid, on) {
        if(!mid)
            return
        set_hid(mid, on)
        sync_current()
        undo_hub.push(on ? qsTr("Message hidden") : qsTr("Message unhidden"),
                      function() { ops.set_hid(mid, !on) })
    }

    function delete_msg_forever(mid, uid) {
        if(!mid)
            return
        core.delete_message(uid, mid)
        refresh()
        sync_current()
    }

    function forward_msg(mid) {
        if(!mid)
            return
        forward_dialog.mid_to_forward = mid
        stack.push(forward_dialog)
    }

    function export_msg(mid) {
        if(!mid)
            return ""
        var export_name = core.export_attachment(mid)
        if(export_name.length > 0) {
            if(Qt.platform.os == "android")
                notificationClient.share_to_mediastore(export_name)
            return export_name.substring(export_name.lastIndexOf('/') + 1)
        }
        return ""
    }

    // msg_report and msg_review are children of MsgView, so they read mid/uid/
    // msg_text off of it. prime them before pushing.
    function prime_msgview(mid, uid, text) {
        themsgview.mid = mid
        themsgview.uid = uid
        themsgview.msg_text = text === undefined ? "" : text
    }

    function report_msg(mid, uid, text) {
        if(!mid)
            return
        prime_msgview(mid, uid, text)
        stack.push(msg_report)
    }

    function review_msg(mid, uid, text) {
        if(!mid)
            return
        prime_msgview(mid, uid, text)
        stack.push(msg_review)
    }

    // ------------------------------------------------------------------
    // bulk messages (the selection lives on the shared themsglist proxy)
    // ------------------------------------------------------------------

    function select_all_msgs() {
        themsglist.set_all_selected()
    }

    function clear_msg_selection() {
        themsglist.set_all_unselected()
    }

    // the models hand back the mids they actually changed, so the count in the
    // banner and the undo set agree with reality (favorites and still-queued
    // messages are skipped by the models, so they are skipped here too)
    function bulk_trash_msgs() {
        var mids = themsglist.trash_all_selected()
        refresh()
        if(mids.length === 0)
            return
        var label = n_label(mids.length, qsTr("Message trashed"), qsTr("%1 messages trashed"))
        var list = mids
        undo_hub.push(label, function() { ops.untrash_mids(list) })
    }

    function bulk_obliterate_msgs() {
        themsglist.obliterate_all_selected()
        refresh()
    }

    function untrash_mids(mids) {
        for(var i = 0; i < mids.length; ++i)
            core.unset_tag_message(mids[i], "_trash")
        refresh()
    }

    function trash_mids(mids) {
        for(var i = 0; i < mids.length; ++i)
            core.set_tag_message(mids[i], "_trash")
        refresh()
    }

    function bulk_untrash_msgs() {
        var mids = themsglist.untag_all_selected("_trash")
        refresh()
        if(mids.length === 0)
            return
        var label = n_label(mids.length, qsTr("Message restored"), qsTr("%1 messages restored"))
        var list = mids
        undo_hub.push(label, function() { ops.trash_mids(list) })
    }

    function bulk_fav_msgs(on) {
        var mids = themsglist.fav_all_selected(on ? 1 : 0)
        refresh()
        if(mids.length === 0)
            return
        var label = on ? n_label(mids.length, qsTr("Message favorited"),
                                 qsTr("%1 messages favorited"))
                       : n_label(mids.length, qsTr("Message removed from favorites"),
                                 qsTr("%1 messages removed from favorites"))
        var list = mids
        undo_hub.push(label, function() {
            for(var i = 0; i < list.length; ++i)
                ops.set_fav(list[i], !on)
        })
    }

    function bulk_hide_msgs(on) {
        var mids = on ? themsglist.tag_all_selected("_hid")
                      : themsglist.untag_all_selected("_hid")
        refresh()
        if(mids.length === 0)
            return
        var label = on ? n_label(mids.length, qsTr("Message hidden"), qsTr("%1 messages hidden"))
                       : n_label(mids.length, qsTr("Message unhidden"), qsTr("%1 messages unhidden"))
        var list = mids
        undo_hub.push(label, function() {
            for(var i = 0; i < list.length; ++i)
                ops.set_hid(list[i], !on)
        })
    }

    // ------------------------------------------------------------------
    // single user
    // ------------------------------------------------------------------

    function set_blocked(uid, on) {
        core.set_ignore(uid, on ? 1 : 0)
    }

    function block_user(uid, on) {
        if(!uid)
            return
        set_blocked(uid, on)
        undo_hub.push(on ? qsTr("User blocked") : qsTr("User unblocked"),
                      function() { ops.set_blocked(uid, !on) })
    }

    function set_fav_user(uid, on) {
        core.set_pal(uid, on ? 1 : 0)
    }

    function fav_user(uid, on) {
        if(!uid)
            return
        set_fav_user(uid, on)
        undo_hub.push(on ? qsTr("User added to favorites")
                         : qsTr("User removed from favorites"),
                      function() { ops.set_fav_user(uid, !on) })
    }

    // tags every non-favorite message from this user with "_trash"
    function trash_user_msgs(uid) {
        if(!uid)
            return
        var mids = ConvListModel.trash_user_msgs(uid)
        refresh()
        if(mids.length === 0)
            return
        var label = n_label(mids.length, qsTr("Message trashed"), qsTr("%1 messages trashed"))
        var list = mids
        undo_hub.push(label, function() { ops.untrash_mids(list) })
    }

    function delete_user(uid) {
        if(!uid)
            return
        core.delete_user(uid)
        refresh()
    }

    function open_conv(uid) {
        if(!uid)
            return
        top_dispatch.last_uid_selected = uid
        stack.push(chatbox)
    }

    // ------------------------------------------------------------------
    // bulk users (selection lives on the shared ConvListModel)
    // ------------------------------------------------------------------

    function select_all_users(on) {
        ConvListModel.set_all_selected(on)
    }

    function bulk_block_users(on) {
        var uids = ConvListModel.selected_uids()
        if(uids.length === 0)
            return
        block_users(uids, on)
        undo_hub.push(n_label(uids.length, qsTr("User blocked"), qsTr("%1 users blocked")),
                      function() { ops.block_users(uids, !on) })
    }

    function block_users(uids, on) {
        for(var i = 0; i < uids.length; ++i)
            core.set_ignore(uids[i], on ? 1 : 0)
    }

    function bulk_fav_users(on) {
        var uids = ConvListModel.selected_uids()
        if(uids.length === 0)
            return
        for(var i = 0; i < uids.length; ++i)
            core.set_pal(uids[i], on ? 1 : 0)
        undo_hub.push(on ? n_label(uids.length, qsTr("User added to favorites"),
                                   qsTr("%1 users added to favorites"))
                         : n_label(uids.length, qsTr("User removed from favorites"),
                                   qsTr("%1 users removed from favorites")),
                      function() {
                          for(var i = 0; i < uids.length; ++i)
                              core.set_pal(uids[i], !on)
                      })
    }

    function bulk_trash_user_msgs() {
        var mids = ConvListModel.trash_all_selected()
        refresh()
        if(mids.length === 0)
            return
        var label = n_label(mids.length, qsTr("Message trashed"), qsTr("%1 messages trashed"))
        var list = mids
        undo_hub.push(label, function() { ops.untrash_mids(list) })
    }

    function bulk_obliterate_users() {
        ConvListModel.obliterate_all_selected()
        refresh()
    }

    // ------------------------------------------------------------------
    // view settings (shared with the settings page, so the two never disagree)
    // ------------------------------------------------------------------

    function set_show_hidden_msgs(on) {
        applicationWindow1.show_hidden = on
        core.set_local_setting("show_hidden", on ? "1" : "0")
        themsglist.set_show_hidden(on ? 1 : 0)
    }

    // note: the caller is responsible for making sure the "explicit content"
    // consent warning has been shown before turning this on. that gate lives
    // in the settings page.
    function set_show_unreviewed(on) {
        applicationWindow1.show_unreviewed = on
        core.set_local_setting("show_unreviewed", on ? "1" : "0")
        if(Qt.platform.os == "android") {
            notificationClient.set_user_property("content", on ? "unrev" : "rev")
        }
        SimpleDirectoryList.clear()
    }

    function set_show_archived(on) {
        applicationWindow1.show_archived_users = on
        core.use_archived = on
    }

    // ------------------------------------------------------------------
    // misc
    // ------------------------------------------------------------------

    // routes a clipboard request to whatever text input currently has focus,
    // so the menu bar's Cut/Copy/Paste work in the chat entry box, the profile
    // editor and the message viewer without each of them wiring up shortcuts.
    function focused_input(op) {
        var it = applicationWindow1.contentItem
        var f = it ? it.activeFocus : null
        while(f) {
            if(f.copy !== undefined && f.selectedText !== undefined) {
                if(op === "cut")
                    f.cut()
                else if(op === "paste")
                    f.paste()
                else if(op === "selectAll")
                    f.selectAll()
                else
                    f.copy()
                return true
            }
            f = f.parent
        }
        return false
    }

    // message lists have no text area to copy out of, so stage the text in a
    // hidden editor. copy() works fine on something that was never shown.
    function copy_text(text) {
        if(text === undefined || text === null || text === "")
            return false
        var plain = core.strip_html(String(text))
        if(plain === "")
            plain = String(text)
        copy_stash.text = plain
        copy_stash.selectAll()
        copy_stash.copy()
        return true
    }

    function notify(msg) {
        toast.show(msg)
    }

    function undo() {
        undo_hub.undo()
    }

    TextEdit {
        id: copy_stash
        visible: false
        width: 0
        height: 0
    }
}
