import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import dwyco

Menu {
    x: parent.width - width
    transformOrigin: Menu.TopRight
    MenuItem {
        text: "Untrash msg"
        onTriggered: {
            Core.unset_tag_message(mid, "_trash")
            themsglist.invalidate_model_filter()
            stack.pop()
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
            Core.set_fav_message(mid, !fav)
        }
    }
    MenuItem {
        text: hid ? "Unhide" : "Hide"
        onTriggered: {
            if(hid)
                Core.unset_tag_message(mid, "_hid")
            else
                Core.set_tag_message(mid, "_hid")
            themsglist.invalidate_model_filter()
        }
    }
    MenuItem {
        text: "Delete forever"
        onTriggered: {
            Core.delete_message(uid, mid)
            themsglist.invalidate_model_filter()
            stack.pop()
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
}
