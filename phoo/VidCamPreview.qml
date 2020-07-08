
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.2
import dwyco 1.0
import QtQuick.Layouts 1.3

// this assumes you are using the internal dwyco-provided
// video capture drivers.

Page {
    property bool dragging

    anchors.fill: parent

    header: SimpleToolbar {
        //extras: extras_button

    }

    onVisibleChanged: {
        if(visible)
        {
            if(core.vid_dev_idx !== 0) {
                core.enable_video_capture_preview(1)
            } else {
                core.enable_video_capture_preview(0)
                viewer.source = mi("ic_videocam_off_black_24dp.png")

            }
        }
        else
        {
            core.enable_video_capture_preview(0)
        }

    }

    Connections {
        target: core
        onCamera_change: {
            if(visible) {
                if(cam_on) {
                    core.enable_video_capture_preview(1)
                } else {
                    core.enable_video_capture_preview(0)
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
                verticalAlignment: Text.verticalCenter
                Layout.fillWidth: true

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        camlist.currentIndex = index
                        core.select_vid_dev(index)
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        ListView {
            Layout.fillWidth: true
            //Layout.fillHeight: true
            Layout.preferredHeight: contentHeight
            id: camlist
            model: camListModel
            delegate: camlist_delegate
            clip: true
            spacing: 5
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
                    target: core
                    onVideo_capture_preview: {
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
