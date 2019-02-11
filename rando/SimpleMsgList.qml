
import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12


Page {

    property alias model: listview.model
    property bool show_sent: true
    property bool show_recv: true
    property string uid

    anchors.fill: parent

    onVisibleChanged: {
        if(visible) {
            themsglist.reload_model()
            core.reset_unviewed_msgs(top_dispatch.last_uid_selected)
        }
    }

    Component {
        id: msg_delegate

        CircularImage {
            width: listview.width
            height: width

            id: img
            visible: (show_sent && SENT == 1) || (show_recv && SENT == 0)
            asynchronous: true
            source: {PREVIEW_FILENAME != "" ? ("file:///" + String(PREVIEW_FILENAME)) : ""}
            fillMode: Image.PreserveAspectCrop
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    themsgview.mid = model.mid
                    themsgview.uid = model.ASSOC_UID
                    themsgview.view_source = source
                    stack.push(themsgview)

                }
            }
        }
    }

    ListView {
        id: listview
        anchors.fill: parent
        clip: true
        delegate: msg_delegate
        model: themsglist
    }

    function snapshot(filename) {
        core.send_simple_cam_pic(the_man, "for review", filename)

    }

    TipButton {
        anchors.bottom: parent.bottom
        anchors.margins: mm(3)
        anchors.horizontalCenter: parent.horizontalCenter
        icon.name: "camera-photo"
        onClicked: {
            cam.next_state = "StopAndPop"
            cam.ok_text = "Upload"
            stack.push(cam)
        }
    }

}
