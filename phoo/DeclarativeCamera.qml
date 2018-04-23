
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

import QtQuick 2.6
import QtMultimedia 5.6
import QtQuick.Controls 2.1

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
                    camera.captureMode = Camera.CaptureStillImage
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
    
    Camera {
        id: camera
        captureMode: Camera.CaptureStillImage

        imageCapture {

            onImageCaptured: {
                photoPreview.source = preview
                cameraUI.state = "PhotoPreview"
                console.log("orientation ", camera.orientation)
                //photoPreview.ok_vis = false
            }

            onCaptureFailed: {
                console.log("cap failed ", message)
            }

            onImageSaved: {
                file_captured = path
                //photoPreview.ok_vis = true
                
            }
        }

    }
    
    onSnapshot: {
       
            //top_dispatch.camera_snapshot(filename);
        
            
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
        source: camera
        autoOrientation: true
        
        PhotoCaptureControls {
            id: stillControls
            //anchors.fill: parent
            camera: camera
            visible: cameraUI.state == "PhotoCapture"
            z: 5
        }
        BusyIndicator {
            id: busy1

            running: {stillControls.visible && !camera.imageCapture.ready}
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }

    }

    

}
