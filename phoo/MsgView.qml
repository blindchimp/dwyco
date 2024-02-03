
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import dwyco

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
    property string export_result
    property bool is_trash: false

    //anchors.fill:parent

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
            if(view_id !== -1) {
            core.stop_zap_view(view_id)
            core.delete_zap_view(view_id)
            }
            is_trash = false
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
        function onQt_app_state_change(app_state) {
            if(app_state === 0) {


            }
            if(app_state !== 0) {
                if(view_id !== -1) {
                core.stop_zap_view(view_id)
                core.delete_zap_view(view_id)
                }
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
            MenuItem {
                id: untrash_item
                text: "Untrash msg"
                visible: false
                onTriggered: {
                    core.unset_tag_message(mid, "_trash")
                    themsglist.invalidate_model_filter()
                    stack.pop()
                }
            }
            MenuItem {
                id: review_item
                text: "Review"
                visible: false
                onTriggered: {
                    stack.push(msg_review)

                }
            }
            Menu {
                id: optionsMenu
                x: parent.width - width
                transformOrigin: Menu.TopRight
                MenuItem {
                    text: is_trash ? "Delete forever" : "Trash msg"
                    onTriggered: {
                        if(is_trash) { 
                            core.delete_message(uid, mid)
                        } else {
                            core.set_tag_message(mid, "_trash")
                        }
                        themsglist.invalidate_model_filter()
                        stack.pop()
                    }
                }
//                MenuItem {
//                    text: "Untrash msg"
//                    visible: is_trash
//                    onTriggered: {
//                        core.unset_tag_message(mid, "_trash")
//                        themsglist.invalidate_model_filter()
//                        stack.pop()
//                    }
//                }
                MenuItem {
                    text: "Forward msg"
                    onTriggered: {
                        forward_dialog.mid_to_forward = mid
                        //forward_dialog.uid_folder = uid
                        stack.push(forward_dialog)
                    }
                }

                MenuItem {
                    text: fav ? "Unfavorite" : "Favorite"
                    onTriggered: {
                        core.set_fav_message(mid, !fav)
                    }
                }
                MenuItem {
                    text: hid ? "Unhide" : "Hide"
                    onTriggered: {
                        if(hid)
                            core.unset_tag_message(mid, "_hid")
                        else
                            core.set_tag_message(mid, "_hid")
                        themsglist.invalidate_model_filter()
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

//                MenuItem {
//                    text: "Review"
//                    visible: core.this_uid === the_man
//                    onTriggered: {
//                        stack.push(msg_review)

//                    }
//                }


                onVisibleChanged: {
                    if(visible) {
                        if(is_trash) {
                            untrash_item.visible = true
                            optionsMenu.insertItem(0, untrash_item)
                        }
                        else {
                            untrash_item.visible = false
                            optionsMenu.removeItem(untrash_item)
                        }
                        if(core.this_uid === the_man) {
                            review_item.visible = true
                            optionsMenu.addItem(review_item)
                        } else {
                            review_item.visible = false
                            optionsMenu.removeItem(review_item)
                        }
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
        id: headbar
        visible: !clean_button.checked
        extras: extras_button

    }

    footer: ToolBar {
        id: footbar
        visible: !clean_button.checked
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
            TipButton {
                id: show_text_button
                checkable: true
                Layout.fillHeight: true
                ToolTip.text: "Show msg text"
                text: "Msg text"

            }
            TipButton {
                id: clean_button
                checkable: true
                Layout.fillHeight: true
                ToolTip.text: "Full screen"
                contentItem: Image {
                    source: mi("ic_fullscreen_black_24dp.png")

                }
            }

            Item {
                Layout.fillWidth: true
            }
            TipButton {
                id: save_button
                contentItem: Image {
                    source: mi("ic_share_black_24dp.png")

                }

                onClicked: {
                    // supposedly you don't need storage permissions to add to
                    // image collections via mediastore on newer android versions
                    if(AndroidPerms.android_api() < 29 && !AndroidPerms.external_storage_permission) {
                        if(!AndroidPerms.request_sync("android.permission.WRITE_EXTERNAL_STORAGE"))
                            return
                    }

                    var export_name = core.export_attachment(mid)
                    if(export_name.length > 0) {
                        export_result = "Saved to " + export_name.substring(export_name.lastIndexOf('/') + 1)
                        if(Qt.platform.os == "android") {
                            notificationClient.share_to_mediastore(export_name)
                        } else {

                        }
                    }
                    else {
                        export_result = "FAILED save "
                    }
                    toast_opacity.stop()
                    toast_opacity.start()
                }

                ToolTip.text: "Save attachment"
                Layout.fillHeight: true
            }
            Item {
                Layout.fillWidth: true
            }
            TipButton {
                id: hid_button
                contentItem: Image {
                    source: mi("ic_visibility_off_black_24dp.png")
                }
                background: Rectangle {
                    visible: hid
                    color: "orange"

                }
                onCheckedChanged: {
                if(checked)
                    core.set_tag_message(mid, "_hid")
                else
                    core.unset_tag_message(mid, "_hid")
                }
                checkable: true
                checked: hid
                ToolTip.text: "Hide msg"
                Layout.fillHeight: true
            }
        }
    }

    Rectangle {
        id: isfav
        width: 32
        height: 32
        anchors.top: parent.top
        anchors.left: parent.left
        visible: fav && !clean_button.checked
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
        visible: hid && !clean_button.checked
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
                onPressAndHold: {
                    clean_button.checked = false
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
        Layout.maximumHeight: viewer.source === "" ? (parent.height * 6) / 10 : parent.height / 3

        wrapMode: Text.Wrap
        visible: show_text_button.checked
    }
    //}

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
            easing.type: Easing.InQuart
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


