import QtQuick 2.4

Image {
    property alias vid_incoming: vid_incoming
    property alias vid_preview: vid_preview
    property alias vid_pause_button: vid_pause_button
    clip: true

    id: vid_incoming
    fillMode: Image.PreserveAspectFit
    Image {
        id: vid_preview
        fillMode: Image.PreserveAspectFit
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: parent.width / 4
        height: parent.height / 4
        clip: true
        CallButtonLink {
            id: vid_pause_button
            anchors.fill: parent
            but_name: "actionPause"
            opacity: checked ? .7 : 0
        }
    }
}
