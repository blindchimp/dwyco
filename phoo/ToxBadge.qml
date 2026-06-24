import QtQuick

Rectangle {
    property bool isTox: false

    radius: width / 2
    color: "limegreen"
    visible: isTox
    z: 3

    Text {
        anchors.centerIn: parent
        text: "T"
        color: "white"
        font.bold: true
        font.pixelSize: parent.height * 0.6
    }
}
