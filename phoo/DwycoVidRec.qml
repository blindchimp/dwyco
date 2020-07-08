
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
import QtQuick.Layouts 1.12
import QtQml.StateMachine 1.12 as DSM

Page {
    property string uid
    property int rzid
    property alias view_source : viewer.source
    property bool dragging
    property int video_chan
    property bool something_recorded

    anchors.fill:parent
    onVisibleChanged: {
        if(!visible)
        {
            core.stop_zap_record(rzid)
            core.stop_zap(rzid)
            core.cancel_zap(rzid)
            rzid = -1
            video_chan = -1
            something_recorded = false
            sm.stop()
        }
        else
        {
            rzid = core.make_zap_composition()
            video_chan = -1
            something_recorded = false
            sm.start()
        }
    }

    Connections {
        target: core
        onVideo_display: {
            if(ui_id !== video_chan)
                return
            viewer.source = img_path
            something_recorded = true
        }

        onVideo_capture_preview: {
            if(!visible)
                return
            if(start.active) {
                viewer.source = img_path
            }
            else {
                viewer_inlay.source = img_path
            }
        }

        onZap_stopped: {
            if(zid != rzid)
                return
            video_chan = -1

        }

        onQt_app_state_change: {
            if(app_state === 0) {
                // resuming
//                if(visible) {
//                    sm.start()
//                }
            } else {
                if(visible) {
                stack.pop()
                }

            }

        }
    }

    DSM.StateMachine {
        id: sm
        initialState: start
        onStopped: {
            core.enable_video_capture_preview(0)
        }

        DSM.State {
            id: start
            DSM.SignalTransition {
                targetState: recording
                signal: rec.clicked
            }
            DSM.SignalTransition {
                targetState: pic_recording
                signal: snapshot.clicked
            }
            DSM.SignalTransition {
                targetState: quick_pic_send
                signal: snap_and_send.clicked
            }

            onEntered: {
                core.enable_video_capture_preview(1)
                core.cancel_zap(rzid)
                rzid = core.make_zap_composition()
                video_chan = -1
            }
            onExited: {
                //core.enable_video_capture_preview(0)
            }
        }

        DSM.State {
            id: quick_pic_send
            DSM.SignalTransition {
                targetState: start
                signal: core.zap_stopped
                onTriggered: {
                    core.send_zap(rzid, uid, 1)
                    rzid = -1
                    video_chan = -1
                }
            }
            DSM.TimeoutTransition {
                targetState: start
                // warn about nothing sent
                timeout: 1000
            }

            onEntered: {
                video_chan = core.start_zap_record_pic(rzid)
                something_recorded = false
            }
            onExited: {
                video_chan = -1
            }

        }

        DSM.State {
            id: pic_recording
            DSM.SignalTransition {
                targetState: pic_staged
                signal: core.zap_stopped
            }
            DSM.TimeoutTransition {
                targetState: pic_staged
                timeout: 1000
            }

            onEntered: {
                video_chan = core.start_zap_record_pic(rzid)
                something_recorded = false
            }
            onExited: {
                video_chan = -1
            }

        }

        DSM.State {
            id: pic_staged
            DSM.SignalTransition {
                targetState: start
                signal: send.clicked
                onTriggered: {
                    core.send_zap(rzid, uid, 1)
                    rzid = -1
                    video_chan = -1
                }
            }
            DSM.SignalTransition {
                targetState: pic_recording
                signal: snapshot.clicked
                onTriggered: {
                    core.cancel_zap(rzid)
                    rzid = core.make_zap_composition()
                }
            }
            DSM.SignalTransition {
                targetState: quick_pic_send
                signal: snap_and_send.clicked
                onTriggered: {
                    core.cancel_zap(rzid)
                    rzid = core.make_zap_composition()
                }
            }
            onEntered: {
                video_chan = core.play_zap(rzid)
            }
            onExited: {
                core.stop_zap(rzid)
                video_chan = -1
            }

        }

        DSM.State {
            id: recording
            DSM.SignalTransition {
                targetState: staged
                signal: stop.clicked
            }
            DSM.SignalTransition {
                targetState: staged
                signal: core.zap_stopped
            }
            onEntered: {
                video_chan = core.start_zap_record(rzid, 1, 1)
                something_recorded = false
            }
            onExited: {
                core.stop_zap_record(rzid)
                video_chan = -1
            }

        }
        DSM.State {
            id: staged
            DSM.SignalTransition {
                targetState: start
                signal: send.clicked
                onTriggered: {
                    core.send_zap(rzid, uid, 1)
                    rzid = -1
                    video_chan = -1
                }
            }
            DSM.SignalTransition {
                targetState: previewing
                signal: preview.clicked
            }
            DSM.SignalTransition {
                targetState: recording
                signal: rec.clicked
                onTriggered: {
                    core.cancel_zap(rzid)
                    rzid = core.make_zap_composition()
                }
            }

        }
        DSM.State {
            id: previewing
            DSM.SignalTransition {
                targetState: staged
                signal: stop.clicked
            }
            DSM.SignalTransition {
                targetState: staged
                signal: core.zap_stopped
            }
            onEntered: {
                video_chan = core.play_zap(rzid)
            }
            onExited: {
                core.stop_zap(rzid)
                video_chan = -1

            }

        }
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
                    text: "Cancel msg"
                    onTriggered: {
                        sm.stop()
                        stack.pop()

                    }
                }

            }
        }
    }

    header: SimpleToolbar {
        extras: extras_button

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

            Image {
                id: viewer_inlay
                //x: parent.x + parent.paintedWidth + (parent.width - parent.paintedWidth) / 2 - width
                //y: parent.y + parent.paintedHeight + (parent.height - parent.paintedHeight) / 2 - height
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    bottomMargin: (parent.height - parent.paintedHeight) / 2 - (height - paintedHeight) / 2
                    rightMargin: (parent.width - parent.paintedWidth) / 2 - (width - paintedWidth) / 2
                }

                width: parent.width / 4
                height: parent.height / 4

                fillMode: Image.PreserveAspectFit
                visible: start.active || pic_staged.active
            }

        }

        RowLayout {
            id: snapshot_controls
            spacing: 8
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
                margins: 8
            }
            width: parent.width
            z: 20
            opacity: .7
            visible: sm.running

            Button {
                id: snapshot
                text: start.active ? "Snap" : "Redo"
                enabled: start.active || pic_staged.active
                visible: start.active || pic_staged.active

                //Layout.fillWidth: true

            }
            Item {
                Layout.fillWidth: true
            }

            Button {
                id: snap_and_send
                text: "Snap && Send"
                enabled: start.active || pic_staged.active
                visible: start.active || pic_staged.active

                //Layout.fillWidth: true

            }

        }

        RowLayout {
            id: camcontrols
            spacing: 8
            anchors {
                bottom: parent.bottom
                margins: 8
                horizontalCenter: parent.horizontalCenter

            }
            width: parent.width
            z:20
            visible: sm.running
            opacity: 0.7
            Button {
                id: rec
                text: start.active ? "Rec" : "Redo"
                enabled: start.active || staged.active
                visible: start.active || staged.active

                Layout.fillWidth: true

            }
            Button {
                id: stop
                text: "Stop"
                enabled: recording.active || previewing.active
                visible: recording.active || previewing.active

                Layout.fillWidth: true
            }
            Button {
                id: preview
                text: "Preview"
                enabled: staged.active
                visible: staged.active

                Layout.fillWidth: true
            }
            Button {
                id: send
                text: "Send"
                enabled: staged.active || pic_staged.active
                visible: staged.active || pic_staged.active

                Layout.fillWidth: true
            }
            Button {
                id: cancel
                text: "Cancel"
                Layout.fillWidth: true
                onClicked: {
                    // jettison and go back to initial state
                    sm.stop()
                    stack.pop()
                }
            }

        }

    }


//    PinchArea {
//        anchors.fill: parent
//        pinch.target: viewer
//        pinch.minimumScale: 0.1
//        pinch.maximumScale: 10
//        pinch.dragAxis: Pinch.XAndYAxis

//        MouseArea {
//            id: dragArea
//            hoverEnabled: true
//            anchors.fill: parent
//            drag.target: viewer
//            scrollGestureEnabled: false

//            onPressed: {
//                dragging = true

//            }
//            onClicked: {
//                stack.pop()
//                core.stop_zap_record(rzid)
//                core.cancel_zap(rzid)
//            }
//        }
//    }
    BusyIndicator {
        id: busy1
        running: {viewer.source == "" && rzid != -1}
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

}
