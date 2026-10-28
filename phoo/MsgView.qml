
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
import QtQuick.Dialogs
import QtCore
import dwyco

Page {
    id: msgviewer
    property string uid
    property string mid
    property int view_id: -1
    property int ui_id: -1
    property alias view_source : viewer.source
    property string formatted_text
    property string raw_text
    property bool _rawMode: false
    property bool dragging
    property bool fav
    property bool hid
    property string text_bg_color: primary_dark
    property string export_result
    property bool is_trash: false
    // set for attachments we can't render a preview for (ie, anything that
    // isn't an image). in that case view_source is empty and we show a panel
    // with the file's name/size and the buttons that save it.
    property bool is_plain_file: false
    property string file_name: ""
    property double file_size: 0

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
            }
            is_trash = false
            is_plain_file = false
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
    onFormatted_textChanged: {
        if(!_rawMode) {
            msg_text.text = formatted_text
            msg_text.textFormat = Text.RichText
        }
    }
    onRaw_textChanged: {
        if(_rawMode) {
            msg_text.text = raw_text
            msg_text.textFormat = Text.PlainText
        }
    }
    onMidChanged: {
        if(mid.length > 0) {
            show_text_button.checked = false
            show_raw_button.checked = false
            _rawMode = false
            msg_text.textFormat = Text.RichText
            msg_text.cursorPosition = 0
        }
    }

    Component.onCompleted: {
        console.log("vid, vsource ", view_id, view_source)
    }

    onView_idChanged: {
        console.log("VIEWID ", msgviewer.view_id)
    }

    onView_sourceChanged: {
        console.log("VIEW_SOURCE", msgviewer.view_source)
    }

    // the name the sender used is not something we want to blindly hand to
    // a file dialog: it comes off the network, and it is often long and
    // useless. keep only the last path component and scrub anything that
    // doesn't belong in a filename.
    function suggested_name() {
        var n = String(file_name)
        n = n.split(/[\\/]/).pop()
        n = n.replace(/[\x00-\x1f<>:"|?*]/g, "_")
        n = n.replace(/^[. ]+|[. ]+$/g, "")
        if(n === "")
            n = "attachment"
        if(n.length > 80)
            n = n.substring(0, 80)
        return n
    }

    // same name, but short enough to live on a button label.
    function short_name(n) {
        if(n.length > 40)
            return n.substring(0, 39) + "\u2026"
        return n
    }

    function default_save_dir() {
        // note: these come back as "file://" urls, not plain paths, so
        // callers must not prepend a scheme of their own.
        var d = StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        if(!d)
            d = StandardPaths.writableLocation(StandardPaths.HomeLocation)
        return d
    }

    // the "just put it somewhere sensible" save, used by the toolbar button
    // and the one-tap button on the file panel.
    function quick_save() {
        // supposedly you don't need storage permissions to add to
        // image collections via mediastore on newer android versions
        var export_name = core.export_attachment(mid)
        if(export_name.length > 0) {
            export_result = "Saved to " + export_name.substring(export_name.lastIndexOf('/') + 1)
            if(Qt.platform.os == "android") {
                notificationClient.share_to_mediastore(export_name)
            }
        }
        else {
            export_result = "FAILED save "
        }
        toast_opacity.stop()
        toast_opacity.start()
    }

    Shortcut {
        sequence: "Esc"
        enabled: !is_mobile && clean_button.checked
        onActivated: clean_button.checked = false
    }

    Timer {
        id: hideOverlayTimer
        interval: 2500
        onTriggered: exit_overlay.opacity = 0
    }

//    fav: { (mid.length > 0) ?
//             (core.get_fav_message(mid) === 1) : false
//    }

//    hid: {mid.length > 0 ? core.has_tag_message(mid, "_hid") === 1 : false}

    Connections {
        target: core
        function onVideo_display(ui_id, frame_number, img_path) {
            if(ui_id === msgviewer.ui_id) {
                msgviewer.view_source = img_path
            }
        }
        function onMid_tag_changed(changed_mid) {
            if(changed_mid != msgviewer.mid)
                return
            fav = core.has_tag_message(msgviewer.mid, "_fav")
            hid = core.has_tag_message(msgviewer.mid, "_hid")
        }
        function onMsg_tag_change_global(changed_mid, huid) {
            if(changed_mid != msgviewer.mid)
                return
            fav = core.has_tag_message(msgviewer.mid, "_fav")
            hid = core.has_tag_message(msgviewer.mid, "_hid")
        }
        function onQt_app_state_change(app_state) {
            if(app_state === 0) {


            }
            if(app_state !== 0) {
                if(msgviewer.view_id !== -1) {
                core.stop_zap_view(msgviewer.view_id)
                core.delete_zap_view(msgviewer.view_id)
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
            onClicked: {
                if(is_trash)
                    optionsMenuTrash.open()
                else
                    optionsMenu.open()
            }

            MsgViewMenu {
                id: optionsMenu
                visible: false
            }
            MsgViewMenuTrash {
                id: optionsMenuTrash
                visible: false
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
                onCheckedChanged: {
                    if(checked)
                        msg_text.cursorPosition = 0
                }

            }
            TipButton {
                id: show_raw_button
                checkable: true
                Layout.fillHeight: true
                ToolTip.text: "Toggle raw/ formatted text"
                text: "Raw"
                onCheckedChanged: {
                    msgviewer._rawMode = checked
                    if(checked) {
                        msg_text.textFormat = Text.PlainText
                        msg_text.text = raw_text
                    } else {
                        msg_text.textFormat = Text.RichText
                        msg_text.text = formatted_text
                    }
                }
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

                onClicked: msgviewer.quick_save()

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
        Layout.fillHeight: (view_source.toString() === "" && !is_plain_file) ? false : true
        Layout.fillWidth: true
        color: primary_dark
        gradient: Gradient {
            GradientStop { position: 0.0; color: primary_dark }
            GradientStop { position: 1.0; color: primary_light}
        }

        ColumnLayout {
            id: plain_file_panel
            visible: msgviewer.is_plain_file
            // note: the PinchArea/dragArea below covers the whole viewer and
            // pops the page when clicked. it is declared after this, so
            // without a z boost it would swallow the clicks on the save
            // buttons.
            z: 1
            anchors.centerIn: parent
            width: Math.min(parent.width - dp(20), dp(500))
            spacing: dp(8)

            FileChip {
                Layout.fillWidth: true
                fileName: msgviewer.file_name
                fileSize: msgviewer.file_size
                textColor: "white"
                // you're already looking at it, so there's nothing to click on.
                hintText: ""
            }

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                text: is_mobile
                      ? "This file can't be previewed here. Save it to keep a copy."
                      : "This file can't be previewed here. The name the sender used can be long and not very useful, so pick something better if you want."
                color: "white"
                opacity: 0.85
                font.pixelSize: dp(12)
            }

            Button {
                Layout.fillWidth: true
                visible: !is_mobile
                text: "Save as " + msgviewer.short_name(msgviewer.suggested_name()) + "..."
                onClicked: {
                    var dir = msgviewer.default_save_dir()
                    // percent-encode: the name is going into a url, and
                    // characters like "#" or "%" would otherwise be eaten
                    // when it is turned back into a path.
                    save_as_dialog.currentFolder = dir
                    save_as_dialog.selectedFile = dir + "/" + encodeURIComponent(msgviewer.suggested_name())
                    save_as_dialog.open()
                }
            }

            Button {
                Layout.fillWidth: true
                visible: !is_mobile
                text: "Save to Documents"
                onClicked: msgviewer.quick_save()
            }

            Button {
                Layout.fillWidth: true
                visible: is_mobile
                text: "Save file"
                onClicked: msgviewer.quick_save()
            }
        }

        FileDialog {
            id: save_as_dialog
            title: "Save attachment as"
            fileMode: FileDialog.SaveFile
            nameFilters: ["All files (*)"]
            currentFolder: msgviewer.default_save_dir()
            onAccepted: {
                var p = core.url_to_filename(selectedFile)
                if(p === "") {
                    export_result = "FAILED save "
                } else if(core.export_attachment_as(msgviewer.mid, p)) {
                    export_result = "Saved to " + p.substring(p.lastIndexOf('/') + 1)
                } else {
                    export_result = "FAILED save "
                }
                toast_opacity.stop()
                toast_opacity.start()
            }
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
                    if(!is_mobile && clean_button.checked) {
                        exit_overlay.opacity = 1
                        hideOverlayTimer.restart()
                    }
                }
                onPositionChanged: {
                    if(!is_mobile && clean_button.checked) {
                        exit_overlay.opacity = 1
                        hideOverlayTimer.restart()
                    }
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

        ToolButton {
            id: exit_overlay
            visible: !is_mobile && clean_button.checked
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 8
            z: 10
            opacity: 0
            Behavior on opacity { NumberAnimation { duration: 250 } }
            contentItem: Image {
                source: mi("ic_fullscreen_exit_black_24dp.png")
            }
            background: Rectangle {
                color: "#66000000"
                radius: width / 2
            }
            onClicked: clean_button.checked = false
        }
    }
    TextArea {
        id: msg_text
        background: Rectangle {
            color: msgviewer.text_bg_color
            radius: 6
        }
        clip: true
        color: "white"
        leftPadding: 12
        rightPadding: 12
        topPadding: 12
        bottomPadding: 12
        readOnly: true
        selectByKeyboard: !is_mobile
        selectByMouse: !is_mobile
        textFormat: Text.RichText
        onLinkActivated: (link)=> {
            Qt.openUrlExternally(link)
        }

        Layout.fillWidth: true
        Layout.minimumHeight: 60
        Layout.maximumHeight: (view_source.toString() === "" && view_id === -1 && !is_plain_file) ? parent.height * 9 / 10 : parent.height / 3

        wrapMode: Text.Wrap
        visible: show_text_button.checked || ((view_source.toString() === "" && view_id == -1) && (raw_text.length > 0 || formatted_text.length > 0))

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
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
        running: view_source.toString() === "" && view_id !== -1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        z: 20
    }


}


