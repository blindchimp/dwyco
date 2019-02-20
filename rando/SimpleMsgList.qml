/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12


Page {
    id: msglist
    property alias model: listview.model
    property bool show_sent: false
    property bool show_recv: true
    property bool multiselect_mode: false
    property string uid

    anchors.fill: parent
    Component.onCompleted: {
        sent.checked = true
    }

    background: Rectangle {
        gradient: Gradient {
            GradientStop { position: 0.0; color: msglist.model.uid === the_man ? amber_light : primary_light }
            GradientStop { position: 1.0; color: msglist.model.uid === the_man ? amber_dark : primary_dark}
        }
    }

    onVisibleChanged: {
        if(visible) {
            if(top_dispatch.last_uid_selected === "") {
                top_dispatch.uid_selected(the_man, "clicked")
            }

            themsglist.reload_model()
            core.reset_unviewed_msgs(top_dispatch.last_uid_selected)
        }
    }

    header: Column {
        width: parent.width
        MultiSelectToolbar {
            id: multi_toolbar
            visible: multiselect_mode
            //extras: extras_button
        }
        ToolBar {
            id: regular_toolbar
            background: Rectangle {
                color: accent
            }
            visible: !multiselect_mode

            implicitWidth: parent.width

            RowLayout {
                Item {
                    Layout.minimumHeight: cm(1)
                }
                anchors.fill: parent
                //spacing: mm(5)

                ToolButton {
                    contentItem: Image {
                        source: mi("ic_menu_black_24dp.png")
                        anchors.centerIn: parent
                    }
                    onClicked: drawer.open()
                    visible: stack.depth === 1
                    Layout.fillHeight: true
                }
                Item {

                    Layout.fillWidth: true
                }

                GridToggle {
                    id: show_grid
                    Layout.fillHeight: true
                }
                Item {

                    Layout.fillWidth: true
                }

                ButtonGroup {
                    id: radio
                }

                ToolButton {
                    id: sent
                    ButtonGroup.group: radio
                    text: "Sent"
                    background: Rectangle {
                        color: amber_dark
                        radius: 3
                    }
                    contentItem: Text {
                        x: sent.leftPadding
                        y: sent.topPadding
                        width: sent.availableWidth
                        height: sent.availableHeight

                        text: sent.text
                        font: sent.font
                        color: sent.checked ? "white" : "gray"
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    checkable: true
                    onCheckedChanged: {
                        if(checked) {

                            top_dispatch.uid_selected(the_man, "clicked")
                            recv.checked = false
                        }
                        show_sent = checked
                    }
                }
                Item {

                    Layout.fillWidth: true
                }
                ToolButton {
                    id: recv
                    ButtonGroup.group: radio
                    text: {"Received" + (core.unread_count > 0 ? " (" + String(core.unread_count) + ")" : "")}
                    background: Rectangle {
                        color: primary_dark
                        radius: 3
                    }
                    contentItem: Text {
                        x: recv.leftPadding
                        y: recv.topPadding
                        width: recv.availableWidth
                        height: recv.availableHeight

                        text: recv.text
                        font: recv.font
                        color: recv.checked ? "white" : "gray"
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    checkable: true
                    onCheckedChanged: {
                        if(checked) {
                            sent.checked = false
                            var i
                            var u
                            for(i = 0; i < ConvListModel.count; i++) {
                                u = ConvListModel.get(i).uid
                                if(u !== the_man) {
                                    top_dispatch.uid_selected(u, "clicked")
                                    break;
                                }
                            }
                            core.reset_unviewed_msgs(u)
                        }
                        show_recv = checked
                    }
                }
                Item {

                    Layout.fillWidth: true
                }
            }
        }
    }

    Component {
        id: msg_delegate

        CircularImage {
            width: listview.width
            height: width

            id: img
            visible: (show_sent && SENT == 1) || (show_recv && SENT == 0)
            asynchronous: true
            source: {PREVIEW_FILENAME != "" ? ("file:///" + String(PREVIEW_FILENAME)) : ""}
            fillMode: Image.PreserveAspectCrop
            sourceSize.width: 512
            sourceSize.height: 512
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    themsgview.mid = model.mid
                    themsgview.uid = model.ASSOC_UID
                    themsgview.view_source = source
                    stack.push(themsgview)

                }
            }
            Rectangle {
                id: isfav
                width: 32
                height: 32
                anchors.top: img.top
                anchors.left: img.left
                visible: IS_FAVORITE === 1
                z: 3
                color: primary_light
                radius: width / 2
                Image {
                    anchors.fill: parent
                    anchors.margins: 2
                    source: mi("ic_star_black_24dp.png")
                }
            }
            Image {
                id: deco2
                visible: IS_QD
                source: mi("ic_cloud_upload_black_24dp.png")
                anchors.left: img.left
                anchors.top: img.top
                width: 32
                height: 32
            }
            Loader {
                anchors.centerIn: img
                //anchors.fill: img
                width: img.width
                anchors.margins: mm(1)
                sourceComponent: ProgressBar {
                    id: pbar
                    width: parent.width
                    visible: IS_ACTIVE
                    value: ATTACHMENT_PERCENT
                    indeterminate: {ATTACHMENT_PERCENT < 0.0}
                    to: 100.0
                    z: 4
                    background: Rectangle {
                        color: "green"
                    }
                }
                visible: IS_ACTIVE
                active: IS_ACTIVE
            }
        }
    }

    ListView {
        id: listview
        anchors.fill: parent
        clip: true
        delegate: msg_delegate
        model: themsglist
    }

    function snapshot(filename) {
        core.send_simple_cam_pic(the_man, "for review", filename)

    }

    ColumnLayout {
        visible: {model.uid === the_man && listview.count === 0}

        anchors.fill: parent
        anchors.margins: mm(5)
        Item {
            Layout.fillHeight: true
        }

        Label {
            Layout.fillWidth: true
            text: "Click the Camera button below to take a picture and anonymously send it to a stranger."
            wrapMode: Text.WordWrap
        }
        Label {
            Layout.fillWidth: true
            text: "You'll receive a picture in return."
            wrapMode: Text.WordWrap

        }
        Item {
            Layout.fillHeight: true
        }
    }

    RoundButton {
        anchors.bottom: parent.bottom
        anchors.margins: mm(3)
        anchors.horizontalCenter: parent.horizontalCenter
        icon.source: mi("ic_add_a_photo_black_24dp.png")
        icon.color: "blue"
        width: cm(2)
        height: cm(2)
        onClicked: {
            cam.next_state = "StopAndPop"
            cam.ok_text = "Upload"
            stack.push(cam)
        }
    }

}
