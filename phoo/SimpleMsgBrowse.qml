
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
//import Qt.labs.platform 1.0 as Mumble

Page {
    id: simple_msg_browse
    property alias model: grid.model
    property string to_uid;
    property bool multiselect_mode: false
    property url cur_source
    property int ind_online: 0
    property int filter_show_sent: 1
    property int filter_show_only_fav: 0

    function star_fun(b) {
        console.log("chatbox star")
        model.fav_all_selected(b ? 1 : 0)
    }

    onFilter_show_only_favChanged: {
        themsglist.set_filter(filter_show_sent, 1, -1, filter_show_only_fav)
    }

    onFilter_show_sentChanged: {
        themsglist.set_filter(filter_show_sent, 1, -1, filter_show_only_fav)
    }

    onTo_uidChanged: {
        if(to_uid === "")
            return
        cur_source = ""
        cur_source = core.uid_to_profile_preview(to_uid)
        top_toolbar_text.rawtext = ""
        top_toolbar_text.rawtext = core.uid_to_name(to_uid)
        ind_online = core.get_established_state(to_uid)
        filter_show_only_fav = 0
        filter_show_sent = 1

    }

    onMultiselect_modeChanged: {
        model.set_all_unselected()
    }
    onVisibleChanged: {
        //console.log("STAACK ", StackView.index, stack.depth)
    }

    // this just gets the case where the msg browser is popped off the
    // stack, we need to reset the options so it doesn't affect other
    // parts of the ui (the msglistmodel is a global, which, seems like
    // it might be complicating things sometimes.)
    Connections {
        target: stack
        function onDepthChanged() {
            console.log("depth mb ", simple_msg_browse.StackView.index, stack.depth)
            if(simple_msg_browse.StackView.index === -1) {
                filter_show_only_fav = 0
                filter_show_sent = 1
            }

        }
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

    header: Column {
        width:parent.width
        MultiSelectToolbar {
            id: multi_toolbar
            visible: multiselect_mode
            extras: extras_button
            delete_warning_inf_text: "KEEPS FAVORITE messages"
            delete_warning_text: "Trash all selected messages?"
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

                ToolButton {
                    id: back_button
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_arrow_back_black_24dp.png")

                    }
                    checkable: false
                    onClicked: {
                        filter_show_only_fav = 0
                        filter_show_sent = 1
                        stack.pop()
                    }
                    Layout.fillHeight: true
                    //Layout.maximumWidth: mm(3)
                    //Layout.rightMargin: 0
                }

                Item {
                    id: prof
                    property bool censor_it
                    property bool regular
                    censor_it: censor && !regular
                    regular: {return init_called && core.uid_profile_regular(to_uid)}
                    //color: "red" //accent
                    //border.width: 1
                    //radius: 3

                    Layout.fillHeight: true
                    Layout.minimumHeight: cm(1)
                    //Layout.leftMargin: 0

                    CircularImage2 {
                        id: top_toolbar_img
                        source: {
                            if(to_uid === "")
                                return
                            return (!prof.censor_it) ?
                                    cur_source :  "qrc:/new/red32/icons/red-32x32/exclamation-32x32.png"
                        }
                        fillMode: Image.PreserveAspectCrop
                        width: parent.height
                        height: parent.height

                    }
                    Text {
                        id: top_toolbar_text
                        //width: dp(160)
                        property string rawtext: ""
                        text: to_uid.length == 0 ? "" : (!prof.censor_it ? rawtext : censor_name(rawtext))
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
                        text: "online"
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
                            filter_show_only_fav = 0
                            filter_show_sent = 1
                            stack.pop()
                        }
                    }

                }

                Item {
                    id: next_item
                    Layout.fillWidth: true
                }

                ConvToolButton {
                    visible: {stack.depth > 2 || core.any_unviewed}
                }


                ToolButton {
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_action_overflow.png")
                    }
                    onClicked: optionsMenu.open()
                    visible: simp_msg_browse.visible

                    Menu {

                        id: optionsMenu
                        x: parent.width - width
                        transformOrigin: Menu.TopRight
                        MenuItem {
                            text: "Show sent"
                            checked: filter_show_sent
                            checkable: true
                            onCheckedChanged: {
                                filter_show_sent = checked
                            }

                        }

                        MenuItem {
                            text: "Show Only Favorites"
                            checked: filter_show_only_fav
                            checkable: true
                            onCheckedChanged: {
                                filter_show_only_fav = checked
                            }

                        }

                        MenuItem {
                            text: "View profile"
                            onTriggered: {
                                stack.push(theprofileview)
                            }
                        }

                        MenuItem {
                            text: "Trash msgs"
                            onTriggered: {
                                confirm_trash.visible = true
                            }
                            MessageYN {
                                id: confirm_trash
                                title: "Trash all messages?"
                                
                                text: "Trash ALL messages from user?"
                                informativeText: "This KEEPS FAVORITE messages."
                                
                                onYesClicked: {
                                    themsglist.set_all_selected()
                                    themsglist.trash_all_selected()
                                    themsglist.invalidate_model_filter()
                                    themsglist.reload_model()
                                    close()
                                }
                                onNoClicked: {
                                    close()
                                }
                            }
                        }


//                        MenuItem {
//                            text: "Clear msgs"
//                            onTriggered: {
//                                confirm_delete2.visible = true
//                            }
//                            MessageDialog {
//                                id: confirm_delete2
//                                title: "Clear?"
//                                icon: StandardIcon.Question
//                                text: "Trash ALL messages from user?"
//                                informativeText: "This KEEPS FAVORITE messages."
//                                standardButtons: StandardButton.Yes | StandardButton.No
//                                onYes: {
//                                    core.clear_messages_unfav(simp_msg_browse.to_uid)
//                                    themsglist.reload_model()
//                                    close()
//                                    stack.pop()
//                                }
//                                onNo: {
//                                    close()
//                                }
//                            }
//                        }

//                        MenuItem {
//                            text: "Delete user"
//                            onTriggered: {
//                                confirm_delete.visible = true
//                            }
//                            MessageDialog {
//                                id: confirm_delete
//                                title: "Bulk delete?"
//                                icon: StandardIcon.Question
//                                text: "Delete ALL messages from user?"
//                                informativeText: "This REMOVES FAVORITE messages too."
//                                standardButtons: StandardButton.Yes | StandardButton.No
//                                onYes: {
//                                    core.delete_user(simp_msg_browse.to_uid)
//                                    themsglist.reload_model()
//                                    close()
//                                    stack.pop()
//                                }
//                                onNo: {
//                                    close()
//                                }
//                            }
//                        }
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



    Component {
        id: msgdelegate
        Rectangle {
            id: ditem
            width: grid.cellWidth
            height: grid.cellHeight
            radius: 3
            border.width: 1
            border.color: divider
            color: {(IS_QD === 1) ? "gray" : ((SENT === 0) ? accent : primary_light)}
            opacity: {multiselect_mode && SELECTED ? 0.5 : 1.0}
            z: 1
            clip: true
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

            Image {
                id: preview
                anchors.fill: parent
                visible: {HAS_ATTACHMENT}
                fillMode: Image.PreserveAspectFit
                // note: the extra "/" in file:// is to accomodate
                // windows which may return "c:/mumble"
                source: { PREVIEW_FILENAME !== "" ? (core.from_local_file(PREVIEW_FILENAME)) :
                //source: {PREVIEW_FILENAME != "" ? ("file://" + PREVIEW_FILENAME) :
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
            }

            Text {
                function gentext(msg) {
                    return msg
                    return "<html>" + msg + "</html>"
                }

                id: msg
                anchors.bottom: datetext.top
                anchors.left: parent.left
                anchors.right: parent.right
                //anchors.top: preview.visible ? undefined : parent.top
                //height: preview.visible ? parent.height : implicitHeight
                text: FETCH_STATE === "manual" ? "(click to fetch)" : gentext(String(MSG_TEXT))
                verticalAlignment: Text.AlignBottom
                wrapMode: preview.visible ? Text.NoWrap : Text.WordWrap
                elide: Text.ElideRight
                textFormat: Text.StyledText
                color: amber_light
                style: Text.Outline
                styleColor: "black"
                padding: 3

                clip: true
            }

            Text {
                function gendate(tm) {
                    var dt = new Date(tm * 1000)
                    if(Date.now() - dt.getTime() > 86400 * 1000) {
                        return Qt.formatDate(dt)
                    } else {
                        return Qt.formatTime(dt)
                    }

                }
                id: datetext
                anchors.bottom: parent.bottom
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.NoWrap
                color: primary_text
                clip: true
                font.italic: true
                //font.weight: Font.Light
                style: Text.Sunken
                styleColor: "white"
                scale: .75
                text: gendate(DATE_CREATED)
            }


            MouseArea {
                anchors.fill: parent
                enabled: !(optionsMenu.visible || moremenu.visible)
                onPressAndHold: {
                    console.log("click msg")
                    console.log(index)
                    grid.currentIndex = index
                    multiselect_mode = true
                    grid.model.toggle_selected(model.mid)
                    if(Qt.platform.os == "android") {
                        notificationClient.vibrate(50)
                    }
                }
                onClicked: {
                    grid.currentIndex = index
                    if(multiselect_mode) {
                        grid.model.toggle_selected(model.mid)
                        if(!grid.model.at_least_one_selected())
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
                            if(model.IS_FILE === 1) {
                                themsgview.view_source = model.PREVIEW_FILENAME === "" ? "" : (core.from_local_file(model.PREVIEW_FILENAME))
                                stack.push(themsgview)
                            }
                            else {
                                if(model.HAS_VIDEO === 1 || model.HAS_AUDIO === 1) {
                                    var vid = core.make_zap_view(model.mid)
                                    themsgview.view_id = vid
                                    //core.play_zap_view(vid)
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
    GridView {
        //width: 200; height: 400
        id: grid
        anchors.fill: parent
        cellWidth: 160 ; cellHeight: 140
        delegate: msgdelegate
        clip: true
        ScrollBar.vertical: ScrollBar { }
    }

}
