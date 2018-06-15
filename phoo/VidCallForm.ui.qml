import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

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
    }
}
