
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.9
import QtQml 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import dwyco 1.0

ApplicationWindow {
    property real contentScaleFactor: screenDpi / 160
    function dp(value) {
        return value
        //return value * contentScaleFactor
    }
    function dpi_to_str() {
        if(screenDpi <= 240)
            return "mdpi"
        else if(screenDpi <= 320)
            return "hdpi"
        else
            return "hdpi"
    }

    function mi(icon) {
        var str
        str = "qrc:/material/icons/material/"
        str += dpi_to_str()
        str += "/"
        str += icon
        return str
    }

    function cm(cm){
        if(Screen.pixelDensity)
            return cm*Screen.pixelDensity*10
        console.warn("Could not calculate 'cm' based on Screen.pixelDensity.")
        return 0
    }
    function mm(mm){
        if(Screen.pixelDensity)
            return mm*Screen.pixelDensity
        console.warn("Could not calculate 'mm' based on Screen.pixelDensity.")
        return 0
    }
    function inch(i){
        if(Screen.pixelDensity)
            return i*Screen.pixelDensity*10*2.54
        console.warn("Could not calculate 'inch' based on Screen.pixelDensity.")
        return 0
    }
    function vw(i){
        if(Screen.width)
            return i*(Screen.width/100)
        console.warn("Could not calculate 'vw' based on Screen.width.")
        return 0
    }
    function vh(i){
        if(Screen.height)
            return i*(Screen.height/100)
        console.warn("Could not calculate 'vh' based on Screen.height.")
        return 0
    }
    
    
    property color primary : "#673AB7"
    property color primary_dark : "#512DA8"
    property color primary_light : "#D1C4E9"
    property color accent : "#009688"
    property color primary_text : "#212121"
    property color secondary_text : "#727272"
    property color icons : "#FFFFFF"
    property color divider : "#B6B6B6"

    property color amber_light : "#FFECB3"
    property color amber_dark : "#FF6F00"
    property color  amber_accent: "#FFAB00"
    property int pct: 20

    property bool sent_badge: false
    property bool recv_badge: false

    property string the_man: "5a098f3df49015331d74"
    property string redist: "13404a7fc7664a943a20"

    function picht() {
        return vh(pct) - mm(.5)
    }

    Material.theme: Material.Light
    Material.accent: "white"
    Material.primary: primary
    //Material.foreground: "white"

    property int profile_bootstrapped : 0
    property bool server_account_created: false
    property int android_img_pick_hack: 0

    property bool dwy_invis: false
    property bool dwy_quiet: false
    property bool dwy_freebies: true
    property bool show_unreviewed: false
    property bool expire_immediate: false
    property bool show_hidden: true
    property bool show_archived_users: false

    function datesec() {
        return Math.round(Date.now() / 1000)
    }

    id: applicationWindow1
    visible: true
    // android will override this and go full screen, which is
    // what we want. for desktop versions, not sure what might be
    // best.
    width: 800
    height: 600
    // this might be helpful for windows users, as they
    // seem to get confused with multiple windows running around
    // at the same time.
    //width: Screen.width
    //height: Screen.height
    title: qsTr("Dwyco Rando")

    property int close_bounce: 0
    onClosing: {
        // special cases, don't let them navigate around the
        // initial app setup
        if(profile_bootstrapped === 0) {
            close.accepted = false
            return
        }

        if(!server_account_created && blank_page.visible) {
            close.accepted = false
            return
        }

        if(stack.depth > 1) {
            close.accepted = false
            stack.pop()
        }
        else {
            if(bounce_opacity.running)
            {
                if(Qt.platform.os == "android") {
                    notificationClient.start_background()
                    notificationClient.set_lastrun()
                }
                //if(pwdialog.allow_access === 0)
                //    expire_immediate = true
                return
            }
            close.accepted = false
            bounce_opacity.start()
        }
    }

    Component.onCompleted: {
        AndroidPerms.request_sync("android.permission.CAMERA")
    }


    Label {
        id: close_bounce_msg
        text: "Back once more to exit..."
        color: "white"
        anchors.centerIn: parent
        z: 5
        background: Rectangle {
            radius: 3
            color: "black"
        }

        opacity: 0.0
        NumberAnimation {
               id: bounce_opacity
               target: close_bounce_msg
               properties: "opacity"
               from: 1.0
               to: 0.0
               duration: 3000
          }
    }

    
    Drawer {
        id: drawer
        interactive: {stack.depth === 1 && profile_bootstrapped === 1 && server_account_created}
        width: Math.min(applicationWindow1.width, applicationWindow1.height) / 3 * 2
        height: applicationWindow1.height

        AppDrawer {
            id: drawer_contents
            padding: 0
            //width: Math.min(applicationWindow1.width, applicationWindow1.height) / 3 * 2
            //height: applicationWindow1.height
            onClose: {
                drawer.close()
            }

            onVisibleChanged: {
                if(visible) {
                    //drawer_contents.circularImage.source = core.uid_to_profile_preview(core.get_my_uid())
                    drawer_contents.text1.text = core.uid_to_name(core.get_my_uid())
                    drawer_contents.tech_uid.text = "(#" + core.get_my_uid().substr(0, 8) + ")"
                    rando_status.refresh()
                }

            }
        }
    }


//    footer: RowLayout {
//            Item {
//                Layout.fillWidth: true
//            }

//            Label {
//                id: hwtext
//            }
//            Label {
//                id: db_status
//                text: core.is_database_online === 0 ? "db off" : "db on"
//            }
//        }

    Item {
        id: top_dispatch
        signal uid_selected(string uid, string action)
        signal profile_updated(int success)
        signal video_display(int ui_id, int frame_number, string img_path)
        signal camera_snapshot(string filename)
        signal uid_resolved(string uid)
        //signal image_picked(string fn)

        property string last_uid_selected: ""
        visible: false
        enabled: false

        onUid_selected: {
            console.log("UID SELECTED", uid)
            last_uid_selected = uid
        }
    }

    Loader {
        id: cam

        property string next_state
        property string ok_text: "Send"
        //anchors.fill: parent
        visible: false
        active: visible

        onLoaded: {
            item.state_on_close = cam.next_state
            item.ok_pv_text = cam.ok_text
        }

        onVisibleChanged: {
            if(visible) {
                source = "qrc:/DeclarativeCamera.qml"
                //vid_cam_preview.active = false
            }
        }

    }

    DSettings {
        id: settings_dialog
        visible: false

    }

    Loader {
        id: about_dialog
        visible: false
        active: visible
        onVisibleChanged: {
            if(visible) {
                source = "qrc:/About.qml"
            }
        }
    }

    Loader {
        id: help_dialog
        visible: false
        active: visible
        onVisibleChanged: {
            if(visible) {
                source = "qrc:/Help.qml"
            }
        }
    }

    SimpleMsgList {
        id: simple_msg_list
        visible: false
    }

    DwycoMsgList {
        id: themsglist
        uid: top_dispatch.last_uid_selected
    }

    MsgView {
        id: themsgview
        //anchors.fill: parent
        visible: false
    }

    Loader {
        id: rando_status
        property int num_sent: 0
        property int num_recv: 0
        property int next_freebie: 0

        function refresh() {
            active = false
            source = ""
            source = core.get_msg_count_url(dwy_freebies ? 1 : 0)
            active = true
        }

        visible: false
        active: false

        onLoaded: {
            num_sent = item.num_sent
            num_recv = item.num_recv
            next_freebie = item.next_freebie
        }
    }

//    SimpleMsgBrowse {
//        id: simp_msg_browse
//        model: themsglist
//        to_uid: top_dispatch.last_uid_selected
//        visible: false

//    }

//    SimpleTagMsgBrowse {
//        id: simp_tag_browse
//        model: themsglist
//        visible: false
//    }

    ProfileDialog {
        id: profile_dialog
        visible: false
    }

    MapImage {
        id: mapimage
        //lat: 39.739200
        //lon: -104.984700
        visible: false
    }


    StackView {
        id: stack
        //initialItem: userlist
        anchors.fill: parent
        visible: true
        onDepthChanged: {
//            if(depth === 1)
//                simp_tag_browse.to_tag = ""
        }

        
    }

    FName {
        id: fname
    }

    DwycoCore {
        id: core
        property int is_database_online: -1
        property int is_chat_online: -1

        objectName: "dwyco_singleton"
        client_name: {"QML-" + Qt.platform.os + "-" + core.buildtime}
        Component.onCompleted: {
            var a
            a = get_local_setting("first-run")
            if(a === "") {
                //stack.push(simple_msg_list)
                //stack.push(blank_page)
                stack.push(profile_dialog)
            } else {
                init()
                stack.push(simple_msg_list)
                profile_bootstrapped = 1
            }
            a = get_local_setting("acct-created")
            if(a === "") {
                server_account_created = false
            } else {
                server_account_created = true
            }
            if(profile_bootstrapped === 1 && !server_account_created) {
                stack.push(blank_page)
            }

            if(Qt.platform.os == "android") {
                notificationClient.cancel()
            }

            a = get_local_setting("quiet")
            if(a === "" || a === "false") {
                dwy_quiet = false
                if(Qt.platform.os == "android") {
                notificationClient.set_quiet(0)
                }
            } else {
                dwy_quiet = true
                if(Qt.platform.os == "android") {
                notificationClient.set_quiet(1)
                }
            }


            a = get_local_setting("invis")
            if(a === "" || a === "false") {
                dwy_invis = false
            } else {
                dwy_invis = true
            }

            a = get_local_setting("show_unreviewed")
            if(a === "" || a === "0") {
                show_unreviewed = false
            } else {
                show_unreviewed = true
            }

            a = get_local_setting("send_freebies")
            if(a === "" || a === "true") {
                dwy_freebies = true
            } else {
                dwy_freebies = false
            }
        }


        Component.onDestruction: {
            console.log("die")
            if(Qt.platform.os == "android") {
                notificationClient.cancel()
                notificationClient.set_lastrun()
            }
            exit()
        }

        onServer_login: {
           
            console.log(msg)
            console.log(what)
            if(what === 1) {
                set_local_setting("acct-created", "true")
                server_account_created = true
                //rando_status.source = core.get_msg_count_url(dwy_freebies ? 1 : 0);
                rando_status.refresh()
            }
            if(Qt.platform.os == "android") {
                notificationClient.set_msg_count_url(core.get_msg_count_url())
                notificationClient.log_event()
                notificationClient.set_lastrun()
            }
        }

        onNew_msg: {
            console.log(from_uid)
            console.log(txt)
            console.log(mid)
            console.log("msglist", themsglist.uid)
            if(from_uid === themsglist.uid) {
                themsglist.reload_model();
                console.log("RELOAD nm")
            }
            if(from_uid === the_man) {
                if(from_uid !== themsglist.uid)
                    sent_badge = true
            } else {
                if(from_uid !== themsglist.uid)
                    recv_badge = true
            }

        }

        onSys_msg_idx_updated: {
            console.log("update idx", uid)
            console.log("upd" + uid + " " + themsglist.uid)
            if(uid === themsglist.uid) {
                themsglist.reload_model()

                console.log("RELOAD msg_idx")
            }
            if(prepend === 1) {
                if(uid === the_man) {

                } else {

                }
            }
        }

        onMsg_send_status: {
            console.log(pers_id, status, recipient)
            //hwtext.text = status
            if(status == DwycoCore.MSG_SEND_SUCCESS) {
                //sound_sent.play()
                if(themsglist.uid == recipient) {
                    themsglist.reload_model()

                }

            }
        }

        onMsg_progress: {
            console.log(pers_id, msg, percent_done)
            //hwtext.text = msg + " " + String(percent_done) + "%"
        }

        onProfile_update: {
            top_dispatch.profile_updated(success)
        }

        onQt_app_state_change: {
            console.log("app state change ", app_state)
            if(app_state === 0) {
                // resuming
                themsglist.reload_model()
                //pwdialog.state = "resume"
            } else {
                drawer.close()
                //pwdialog.state = "pause"
            }
            if(Qt.platform.os == "android") {
                notificationClient.set_lastrun()
            }
                

        }

//        onImage_picked: {
//            console.log("image " + fn)
//            if(android_img_pick_hack === 1)
//            {
//                profile_update_dialog.android_img_filename = fn
//                profile_update_dialog.android_hack = true
//            }
//            else if(android_img_pick_hack === 2)
//            {
//                chatbox.android_img_filename = fn
//                chatbox.android_hack = true

//            }
//        }

        onUnread_countChanged: {
            set_badge_number(unread_count)
        }

    }

    Rectangle {
        id: blank_page
        visible: false
        color: "green"
        BusyIndicator {
           running: parent.visible
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
        z: 5
    }


    onServer_account_createdChanged: {
        if(server_account_created && blank_page.visible) {
            stack.pop()
        }
        if(server_account_created && profile_dialog.visible) {
            stack.replace(simple_msg_list)
        }
    }


    Timer {
        id: service_timer
        interval: 30; running:true; repeat:true
        onTriggered: {
            if(core.database_online() !== core.is_database_online) {
                core.is_database_online = core.database_online()
            }
            if(core.service_channels() === 1)
                service_timer.interval = 1
            else
                service_timer.interval = 30
        }
    }
}




