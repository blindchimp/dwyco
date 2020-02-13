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
import QtQuick.Controls.Material 2.2
import QtPositioning 5.12


Page {
    id: msglist
    property alias model: listview.model
    property bool show_sent: false
    property bool show_recv: true
    property bool multiselect_mode: false
    property int storage_warning: 1
    property string uid

    background: Rectangle {
        gradient: Gradient {
            GradientStop { position: 1.0; color: msglist.model.uid === the_man ? amber_light : primary_light }
            GradientStop { position: 0.0; color: msglist.model.uid === the_man ? amber_dark : primary_dark}
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


            }
        }
    }

    footer: ToolBar {
        id: footer_toolbar
        width: parent.width
        RowLayout {
            anchors.fill: parent
        ButtonGroup {
            id: radio
        }

        ToolButton {
            id: sent

            ButtonGroup.group: radio
            Layout.fillHeight: true
            Layout.margins: mm(.25)
            Layout.fillWidth: true
            Layout.minimumWidth: parent.width / 2
            Layout.maximumWidth: parent.width / 2

            text: "Sent"
            icon.source: mi("ic_cloud_upload_black_24dp.png")
            display: AbstractButton.TextUnderIcon
            checkable: true
            onCheckedChanged: {
                if(checked) {
                    top_dispatch.uid_selected(the_man, "clicked")
                    recv.checked = false
                    sent_badge = false
                }
                show_sent = checked
                show_recv = false
            }
            Rectangle {
                id: badge1
                width: 16
                height: 16
                anchors.right: parent.right
                anchors.top: parent.top
                radius: width / 2
                color: "red"
                visible: /* core.has_unseen_geo || */ sent_badge
            }
        }
//        Item {

//            Layout.fillWidth: true
//        }
        ToolButton {
            id: recv
            ButtonGroup.group: radio

            text: "Received"
            icon.source: mi("ic_cloud_download_black_24dp.png")
            display: AbstractButton.TextUnderIcon

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: mm(.25)

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
                    recv_badge = false
                    core.clear_unseen_rando()
                }
                show_recv = checked
                show_sent = false

            }
            Rectangle {
                id: badge2
                width: 16
                height: 16
                anchors.right: parent.right
                anchors.top: parent.top
                radius: width / 2
                color: "red"
                visible: /*core.has_unseen_rando ||*/ recv_badge
            }
        }

        }

    }

    property real items_margin: mm(2)

    Component {
        id: msg_delegate

        CircularImage {
            id: img
            property bool click_to_fetch
            click_to_fetch: model.uid !== the_man && !IS_ACTIVE && FETCH_STATE === "manual"
            x: items_margin
            //y: mm(10)
            width: listview.width - 2 * items_margin
            height: {
                if(click_to_fetch)
                    return width
                return (((show_sent && SENT === 0) || (show_recv && SENT === 1)) || IS_FILE === 0) ? 0 : width
            }
            visible: click_to_fetch || IS_ACTIVE || IS_QD || ((((show_sent && SENT === 1) || (show_recv && SENT === 0)) && IS_FILE === 1))
            asynchronous: true
            source: {
                click_to_fetch ? mi("ic_cloud_download_black_24dp.png") :
                (PREVIEW_FILENAME !== "" ? ("file:///" + String(PREVIEW_FILENAME)) : "")
            }

            fillMode: click_to_fetch ? Image.Pad : Image.PreserveAspectCrop
            Text {
                visible: click_to_fetch
                text: "(tap to retry fetch)"
                horizontalAlignment: Text.AlignHCenter
                anchors.top: img.top
                anchors.margins: mm(1)
                z: 10
            }

            sourceSize.width: 512
            sourceSize.height: 512
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if(click_to_fetch) {
                        console.log("click to refetch")
                        core.retry_auto_fetch(model.mid)
                    } else {
                        themsgview.mid = model.mid
                        themsgview.uid = model.ASSOC_UID
                        themsgview.view_source = source
                        stack.push(themsgview)
                    }
                }
            }
            Behavior on visible {
                //SequentialAnimation {
                    NumberAnimation {
                        target: img
                        property: "opacity"
                        duration: 250
                        easing.type: Easing.InOutQuad
                        from: 0.0
                        to: 1.0
                    }
                //}
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
                id: failed_review
                anchors.top: img.top
                anchors.left: img.left
                anchors.margins: mm(.5)
                visible: !IS_QD && REVIEW_RESULTS != "Unknown" && msglist.model.uid === the_man
                source: mi("ic_not_interested_black_24dp.png")

                z: 10
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if(fail_review_msg.state == "moveIn")
                            fail_review_msg.state = "moveOut"
                        else
                            fail_review_msg.state = "moveIn"
                        core.hash_clear_tag(ASSOC_HASH, "unviewed")
                    }
                }

                SequentialAnimation {
                    running: IS_UNSEEN === 1
                    loops: Animation.Infinite
                    onStopped: {
                        failed_review.scale = 1.0
                    }

                NumberAnimation {
                    target: failed_review
                    property: "scale"
                    duration: 300
                    easing.type: Easing.InOutQuad
                    from: 1.0
                    to: 0.5
                }
                NumberAnimation {
                    target: failed_review
                    property: "scale"
                    duration: 300
                    easing.type: Easing.InOutQuad
                    from: 0.5
                    to: 1.0
                }
                }


                Label {
                    id: fail_review_msg
                    visible: x > -width
                    x: -width
                    state: "moveIn"
                    text: "Sorry, pic did not pass review... try again."
                    color: amber_light
                    style: Text.Outline
                    styleColor: "black"

                    states: [
                        State {
                            name: "moveOut";
                            PropertyChanges { target: fail_review_msg; x: failed_review.width ; y: 0 }
                        },
                        State {
                            name: "moveIn";
                            PropertyChanges { target: fail_review_msg; x: -width; y: 0 }
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
                id: has_geo_info
                source: msglist.model.uid === the_man ? mi("ic_language_black_24dp.png") : mi("ic_language_white_24dp.png")
                anchors.top: img.top
                anchors.left: img.left
                anchors.margins: mm(.5)
                visible: location.text.length > 0
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if(core.geo_count_from_hash(ASSOC_HASH) > 1) {
                        geolist.hash = ASSOC_HASH
                        stack.push(geolist)
                        }
                        else {
                        if(SENT === 0) {
                            var o = JSON.parse(MSG_TEXT)
                            if('lat' in o && 'lon' in o)
                            {
                                mapimage.lat = parseFloat(o.lat)
                                mapimage.lon = parseFloat(o.lon)
                                mapimage.center = QtPositioning.coordinate(parseFloat(o.lat), parseFloat(o.lon))
                                mapimage.placename = location.text
                                mapimage.zoom = default_map_zoom
                                stack.push(mapimage)
                            }
                            else
                            {
                                if(location.state == "moveIn")
                                    location.state = "moveOut"
                                else
                                    location.state = "moveIn"
                            }
                        } else {
                            if(location.text.length > 0 && SENT_TO_LAT != "" &&
                                    SENT_TO_LON != "") {
                                mapimage.lat = parseFloat(SENT_TO_LAT)
                                mapimage.lon = parseFloat(SENT_TO_LON)
                                mapimage.center = QtPositioning.coordinate(parseFloat(SENT_TO_LAT), parseFloat(SENT_TO_LON))
                                mapimage.placename = location.text
                                mapimage.zoom = default_map_zoom
                                stack.push(mapimage)

                            }
                            else
                            {
                                if(location.state == "moveIn")
                                    location.state = "moveOut"
                                else
                                    location.state = "moveIn"
                            }
                        }
                        }

                        core.hash_clear_tag(ASSOC_HASH, "unviewed")
                    }
                }
                SequentialAnimation {
                    running: IS_UNSEEN === 1
                    loops: Animation.Infinite
                    onStopped: {
                        has_geo_info.scale = 1.0
                    }

                NumberAnimation {
                    target: has_geo_info
                    property: "scale"
                    duration: 300
                    easing.type: Easing.InOutQuad
                    from: 1.0
                    to: 0.5
                }
                NumberAnimation {
                    target: has_geo_info
                    property: "scale"
                    duration: 300
                    easing.type: Easing.InOutQuad
                    from: 0.5
                    to: 1.0
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
                            if('loc' in o)
                                return o.loc
                            return ""
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
                            if(pers_id === model.mid) {
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
                //width: img.width
                anchors.margins: mm(1)
                sourceComponent: ProgressRound {
                    id: pbar
                    //width: pbar_loader.width
                    visible: model.IS_ACTIVE
                    value: ATTACHMENT_PERCENT
                    indeterminate: {ATTACHMENT_PERCENT < 0.0}
                    to: 100.0
                    z: 4
//                    background: Rectangle {
//                        color: "green"
//                    }
                }
                visible: msglist.model.uid !== the_man && IS_ACTIVE
                active: msglist.model.uid !== the_man && IS_ACTIVE
            }
        }
    }

    ListView {
        id: listview
        anchors.fill: parent
        anchors.topMargin: items_margin
        clip: true
        delegate: msg_delegate
        model: themsglist
        spacing: items_margin
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
    Pane {
        visible: {model.uid === the_man && listview.count === 0}
        font.bold: true
        anchors.fill: parent
        background: parent.background

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(5)

        Item {
            Layout.fillHeight: true
        }

        Label {
            Layout.fillWidth: true
            text: "Click the Camera button below to take a picture and anonymously send it to a random person."
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
    }

    RoundButton {
        anchors.bottom: parent.bottom
        anchors.margins: mm(3)
        anchors.horizontalCenter: parent.horizontalCenter
        icon.source: mi("ic_add_a_photo_black_24dp.png")
        icon.color: "blue"
        width: cm(1.5)
        height: cm(1.5)
        onClicked: {
            cam.next_state = "StopAndPop"
            cam.ok_text = "Upload"
            stack.push(cam)
        }
    }

    TipButton {
        id: go_to_top
        width: mm(10)
        height: mm(10)
        anchors.margins: mm(3)
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        background: Rectangle {
            id: gtb_bg
            color: accent
            radius: 20
            opacity: .5
        }

        contentItem: Image {
            id: gtb_img
            anchors.centerIn: gtb_bg
            source: mi("ic_vertical_align_top_black_24dp.png")
            opacity: .5
        }

        visible: !listview.atYBeginning

        onClicked: {
            listview.positionViewAtBeginning()
            //lock_to_bottom = true
        }
        ToolTip.text: "Skip to top"

    }

    Warning {
        id: warn
        visible: false
        z: 3
        warning: "You denied access to storage, which is OK. BUT if you uninstall the app, the pictures stored by this app are also removed. IF YOU WOULD LIKE TO KEEP THE PICTURES YOU GET, EVEN IF YOU UNINSTALL, click the button below to quit the app. Then restart the app, and when it asks for permission to access storage, answer YES."
        inhibit_key: "storage_warning"
        oops_text: "Quit (give storage permission next time)"

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
