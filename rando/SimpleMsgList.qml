
import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12


Page {

    property alias model: listview.model
    property bool only_sent: true

    anchors.fill: parent

    Component {
        id: msg_delegate

        CircularImage {
            width: listview.width
            height: width

            id: img
            visible: only_sent && SENT == 1
            asynchronous: true
            source: {PREVIEW_FILENAME != "" ? ("file:///" + String(PREVIEW_FILENAME)) : ""}
            fillMode: Image.PreserveAspectCrop
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
