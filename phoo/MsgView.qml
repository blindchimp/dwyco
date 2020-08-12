
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import dwyco 1.0

Page {
    id: msgviewer
    property string uid
    property string mid
    property int view_id: -1
    property int ui_id: -1
    property alias view_source : viewer.source
    property alias msg_text: msg_text.text
    property bool dragging
    property bool fav
    property bool hid
    property string text_bg_color: primary_dark

    anchors.fill:parent

//    background: Rectangle {
//        color: primary_dark
//        gradient: Gradient {
//            GradientStop { position: 0.0; color: "green" }
//            GradientStop { position: 1.0; color: primary_light}
//        }
//    }

    onVisibleChanged: {
        if(!visible)
        {
            core.stop_zap_view(view_id)
            core.delete_zap_view(view_id)
        }
        else
        {
            if(view_id !== -1) {
                ui_id = core.play_zap_view(view_id)
            }
        }
    }

    fav: { (mid.length > 0) ?
             (core.get_fav_message(mid) === 1) : false
    }

    hid: {mid.length > 0 ? core.has_tag_message(mid, "_hid") === 1 : false}

    Connections {
        target: core
        onVideo_display: {
            if(ui_id === msgviewer.ui_id) {
                view_source = img_path
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
//                MenuItem {
//                    text: "Delete msg"
//                    onTriggered: {
//                        core.delete_message(uid, mid)
//                        themsglist.reload_model()
//                        stack.pop()

//                    }
//                }

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
                    text: hid ? "Unhide" : "Hide"
                    onTriggered: {
                        if(hid)
                            core.unset_tag_message(mid, "_hid")
                        else
                            core.set_tag_message(mid, "_hid")
                        var save_mid = mid
                        mid = ""
                        mid = save_mid
                        //themsglist.reload_model()
                    }
                }
                MenuItem {
                    text: "Copy Text"
                    onTriggered: {
                        msg_text.selectAll()
                        msg_text.copy()
                    }
                }

                MenuItem {
                    text: "Report"
                    onTriggered: {
                        stack.push(msg_report)

                    }
                }

                MenuItem {
                    text: "Review"
                    visible: core.this_uid === the_man
                    onTriggered: {
                        stack.push(msg_review)

                    }
                }


            }
        }
    }

    MsgReport {
        id: msg_report
        visible: false
    }

    MsgReview {
        id: msg_review
        visible: false
    }

    header: SimpleToolbar {
        extras: extras_button

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
    Rectangle {
        id: hidden
        width: 32
        height: 32
        anchors.right:parent.right
        anchors.top:parent.top
        visible: hid
        z: 3
        color: "orange"
    }

    ColumnLayout {
        anchors.fill: parent

    Rectangle {
        //anchors.fill:parent
        Layout.fillHeight: view_source === "" ? false : true
        Layout.fillWidth: true
        color: primary_dark
        gradient: Gradient {
            GradientStop { position: 0.0; color: primary_dark }
            GradientStop { position: 1.0; color: primary_light}
        }

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
                    core.stop_zap_view(view_id)
                    core.delete_zap_view(view_id)
                    stack.pop()
                }
            }
        }
    }
    // note: couldn't get scrolling to work, this is really a corner
    // case i'll have to check out later.
    //ScrollView {
    //    Layout.fillWidth: true
    //    Layout.maximumHeight: viewer_source === "" ? (parent.height * 9) /10 : parent.height / 3

    TextArea {
        id: msg_text
        //anchors.fill: parent
        background: Rectangle {
            color: msgviewer.text_bg_color
            radius: 6
        }
        readOnly: true
        selectByKeyboard: !is_mobile
        selectByMouse: !is_mobile
        textFormat: Text.AutoText
        onLinkActivated: {
            Qt.openUrlExternally(link)
        }

        Layout.fillWidth: true
        Layout.maximumHeight: viewer_source === "" ? (parent.height * 6) / 10 : parent.height / 3

        wrapMode: Text.Wrap
    }
    //}

    }

    BusyIndicator {
        id: busy1
        running: view_source == "" && view_id !== -1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        z: 20
    }


}


