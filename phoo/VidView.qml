
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQml
import QtQuick.Controls
import dwyco

Page {
    property string uid
    property string mid
    property alias view_source : viewer.source
    property bool dragging

    anchors.fill:parent
    onVisibleChanged: {
    }

    Component {
        id: extras_button

        ToolButton {
            contentItem: Image {
                anchors.centerIn: parent
                source: mi("ic_action_overflow.png")
            }
            onClicked: optionsMenu.open()

            Menu {
                id: optionsMenu
                x: parent.width - width
                transformOrigin: Menu.TopRight
                MenuItem {
                    text: "Hangup"
                    onTriggered: {
                        stack.pop()
                    }
                }

            }
        }
    }

    header: SimpleToolbar {
        extras: extras_button
    }

    Connections {
        target: Core
        function onVideo_display(ui_id, frame_number, img_path) {
            view_source = img_path
        }
    }

    Rectangle {
        anchors.fill:parent

        Image {
            id: viewer
            anchors.top: dragging ? undefined : parent.top
            anchors.right: dragging ? undefined : parent.right
            anchors.left: dragging ? undefined : parent.left
            anchors.bottom: dragging ? undefined : parent.bottom
            //anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
            onVisibleChanged: {
                dragging = false
                scale = 1.0
            }

        }
    }


    PinchArea {
        anchors.fill: parent
        pinch.target: viewer
        pinch.minimumScale: 0.1
        pinch.maximumScale: 10
        pinch.dragAxis: Pinch.XAndYAxis

        MouseArea {
            id: dragArea
            hoverEnabled: true
            anchors.fill: parent
            drag.target: viewer
            scrollGestureEnabled: false

            onPressed: {
                dragging = true

            }
            onClicked: {

            }
        }
    }
    BusyIndicator {
        id: busy1
        running: {viewer.source == ""}
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }


}


