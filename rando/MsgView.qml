
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
    id: msgviewer
    property string uid
    property string mid
    property int view_id: -1
    property int ui_id: -1
    property alias view_source : viewer.source
    //property alias msg_text: msg_text.text
    property bool dragging
    property bool fav
    property bool hid

    //anchors.fill:parent


    fav: { (mid.length > 0) ?
             (core.get_fav_message(mid) === 1) : false
    }

    hid: {mid.length > 0 ? core.has_tag_message(mid, "_hid") === 1 : false}


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
                    text: fav ? "Unfavorite" : "Favorite"
                    onTriggered: {
                        core.set_fav_message(mid, !fav)
                        // oops, breaks binding
                        //fav = !fav
                        // this just causes the binding to be
                        // recomputed, probably a better way of doing this
                        // eventually, like onMid_tag_changed signal
                        var save_mid = mid
                        mid = ""
                        mid = save_mid
                        //themsglist.reload_model()
                    }
                }
                MenuItem {
                    text: "Report msg"
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
            anchors.bottom: dragging ? undefined : parent.bottom
            //anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
            onVisibleChanged: {
                dragging = false
                scale = 1.0
            }

        }

        Rectangle {
            id: isfav
            width: 32
            height: 32
            anchors.top: parent.top
            anchors.left: parent.left
            visible: fav
            z: 3
            color: primary_light
            radius: width / 2
            Image {
                anchors.fill: parent
                anchors.margins: 2
                source: mi("ic_star_black_24dp.png")
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
    BusyIndicator {
        id: busy1
        running: view_source == "" && view_id !== -1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        z: 20
    }


}


