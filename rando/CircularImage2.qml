import QtQuick
import QtQuick.Effects
// https://forum.qt.io/topic/145956/rounded-image-in-qt6/10
// with slight tweaks to keep it round
//
Item {
    property alias source: sourceItem.source
    property real radius: Math.min(width, height) / 2.0

    anchors.fill: parent
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

