
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
import QtQuick.Controls.Universal 2.12

// this is a simplified camera setup for android phone that will
// automatically connect to the selfie-camera.
// typically used if you are using an old phone as a
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
        background: Rectangle {
                    color: Universal.color(Universal.Lime)
                }


        RowLayout {
            width: parent.width
            ToolButton {
                text: "Back"
                onClicked: {
                    stack.pop()
                }
                focus: true
                Component.onCompleted: {
                    forceActiveFocus()
                }
                KeyNavigation.right: capture_name
                KeyNavigation.up: preview_button
            }

//            CheckBox {
//                id: show_all_checkbox
//                text: "Edit mode"
//            }
            Item {
                Layout.fillWidth: true
            }

            TextField {
                id: capture_name
                placeholderText: "Enter cam name"
                Layout.maximumWidth: cm(6)
                readOnly: {core.is_database_online !== 1}
                onAccepted: {
                    console.log("UPDATE NAME")
                    done_button.clicked()
                }
                KeyNavigation.right: done_button
                KeyNavigation.up: preview_button
            }
            ToolButton {
                id: done_button
                text: qsTr("Update")
                enabled: capture_name.text.length > 0 && profile_sent === 0
                onClicked: {
                    Qt.inputMethod.commit()
                    if(core.set_simple_profile(capture_name.text, "", "", "") === 1) {
                        profile_sent = 1
                    }
                    else
                    {
                        animateOpacity.start()
                    }
                }
                KeyNavigation.up: preview_button
            }
        }
    }

    Component.onCompleted: {
        core.hangup_all_calls()
        core.set_local_setting("mode", "capture")
        //preview_cam.start()
        core.select_vid_dev(2)
        //core.enable_video_capture_preview(1)
        capture_name.text = core.uid_to_name(core.this_uid)
        core.set_invisible_state(0)
        core.inhibit_all_incoming_calls(0)
    }

    Component.onDestruction: {
        //core.set_local_setting("mode", "")
        core.hangup_all_calls()
        //preview_cam.stop()
        core.select_vid_dev(0)
        core.enable_video_capture_preview(0)
        core.set_invisible_state(1)
        core.inhibit_all_incoming_calls(1)
    }
    // note: visible doesn't work too well if you are using Loader
    // on this component. use complete/destruct instead

//    onVisibleChanged: {
//        if(visible)
//        {
//            core.hangup_all_calls()
//            core.set_local_setting("mode", "capture")
//            //preview_cam.start()
//            core.select_vid_dev(2)
//            core.enable_video_capture_preview(1)
//            capture_name.text_input = core.uid_to_name(core.this_uid)
//        }
//        else
//        {
//            core.hangup_all_calls()
//            //preview_cam.stop()
//            core.select_vid_dev(0)
//            core.enable_video_capture_preview(0)
//        }

//    }

    DSM.StateMachine {
        id: streaming_state_machine

        initialState: idle
        running: true

        DSM.State {
            id: idle
            onEntered: {
                console.log("IDLE")
                core.enable_video_capture_preview(0)
                preview_cam.stop()
                viewer.source = mi("ic_videocam_off_black_24dp.png")
            }

            DSM.SignalTransition {
                signal: CallContextModel.countChanged
                guard: CallContextModel.count === 1
                targetState: streaming
            }

            DSM.SignalTransition {
                signal: preview_button.clicked
                targetState: pview_on
            }
        }

        DSM.State {
            id: streaming
            onEntered: {
                console.log("STREAMING")
                core.enable_video_capture_preview(1)
                preview_cam.start()
            }

            DSM.SignalTransition {
                signal: CallContextModel.countChanged
                guard: CallContextModel.count === 0
                targetState: idle
            }

            DSM.SignalTransition {
                signal: preview_button.clicked
                targetState: pview_on
            }
        }

        DSM.State {
            id: pview_on
            onEntered: {
                core.inhibit_all_incoming_calls(1)
                core.hangup_all_calls()
                core.enable_video_capture_preview(1)
                preview_cam.start()
            }

            onExited: {
                core.inhibit_all_incoming_calls(0)
            }

            DSM.TimeoutTransition {
                targetState: idle
                timeout: 15000
            }

            DSM.SignalTransition {
                targetState: idle
                signal: stop_button.clicked
            }

        }
    }


    Connections {
        target: core

        function onSc_connect_terminated(uid) {
            console.log("CONNECT TERMINATED ", uid)
        }

        function onSc_connectedChanged(uid, connected) {
            console.log("ConnectedChanged ", connected, uid)
            if(connected === 0) {
                core.delete_call_context(uid)

                //vid_panel.vid_incoming.source = mi("ic_videocam_off_black_24dp.png")

            } else {
                call_buttons_model = core.get_button_model(uid)
                call_buttons_model.get("send_video").clicked()
            }
        }

        function onProfile_update(success) {
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

        function onCountChanged() {
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

    QmlCamera {
        id: preview_cam

    }


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
                    function onVideo_capture_preview(img_path) {
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

    Button {
        id: preview_button
        text: "Preview"

        anchors.top: parent.top
        anchors.right: parent.right
        visible: !pview_on.active
        onVisibleChanged: {
            if(visible) {
                if(stop_button.activeFocus) {
                    forceActiveFocus()
                }
            }
        }
    }
    Button {
        id: stop_button
        text: "Done"

        anchors.top: parent.top
        anchors.right: parent.right
        visible: pview_on.active
        onVisibleChanged: {
            if(visible) {
                if(preview_button.activeFocus) {
                    forceActiveFocus()
                }
            }
        }
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
