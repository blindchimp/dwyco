import QtQuick
import QtQuick.Effects
// https://forum.qt.io/topic/145956/rounded-image-in-qt6/10
// with slight tweaks to keep it round
//
Item {
    property alias source: sourceItem.source
    property real radius: Math.min(width, height) / 2.0

    property alias fillMode: sourceItem.fillMode
    property alias sourceSize: sourceItem.sourceSize
    property alias mipmap: sourceItem.mipmap
    property alias status: sourceItem.status
    property alias verticalAlignment: sourceItem.verticalAlignment
    property alias smooth: sourceItem.smooth
    property alias horizontalAlignment: sourceItem.horizontalAlignment
    property alias progress: sourceItem.progress
    property alias asynchronous: sourceItem.asynchronous
    property alias paintedWidth: sourceItem.paintedWidth
    property alias paintedHeight: sourceItem.paintedHeight

    implicitWidth: sourceItem.implicitWidth
    implicitHeight: sourceItem.implicitHeight

    Image {
        id: sourceItem
        source: ""
        anchors.centerIn: parent
        //anchors.fill: parent
        width: radius * 2
        height: width
        visible: false
    }

    MultiEffect {
        source: sourceItem
        anchors.fill: sourceItem
        maskEnabled: true
        maskSource: mask
        maskThresholdMin: 0.5
        maskSpreadAtMin: 1.0
    }

    Item {
        id: mask
        width: sourceItem.width
        height: sourceItem.height
        layer.enabled: true
        visible: false
        layer.smooth: true

        Rectangle {
            width: sourceItem.width
            height: sourceItem.height
            radius: width/2
            color: "black"
        }
    }
}

