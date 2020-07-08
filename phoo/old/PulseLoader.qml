
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12

Item {

    // ----- Public Properties ----- //

    property alias barCount: repeater.model
    property color color: "white"
    property int spacing: 5

    id: root

    Repeater {
        id: repeater
        model: 5
        delegate: Component {
            Rectangle {
                width: (root.width / root.barCount) - root.spacing
                height: root.height
                x: index * width + root.spacing * index
                transform: Scale {
                    id: rectScale
                    origin {
                        x: width / 2
                        y: height / 2
                    }
                }
                transformOrigin: Item.Center
                color: root.color

                SequentialAnimation {
                    id: anim
                    loops: Animation.Infinite

                    NumberAnimation { target: rectScale; property: "yScale"; from: 1; to: 1.5; duration: 300 }
                    NumberAnimation { target: rectScale; property: "yScale"; from: 1.5; to: 1; duration: 300 }
                    PauseAnimation { duration: root.barCount * 150 }
                }

                function playAnimation() {
                    if (anim.running == false) {
                        anim.start()
                    }
                }
            }
        }
    }

    Timer {
        // ----- Private Properties ----- //
        property int _barIndex: 0

        interval: 80
        repeat: true
        onTriggered: {
            if (_barIndex === root.barCount) {
                stop()
            }
            else {
                repeater.itemAt(_barIndex).playAnimation()
                _barIndex++
            }
        }
        Component.onCompleted: start()
    }
}
