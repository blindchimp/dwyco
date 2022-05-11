
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
    //property alias msg_text: msg_text.text
    property bool dragging
    property bool fav
    property bool hid
    property string text_bg_color: primary_dark
    property string export_result

    //anchors.fill:parent


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
            fav = core.has_tag_message(mid, "_fav")
            hid = core.has_tag_message(mid, "_hid")
        }
    }

//    fav: { (mid.length > 0) ?
//             (core.get_fav_message(mid) === 1) : false
//    }

//    hid: {mid.length > 0 ? core.has_tag_message(mid, "_hid") === 1 : false}

    Connections {
        target: core
        function onVideo_display(ui_id, frame_number, img_path) {
            if(ui_id === msgviewer.ui_id) {
                view_source = img_path
            }
        }
        function onMid_tag_changed(changed_mid) {
            if(changed_mid != mid)
                return
            fav = core.has_tag_message(mid, "_fav")
            hid = core.has_tag_message(mid, "_hid")
        }
        function onMsg_tag_change_global(changed_mid, huid) {
            if(changed_mid != mid)
                return
            fav = core.has_tag_message(mid, "_fav")
            hid = core.has_tag_message(mid, "_hid")
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
                    text: fav ? "Unfavorite" : "Favorite"
                    onTriggered: {
                        core.set_fav_message(mid, !fav)
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

    footer: ToolBar {
        id: footbar
        //visible: !clean_button.checked
        implicitWidth: parent.width
        RowLayout {
            width: parent.width
            TipButton {
                id: fav_button
                contentItem: Image {
                    source: mi("ic_star_black_24dp.png")

                }
                background: Rectangle {
                    visible: fav
                    color: primary_light
                    radius: width / 2
                }
                onCheckedChanged: {
                    core.set_fav_message(mid, checked)
                }

                checkable: true
                checked: fav
                ToolTip.text: "Favorite msg"
                Layout.fillHeight: true
            }

//            TipButton {
//                id: clean_button
//                checkable: true
//                Layout.fillHeight: true
//                ToolTip.text: "Full screen"
//                contentItem: Image {
//                    source: mi("ic_fullscreen_black_24dp.png")

//                }
//            }

            Item {
                Layout.fillWidth: true
            }
            TipButton {
                id: save_button
                contentItem: Image {
                    source: mi("ic_share_black_24dp.png")

                }

                onClicked: {
                    var export_name = core.export_attachment(mid)
                    if(export_name.length > 0)
                        export_result = "Saved to " + export_name
                    else
                        export_result = "FAILED save "
                    toast_opacity.stop()
                    toast_opacity.start()
                }

                ToolTip.text: "Save rando"
                Layout.fillHeight: true
            }
            Item {
                Layout.fillWidth: true
            }
//            TipButton {
//                id: hid_button
//                contentItem: Image {
//                    source: mi("ic_visibility_off_black_24dp.png")
//                }
//                background: Rectangle {
//                    visible: hid
//                    color: "orange"

//                }
//                onCheckedChanged: {
//                if(checked)
//                    core.set_tag_message(mid, "_hid")
//                else
//                    core.unset_tag_message(mid, "_hid")
//                }
//                checkable: true
//                checked: hid
//                ToolTip.text: "Hide msg"
//                Layout.fillHeight: true
//            }
        }
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

    Label {
        id: save_toast
        text: export_result
        color: "white"
        anchors.centerIn: parent
        z: 5
        background: Rectangle {
            radius: 3
            color: "black"
        }

        opacity: 0.0
        NumberAnimation {
            id: toast_opacity
            target: save_toast
            easing: Easing.InQuart
            properties: "opacity"
            from: 1.0
            to: 0.0
            duration: 5000
        }

        onVisibleChanged: {
            toast_opacity.stop()
            opacity = 0.0
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


