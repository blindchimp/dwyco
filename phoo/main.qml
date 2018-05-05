
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
import QtMultimedia 5.4
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

    property string the_man: "5a098f3df49015331d74"

    function picht() {
        return vh(pct) - mm(.5)
    }

    Material.theme: Material.Light
    Material.accent: accent
    Material.primary: primary

    property int profile_bootstrapped : 0
    property bool server_account_created: false
    property int android_img_pick_hack: 0

    property bool dwy_invis: false
    property bool dwy_quiet: false
    property bool show_unreviewed: false
    property bool expire_immediate: false
    function pin_expire() {
        var expire
        var duration
        duration = core.get_local_setting("pin_duration")
        if(duration === "") {
            core.set_local_setting("pin_duration", "0")
            duration = "0"
        }
        if(expire_immediate)
            duration = "0"
        expire = datesec() + parseInt(duration, 10)
        return expire
    }

    function datesec() {
        return Math.round(Date.now() / 1000)
    }

    id: applicationWindow1
    visible: true
    width: 1280
    height: 768
    title: qsTr("Dwyco ")

//    FileDialog {
//        id: filedialog
//        title: "Choose a file"
//        onAccepted: {
//            console.log(fileUrl)
//        }
//        onRejected: {

//        }
//    }

    property int close_bounce: 0
    onClosing: {
        // special cases, don't let them navigate around the
        // initial app setup
        if(!profile_bootstrapped) {
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
                }
                if(pwdialog.allow_access === 0)
                    expire_immediate = true
                return
            }
            close.accepted = false
            bounce_opacity.start()
        }
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
        interactive: {stack.depth === 1}

        AppDrawer {

            padding: 0
            width: Math.min(applicationWindow1.width, applicationWindow1.height) / 3 * 2
            height: applicationWindow1.height
            onClose: {
                drawer.close()
            }

        }
    }


    footer: RowLayout {
            Label {
                id: ind_invis
                text: "Invis"
                visible: dwy_invis
                color: "red"

            }
            Item {
                Layout.fillWidth: true
            }

            Label {
                id: hwtext

            }
            Label {
                id: db_status
                text: core.is_database_online == 0 ? "offline" : "online"
            }
        }

    
    Menu {
        id: moremenu
        x: parent.width - width
        transformOrigin: Menu.TopRight
        MenuItem {
            text: "Block user"
            onTriggered: {
                core.set_ignore(chatbox.to_uid, 1)
                stack.pop()
            }
        }
        MenuItem {
            text: "Block and Delete user"
            onTriggered: {
                core.set_ignore(chatbox.to_uid, 1)
                core.delete_user(chatbox.to_uid)
                themsglist.reload_model()
                stack.pop()

            }
        }

    }


    Item {
        id: top_dispatch
        signal uid_selected(string uid, string action)
        signal profile_updated(int success)
        signal video_display(int ui_id, int frame_number, string img_path)
        signal camera_snapshot(string filename)
        signal uid_resolved(string uid)
        //signal image_picked(string fn)

        property string last_uid_selected
        visible: false
        enabled: false

        onUid_selected: {
            console.log("UID SELECTED", uid)
            last_uid_selected = uid

            if(action === "clicked") {

                stack.push(chatbox)
            }
            else if(action == "hold")
            {
                //stack.push(theprofileview)
               //stack.push(user_action_popup)
                user_action_popup.popup()
                //user_action_popup.visible = true

            }
        }

        onVideo_display: {
            //themsgview.view_source = img_path
        }
    }



    Item {
        id: cam
        property alias camitem: cam_loader.item
        property string next_state
        property string ok_text: "Send"
        anchors.fill: parent
        visible: false
        Loader {
            id: cam_loader
            anchors.fill: parent
            onLoaded: {
                cam_loader.item.state_on_close = cam.next_state
                cam_loader.item.ok_pv_text = cam.ok_text
            }
        }

        onVisibleChanged: {
            if(visible == true && cam_loader.source == "") {
                cam_loader.source = "qrc:/DeclarativeCamera.qml"
            }
            if(visible == false) {
                cam_loader.source = ""
            }
        }

   }

    DSettings {
        id: settings_dialog
        visible: false
    }

    About {
        id: about_dialog
        visible: false
    }

    ConvList {
        id: convlist
        visible: false
    }

    ChatList {
        
        id: chatlist
        visible: false
    }

    ContactList {
        id: contact_list
        visible: false
    }

    Loader {
        id: cqres
        anchors.fill: parent
        source: "qrc:/CQRes.qml"
        visible: false
        active: false

        onVisibleChanged: {
            if(visible)
                cqres.active = true
        }
    }

