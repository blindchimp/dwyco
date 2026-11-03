import QtQuick

Item {

    property alias barCount: repeater.model
    property color color: "white"
    property int spacing: 5

    property bool running: true

    id: root

    Repeater {
        id: repeater
        delegate: Rectangle {
            width: (root.width / root.barCount) - root.spacing
            height: root.height
            x: index * width + root.spacing * index
            color: root.color
            radius: height / 2
            transform: Scale {
                id: barScale
                origin.x: width / 2
                origin.y: height / 2
            }

            SequentialAnimation {
                running: root.running
                loops: Animation.Infinite

                PauseAnimation { duration: index * 100 }
                NumberAnimation { target: barScale; property: "yScale"; from: 1.0; to: 1.5; duration: 200; easing.type: Easing.InOutQuad }
                NumberAnimation { target: barScale; property: "yScale"; from: 1.5; to: 1.0; duration: 200; easing.type: Easing.InOutQuad }
                PauseAnimation { duration: (root.barCount - 1 - index) * 100 }
            }
        }
    }
}
