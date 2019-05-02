
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQml 2.2
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.3
import Qt.labs.platform 1.1 as NL
import dwyco 1.0


Page {
    id: chatbox_page
    //anchors.fill: parent
    property alias model: listView1.model
    property alias listview: listView1
    property string to_uid
    property string file_to_send
    property url url_to_send
    //property alias pick_pic: picture_picker.visible
    property bool android_hack: false
    property string android_img_filename
    property int ind_typing: 0
    property int ind_online: 0
    property int inh_block_warning: 0
    property bool multiselect_mode: false
    property url cur_source
    property var call_buttons_model
    property bool lock_to_bottom: false

    function star_fun(b) {
        console.log("chatbox star")
        model.fav_all_selected(b ? 1 : 0)
    }

    Component.onCompleted: {
        if(core.get_local_setting("inh_block_warning") === "")
            inh_block_warning = 0
        else
            inh_block_warning = 1
    }

    Component {
        id: extras_button

        ToolButton {
            contentItem: Image {
                anchors.centerIn: parent
                source: mi("ic_more_vert_white_24dp.png")
            }
            onClicked: optionsMenu.open()

            Menu {
                id: optionsMenu
                x: parent.width - width
                transformOrigin: Menu.TopRight
                MenuItem {
                    text: "Unfavorite"
                    onTriggered: {
                        star_fun(false)
                        multiselect_mode = false
                    }
                }
                MenuItem {
                    text: "Hide"
                    onTriggered: {
                        model.tag_all_selected("_hid")
                        multiselect_mode = false
                    }
                }
                MenuItem {
                    text: "UnHide"
                    onTriggered: {
                        model.untag_all_selected("_hid")
                        multiselect_mode = false
                    }
                }
                MenuItem {
                    text: "Select All"
                    onTriggered: {
                        model.set_all_selected()
                    }
                }
            }

        }
    }

    // note: this column thing is needed because otherwise
    // it doesn't appear to get the height of the toolbar
    // correct
    header: Column {
        width:parent.width
        MultiSelectToolbar {
            id:multi_toolbar
            visible: multiselect_mode
            extras: extras_button
            delete_warning_inf_text: "Does NOT delete FAVORITE messages"
            delete_warning_text: "Delete all selected messages?"
        }

        ToolBar {
            background: Rectangle {
                color: accent
            }
            visible: !multiselect_mode

            implicitWidth: parent.width

            RowLayout {

                anchors.fill: parent
                spacing: mm(2)
                visible: {profile_bootstrapped === 1 && server_account_created}
                Item {
                    Layout.minimumHeight: cm(1)
                }

                TipButton {
                    id: back_button
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_arrow_back_black_24dp.png")                      

                    }
                    checkable: false
                    onClicked: {
                        stack.pop()
                    }
                    Layout.fillHeight: true
                    ToolTip.text: "Go back"

                    //Layout.maximumWidth: mm(3)
                    //Layout.rightMargin: 0
                }

                Item {
                    id: prof
                    //color: "red" //accent
                    //border.width: 1
                    //radius: 3

                    Layout.fillHeight: true
                    Layout.minimumHeight: cm(1)
                    //Layout.leftMargin: 0

                    CircularImage {
                        id: top_toolbar_img
                        source: {
                            if(to_uid === "")
                                return
                            return (show_unreviewed || (server_account_created && core.uid_profile_regular(to_uid))) ?
                                    cur_source :  "qrc:/new/red32/icons/red-32x32/exclamation-32x32.png"
                        }
                        fillMode: Image.PreserveAspectCrop
                        width: parent.height
                        height: parent.height

                    }
                    Text {
                        id:top_toolbar_text
                        //width: dp(160)
                        clip: true
                        anchors.leftMargin: 2
                        fontSizeMode: Text.Fit
                        anchors.top: parent.top
                        //anchors.bottom: parent.bottom
                        anchors.left: top_toolbar_img.right
                    }
                    Label {
                        anchors.bottom: parent.bottom
                        anchors.left: top_toolbar_img.right
                        anchors.leftMargin: 2
                        text: ind_typing === 1 ? "typing..." : "online"
                        color: "white"
                        background: Rectangle {
                            color: "indigo"
                            radius: 3
                        }
                        visible: ind_online === 1
                    }
                    MouseArea {
                        //anchors.fill: parent
                        anchors.left: parent.left
                        anchors.right: top_toolbar_img.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
//                        color: "black"
//                        z:10
//                        visible: true
                        onClicked: {
                            stack.pop()
                        }
                    }

                }

                Item {
                    id: next_item
                    Layout.fillWidth: true
                }

                ConvToolButton {
                    visible: {stack.depth > 2 || core.unread_count > 0}
                }

                CallButtonLink {
                    id: vidcall_button
                    but_name: "send_video"
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_videocam_black_24dp.png")
                    }
                    ToolTip.text: "Request live video"
                }
                CallButtonLink {
                    id: cancel_req_button
                    but_name: "cancel_req"
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_cancel_black_24dp.png")
                    }
                    background: Rectangle {
                        id: bgblink4
                        ParallelAnimation {
                            loops: Animation.Infinite
                            running: cancel_req_button.visible
                            ColorAnimation {
                                target: bgblink4
                                property: "color"
                                from: "red"
                                to: "white"
                                duration: 1000
                            }
                        }
                    }
                    ToolTip.text: "Hangup"

                }
                CallButtonLink {
                    but_name: "hangup"
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_cancel_black_24dp.png")
                    }
                    onVisibleChanged: {
                        if(visible) {
                            vidpanel.visible = true
                            core.enable_video_capture_preview(1)
                        } else {
                            vidpanel.visible = false
                            core.enable_video_capture_preview(0)
                        }
                    }
                    ToolTip.text: "Hangup"

                }
                CallButtonLink {
                    id: call_accept_button
                    but_name: "accept"
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_videocam_black_24dp.png")
                    }
                    background: Rectangle {
                        id: bgblink
                        ParallelAnimation {
                            loops: Animation.Infinite
                            running: call_accept_button.visible
                            ColorAnimation {
                                target: bgblink
                                property: "color"
                                from: "white"
                                to: "green"
                                duration: 1000
                            }
                        }
                    }
                    ToolTip.text: "Accept live video"
                }
                CallButtonLink {
                    id: call_send_accept_button
                    but_name: "accept_and_send"
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_videocam_black_24dp.png")
                    }
                    background: Rectangle {
                        id: bgblink2
                        ParallelAnimation {
                            loops: Animation.Infinite
                            running: call_send_accept_button.visible
                            ColorAnimation {
                                target: bgblink2
                                property: "color"
                                from: "white"
                                to: "green"
                                duration: 1000
                            }
                        }
                    }
                    ToolTip.text: "Start two-way video"
                }
                CallButtonLink {
                    id: call_reject_button
                    but_name: "reject"
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_cancel_black_24dp.png")
                    }
                    background: Rectangle {
                        id: bgblink3
                        ParallelAnimation {
                            loops: Animation.Infinite
                            running: call_reject_button.visible
                            ColorAnimation {
                                target: bgblink3
                                property: "color"
                                from: "red"
                                to: "white"
                                duration: 1000
                            }
                        }
                    }
                    ToolTip.text: "Decline live video"

                }

                CallButtonLink {
                    id: mute_button
                    but_name: "mute_button"
                    contentItem: Image {
                        id: mic_icon
                        anchors.centerIn: parent
                        source: mi("ic_mic_black_24dp.png")
                    }
                    onCheckedChanged: {
                        mic_icon.source = checked ? mi("ic_mic_black_24dp.png") : mi("ic_mic_off_black_24dp.png")
                    }

//                    background: Rectangle {
//                        id: bgblink3
//                        ParallelAnimation {
//                            loops: Animation.Infinite
//                            running: call_reject_button.visible
//                            ColorAnimation {
//                                target: bgblink3
//                                property: "color"
//                                from: "red"
//                                to: "white"
//                                duration: 1000
//                            }
//                        }
//                    }
                    ToolTip.text: "Toggle mic"

                }


                TipButton {
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_action_overflow.png")
                    }
                    onClicked: optionsMenu.open()
                    visible: chatbox.visible
                    ToolTip.text: "More actions"

                    Menu {

                        id: optionsMenu
                        x: parent.width - width
                        transformOrigin: Menu.TopRight

                        MenuItem {
                            text: "View profile"
                            onTriggered: {
                                stack.push(theprofileview)
                            }
                        }
                        MenuItem {
                            text: "Send picture"
                            onTriggered: {
                                if(Qt.platform.os == "android") {
                                    // ugh, what a hack
                                    android_img_pick_hack = 0
                                    android_img_pick_hack = 2
                                    notificationClient.open_image()
                                } else {
                                    picture_picker.visible = true
                                }

                            }
                        }
                        MenuItem {
                            text: "Send video message"
                            onTriggered: {
                                core.try_connect(to_uid)
                                dwyco_vid_rec.uid = to_uid
                                stack.push(dwyco_vid_rec)
                            }
                        }

                        MenuItem {
                            text: "Browse Msgs"
                            onTriggered: {
                                stack.push(simp_msg_browse)
                            }
                        }

                        MenuItem {
                            text: "Clear msgs"
                            onTriggered: {
                                core.clear_messages_unfav(chatbox.to_uid)

                                themsglist.reload_model()
                            }
                        }

                        MenuItem {
                            text: "Delete user"
                            onTriggered: {
                                confirm_delete.visible = true
                            }
                            MessageDialog {
                                id: confirm_delete
                                title: "Bulk delete?"
                                icon: StandardIcon.Question
                                text: "Delete ALL messages from user?"
                                informativeText: "This removes FAVORITE messages too."
                                standardButtons: StandardButton.Yes | StandardButton.No
                                onYes: {
                                    core.delete_user(chatbox.to_uid)
                                    themsglist.reload_model()
                                    close()
                                    stack.pop()
                                }
                                onNo: {
                                    close()
                                }
                            }
                        }
                        MenuItem {
                            text: "More..."
                            onTriggered: {
                                moremenu.open()

                            }
                        }

                    }
                }
            }
        }

        }

    background: Rectangle {
        color: primary_dark
        gradient: Gradient {
            GradientStop { position: 0.0; color: primary_light }
            GradientStop { position: 1.0; color: primary_dark}
        }
    }
    
    function snapshot(filename) {
        console.log("CAMERA SNAP2", filename)
        core.send_simple_cam_pic(to_uid, "", filename)
        themsglist.reload_model()
        listview.positionViewAtBeginning()
        
    }

    Connections {
        target: core
        onSc_rem_keyboard_active : {
            if(uid === to_uid) {
                ind_typing = active
            }
        }
        onNew_msg : {
            // if we're visible, reset the unviewed msgs thing since presumably
            // we can see it. might want to set it if the view is scrolled up
            // and we can't actually see it until we scroll down, but for
            // another day...
            if(visible && from_uid === to_uid) {
                core.reset_unviewed_msgs(to_uid)
            }
        }
        onSys_uid_resolved: {
            if(chatbox.to_uid === uid) {
                // try to defeat caching since the actual name
                // of the "preview url" hasn't changed, but the contents have
                cur_source = ""
                cur_source = core.uid_to_profile_preview(uid)
                top_toolbar_text.text = core.uid_to_name(uid)
            }
        }
        onSc_connect_terminated: {
            if(chatbox.to_uid === uid) {
                console.log("CONNECT TERMINATED")
            }
        }

        onSc_connectedChanged: {
                if(chatbox.to_uid === uid) {
                    console.log("ConnectedChanged ", connected)
                    if(connected === 0 && vidpanel.visible) {
                        vidpanel.visible = false
                        core.enable_video_capture_preview(0)
                    }
                    ind_online = connected === 1 ? true : false
                }
            }
//        onIgnore_event: {
//            if(uid === to_uid) {
//               to_uid = ""
//            }
//        }

    }

    onAndroid_hackChanged: {
        if(android_hack) {
            console.log(android_img_filename)

            chat_pic_preview.source = "file://" + android_img_filename
            chat_pic_preview.ok_vis = true
            stack.push(chat_pic_preview)
            android_hack = false
        }
    }

    onTo_uidChanged: {
        if(to_uid === "")
            return
        textField1.text = ""
        core.reset_unviewed_msgs(to_uid)
        cur_source = core.uid_to_profile_preview(to_uid)
        top_toolbar_text.text = core.uid_to_name(to_uid)
        ind_typing = core.get_rem_keyboard_state(to_uid)
        ind_online = core.get_established_state(to_uid)
        call_buttons_model = core.get_button_model(to_uid)
    }

    Loader {
        id: picture_picker
        active: visible
        visible: false


        sourceComponent: NL.FileDialog {

            title: "Pick a picture"
            folder: shortcuts.pictures
            onAccepted: {
                console.log("PICK ", fileUrl)
                console.log("PICKU ", Qt.resolvedUrl(fileUrl))
                //img_preview.source = fileUrl
                chat_pic_preview.source = fileUrl
                chat_pic_preview.ok_vis = true
                stack.push(chat_pic_preview)
            }
            onRejected: {picture_picker.visible = false}
            Component.onCompleted: {visible = true}
        }
    }

    PicPreview {
        id: chat_pic_preview
        onClosed: {
            if(ok) {
                url_to_send = source
                core.simple_send_url(to_uid, "", url_to_send)
                themsglist.reload_model()
                listView1.positionViewAtBeginning()
            }
            file_to_send = ""
            url_to_send = ""
            stack.pop()
        }
    }

    VidView {
        id: vid_call_view
        visible: false
    }

    RowLayout {
        anchors.bottom: textField1.top
        anchors.bottomMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0
        Layout.margins: mm(1)
        VidCall {
            id: vidpanel
            visible: false
            Layout.fillHeight: true
            Layout.minimumWidth: parent.width / 2
        }

        ListView {
            id: listView1
            Layout.fillHeight: true
            Layout.fillWidth: true
            delegate: msglist_delegate
            clip: true
            spacing: 5
            ScrollBar.vertical: ScrollBar {
                onPressedChanged: {
                    lock_to_bottom = false
                }
            }
            verticalLayoutDirection: ListView.BottomToTop
            onMovementStarted: {
                if(atYEnd)
                    lock_to_bottom = false
                //console.log("move start aty ", atYEnd, "lb ", lock_to_bottom)
            }

            onAtYEndChanged: {
                //console.log("at y end ", atYEnd)
                if(lock_to_bottom && !atYEnd)
                {
                    listView1.positionViewAtBeginning()
                }
                else if(atYEnd && !lock_to_bottom)
                    lock_to_bottom = true

            }
            onAtYBeginningChanged: {
                //console.log("at y beg ", atYBeginning)
            }
        }
    }


    Component {
        id: msglist_delegate

        Rectangle {
            id: ditem

            radius: 6
            height: clayout.height + 20
            width: clayout.width + 20

            border.width: 1
            border.color: divider
            color: {(IS_QD == 1) ? "gray" : ((SENT == 0) ? accent : primary_light)}

            anchors.left: {(SENT == 0) ? parent.left : undefined}
            anchors.right: {(SENT == 1) ? parent.right : undefined}
            anchors.margins: 3
            opacity: {multiselect_mode && SELECTED ? 0.5 : 1.0}
            onHeightChanged: {
                //console.log("del ", model.index, "ch to ", ditem.height)
//                if(lock_to_bottom) {
//                    listView1.positionViewAtBeginning()
//                }
            }

            Image {
                id: deco2
                visible: IS_QD
                source: decoration
                anchors.left: ditem.left
                anchors.top: ditem.top
                width: 16
                height: 16
            }
            Image {
                id: ggtiny
                width: dp(16)
                height: dp(16)
                source: "qrc:/new/prefix1/icons/ggtiny.png"
                anchors.top: ditem.top
                anchors.left: ditem.left
                visible: {multiselect_mode && SELECTED}
                opacity: 1.0
                z: 4
            }
            Rectangle {
                id: isfav
                width: 16
                height: 16
                anchors.top: ditem.top
                anchors.left: ditem.left
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
            Rectangle {
                id: is_forwarded
                width: 16
                height: 16
                anchors.top: ditem.top
                anchors.left: isfav.right
                visible: IS_FORWARDED === 1
                z: 3
                color: primary_light
                radius: width / 2
                Image {
                    anchors.fill: parent
                    anchors.margins: 2
                    source: mi("ic_open_in_new_black_24dp.png")
                }
            }
            Rectangle {
                id: multimedia
                width: 16
                height: 16
                anchors.top: ditem.top
                anchors.left: is_forwarded.right
                visible: {!IS_QD && (HAS_VIDEO && !HAS_SHORT_VIDEO)}
                z: 3
                color: primary_light
                radius: width / 2
                Image {
                    anchors.fill: parent
                    anchors.margins: 2
                    source: mi("ic_videocam_black_24dp.png")
                }
            }
            Rectangle {
                id: hidden
                width: 16
                height: 16
                anchors.right:ditem.right
                anchors.top:ditem.top
                visible: IS_HIDDEN === 1
                z: 3
                color: "orange"
            }
            z: 1

        ColumnLayout {
                id: clayout
                z: 2
                Layout.margins: 1
                anchors.centerIn: ditem
                Image {
                    id: preview

                    visible: {PREVIEW_FILENAME != "" || HAS_AUDIO}
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.maximumWidth: (listView1.width * 3) / 4
                    //Layout.minimumWidth: (listView1.width * 3) / 4
                    Layout.maximumHeight: listView1.height / 2
                    Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter

                    fillMode: Image.PreserveAspectFit
                    // note: the extra "/" in file:// is to accomodate
                    // windows which may return "c:/mumble"
                    //source: { PREVIEW_FILENAME == "" ? "" : ("file:///" + String(PREVIEW_FILENAME)) }
                    source: {PREVIEW_FILENAME != "" ? ("file:///" + String(PREVIEW_FILENAME)) :
                                                      (HAS_AUDIO === 1 ? mi("ic_audiotrack_black_24dp.png") : "")}

                    asynchronous: true
                    sourceSize.height: 256
                    sourceSize.width: 256
                    onStatusChanged: {
                        if (preview.status == Image.Ready) {
                            //preview.source = "file:///" + String(PREVIEW_FILENAME)
                            console.log(PREVIEW_FILENAME)
                        }
                    }


//                    Image {
//                        id: deco
//                        visible: {!IS_QD && (HAS_VIDEO && !HAS_SHORT_VIDEO)}
//                        source: decoration
//                        anchors.left: parent.left
//                        anchors.top: parent.top
//                        width: 32
//                        height: 32
//                    }
                }




                Text {
                    function gentext(msg, tm) {
                        var dt = new Date(tm * 1000)
                        //                        return "<table><tr><td>" + msg + "</td>" +
                        //                                "</tr><tr><td align=\"right\"><small>" + Qt.formatTime(dt) +
                        //                                "</td></tr></table>"
                        if(Date.now() - dt.getTime() > 86400 * 1000) {
                            return "<html>" + msg + "<sub>" + Qt.formatDate(dt) + "</sub></html>"
                        } else {
                            return "<html>" + msg + "<sub>" + LOCAL_TIME_CREATED + "</sub></html>"
                        }

                    }

                    id: msg
                    text: FETCH_STATE === "manual" ? "(click to fetch)" : gentext(String(MSG_TEXT), DATE_CREATED)
                    Layout.maximumWidth: (listView1.width * 3) / 4
                    horizontalAlignment: { (SENT == 1) ? Text.AlignRight : Text.AlignLeft}
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                    color: primary_text
                    onLinkActivated: {
                        console.log(link + " link activated")
                        Qt.openUrlExternally(link)
                    }
                    //font.family: "Noto Color Emoji"
                }

            }
        Loader {
            anchors.centerIn: ditem
            anchors.fill: ditem
            anchors.margins: mm(1)
            sourceComponent: ProgressBar {
                id: pbar
                anchors.fill: parent
                visible: IS_ACTIVE
                value: ATTACHMENT_PERCENT
                indeterminate: {ATTACHMENT_PERCENT < 0.0}
                to: 100.0
                z: 4
                background: Rectangle {
                    color: "white"
                }
            }
            visible: IS_ACTIVE
            active: IS_ACTIVE
        }
//        Loader {
//            id: pulse
//            anchors.centerIn: ditem
//            anchors.fill: ditem
//            anchors.margins: mm(1)
//            visible: {IS_ACTIVE && ATTACHMENT_PERCENT === 0.0}
//            source: "qrc:/PulseLoader.qml"
//            active: IS_ACTIVE
//        }


        MouseArea {
                anchors.fill: parent
                // this is only needed as a workaround
                // for the qt labs controls menu popup...
                // apparently, if you click outside the menu
                // to dismiss it, the mouse click isn't
                // absorbed, instead it is sent on down, to other
                // controls... which is different from the regular
                // menus, which don't have this behavior.
                enabled: !(optionsMenu.visible || moremenu.visible)
                onPressAndHold: {
                    console.log("click msg")
                    console.log(index)
                    //msg_action_popup.popup()
                    //msg_action_popup.mid = model.mid

                    listView1.currentIndex = index
                    multiselect_mode = true
                    listView1.model.toggle_selected(model.mid)
                    if(Qt.platform.os == "android") {
                    notificationClient.vibrate(50)
                    }
                }
                onClicked: {
                    listView1.currentIndex = index
                    if(multiselect_mode) {
                        listView1.model.toggle_selected(model.mid)
                        if(!listView1.model.at_least_one_selected())
                            multiselect_mode = false
                    } else {

                        if(model.FETCH_STATE === "manual") {
                            core.retry_auto_fetch(model.mid)
                            console.log("retry fetch")

                        } else {
                            console.log("show msg")
                            themsgview.msg_text = model.MSG_TEXT
                            themsgview.view_id = -1
                            themsgview.mid = model.mid
                            themsgview.uid = to_uid
                            themsgview.text_bg_color = ditem.color
                            if(model.IS_FILE === 1) {
                                themsgview.view_source = model.PREVIEW_FILENAME === "" ? "" : ("file:///" + String(model.PREVIEW_FILENAME))
                                stack.push(themsgview)
                            }
                            else {
                                if(model.HAS_VIDEO === 1 || model.HAS_AUDIO === 1) {
                                    var vid = core.make_zap_view(to_uid, model.mid)
                                    themsgview.view_id = vid

                                    if(model.HAS_AUDIO === 1 && model.HAS_VIDEO === 0) {
                                        themsgview.view_source = mi("ic_audiotrack_black_24dp.png")
                                    } else {
                                        themsgview.view_source = ""
                                    }
                                }  else {
                                    themsgview.view_source = ""
                                }
                                stack.push(themsgview)
                            }
                        }
                    }


                }

            }

        }
    }

    onMultiselect_modeChanged: {
        model.set_all_unselected()
    }
    onVisibleChanged: {
        multiselect_mode = false
        if(inh_block_warning === 0 && core.get_ignore(to_uid) !== 0) {
            warn.visible = true
        } else {
            warn.visible = false
        }
    }

    TextField {
        id: textField1

        anchors.right: toolButton1.left
        anchors.rightMargin: 1
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.bottom: parent.bottom
        //anchors.bottomMargin: 0
        placeholderText: qsTr("Type...")
        wrapMode: TextInput.WordWrap
        height: implicitHeight // Math.max(implicitHeight * 2, contentHeight)
        
        background: Rectangle {
            radius: 10
            anchors.fill: parent
            border.color: "#333"
            border.width: 1
            color: icons
        }

        onLengthChanged: {
            core.uid_keyboard_input(to_uid)
        }
        onInputMethodComposingChanged: {
            core.uid_keyboard_input(to_uid)
        }

        onAccepted: {
            if(textField1.length > 0) {
                core.simple_send(to_uid, core.strip_html(textField1.text))
                core.try_connect(to_uid)

                themsglist.reload_model()
                textField1.text = ""
                listView1.positionViewAtBeginning()
            }

        }
        // this is here because on iOS, when you pop this
        // item, the focus stays in here somehow and the keyboard
        // will pop up on the previous screen, wtf.
        // qt 5.11 seems focus is handled a little differently
        //focus: visible
        onVisibleChanged: {
            if(Qt.platform.os == "android") {
            if(!visible)
                focus = false
            } else {
                focus = visible
            }
        }

        TipButton {
            id: cam_button
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: mm(3)
            contentItem: Image {
                anchors.centerIn: parent
                source: mi("ic_add_a_photo_black_24dp.png")
            }
            checkable: false
            visible: !cam.visible && !(textField1.inputMethodComposing || textField1.length > 0 || textField1.text.length > 0)
            onClicked: {
               stack.push(cam, {"next_state" : "PhotoCapture"})
            }
            ToolTip.text: "Take pic from camera"
        }
    }

    Button {
        property int but_width
        property int but_height
        id: toolButton1
        height: but_height
        width: but_width
        
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 1
        anchors.bottomMargin: 1
        //anchors.verticalCenter: textField1.verticalCenter
        
        enabled: {textField1.inputMethodComposing || textField1.length > 0 || textField1.text.length > 0}
        
        background: Rectangle {
            id: bg
            color: accent
            radius: 20
            // this is weird, setting size is supposed to be unnecessary...
            // qt5.10 didn't have a problem with the previous code that was here.
            anchors.fill: toolButton1
        }
        contentItem: Image {
            anchors.centerIn: bg
            source: mi("ic_send_black_24dp.png")
            opacity: toolButton1.enabled ? 1.0 : 0.3
        }
        

        text: "send"
        onClicked: {
            Qt.inputMethod.commit()
            Qt.inputMethod.reset()
            core.simple_send(to_uid, textField1.text)
            core.try_connect(to_uid)
            themsglist.reload_model()
            textField1.text = ""
            listView1.positionViewAtBeginning()
        }
        Component.onCompleted: {
            but_height = textField1.height
            but_width = but_height
        }
        focusPolicy: Qt.NoFocus
    }

    TipButton {
        id: go_to_bottom
        width: toolButton1.width
        height: toolButton1.height
        anchors.bottom: toolButton1.top
        anchors.right: toolButton1.right

        background: Rectangle {
            id: gtb_bg
            color: accent
            radius: 20
            opacity: .5
        }

        contentItem: Image {
            id: gtb_img
            anchors.centerIn: gtb_bg
            source: mi("ic_system_update_alt_black_24dp.png")
            opacity: .5
        }

        visible: !listView1.atYEnd

        onClicked: {
            listView1.positionViewAtBeginning()
            lock_to_bottom = true
        }
        ToolTip.text: "Skip to bottom"

    }

    TipButton {
        property int zid: -1
        property int chan: -1

        id: audio_zap_button
        height: toolButton1.height
        width: toolButton1.width

        anchors.fill: toolButton1
        //anchors.verticalCenter: textField1.verticalCenter

        enabled: !toolButton1.enabled && core.has_audio_input
        visible: !toolButton1.enabled && core.has_audio_input
        z: 5
        background: Rectangle {
            id: bg2
            color: audio_zap_button.zid === -1 ? accent : "lime"
            radius: 20
        }
        contentItem: Image {
            id: quick_audio
            anchors.centerIn: bg2
            source: mi("ic_mic_black_24dp.png")
        }
        ToolTip.text: "Press while talking to send audio msg"

        onPressed: {
            core.try_connect(to_uid)
            zid = core.make_zap_composition()
            chan = core.start_zap_record(zid, 0, 1)
        }
        onReleased: {
            core.stop_zap_record(zid)

        }
        Connections {
            target: core
            onZap_stopped: {
                if(zid === audio_zap_button.zid) {
                    core.send_zap(zid, to_uid, 1)
                    audio_zap_button.zid = -1
                    themsglist.reload_model()
                }
            }
        }

        focusPolicy: Qt.NoFocus
    }
    ProgressBar {
        id: progbar
        anchors.fill: textField1
        visible: audio_zap_button.pressed
        background: Rectangle {
                  color: "black"
                  radius: 10
              }
        from: 0
        to: 30
        onVisibleChanged: {
            if(visible) {
                value = 0
                sound_alert.play()
            }
        }
        z: 5


        Timer {
            running: progbar.visible
            repeat: true
            interval: 100
            triggeredOnStart: true
            onTriggered: {
                progbar.value = progbar.value + (interval / 1000.0)
            }
        }
    }

    BusyIndicator {
        id: busy1

        visible: {!listView1.visible}
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        z: 20

    }

    Warning {
        id: warn
        visible: false
        z: 3
        warning: "You have BLOCKED this user. They will not receive any messages you send them. You will not receive any messages from them. You are also invisible to each other. To UNBLOCK them, visit the Block List settings item."
        inhibit_key: "inh_block_warning"
        //oops_text: "Oops, no I don't want that"

        onVisibleChanged: {
            if(visible) {
                oops = false
            } else {
                if(core.get_local_setting("inh_block_warning") === "")
                    inh_block_warning = 0
                else
                    inh_block_warning = 1
            }
        }

    }
}
