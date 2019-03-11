import QtQuick 2.0

Rectangle {
    id: main
    width: parent.width
    height: parent.height
    color: "blue"
    property bool moveOut: false
    property bool moveIn: false
    state: "moveIn"

    states: [
        State {
            name: "moveOut"; when: main.moveOut
            PropertyChanges { target: main; x: width; y: 0 }
        },
        State {
            name: "moveIn"; when: main.moveIn
            PropertyChanges { target: main; x: 0; y: 0 }
        }
    ]

    transitions: [
        Transition {
            to: "moveOut"
            NumberAnimation { properties: "x,y"; easing.type: Easing.InOutQuad; duration: 400; loops: 1 }
        },
        Transition {
            to: "moveIn"
            NumberAnimation { properties: "x,y"; easing.type: Easing.InOutQuad; duration: 400; loops: 1 }
        }
    ]




}
