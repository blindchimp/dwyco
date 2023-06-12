
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml 2.12
import QtQuick 2.12
import QtMultimedia
import QtQuick.Controls

// the API to this object is ugly... essentially, the requirement is
// that the user of the object must reside in a stackview, and
// push the cam onto the same stackview. the user should also tell the
// camera object what state to start in: "PhotoCapture" for multiple capture
// or "StopAndPop" to capture one image and pop immediately.
//
// the results of the image capture are returned via a signal that is
// sent to a javascript "snapshot(filename)" function that is defined in
// the users object, which must be the previous object on the stackview
// stack.
//
// the main reason for this is that we use this camera is multiple slightly
// different contexts. we really don't want to send a signal to everyone that
// might be listening for a camera-result, just the one "caller".
// honestly, there must be a simpler way to do something like this, but
// i couldn't find a way to, for example, expose a property in another
// object that this object could "fill in" without the need for a signal.
// the loader complicates things too. maybe someday it'll dawn on me what
// to do in cases like this.
//
Rectangle {
    id : cameraUI
    property string file_captured
    property string state_on_close
    property alias ok_pv_text: photoPreview.ok_text
   
    signal snapshot(string filename)
    anchors.fill: parent

    color: "black"
    state: "PhotoCapture"
    
    Component.onCompleted: {
        cameraUI.snapshot.connect(stack.get(stack.depth - 2).snapshot)
        console.log("WTF CAMERA")
    }

    states: [
        State {
            name: "Idle"
            StateChangeScript {
                script: {
                    camera.stop()
                }
            }
        },

        State {
            name: "PhotoCapture"
            StateChangeScript {
                script: {
                    //camera.captureMode = Camera.CaptureStillImage
                    camera.start()
                }
            }
        },
        State {
            name: "PhotoPreview"
        },
        
        State {
            name: "StopAndPop"
            StateChangeScript {
                script: {
                    camera.stop()
                    stack.pop()                    
                }
            }
        },

        State {
            name: "VideoCapture"
            StateChangeScript {
                script: {
                    camera.captureMode = Camera.CaptureVideo
                    camera.start()
                }
            }
        },
        State {
            name: "VideoPreview"
            StateChangeScript {
                script: {
                    camera.stop()
                }
            }
        }
    ]

    MediaDevices {
        id: devices
    }

    CaptureSession {
        id: capture_session
        imageCapture: ImageCapture {
               id: img_cap
            onImageCaptured: {
                photoPreview.source = img_cap.preview
                cameraUI.state = "PhotoPreview"
                //console.log("orientation ", camera.orientation)
                //console.log("focus auto ",focus.isFocusModeSupported(CameraFocus.FocusAuto))
                //camera.unlock()
                //photoPreview.ok_vis = false
            }

            onErrorOccurred: {
                console.log("cap failed ", message)
                //camera.unlock()
            }

            onImageSaved: (req, path)=> {
                file_captured = path
                //photoPreview.ok_vis = true

            }
        }
        camera: camera
        videoOutput: viewfinder
    }

    Camera {
        id: camera
        active: cameraUI.visible
        //captureMode: Camera.CaptureStillImage
        focusMode: Camera.FocusModeAuto
        whiteBalanceMode: Camera.WhiteBalanceAuto

//        onLockStatusChanged: {
//            console.log("lock status ", lockStatus)
//            if(lockStatus == Camera.Locked) {
//                camera.imageCapture.captureToLocation(core.tmp_dir)

//            }

//        }


    }

    PhotoPreview {
        id : photoPreview
        anchors.fill : parent
        onClosed: {
            if(ok)
                cameraUI.state = state_on_close //"PhotoCapture"
            else
                cameraUI.state = "PhotoCapture"
        }
        visible: cameraUI.state == "PhotoPreview"
        ok_vis: file_captured != ""

    }


    VideoOutput {
        id: viewfinder
        visible: cameraUI.state == "PhotoCapture" || cameraUI.state == "VideoCapture"

        anchors.fill: parent
        //source: camera
        //autoOrientation: true
        
        PhotoCaptureControls {
            id: stillControls
            //anchors.fill: parent
            //camera: camera
            visible: cameraUI.state == "PhotoCapture"
            z: 5
        }
        BusyIndicator {
            id: busy1

            running: {stillControls.visible && !capture_session.imageCapture.readyForCapture}
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }

    }
    Label {
        id: no_cams
        anchors.fill: parent

        anchors.margins: mm(3)
        wrapMode: Text.WordWrap
        background: Rectangle {
            color: "white"
        }

        text: qsTr("(No camera devices available)")
        z: 6
        visible: {devices.videoInputs.length === 0}
        Button {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: mm(3)
            text: "Back"
            onClicked: {
                stack.pop()
            }
        }


    }

    

}
