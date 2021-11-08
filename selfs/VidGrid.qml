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
import QtQml.StateMachine 1.12 as DSM
import SortFilterProxyModel 0.2
import QtQuick.Controls.Universal 2.12
import QtQuick.Window 2.12

Page {
    anchors.fill: parent

    header:
        Label {
            text: "Dwyco Selfie Stream Viewer " + core.buildtime + " " + (core.is_database_online === 1 ? "(online)" : "(offline)")
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
            }
            CheckBox {
                id: show_all_checkbox
                text: "Edit mode"
            }
            Item {
                Layout.fillWidth: true
            }

            TextFieldX {
                id: watch_name
                //visible: cam_watcher.checked
                placeholder_text: "Enter cam name"
                Layout.maximumWidth: cm(6)
                onAccepted: {
                    console.log("WTF")
                    var sname = text_input
                    core.name_to_uid(sname)
                    //core.set_local_setting("camera-to-watch", text_input)
                }
                onText_inputChanged: {
                    //core.set_local_setting("camera-to-watch", text_input)
                }


            }
            ToolButton {
                id: add_button
                text: qsTr("Add")
                enabled: watch_name.text_input.length > 0
                onClicked: {
                    Qt.inputMethod.commit()
                    watch_name.accepted()
                }
            }
        }
    }

    Component.onCompleted: {
        core.set_local_setting("mode", "watch")
    }

    Component.onDestruction: {
        //core.set_local_setting("mode", "")
        core.set_invisible_state(1)
        core.inhibit_all_incoming_calls(1)
    }

    Connections {
        target: core
        function onName_to_uid_result(uid, handle) {
            if(uid != "")
                core.set_pal(uid, 1)
        }

        function onPal_event(uid) {
            DiscoverList.load_users_to_model()
        }
    }


    // note: this is a little bit dicey, since technically these delegates
    // can be deleted and created as needed. but for now, since we only
    // want to have a few on the screen, we'll just assume there aren't
    // a bunch of deletegates streaming offscreen.
    // to fix this, it might be possible to use a Repeater to just instantiate
    // everything as the start, and avoid problems of dynamic creation
    // and deletion. the SortProxy used below also seems to confuse/recreate
    // delegates if the filters are changed. not unexpected, but something
    // to take into account. currently, i just terminate all streams when
    // the filter changes, since we are kinda changing "modes" to edit
    // some item or other.
    Component {
        id: video_delegate


        ColumnLayout {
            signal lookup_failed
            signal lookup_succeeded
            signal connect_succeeded
            signal connect_failed

            property string attempt_uid: ""
            property string attempt_handle: ""
            property var call_buttons_model

            Component.onDestruction: {
                if(attempt_uid != "")
                    core.delete_call_context(attempt_uid)
            }
            DSM.StateMachine {
                id: sm
                initialState: idle
                running: true
                DSM.State {
                    id: idle
                    onEntered: {
                        if(attempt_uid != "")
                            core.delete_call_context(attempt_uid)
                        call_buttons_model = null
                        attempt_uid = ""
                        attempt_handle = ""
                        status_label.text = ""
                        // if the show all checkbox is toggled,
                        // act as if the cancel button is clicked
                        show_all_checkbox.clicked.connect(cancel_button.clicked)
                    }

                    DSM.SignalTransition {
                        signal: connect_button.clicked
                        targetState: lookup_name
                    }
                }

                DSM.State {
                    id: lookup_name
                    onEntered: {
                        attempt_handle = model.display
                        core.name_to_uid(model.display)
                    }
                    DSM.SignalTransition {
                        signal: cancel_button.clicked
                        targetState: idle
                    }
                    DSM.SignalTransition {
                        signal: lookup_failed
                        targetState: idle
                    }
                    DSM.SignalTransition {
                        signal: lookup_succeeded
                        targetState: start_connect
                    }
                    DSM.TimeoutTransition {
                        timeout: 20000
                        targetState: idle
                    }
                }

                DSM.State {
                    id: start_connect
                    onEntered: {
                        core.start_control(attempt_uid)
                        call_buttons_model = core.get_button_model(attempt_uid)
                    }
                    DSM.SignalTransition {
                        signal: cancel_button.clicked
                        targetState: idle
                    }
                    DSM.SignalTransition {
                        signal: connect_failed
                        targetState: idle
                    }
                    DSM.TimeoutTransition {
                        timeout: 20000
                        targetState: idle
                    }

                    DSM.SignalTransition {
                        signal: connect_succeeded
                        targetState: streaming
                    }
                }

                DSM.State {
                    id: streaming
                    DSM.SignalTransition {
                        signal: cancel_button.clicked
                        targetState: idle
                    }
                    DSM.SignalTransition {
                        signal: connect_failed
                        targetState: idle
                    }
                }
            }

            Connections {
                target: core
                function onName_to_uid_result(uid, handle) {
                    if(handle != attempt_handle)
                        return
                    console.log("GOT UID FOR NAME ", uid, handle)
                    if(uid == "") {
                        lookup_failed()
                    } else {
                        core.set_pal(uid, 1)
                        attempt_uid = uid
                        lookup_succeeded()
                    }
                }

                function onSc_connect_failed(uid) {
                    if(uid != attempt_uid)
                        return
                    connect_failed()
                }

                function onSc_connect_terminated(uid) {
                    if(uid != attempt_uid)
                        return
                    connect_failed()
                }

                function onSc_connectedChanged(uid, connected) {
                    if(uid != attempt_uid)
                        return
                    console.log("ConnectedChanged ", connected, uid)
                    if(connected === 1) {
                        connect_succeeded()
                    } else {
                        connect_failed()
                    }
                }
                function onSc_connect_progress(uid, msg) {
                    if(uid != attempt_uid)
                        return
                    status_label.text = msg
                }

                function onSc_associate_uid_with_ui_id(uid, ui_id) {
                    if(uid != attempt_uid)
                        return
                    vc.a_ui_id = ui_id
                }
            }

            property int connected: 0
            width: gview.cellWidth
            height: gview.cellHeight
            RowLayout {
                Layout.fillWidth: true
                ToolButton {
                    id: cancel_button
                    text: "X"
                    onClicked: {
                        console.log("cancel ", model.uid)
                        if(call_buttons_model != null)
                            call_buttons_model.get("cancel_req").clicked()
                    }
                    Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter
                }
                ToolButton {
                    icon.source: mi("ic_delete_black_24dp.png")
                    visible: show_all_checkbox.checked
                    Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter

                    onClicked: {
                        console.log("unpal ", model.uid)
                        core.set_pal(model.uid, 0)
                    }
                }

                Label {
                    text: model.display
                    Layout.alignment: Qt.AlignLeft|Qt.AlignVCenter
                    color: model.online ? "white" : "black"
                    background: Rectangle {
                        color: model.online ? "green" : "white"
                    }
                }
                Label {
                    text: model.ip
                }
                Label {
                    id: status_label
                    visible: !idle.active
                }

                Item {
                    Layout.fillWidth: true
                }

            }
            Button {
                id: connect_button
                text: "connect"
                Layout.fillHeight: true
                Layout.fillWidth: true
                visible: model.online && idle.active
            }
            Label {
                text: "Connecting..."
                font.bold: true
                Layout.fillHeight: true
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                visible: !streaming.active && !idle.active
            }
            Label {
                text: "Offline"
                font.bold: true
                Layout.fillHeight: true
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                visible: connected === 0 && !model.online
            }

            VidCall {
                id: vc
                visible: streaming.active
                Layout.fillHeight: true
                Layout.fillWidth: true

            }
        }
    }

    SortFilterProxyModel {
        id: filtered_discover
        sourceModel: DiscoverList
        filters: [
            ValueFilter {
                roleName: "online"
                value: true
                enabled: !show_all_checkbox.checked
            }
        ]
    }

    GridView {
        id: gview
        anchors.fill: parent
        property bool use_mobile_layout
        use_mobile_layout: Screen.width < 640
        cellHeight: use_mobile_layout ? parent.height : (model.count <= 2 ? parent.height : parent.height / 2)
        cellWidth: use_mobile_layout ? parent.width : (model.count == 1 ? parent.width : parent.width / 2)

        model: filtered_discover
        delegate: video_delegate
        visible: model.count > 0

    }

    Label {
        anchors.fill: parent
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        text: "No Cameras Found Online\n(use Edit Mode to add one)\nLocal network cams appear automatically."
        visible: filtered_discover.count == 0

    }

}
