
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml
import QtQuick
import QtQuick.Controls
import dwyco
import QtQuick.Layouts

// this assumes you are using the internal dwyco-provided
// video capture drivers.

Page {
    property bool dragging

    anchors.fill: parent

    header: SimpleToolbar {
        //extras: extras_button

    }



    Component.onCompleted: {
        //if(visible)
        //{
            if(Core.vid_dev_idx !== 0) {
                Core.enable_video_capture_preview(1)
            } else {
                Core.enable_video_capture_preview(0)
                viewer.source = mi("ic_videocam_off_black_24dp.png")

            }
            camlist.currentIndex = Core.vid_dev_idx
            console.log("CAM IDX ", camlist.currentIndex)
        //}
        //else
    }
    Component.onDestruction: {
        {
            Core.enable_video_capture_preview(0)
        }

    }

    Connections {
        target: Core
        function onCamera_change(cam_on) {
            if(visible) {
                if(cam_on) {
                    Core.enable_video_capture_preview(1)
                } else {
                    Core.enable_video_capture_preview(0)
                    viewer.source = mi("ic_videocam_off_black_24dp.png")
                }
            }
        }
    }

    Component {
        id: camlist_delegate
        RowLayout {

            width: camlist.width
            height: name.implicitHeight

            Label {
                id: name

                text: modelData
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        camlist.currentIndex = index
                        Core.select_vid_dev(index)

                        console.log("CAM IDX ", index)
                    }
                }
            }
        }

    }

    ColumnLayout {
        anchors.fill: parent
        ListView {
            id: camlist

            Layout.fillWidth: true
            //Layout.fillHeight: true
            Layout.preferredHeight: contentHeight
            model: camListModel
            delegate: camlist_delegate
            clip: true
            spacing: 5
            highlight: Rectangle { z:3 ; color: amber_accent; opacity: .3}
            highlightMoveDuration: 200
            highlightMoveVelocity: -1
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

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
                Connections {
                    target: Core
                    function onVideo_capture_preview(img_path) {
                        if(visible)
                            viewer.source = img_path
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
                        stack.pop()

                    }
                }
            }
        }
    }

    BusyIndicator {
        id: busy1
        running: {viewer.source === "" }
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

}
