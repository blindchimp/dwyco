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
    property int storage_warning: 1
    property string uid

    //anchors.fill: parent


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
            if(!sent.checked && !recv.checked)
                sent.checked = true
            if(core.get_local_setting("storage_warning") === "")
                storage_warning = 1
            else
                storage_warning = 0
            AndroidPerms.load()
            if(storage_warning === 1 && !AndroidPerms.external_storage_permission) {
                warn.visible = true
            }
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
                    property bool hoopty
                    hoopty: checked
                    ButtonGroup.group: radio
                    Layout.fillHeight: true
                    Layout.margins: mm(.25)
                    Layout.minimumWidth: recv.width

                    text: "Sent"
                    background: Rectangle {
                        id: sent_bg_rect
                        color: amber_dark
                        radius: 6
                    }
                    contentItem: Text {
                        x: sent.leftPadding
                        y: sent.topPadding
                        width: sent.availableWidth
                        height: sent.availableHeight

                        text: sent.text
                        //font: sent.font
                        color: "white" //sent.checked ? "white" : "gray"
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
                        show_recv = false

                    }
                    Rectangle {
                        visible: sent.checked
                        width: parent.width * .75
                        anchors.margins: mm(1)
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom

                        height: mm(.5)
                        color: "white"
                    }

                }
                Item {

                    Layout.fillWidth: true
                }
                ToolButton {
                    id: recv
                    ButtonGroup.group: radio
                    text: {"Received" + (core.unread_count > 0 ? " (" + String(core.unread_count) + ")" : "")}
                    Layout.fillHeight: true
                    Layout.margins: mm(.25)

                    background: Rectangle {
                        color: primary_dark
                        radius: 6
                    }
                    contentItem: Text {
                        x: recv.leftPadding
                        y: recv.topPadding
                        width: recv.availableWidth
                        height: recv.availableHeight

                        text: recv.text
                        //font: recv.font
                        color: "white" //recv.checked ? "white" : "gray"
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
                        show_sent = false

                    }
                    Rectangle {
                        visible: recv.checked
                        width: parent.width * .75
                        anchors.margins: mm(1)
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        height: mm(.5)
                        color: "white"
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
            height: (((show_sent && SENT === 0) || (show_recv && SENT === 1)) || IS_FILE === 0) ? 0 : width

            id: img
            visible: IS_ACTIVE || IS_QD || ((((show_sent && SENT === 1) || (show_recv && SENT === 0)) && IS_FILE === 1))
            asynchronous: true
            source: {PREVIEW_FILENAME !== "" ? ("file:///" + String(PREVIEW_FILENAME)) : ""}
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
            Behavior on visible {
                SequentialAnimation {

                NumberAnimation {
                    target: img
                    property: "opacity"
                    duration: 250
                    easing.type: Easing.InOutQuad
                    from: 0.0
                    to: 1.0
                }
                }
            }

            Rectangle {
                id: isfav
                width: 32
                height: 32
                anchors.top: img.top
                anchors.right: img.right
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
                id: has_geo_info
                source: msglist.model.uid === the_man ? mi("ic_language_black_24dp.png") : mi("ic_language_white_24dp.png")
                anchors.top: img.top
                anchors.left: img.left
                anchors.margins: mm(.5)
                visible: location.text.length > 0
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if(location.state == "moveIn")
                            location.state = "moveOut"
                        else
                            location.state = "moveIn"
                    }
                }


            Label {
                id: location
                visible: x > -width
                //anchors.top: parent.top
                //anchors.left: parent.left
                //property alias tstate: state
                x: -width
                state: "moveIn"
                text: {
                    if(SENT === 0) {
                        try
                        {
                            var o = JSON.parse(MSG_TEXT)
                            return o.loc
                        }
                        catch(e)
                        {
                            console.log(e)
                            console.log(mid)
                        }
                        return ""
                    } else {
                        if(SENT_TO_LOCATION == "Unknown") {
                            return ""
                        } else {
                            return SENT_TO_LOCATION
                        }
                    }
                }
                color: amber_light
                style: Text.Outline
                styleColor: "black"

                states: [
                    State {
                        name: "moveOut";
                        PropertyChanges { target: location; x: has_geo_info.width ; y: 0 }
                    },
                    State {
                        name: "moveIn";
                        PropertyChanges { target: location; x: -width; y: 0 }
                    }
                ]

                transitions: [
                    Transition {
                        to: "moveOut"
                        NumberAnimation { properties: "x,y"; easing.type: Easing.InOutQuad; duration: 400; loops: 1 }
                    },
                    Transition {
                        to: "moveIn"
                        NumberAnimation { properties: "x,y"; easing.type: Easing.InOutQuad; duration: 400; loops: 1 }
                    }
                ]
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
                z: 4
                ProgressBar {
                    id: pb
                    to: 100
                    width: parent.width
                    anchors.top: parent.bottom
                    anchors.left: parent.left
                    Connections {
                        target: core
                        onMsg_progress : {
                            console.log("PB ", pers_id, msg, percent_done, model.mid)
                            if(pers_id == model.mid) {
                                pb.value = percent_done
                            }
                        }
                    }
                }
            }
            Loader {
                id: pbar_loader
                anchors.centerIn: img
                //anchors.fill: img
                width: img.width
                anchors.margins: mm(1)
                sourceComponent: ProgressBar {
                    id: pbar
                    width: pbar_loader.width
                    visible: model.IS_ACTIVE
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
/*
// sadly, this doesn't work very well
// i was trying to allow a "swipe to switch sent/recv"
// this almost works, but the swipeview eats the mouse
events for the delegates in the listview, rendering all the
per-delegate mouse handling useless.
no combination of putting the swipeview outside the listview or
anything else seemed to solve the problem, it either disables the
scrolling in the listview or doesn't recognizing the swipe.
*/

//        SwipeView {
//            id: swiper
//            interactive: parent.visible
//            anchors.fill: parent
//            currentIndex: sent.checked ? 0 : (recv.checked ? 1 : 0)
//            Item {

//            }
//            Item {

//            }
//            onCurrentIndexChanged: {
//                console.log("swipe ", currentIndex)
//                if(currentIndex === 0)
//                    sent.checked = true
//                if(currentIndex === 1)
//                    recv.checked = true
//            }

//        }

    }

    function snapshot(filename) {
        var a
        a = core.get_local_setting("first-pic")
        if(a === "") {
            core.simple_send(redist, "first")
            core.set_local_setting("first-pic", "done")
        }

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
            text: "Each picture you send, you'll receive a picture in return!"
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

    Warning {
        id: warn
        visible: false
        z: 3
        warning: "You denied access to storage, which is OK. BUT if you uninstall the app, the pictures stored by this app are removed too. IF YOU WOULD LIKE TO KEEP THE PICTURES YOU GET, EVEN IF YOU UNINSTALL, click the button below to quit the app. Then restart the app, and when it asks for permission to access storage, answer YES."
        inhibit_key: "storage_warning"
        oops_text: "Quit and reasses my life-choices"

        onVisibleChanged: {
            if(visible) {
                oops = false
            } else {
                if(core.get_local_setting("storage_warning") === "")
                    storage_warning = 1
                else
                    storage_warning = 0
            }
        }

        onOopsChanged: {
            if(oops)
                Qt.quit()
        }
    }


}
