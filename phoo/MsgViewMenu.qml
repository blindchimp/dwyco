import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import dwyco 1.0

Menu {

    x: parent.width - width
    transformOrigin: Menu.TopRight
    MenuItem {
        text: "Trash msg"
        onTriggered: {
            core.set_tag_message(mid, "_trash")
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
}