//    AdminBrowse {
//        id: adminfolder
//        folder: "file:///home/dwight/Downloads/tmp2"
//    }


    Item {

        id: simpdir_rect
        property url xml_url : ""
        //anchors.fill:parent
        visible: false
        Loader {
            id: xmloader
            anchors.fill: parent
        }
        onVisibleChanged: {
            if(visible) {
                var tmp
                tmp = core.get_simple_xml_url()
                if(xml_url !== tmp) {
                    xml_url = tmp
                }
            }

            if(visible === true && xmloader.source === "") {
                xmloader.source = "qrc:/SimpDir.qml"
            }
        }

    }

    FName {
        id: fname
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
    ForwardToList {
        id: forward_dialog
        visible: false
    }

    SimpleChatBox {
        id: chatbox
        //anchors.fill: parent
        model: themsglist
        to_uid: top_dispatch.last_uid_selected
        visible: false

        onVisibleChanged: {
            core.reset_unviewed_msgs(to_uid)
        }

    }

    SimpleMsgBrowse {
        id: simp_msg_browse
        model: themsglist
        to_uid: top_dispatch.last_uid_selected
        visible: false

    }


    ProfileView {
        id: theprofileview
        uid: top_dispatch.last_uid_selected
        //anchors.fill: parent
        visible: false
    }


    ProfileDialog {
        id: profile_dialog
        visible: false
        //anchors.fill: parent

    }


    PINDialog {
        id: pwdialog
        property int allow_access
        property int password_expired
        property int pw_expire_time: 0
        property int resume_time: 0

        onPw_expire_timeChanged: {
            console.log("exp time chg", pw_expire_time)
        }
        onResume_timeChanged: {
            console.log("resume chg", resume_time)
        }
        onPassword_expiredChanged: {
            console.log("pw expired chg ", password_expired)
        }
        onPassword_okChanged: {
            console.log("pw_ok chg", password_ok)
        }
        onPwChanged: {
            console.log("pw chg ", pw)
        }
        password_expired: {
            if(resume_time > pw_expire_time) {
                pw = ""
                return 1
            }
            return 0

        }

        allow_access: {
            if(password_expired === 0)
                return 1
            return password_ok
        }
        onStateChanged: {
            console.log("change to ", state)
            console.log("exp ", pwdialog.pw_expire_time)
            console.log("resume time ", pwdialog.resume_time)
        }

        onAllow_accessChanged: {
            if(allow_access === 1) {
                Qt.inputMethod.hide()
                if(state === "start") {
                    core.init()
                }
            }
        }
        visible: allow_access === 0

        states: [
            State {
                name: "start"
                StateChangeScript {
                    script: {
                        //load password expire from disk
                        var pexp = core.get_local_setting("pin_expire")
                        if(pexp === "")
                            pexp = 0
                        else
                            pexp = parseInt(pexp, 10)
                        pwdialog.pw_expire_time = pexp
                        pwdialog.resume_time = datesec()

                    }
                }
            },

            State {
                name: "pause"
                StateChangeScript {
                    script: {
                        var expire = pin_expire()
                        core.set_local_setting("pin_expire", expire.toString())
                        pwdialog.pw_expire_time = expire
                    }
                }
            },

            State {
                name: "resume"
                StateChangeScript {
                    script: {
                        pwdialog.resume_time = datesec()
                        if(pwdialog.password_expired === 1) {
                            pwdialog.pw = ""
                            //pwdialog.resume_time = 0
                        }
                    }
                }
            }

        ]

        z: 10
        exit_button.onClicked: {
            expire_immediate = true
            Qt.quit()
        }
        Component.onCompleted: {
            load_cpw()
        }

    }
    PINChangeDialog {
        id: pwchange_dialog
        visible: false
    }


    ProfileUpdateDialog {
        id: profile_update_dialog      
        visible: false
    }

    IgnoreListDialog {
        id: iglist_dialog
        visible: false
    }

    PublicChat {
        id: public_chat
        //anchors.fill: parent
        visible: false
    }


    UserActionMenu {
        id: user_action_popup
        uid: top_dispatch.last_uid_selected
    }

    VidCamPreview {
        id: vid_cam_preview
        visible: false
    }

    DwycoVidRec {
        id: dwyco_vid_rec
        visible: false
    }
  

    SoundEffect {
        id: sound_sent
        source: "qrc:/androidinst/assets/space-zap.wav"
    }
    SoundEffect {
        id: sound_recv
        source: "qrc:/androidinst/assets/space-zap.wav"
        volume: {dwy_quiet ? 0.0 : 1.0}
        muted: dwy_quiet
    }

    
    StackView {
        id: stack
        //initialItem: userlist
        anchors.fill: parent
        visible: {pwdialog.allow_access === 1}

        
    }

    DwycoCore {
        id: core
        property int is_database_online: -1

        objectName: "dwyco_singleton"
        client_name: {"QML-" + Qt.platform.os + "-" + core.buildtime}
        Component.onCompleted: {
            var a
            a = get_local_setting("first-run")
            if(a === "") {
                //profile_dialog.visible = true
                stack.push(convlist)
                stack.push(blank_page)
                stack.push(profile_dialog)
            } else {
                stack.push(convlist)
                profile_bootstrapped = 1
                pwdialog.state = "start"
            }
            a = get_local_setting("acct-created")
            if(a === "") {
                server_account_created = false
            } else {
                server_account_created = true
            }
            if(profile_bootstrapped && !server_account_created) {
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
            //drawer_model.setProperty(4, "chked", dwy_quiet)


            a = get_local_setting("invis")
            if(a === "" || a === "false") {
                dwy_invis = false
            } else {
                dwy_invis = true
            }
            //drawer_model.setProperty(5, "chked", dwy_invis)

            a = get_local_setting("show_unreviewed")
            if(a === "" || a === "0") {
                show_unreviewed = false
            } else {
                show_unreviewed = true
            }

            if(pwdialog.allow_access === 1) {
                init()
            }
        }


        Component.onDestruction: {
            console.log("die")
            if(Qt.platform.os == "android") {
                notificationClient.cancel()
            }
            var expire = pin_expire()
            core.set_local_setting("pin_expire", expire.toString())
            exit()
        }

        onRem_keyboard_active: {

        }

        onServer_login: {
           
            console.log(msg)
            console.log(what)
            if(what === 1) {
                set_local_setting("acct-created", "true")
                server_account_created = true
            }
            if(Qt.platform.os == "android") {
                notificationClient.set_msg_count_url(core.get_msg_count_url())
            }
            if(simpdir_rect.visible && simpdir_rect.xml_url === "")
                simpdir_rect.xml_url = core.get_simple_xml_url()
        }

        onNew_msg: {
            console.log(from_uid)
            console.log(txt)
            console.log(mid)
            console.log("msglist", themsglist.uid)
            if(from_uid == themsglist.uid) {
                themsglist.reload_model()
                // note: this could be annoying if the person is
                // browsing back, need to check to see if so and not
                // do this, or display a "go to bottom" icon
                chatbox.listview.positionViewAtBeginning()
                console.log("RELOAD")
            }
            //notificationClient.notification = "New messages"
            sound_recv.play()
        }

        onSys_msg_idx_updated: {
            console.log("update idx", uid)
            console.log("upd" + uid + " " + themsglist.uid)
            if(uid == themsglist.uid) {
                themsglist.reload_model()
                console.log("RELOAD")
            }
        }

        onMsg_send_status: {
            console.log(pers_id, status, recipient)
            hwtext.text = status
            if(status == DwycoCore.MSG_SEND_SUCCESS) {
                //sound_sent.play()
                if(themsglist.uid == recipient) {
                    themsglist.reload_model()
                    chatbox.listview.positionViewAtBeginning()
                }

            }
        }

        onMsg_progress: {
            console.log(pers_id, msg, percent_done)
            hwtext.text = msg + " " + String(percent_done) + "%"
        }

        onSys_uid_resolved: {
        }
        onSys_invalidate_profile: {
        }

        onProfile_update: {
            top_dispatch.profile_updated(success)
        }

        onVideo_display: {
            //top_dispatch.video_display(ui_id, frame_number, img_path)
        }

        onIgnore_event: {

        }
        onQt_app_state_change: {
            console.log("app state change ", app_state)
            if(app_state === 0) {
                // resuming
                themsglist.reload_model()
                //stack.pop()

                pwdialog.state = "resume"

            } else {

                //stack.push(blank_page)
                pwdialog.state = "pause"
            }

        }

        onImage_picked: {
            console.log("image " + fn)
            if(android_img_pick_hack == 1)
            {
                profile_update_dialog.android_img_filename = fn
                profile_update_dialog.android_hack = true
            }
            else if(android_img_pick_hack == 2)
            {
                chatbox.android_img_filename = fn
                chatbox.android_hack = true

            }
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
    }


    Timer {
        id: service_timer
        interval: 30; running:true; repeat:true
        onTriggered: {
            //time.text = Date().toString()
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




