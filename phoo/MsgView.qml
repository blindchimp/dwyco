
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
import QtQuick.Controls 2.2
import dwyco 1.0

Page {
    property string uid
    property string mid
    property int view_id
    property alias view_source : viewer.source
    property alias msg_text: msg_text.text
    property bool dragging
    property bool fav

    anchors.fill:parent
    onVisibleChanged: {
        if(!visible)
        {
            core.stop_zap_view(view_id)
            core.delete_zap_view(view_id)
        }
    }

    fav: { (mid.length > 0) ?
             (core.get_fav_message(mid) === 1) : false
    }

    Connections {
        target: core
        onVideo_display: {
            view_source = img_path
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
                    text: "Delete msg"
                    onTriggered: {
                        core.delete_message(uid, mid)
                        themsglist.reload_model()
                        stack.pop()

                    }
                }

                MenuItem {
                    text: "Forward msg"
                    onTriggered: {
                        forward_dialog.mid_to_forward = mid
                        forward_dialog.uid_folder = uid
                        stack.push(forward_dialog)
                    }
                }

                MenuItem {
                    text: fav ? "Unfavorite" : "Favorite"
                    onTriggered: {
                        core.set_fav_message(uid, mid, !fav)
                        // oops, breaks binding
                        //fav = !fav
                        var save_mid = mid
                        mid = ""
                        mid = save_mid
                        themsglist.reload_model()
                    }
                }

                MenuItem {
                    text: "Report"
                    onTriggered: {
                        stack.push(msg_report)

                    }
                }


            }
        }
    }

    MsgReport {
        id: msg_report
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
            anchors.bottom: dragging ? undefined : msg_text.top
            //anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
            onVisibleChanged: {
                dragging = false
                scale = 1.0
            }

        }

        Text {
            id: msg_text
            //height: 16
            //anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.left:parent.left
            anchors.right:parent.right
            wrapMode: Text.Wrap
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
                core.stop_zap_view(view_id)
                core.delete_zap_view(view_id)
            }
        }
    }
    BusyIndicator {
        id: busy1
        running: {viewer.source == "" && view_id != -1}
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }


}


