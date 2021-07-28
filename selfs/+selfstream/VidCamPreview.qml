
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

// this is a simplified camera setup for android phone that will
// automatically connect to the selfie-camera.
// typically used if you are doing to use an old camera as a
// security camera.

Page {
    //property bool dragging
    id: campage
    property string serving_uid
    property string attempt_uid
    property var call_buttons_model
    property int profile_sent: 0

    anchors.fill: parent

    function hangup() {
        core.hangup_all_calls()
    }

    header: Label {
        text: "Dwyco Selfie Stream " + (core.is_database_online === 1 ? "(online)" : "(offline)")
        font.bold: true
        color: "white"
        background: Rectangle {
            color: core.is_database_online === 1 ? "green" : "red"
        }
    }

    onVisibleChanged: {
        if(visible)
        {
            var mode = core.get_local_setting("mode")
            if(mode === "") {
                core.set_local_setting("mode", "watch")
                mode = "watch"
            }
            var wname = core.get_local_setting("camera-to-watch")
            if(wname.length === 0) {
                watch_name.placeholder_text = "enter name of camera to watch"
            } else {
                watch_name.text_input = wname
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
                capture_name.text_input = core.uid_to_name(core.this_uid)
            }
        }
        else
        {
            core.enable_video_capture_preview(0)
        }

    }

    onAttempt_uidChanged: {
        if(attempt_uid.length > 0) {
            console.log("start control to ", attempt_uid)
            hangup()
            core.start_control(attempt_uid)
        } else {
            hangup()
        }
    }


    Connections {
        target: core
        onName_to_uid_result: {
            console.log("GOT UID FOR NAME ", uid, handle)
            core.set_pal(uid, 1)
            attempt_uid = uid
        }

        onSc_connect_terminated: {
            console.log("CONNECT TERMINATED ", uid)
            serving_uid = ""
            attempt_uid = ""
        }

        onSc_connectedChanged: {
            console.log("ConnectedChanged ", connected, uid)
            if(connected === 0) {
                if(serving_uid === uid)
                {
                    serving_uid = ""
                    call_buttons_model = null
                }
                vid_panel.vid_incoming.source = mi("ic_videocam_off_black_24dp.png")
                core.delete_all_call_contexts()
            } else {
                if(serving_uid === "")
                {
                    serving_uid = uid
                    call_buttons_model = core.get_button_model(serving_uid)
                    if(cam_sender.checked)
                        call_buttons_model.get("send_video").clicked()
                }
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

        onPal_event: {
            DiscoverList.load_users_to_model();
        }

//        onQt_app_state_change: {
//            console.log("app state change ", app_state)
//            if(app_state === 0) {
//                // resuming
//                campage.visible = true
//            } else {
//                // pausing
//                campage.visible = false
//            }

//        }
    }

    Camera {
        id: preview_cam
        objectName: "qrCameraQML"
        viewfinder {
            resolution: Qt.size(320, 240)
            maximumFrameRate: 20
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
                    core.hangup_all_calls()
                    capture_name.text_input = core.uid_to_name(core.this_uid)
                    core.set_local_setting("mode", "capture")
                    preview_cam.start()
                    core.select_vid_dev(2)
                    core.enable_video_capture_preview(1)
                    attempt_uid = ""
                    serving_uid = ""
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
                    core.hangup_all_calls()
                    preview_cam.stop()
                    core.select_vid_dev(0)
                    core.enable_video_capture_preview(0)
                    //core.name_to_uid(watch_name.text_input)
                    core.set_local_setting("mode", "watch")
                    viewer.source = ""
                    attempt_uid = ""
                    serving_uid = ""
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
            readOnly: {core.is_database_online !== 1}
            onAccepted: {
                console.log("UPDATE NAME")
                done_button.clicked()
            }
            Button {
                id: done_button

                text: qsTr("Update")
                enabled: {profile_sent === 0}
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
        TextFieldX {
            id: watch_name
            visible: cam_watcher.checked
            Layout.fillWidth: true
            onAccepted: {
                console.log("WTF")
                attempt_uid = ""
                var sname = text_input
                core.name_to_uid(sname)
                core.set_local_setting("camera-to-watch", text_input)
            }
        }
        ListView {
            id: clients
            model: CallContextModel
            highlight: Rectangle { z:3 ; color: primary_light; opacity: .3}
            highlightMoveDuration: 100
            highlightMoveVelocity: -1
            visible: cam_sender.checked
            spacing: mm(2)
            delegate: Item {
                id: wrapper2
                width: parent.width
                height: drow2.implicitHeight
                // note: this mousearea *must* be here to allow selecting
                // the entire row, while allowing the mousearea for the little
                // "delete" button to work properly below.
                MouseArea {
                    anchors.fill: drow2
                    onClicked: {
                        clients.currentIndex = index
                        //attempt_uid = uid
                        //watch_name.text_input = display
                    }
                }
                RowLayout {
                    id: drow2
                    anchors.fill: parent
                    spacing: mm(3)
                    Rectangle {
                        Layout.minimumHeight: parent.height
                        Layout.maximumHeight: parent.height
                        Layout.minimumWidth: parent.height
                        Layout.maximumWidth: parent.height
                        color: wrapper2.ListView.isCurrentItem ? "black" : "white"
                        MouseArea {
                            anchors.fill: parent
                            z: 5
                            cursorShape: Qt.CrossCursor
                            onClicked: {
                                //core.set_pal(uid, 0)
                            }
                        }
                    }

                    Label {
                        text: uid
                        Layout.alignment: Qt.AlignLeft
                        elide: Text.ElideRight
                        Layout.preferredWidth: cm(2)
                        background: Rectangle {
                            visible: connected
                            color: sending_video ? "yellow" : "red"
                        }
                    }
                    Label {
                        text: "foo"
                        elide: Text.ElideRight
                        Layout.alignment: Qt.AlignLeft
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
            clip: true
            Layout.fillWidth: true
            Layout.preferredHeight: cm(2)

        }
        ListView {
            id: disco
            model: DiscoverList
            highlight: Rectangle { z:3 ; color: primary_light; opacity: .3}
            highlightMoveDuration: 100
            highlightMoveVelocity: -1
            spacing: mm(2)
            visible: cam_watcher.checked
            delegate: Item {
                id: wrapper
                width: parent.width
                height: drow.implicitHeight
                // note: this mousearea *must* be here to allow selecting
                // the entire row, while allowing the mousearea for the little
                // "delete" button to work properly below.
                MouseArea {
                    anchors.fill: drow
                    onClicked: {
                        disco.currentIndex = index
                        attempt_uid = uid
                        watch_name.text_input = display
                    }
                }
                RowLayout {
                    id: drow
                    anchors.fill: parent
                    spacing: mm(3)
                    Rectangle {
                        Layout.minimumHeight: parent.height
                        Layout.maximumHeight: parent.height
                        Layout.minimumWidth: parent.height
                        Layout.maximumWidth: parent.height
                        color: wrapper.ListView.isCurrentItem ? "black" : "white"
                        MouseArea {
                            anchors.fill: parent
                            z: 5
                            cursorShape: Qt.CrossCursor
                            onClicked: {
                                core.set_pal(uid, 0)
                            }
                        }
                    }

                    Label {
                        text: uid
                        Layout.alignment: Qt.AlignLeft
                        elide: Text.ElideRight
                        Layout.preferredWidth: cm(2)
                        background: Rectangle {
                            visible: online
                            color: "green"
                        }
                    }
                    Label {
                        text: display
                        elide: Text.ElideRight
                        Layout.alignment: Qt.AlignLeft
                        Layout.preferredWidth: cm(2)
                        font.bold: true
                    }
                    Label {
                        text: ip
                        elide: Text.ElideRight
                        Layout.alignment: Qt.AlignRight
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
            clip: true
            Layout.fillWidth: true
            Layout.preferredHeight: cm(2)

        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: cam_sender.checked
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
        VidCall {
            id: vid_panel
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: cam_watcher.checked
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
