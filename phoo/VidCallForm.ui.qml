import QtQuick 2.4

Image {
    property alias vid_incoming: vid_incoming
    property alias vid_preview: vid_preview
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
            anchors.fill: parent
            but_name: "actionPause"
            opacity: checked ? .7 : 0
            contentItem: Image {
                anchors.centerIn: parent
                source: mi("ic_pause_circle_outline_white_24dp.png")
            }
            onClicked: {
                console.log("okok")
            }
        }
    }
}
