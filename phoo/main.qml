
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml 2.12
import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import QtMultimedia 5.12
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
    // attempt to convert an absolute length into the
    // number of pixels on the screen
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
    // takes a percent, and returns the number of pixels
    // corresponding that percentage of the given dimension
    // used when you just want to estimate that something should
    // take a certain percentage of the screen
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
    //font.pixelSize: {is_mobile ? Screen.pixelDensity * 2.5 : font.pixelSize}
    font.weight: Font.Bold
    
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
    property bool show_hidden: true
    property bool show_archived_users: false

    property bool up_and_running : {pwdialog.allow_access === 1 && profile_bootstrapped === 1 && server_account_created && core.is_database_online === 1}
    property int qt_application_state: 0
    property bool is_mobile

    is_mobile: {Qt.platform.os === "android" || Qt.platform.os === "ios"}

    property bool group_active
    group_active: core.active_group_name.length > 0 && core.group_status === 0 && core.group_private_key_valid === 1

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

    function censor_name(namestr) {
        var first = namestr.substr(0, 3)
        return first.concat("***")

    }
    function regular_profile(reviewed, regular) {
        return reviewed == 1 && regular == 1
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
    title: {
        qsTr("Dwyco ") + core.this_handle + (core.group_private_key_valid === 1 ?
                                                 " (" + core.active_group_name + " " + core.percent_synced + "%)" :
                                                 (core.group_status === 1 ?
                                                      "(Requesting " + core.active_group_name + ")" : ""))

    }
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
                if(pwdialog.allow_access === 0)
                    expire_immediate = true
                return
            }
            close.accepted = false
            bounce_opacity.start()
        }
    }

    Component.onCompleted: {
        AndroidPerms.request_sync("android.permission.CAMERA")
        AndroidPerms.request_sync("android.permission.POST_NOTIFICATIONS")
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
        interactive: {stack.depth === 1 && pwdialog.allow_access === 1 && profile_bootstrapped === 1 && server_account_created}
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

            Connections {
                target: core
                function onProfile_update(success) {
                    drawer_contents.circularImage.source = core.uid_to_profile_preview(core.get_my_uid())
                    drawer_contents.text1.text = core.uid_to_name(core.get_my_uid())
                }
            }

            onVisibleChanged: {
                if(visible) {
                    drawer_contents.circularImage.source = core.uid_to_profile_preview(core.get_my_uid())
                    drawer_contents.text1.text = core.uid_to_name(core.get_my_uid())
                }

            }
        }
    }


    footer: RowLayout {
        visible: dwyco_debug
            Label {
                id: ind_invis
                text: "Invis"
                visible: core.invisible
                color: "red"

            }
            Label {
                text: "Archived " + (core.total_users - ConvListModel.count).toString()
                visible: core.total_users > ConvListModel.count
                color: "red"
            }

            Item {
                Layout.fillWidth: true
            }


            Label {
                id: conns
                text: "conns " + SyncDescModel.connection_count.toString()
            }

            Label {
                id: hwtext

            }
            Label {
                id: db_status
                text: core.is_database_online === 0 ? "db off" : "db on"
            }
            Label {
                id: chat_status
                text: core.is_chat_online === 0 ? "chat off" : "chat on"
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
                confirm_block_delete.visible = true
            }
            MessageDialog {
                id: confirm_block_delete
                title: "Block and delete?"
                icon: StandardIcon.Question
                text: "Delete ALL messages from user and BLOCK them?"
                informativeText: "This removes FAVORITE and HIDDEN messages too."
                standardButtons: StandardButton.Yes | StandardButton.No
                onYes: {
                    core.set_ignore(chatbox.to_uid, 1)
                    core.delete_user(chatbox.to_uid)
                    themsglist.reload_model()
                    stack.pop()

                }
                onNo: {
                    stack.pop()
                }
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
    }

    Loader {
        id: cam

        property string next_state
        property string ok_text: "Send"
        anchors.fill: parent
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

//    Loader {
//        id: settings_dialog
//        visible: false

//        onVisibleChanged: {
//            if(visible) {
//                source = "qrc:/DSettings.qml"
//            }
//        }
//    }
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
        id: restore_auto_backup
        visible: false
        active: visible
        onVisibleChanged: {
            if(visible) {
                source = "qrc:/RestoreAutoBackup.qml"
            }
        }

    }

    DevGroup {
        id: device_group
        visible: false
        onQuitnowChanged: {
            if(quitnow === true)
            {
                stack.push(device_group)
            }
        }
    }

    ConvList {
        id: convlist
        visible: false
    }

    ChatList {
        id: chatlist
        visible: false
    }

    Item {
        id: chat_server

        property int connect_server: 0
        property bool auto_connect: false

        Connections {
            target: core
            function onQt_app_state_change(app_state) {
                if(app_state === 0) {
                    console.log("CHAT SERVER RESUME ")

                }
                if(app_state !== 0) {
                    console.log("CHAT SERVER PAUSE");

                    //core.disconnect_chat_server()
                }
            }

            function onServer_login(msg, what) {
                console.log("CHAT SERVER RESTART ", what, chat_server.auto_connect)
                if(what > 0) {
                    if(chat_server.auto_connect) {
                        core.switch_to_chat_server(chat_server.connect_server)
                    }
                }
            }
        }

    }


    Loader {
        id: cqres
        //anchors.fill: parent
        visible: false
        Component {
            id: wtf
            CQRes {}
        }
        onVisibleChanged: {
            if(visible)
                sourceComponent = wtf
        }

        //sourceComponent: wtf
    }

//    AdminBrowse {
//        id: adminfolder
//        folder: "file:///home/dwight/Downloads/tmp2"
//    }

    Loader {
        id: simpdir_rect

        visible: false
        onVisibleChanged: {
            if(visible) {
                source = "qrc:/SimpDir.qml"
            }
        }
        onLoaded: {
            if(SimpleDirectoryList.count === 0)
                core.refresh_directory()
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

    Loader {
        id: forward_dialog
        property string mid_to_forward
        visible: false
        active: visible
        onVisibleChanged: {
            if(visible)
                source = "qrc:/ForwardToList.qml"
        }
    }

    Loader {
        id: send_multi_report
        visible: false
        active: visible
        onVisibleChanged: {
            if(visible)
                source = "qrc:/SendMulti.qml"
        }

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

    SimpleTagMsgBrowse {
        id: simp_tag_browse
        model: themsglist
        visible: false
    }


    ProfileView {
        id: theprofileview
        uid: top_dispatch.last_uid_selected
        //anchors.fill: parent
        visible: false
    }

    Loader {
        id: profile_dialog
        visible: false
        active: visible
        onVisibleChanged: {
            if(visible) {
                source = "qrc:/ProfileDialog.qml"
            }
        }
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

    Loader {
        id: pwchange_dialog
        visible: false
        active: visible
        onVisibleChanged: {
            if(visible) {
                source = "qrc:/PINChangeDialog.qml"
            }
        }

    }


    ProfileUpdateDialog {
        id: profile_update_dialog
        visible: false
    }

    Loader {
        id: iglist_dialog
        visible: false

        onVisibleChanged: {
            if(visible) {
                source = "qrc:/IgnoreListDialog.qml"
            }
        }
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

//    VidCamPreview {
//        id: vid_cam_preview
//        visible: false
//    }

    Loader {
        id: vid_cam_preview
        active: false
        visible: false
        onVisibleChanged: {
            if(visible) {
                source = "qrc:/VidCamPreview.qml"
                active = true

            }
        }
    }

    DwycoVidRec {
        id: dwyco_vid_rec
        visible: false
    }
  

    SoundEffect {
        id: sound_sent
        source: "qrc:/androidinst2/assets/space-zap.wav"
    }
    SoundEffect {
        id: sound_recv
        source: "qrc:/androidinst2/assets/space-zap.wav"
        volume: {dwy_quiet ? 0.0 : 1.0}
        muted: dwy_quiet
    }
    SoundEffect {
        id: sound_alert
        source: "qrc:/androidinst2/assets/space-incoming.wav"
        volume: {dwy_quiet ? 0.0 : 1.0}
        muted: dwy_quiet
    }

    
    StackView {
        id: stack
        //initialItem: userlist
        anchors.fill: parent
        visible: {pwdialog.allow_access === 1}
        onDepthChanged: {
            if(depth === 1)
                simp_tag_browse.to_tag = ""
        }

        
    }

    Migrate {
        id: migrate_page
        visible: false
    }

    Reindex {
        id: background_reindex
        visible: false
    }

    DwycoCore {
        id: core
        property int is_database_online: -1
        property int is_chat_online: -1

        objectName: "dwyco_singleton"
        client_name: {"phoo-" + Qt.platform.os + "-" + core.buildtime}
        Component.onCompleted: {
            if(core.android_migrate === 1)
            {
                stack.push(migrate_page)
                return
            }
            var a
            a = get_local_setting("first-run")
            if(a === "") {
                //profile_dialog.visible = true
                // don't need a reindex_complete
                set_local_setting("reindex1", "1")
                stack.push(convlist)
                stack.push(blank_page)
                stack.push(profile_dialog)
            } else {
                a = get_local_setting("reindex1")
                if(a === "")
                {
                    stack.push(background_reindex)
                    return
                }
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


//            a = get_local_setting("invis")
//            if(a === "" || a === "false") {
//                dwy_invis = false
//            } else {
//                dwy_invis = true
//            }

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
                notificationClient.set_lastrun()
            }
            var expire = pin_expire()
            core.set_local_setting("pin_expire", expire.toString())
            exit()
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
                notificationClient.log_event()
                notificationClient.set_lastrun()
            }
            if(simpdir_rect.visible && SimpleDirectoryList.count === 0)
                refresh_directory()
            //applicationWindow1.title = "Dwyco " + core.uid_to_name(core.get_my_uid())
        }

        onNew_msg: {
            console.log("new msglist ", themsglist.uid, ' ', from_uid, " ", mid)
            if(from_uid === themsglist.uid || core.map_to_representative(from_uid) === core.map_to_representative(themsglist.uid)) {
                themsglist.reload_model();
                // note: this could be annoying if the person is
                // browsing back, need to check to see if so and not
                // do this, or display a "go to bottom" icon
//                if(chatbox.listview.atYEnd) {
//                    chatbox.listview.positionViewAtBeginning()
//                }
                console.log("RELOAD nm")
                //themsglist.reload_model()
            }
            //notificationClient.notification = "New messages"
            sound_recv.play()
        }

        onSys_msg_idx_updated: {
            console.log("upd " + uid + " " + themsglist.uid)
            if(uid === themsglist.uid || core.map_to_representative(uid) === core.map_to_representative(themsglist.uid)) {
                themsglist.reload_model()

                console.log("RELOAD msg_idx")
            }
        }

        onMsg_send_status: {
            console.log(pers_id, status, recipient)
            //hwtext.text = status
            if(status == DwycoCore.MSG_SEND_SUCCESS) {
                //sound_sent.play()
                if(themsglist.uid == recipient || core.map_to_representative(themsglist.uid) === core.map_to_representative(recipient)) {
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
                pwdialog.state = "resume"
            } else {
                drawer.close()
                pwdialog.state = "pause"
            }
            if(Qt.platform.os == "android") {
                notificationClient.set_lastrun()
            }
            qt_application_state = app_state
        }

        onImage_picked: {
            console.log("image " + fn)
            if(android_img_pick_hack === 1)
            {
                profile_update_dialog.android_img_filename = fn
                profile_update_dialog.android_hack = true
            }
            else if(android_img_pick_hack === 2)
            {
                chatbox.android_img_filename = fn
                chatbox.android_hack = true

            }
        }

        onAny_unviewedChanged: {
            if(any_unviewed)
                set_badge_number(1)
            else
                set_badge_number(0)
        }

        onClient_nameChanged: {
            core.update_dwyco_client_name(core.client_name)
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
        id: sync_debug
        interval: 10000
        repeat: true
        running: group_active && server_account_created && qt_application_state === 0
        onTriggered: {
            SyncDescModel.load_model()
        }
    }

    Timer {
        id: service_timer
        interval: 30; running:true; repeat:true
        onTriggered: {
            if(pwdialog.allow_access === 0)
                return
            //time.text = Date().toString()
            if(core.database_online() !== core.is_database_online) {
                core.is_database_online = core.database_online()
            }
            if(core.chat_online() !== core.is_chat_online) {
                core.is_chat_online = core.chat_online()
            }

            // note: trying to schedule out another service channels call
            // this way doesn't work too well unless we can kick the
            // timer into action based on a dwyco* call (for example, we might
            // be in a situation where we are waiting 10 seconds for the next
            // network event, but the user clicks something that causes a
            // video to record, which we want to start servicing immediately.
            // have to think about this. also, the networking stuff really needs to be
            // integrated into qt event loop using qsocketnotifiers.
            // as a side note: it might be advantageous to "synchronize" multiple timers
            // for things like channel sockets, so they can be serviced in a batch with one
            // call to service channels. if you leave them unsynchronized, you get "next"
            // expirations that are more or less random based on when the timer was started,
            // which isn't really necessary.
            var sc_next = core.service_channels()
            //console.log("next ", sc_next)
            if(sc_next === 1 || sc_next < 0)
            {
                service_timer.interval = 1
            }
            else
            {
                service_timer.interval = (sc_next === 0 ? 100 : Math.min(100, sc_next))
            }
        }

    }


}




