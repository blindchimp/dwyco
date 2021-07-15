
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12
import dwyco 1.0
import QtQuick.Layouts 1.3
import QtMultimedia 5.12

// this is a simplified camera setup for android phone that will
// automatically connect to the selfie-camera.
// typically used if you are doing to use an old camera as a
// security camera.

Page {
    property bool dragging
    property string serving_uid
    property string attempt_uid
    property var call_buttons_model

    anchors.fill: parent

    header: SimpleToolbar {
        //extras: extras_button

    }

    onVisibleChanged: {
        if(visible)
        {
//            if(core.vid_dev_idx === 2) {
//                preview_cam.start()
//                core.enable_video_capture_preview(1)
//            } else if(core.vid_dev_idx === 1) {
//                // files
//                preview_cam.stop()
//                core.enable_video_capture_preview(1)
//            } else {
//                preview_cam.stop()
//                core.enable_video_capture_preview(0)
//                viewer.source = mi("ic_videocam_off_black_24dp.png")
//            }
            var mode = core.get_local_setting("mode")
            if(mode === "") {
                core.set_local_setting("mode", "watch")
                mode = "watch"
            }
            var wname = core.get_local_setting("camera-to-watch")
            if(wname.length === 0) {
                watch_name.placeholderText = "enter name of camera to watch"
            } else {
                watch_name.text = wname
            }

            if(mode === "watch") {
                preview_cam.stop()
                core.enable_video_capture_preview(0)
                core.select_vid_dev(0)
                viewer.source = mi("ic_videocam_off_black_24dp.png")
                cam_watcher.checked = true
            }
            else {
                preview_cam.start()
                core.select_vid_dev(2)
                core.enable_video_capture_preview(1)
                cam_sender.checked = true
            }
        }
        else
        {
            core.enable_video_capture_preview(0)
        }

    }

    onAttempt_uidChanged: {
        if(attempt_uid.length > 0)
            core.start_control(attempt_uid)
    }

    Connections {
        target: core
//        onCamera_change: {
//            if(visible) {
//                if(cam_on === 1 && core.vid_dev_idx === 2) {
//                    preview_cam.start()
//                    core.enable_video_capture_preview(1)
//                } else if(cam_on === 1 && core.vid_dev_idx === 1) {
//                    preview_cam.stop()
//                    core.enable_video_capture_preview(1)
//                } else {
//                    preview_cam.stop()
//                    core.enable_video_capture_preview(0)
//                    viewer.source = mi("ic_videocam_off_black_24dp.png")
//                }
//            }
//        }
        onName_to_uid_result: {
            console.log("GOT UID FOR NAME ", uid, handle)
            attempt_uid = uid
        }

        onSc_connect_terminated: {
                console.log("CONNECT TERMINATED ", uid)
                serving_uid = ""
        }

        onSc_connectedChanged: {
                    console.log("ConnectedChanged ", connected, uid)
                    if(connected === 0) {
                        if(serving_uid === uid)
                        {
                            serving_uid = ""
                            call_buttons_model = null
                        }

                    } else {
                        if(serving_uid === "")
                        {
                            serving_uid = uid
                            call_buttons_model = core.get_button_model(serving_uid)
                        }
                    }
            }
    }

    Camera {
        id: preview_cam
        objectName: "qrCameraQML"
        viewfinder {
            resolution: Qt.size(640, 480)
            //maximumFrameRate: 10
        }
        position: Camera.FrontFace
        captureMode: Camera.captureVideo
        onCameraStateChanged: {
            //if(state === Camera.ActiveState) {
                var res = preview_cam.supportedViewfinderResolutions();
                console.log("RESOLUTIONS ")
            for(var i = 0; i < res.length; i++) {
                console.log(res[i].width)
                console.log(res[i].height)
            }
            //}
        }
        onCameraStatusChanged: {
            //if(state === Camera.ActiveState) {
                var res = preview_cam.supportedViewfinderResolutions();
                console.log("RESOLUTIONS ")
            for(var i = 0; i < res.length; i++) {
                console.log(res[i].width)
                console.log(res[i].height)
            }

            //}
        }
    }


    ColumnLayout {
        anchors.fill: parent

        RadioButton {
            id: cam_sender
            checked: false
            text: "Capture Camera"
            onClicked: {
                if(checked) {
                    // set pw requirements, enable camera, etc
                    capture_name.text_input = core.uid_to_name(core.this_uid)
                    core.set_local_setting("mode", "capture")
                    preview_cam.start()
                    core.select_vid_dev(2)
                    core.enable_video_capture_preview(1)
                } else {

                }
            }
        }
        RadioButton {
            id: cam_watcher
            checked: true
            text: "Watch Camera"
            onClicked: {
                if(checked) {
                    // disable camera, fetch last watched camera, lookup
                    // uid, try connecting to that uid
                    preview_cam.stop()
                    core.select_vid_dev(0)
                    core.enable_video_capture_preview(0)
                    //core.name_to_uid(watch_name.text_input)
                    core.set_local_setting("mode", "watch")
                }
            }
        }
        Label {
            text: cam_watcher.checked ? "Camera to watch" : "Camera name"
        }

        TextFieldX {
            id: capture_name
            visible: cam_sender.checked
            Layout.fillWidth: true
        }
        TextField {
            id: watch_name
            visible: cam_watcher.checked
            Layout.fillWidth: true
            onAccepted: {
                attempt_uid = ""
                core.name_to_uid(text)
                core.set_local_setting("camera-to-watch", text)
            }
        }


        Switch {
            id: camtoggle
            text: "Enable selfie-cam"
            Layout.fillWidth: true
            visible: cam_sender.checked
            onClicked: {
                if(checked) {
                    core.select_vid_dev(2);
                    preview_cam.start()
                } else {
                    preview_cam.stop()
                    core.select_vid_dev(0);
                    viewer.source = mi("ic_videocam_off_black_24dp.png")
                }

            }

        }

//        VideoOutput {
//            id: pview
//            Layout.fillHeight: true
//            Layout.fillWidth: true
//            source: preview_cam
//            autoOrientation: true
//            visible: true
//        }

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
        z: 10
    }

}
