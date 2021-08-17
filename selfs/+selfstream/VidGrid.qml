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

            DSM.StateMachine {
                id: sm
                initialState: idle
                running: true
                DSM.State {
                    id: idle
                    onEntered: {
                        if(attempt_uid != "")
                            core.delete_call_context(attempt_uid)
                        attempt_uid = ""
                        attempt_handle = ""
                        status_label.text = ""
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
                        signal: lookup_failed
                        targetState: idle
                    }
                    DSM.SignalTransition {
                        signal: lookup_succeeded
                        targetState: start_connect
                    }
                }

                DSM.State {
                    id: start_connect
                    onEntered: {
                        core.start_control(attempt_uid)
                    }
                    DSM.SignalTransition {
                        signal: connect_failed
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
                onName_to_uid_result: {
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

                onSc_connectedChanged: {
                    if(uid != attempt_uid)
                        return
                    console.log("ConnectedChanged ", connected, uid)
                    if(connected === 1) {
                        connect_succeeded()
                    } else {
                        connect_failed()
                    }
                }
                onSc_connect_progress: {
                    if(uid != attempt_uid)
                        return
                    status_label.text = msg
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
                        console.log("delete ", model.uid)
                    }
                    Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter
                }
                Label {
                    text: model.display
                    Layout.alignment: Qt.AlignLeft|Qt.AlignVCenter
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
            }
        ]
    }

    GridView {
        id: gview
        anchors.fill: parent
        cellHeight: parent.height / 2
        cellWidth: parent.width / 2

        model: filtered_discover
        delegate: video_delegate

    }

}
