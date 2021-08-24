
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.12
import dwyco 1.0
import QtQuick.Layouts 1.3
import QtMultimedia 5.12
import QtQml.StateMachine 1.12 as DSM

// NOTE! THIS IS HACKED SO IT WORKS FOR CAPTURE ON DESKTOP
// (ie, it does NOT use the qrCameraQML objectName
//
// this is a simplified camera setup for android phone that will
// automatically connect to the selfie-camera.
// typically used if you are doing to use an old camera as a
// security camera.

Page {
    id: campage

    property var call_buttons_model
    property int profile_sent: 0

    anchors.fill: parent

    function hangup() {
        core.hangup_all_calls()
    }

    header: Label {
        text: "Dwyco Selfie Stream Capture " + core.buildtime + " " + (core.is_database_online === 1 ? "(online)" : "(offline)")
        font.bold: true
        color: "white"
        background: Rectangle {
            color: core.is_database_online === 1 ? "green" : "red"
        }
    }

    footer: ToolBar {
//        background: Rectangle {
//            color: "green"
//        }

        RowLayout {
            width: parent.width
//            CheckBox {
//                id: show_all_checkbox
//                text: "Edit mode"
//            }
            Item {
                Layout.fillWidth: true
            }

            TextFieldX {
                id: capture_name
                placeholder_text: "Enter cam name"
                Layout.maximumWidth: cm(6)
                readOnly: {core.is_database_online !== 1}
                onAccepted: {
                    console.log("UPDATE NAME")
                    done_button.clicked()
                }
            }
            ToolButton {
                id: done_button
                text: qsTr("Update")
                enabled: capture_name.text_input.length > 0 && profile_sent === 0
                onClicked: {
                    Qt.inputMethod.commit()
                    if(core.set_simple_profile(capture_name.text_input, "", "", "") === 1) {
                        profile_sent = 1
                    }
                    else
                    {
                        animateOpacity.start()
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        core.hangup_all_calls()
        core.set_local_setting("mode", "capture")
        //preview_cam.start()
        core.select_vid_dev(2)
        core.enable_video_capture_preview(1)
        capture_name.text_input = core.uid_to_name(core.this_uid)
    }

    onVisibleChanged: {
        if(visible)
        {
            core.hangup_all_calls()
            core.set_local_setting("mode", "capture")
            //preview_cam.start()
            core.select_vid_dev(2)
            core.enable_video_capture_preview(1)
            capture_name.text_input = core.uid_to_name(core.this_uid)
        }
        else
        {
            core.hangup_all_calls()
            //preview_cam.stop()
            core.select_vid_dev(0)
            core.enable_video_capture_preview(0)
        }

    }

    DSM.StateMachine {
        id: streaming_state_machine

        initialState: idle
        running: true

        DSM.State {
            id: idle
            onEntered: {
                console.log("IDLE")
                core.enable_video_capture_preview(0)
                viewer.source = mi("ic_videocam_off_black_24dp.png")
            }

            DSM.SignalTransition {
                signal: CallContextModel.countChanged
                guard: CallContextModel.count === 1
                targetState: streaming
            }
        }

        DSM.State {
            id: streaming
            onEntered: {
                console.log("STREAMING")
                core.enable_video_capture_preview(1)
            }

            DSM.SignalTransition {
                signal: CallContextModel.countChanged
                guard: CallContextModel.count === 0
                targetState: idle
            }
        }
    }



    Connections {
        target: core

        onSc_connect_terminated: {
            console.log("CONNECT TERMINATED ", uid)
        }

        onSc_connectedChanged: {
            console.log("ConnectedChanged ", connected, uid)
            if(connected === 0) {
                core.delete_call_context(uid)

                //vid_panel.vid_incoming.source = mi("ic_videocam_off_black_24dp.png")

            } else {
                call_buttons_model = core.get_button_model(uid)
                call_buttons_model.get("send_video").clicked()
            }
        }

        onProfile_update: {
            if(success === 1) {
                profile_sent = 0
            } else {
                profile_sent = 0
                animateOpacity.start()
            }
        }
    }

    Connections {
        target: CallContextModel

        onCountChanged: {
            console.log("CALL COUNT ", CallContextModel.count)
//            if(CallContextModel.count === 1) {
//                preview_cam.start()
//                core.enable_video_capture_preview(1)
//            } else if(CallContextModel.count === 0) {
//                preview_cam.stop()
//                core.enable_video_capture_preview(0)
//            }
        }
    }

//    Camera {
//        id: preview_cam
//        objectName: "qrCameraQML"
//        viewfinder {
//            resolution: Qt.size(320, 240)
//            maximumFrameRate: 20
//        }
//        position: Camera.FrontFace
//        captureMode: Camera.CaptureViewfinder
//        onCameraStateChanged: {
//            //if(state === Camera.ActiveState) {
//                var res = preview_cam.supportedViewfinderResolutions();
//                console.log("RESOLUTIONS ")
//            for(var i = 0; i < res.length; i++) {
//                console.log(res[i].width)
//                console.log(res[i].height)
//            }
//            //}
//        }
//        onCameraStatusChanged: {
//            //if(state === Camera.ActiveState) {
//                var res = preview_cam.supportedViewfinderResolutions();
//                console.log("RESOLUTIONS ")
//            for(var i = 0; i < res.length; i++) {
//                console.log(res[i].width)
//                console.log(res[i].height)
//            }

//            //}
//        }
//    }


    Rectangle {
        anchors.fill: parent
        // bottom rectangle is preview of video
        Rectangle {
            anchors.fill: parent
            Image {
                id: viewer
                anchors.fill: parent
                //anchors.horizontalCenter: parent.horizontalCenter
                fillMode: Image.PreserveAspectFit
                Connections {
                    target: core
                    onVideo_capture_preview: {
                        if(visible)
                            viewer.source = img_path
                    }
                }
            }
        }

        // on top of that is the display of the model of who we
        // are sending it to
        ListView {
            id: clients
            model: CallContextModel
            anchors.fill: parent

            spacing: mm(2)
            delegate: Item {
                id: wrapper2
                width: parent.width
                height: drow2.implicitHeight
                RowLayout {
                    id: drow2
                    anchors.fill: parent
                    spacing: mm(3)
                    Label {
                        text: uid
                        Layout.alignment: Qt.AlignLeft
                        elide: Text.ElideRight
                        Layout.preferredWidth: cm(2)
                        background: Rectangle {
                            visible: connected
                            color: sending_video ? "yellow" : "red"
                            opacity: .6
                        }
                    }
                    Label {
                        text: core.uid_to_name(uid)
                        elide: Text.ElideRight
                        Layout.alignment: Qt.AlignLeft
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
            clip: true
        }

    }

    BusyIndicator {
        id: busy1
        running: {profile_sent === 1}
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        z: 10
    }

    Text {
        id: failed_msg
        text: "Name update failed... " + ((core.is_database_online !== 1) ? "(offline)" : "")
        anchors.centerIn: parent

        opacity: 0.0
        NumberAnimation {
               id: animateOpacity
               target: failed_msg
               properties: "opacity"
               from: 1.0
               to: 0.0
               duration: 3000


          }
    }

}
