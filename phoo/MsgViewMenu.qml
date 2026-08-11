import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import dwyco

Menu {

    x: parent.width - width
    transformOrigin: Menu.TopRight
    MenuItem {
        property bool queued: mid.length > 0 ? core.is_queued_message(mid) : false
        text: queued ? "Cancel send" : "Trash msg"
        onTriggered: {
            if(queued) {
                confirm_cancel_send.text = "Cancel sending this message to " + core.uid_to_name(uid) + "? The unsent message will be DELETED."
                confirm_cancel_send.visible = true
            } else {
                core.set_tag_message(mid, "_trash")
                themsglist.reload_model()
                stack.pop()
            }
        }
        MessageYN {
            id: confirm_cancel_send
            title: "Cancel send?"
            text: "Cancel sending this message?"
            informativeText: "The unsent message will be DELETED. (No UNDO)"
            onYesClicked: {
                core.delete_message(uid, mid)
                themsglist.reload_model()
                stack.pop()
                close()
            }
            onNoClicked: {
                close()
            }
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
        onTriggered: {
            core.set_fav_message(mid, !fav)
        }
    }
    MenuItem {
        text: hid ? "Unhide" : "Hide"
        onTriggered: {
            if(hid)
                core.unset_tag_message(mid, "_hid")
            else
                core.set_tag_message(mid, "_hid")
            themsglist.invalidate_model_filter()
        }
    }
    MenuItem {
        text: "Copy Text"
        onTriggered: {
            msg_text.selectAll()
            msg_text.copy()
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
}
