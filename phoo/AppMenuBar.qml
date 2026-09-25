
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// The desktop menu bar. Set as ApplicationWindow.menuBar and hidden on mobile
// (see main.qml), where the per-page overflow buttons remain the entry point.
//
// Since Qt 6.8 a MenuBar is a real system menu bar on macOS, and since the
// items here are Actions with shortcuts, the same file gives keyboard
// accelerators everywhere.
//
// Nothing in here implements an operation. Every entry point calls ops.qml, so
// the menu bar and a right-click on the same message do the same thing and
// produce the same undo banner.

import QtQuick
import QtQuick.Controls
import dwyco

MenuBar {
    id: bar

    readonly property bool is_moderator: core.this_uid === applicationWindow1.the_man
    readonly property bool has_current_msg: ops.current_mid !== ""

    // ------------------------------------------------------------------
    Menu {
        title: qsTr("&File")

        Action {
            text: qsTr("&New Conversation...")
            shortcut: "Ctrl+N"
            onTriggered: applicationWindow1.push_page(cqres)
        }
        Action {
            text: qsTr("&Browse Directory")
            shortcut: "Ctrl+D"
            onTriggered: applicationWindow1.push_page(simpdir_rect)
        }
        MenuSeparator {}

        Action {
            text: qsTr("Browse &Favorites")
            shortcut: "Ctrl+Shift+F"
            onTriggered: {
                simp_tag_browse.to_tag = "_fav"
                applicationWindow1.push_page(simp_tag_browse)
            }
        }
        Action {
            text: qsTr("Browse &Hidden Messages")
            shortcut: "Ctrl+Shift+H"
            enabled: show_hidden
            onTriggered: {
                simp_tag_browse.to_tag = "_hid"
                applicationWindow1.push_page(simp_tag_browse)
            }
        }
        Action {
            text: qsTr("View &Trash")
            shortcut: "Ctrl+Shift+T"
            onTriggered: applicationWindow1.push_page(trash_browse)
        }
        MenuSeparator {}

        Action {
            text: qsTr("Se&ttings...")
            shortcut: StandardKey.Preferences
            onTriggered: applicationWindow1.push_page(settings_dialog)
        }
        Action {
            text: qsTr("&About Dwyco")
            onTriggered: applicationWindow1.show_about()
        }
        MenuSeparator {}

        Action {
            text: qsTr("&Lock and Exit")
            shortcut: "Ctrl+Q"
            onTriggered: {
                expire_immediate = true
                core.power_clean()
                if(Qt.platform.os === "android") {
                    notificationClient.start_background()
                    notificationClient.set_lastrun()
                }
                Qt.quit()
            }
        }
    }

    // ------------------------------------------------------------------
    Menu {
        title: qsTr("&Edit")

        Action {
            // the label says what is being undone, so a mis-click is obvious
            // before it happens
            text: undo_hub.can_undo ? qsTr("&Undo ") + undo_hub.last_label : qsTr("&Undo")
            shortcut: StandardKey.Undo
            enabled: undo_hub.can_undo
            onTriggered: undo_hub.undo()
        }
        MenuSeparator {}

        Action {
            text: qsTr("Cu&t")
            shortcut: StandardKey.Cut
            onTriggered: ops.focused_input("cut")
        }
        Action {
            text: qsTr("&Copy")
            shortcut: StandardKey.Copy
            onTriggered: ops.focused_input("copy")
        }
        Action {
            text: qsTr("&Paste")
            shortcut: StandardKey.Paste
            onTriggered: ops.focused_input("paste")
        }
        Action {
            text: qsTr("Select &All")
            shortcut: StandardKey.SelectAll
            // a focused text field gets its text selected, otherwise the
            // message list on top goes into multi-select
            onTriggered: {
                if(!ops.focused_input("selectAll"))
                    top_dispatch.select_all_requested()
            }
        }
    }

    // ------------------------------------------------------------------
    Menu {
        title: qsTr("&Message")
        enabled: bar.has_current_msg

        Action {
            text: ops.current_is_trash ? qsTr("&Restore Message") : qsTr("&Trash Message")
            shortcut: StandardKey.Delete
            onTriggered: {
                if(ops.current_is_trash)
                    ops.untrash_msg(ops.current_mid)
                else
                    ops.trash_msg(ops.current_mid)
            }
        }
        MenuSeparator {}

        Action {
            text: ops.current_is_fav ? qsTr("Remove Favo&rite")
                                    : qsTr("Favo&rite Message")
            onTriggered: ops.fav_msg(ops.current_mid, !ops.current_is_fav)
        }
        Action {
            text: ops.current_is_hid ? qsTr("&Unhide Message") : qsTr("&Hide Message")
            onTriggered: ops.hide_msg(ops.current_mid, !ops.current_is_hid)
        }
        Action {
            text: qsTr("&Forward...")
            shortcut: StandardKey.Find
            onTriggered: ops.forward_msg(ops.current_mid)
        }
        Action {
            text: qsTr("Re&port Message")
            shortcut: "Ctrl+R"
            onTriggered: ops.report_msg(ops.current_mid, ops.current_uid, "")
        }
        Action {
            text: qsTr("Re&view Message")
            enabled: bar.is_moderator
            onTriggered: ops.review_msg(ops.current_mid, ops.current_uid, "")
        }
        MenuSeparator {}

        Action {
            text: qsTr("Delete &Forever")
            enabled: ops.current_is_trash
            onTriggered: ops.delete_msg_forever(ops.current_mid, ops.current_uid)
        }
    }

    // ------------------------------------------------------------------
    Menu {
        title: qsTr("&View")

        Action {
            text: qsTr("&Conversations")
            shortcut: "Alt+1"
            onTriggered: {
                while(stack.depth > 1)
                    stack.pop()
            }
        }
        Action {
            text: qsTr("Chat &Room")
            shortcut: "Alt+2"
            onTriggered: applicationWindow1.push_page(public_chat)
        }
        Action {
            text: qsTr("&People in Room")
            shortcut: "Alt+3"
            onTriggered: applicationWindow1.push_page(chatlist)
        }
        MenuSeparator {}

        Action {
            text: qsTr("Show &Hidden Messages")
            shortcut: "Ctrl+H"
            // deliberately not a checkable Action: Qt toggles "checked" from
            // C++ on trigger, which silently breaks any QML binding on it, so
            // the tick would stop tracking the setting. dynamic labels are what
            // the rest of the app's menus already do.
            onTriggered: ops.set_show_hidden_msgs(!applicationWindow1.show_hidden)
        }
        Action {
            text: applicationWindow1.show_unreviewed ? qsTr("Hide &Unreviewed Content")
                                                     : qsTr("Show &Unreviewed Content")
            enabled: !corporate_censorship
            onTriggered: {
                var want = !applicationWindow1.show_unreviewed
                // the first time somebody asks for unreviewed content they
                // have to see the consent warning, which lives in settings
                if(want && core.get_local_setting("inh_content_warning") === "") {
                    applicationWindow1.push_page(settings_dialog)
                    return
                }
                ops.set_show_unreviewed(want)
            }
        }
        Action {
            text: applicationWindow1.show_archived_users ? qsTr("Hide A&rchived Users")
                                                         : qsTr("Show A&rchived Users")
            onTriggered: ops.set_show_archived(!applicationWindow1.show_archived_users)
        }
        Action {
            text: convlist.grid_checked ? qsTr("Conversations as &List")
                                       : qsTr("Conversations as &Grid")
            shortcut: "Ctrl+G"
            onTriggered: convlist.grid_checked = !convlist.grid_checked
        }
        MenuSeparator {}

        Action {
            text: qsTr("Re&fresh Conversations")
            shortcut: "F5"
            onTriggered: ConvListModel.reload_convlist()
        }
    }

    // ------------------------------------------------------------------
    Menu {
        title: qsTr("&Help")

        Action {
            text: qsTr("&About Dwyco")
            onTriggered: applicationWindow1.show_about()
        }
    }
}
