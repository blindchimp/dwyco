
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

Page {
    property alias model: grid.model

    property bool multiselect_mode: false

    function star_fun(b) {
        console.log("untrash star")
        model.untag_all_selected("_trash")

    }

    onMultiselect_modeChanged: {
        model.set_all_unselected()
    }
    onVisibleChanged: {
        multiselect_mode = false
        if(visible) {
            themsglist.set_filter(1, 1, -1, 0);
            themsglist.set_show_hidden(1)
            themsglist.set_show_trash(true)
            themsglist.tag = "_trash"
        } else {
            themsglist.set_show_trash(false)
            themsglist.set_show_hidden(show_hidden)
            themsglist.tag = ""
            ConvListModel.reload_convlist()
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
                    text: "Untrash"
                    onTriggered: {
                        model.untag_all_selected("_trash")
                        multiselect_mode = false
                    }
                }
                MenuItem {
                    text: "Select All"
                    onTriggered: {
                        model.set_all_selected()
                    }
                }
                MenuItem {
                    text: "Delete forever"
                    onTriggered: {
                        confirm_delete.visible = true
                    }
                    MessageYN {
                        id: confirm_delete
                        title: "Delete selected msgs?"
                        text: "Delete selected msgs on ALL devices?"
                        informativeText: "No UNDO"

                        onYesClicked: {
                            model.obliterate_all_selected()
                            //model.tag_all_selected("_hid")
                            multiselect_mode = false
                            close()
                        }
                        onNoClicked: {
                            close()
                        }
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
            delete_warning_inf_text: "Deletes on ALL your devices"
            delete_warning_text: "Delete selected messages FOREVER?"
            star_icon: mi("delete-restore-black.png")
            is_trash: true
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
                        stack.pop()
                    }
                    Layout.fillHeight: true
                }

                Item {
                    id: prof
                    //color: "red" //accent
                    //border.width: 1
                    //radius: 3

                    Layout.fillHeight: true
                    Layout.minimumHeight: cm(1)
                    //Layout.leftMargin: 0



                    Text {
                        id:top_toolbar_text
                        //width: dp(160)
                        clip: true
                        anchors.leftMargin: 2
                        fontSizeMode: Text.Fit
                        anchors.top: parent.top
                        //anchors.bottom: parent.bottom
                        anchors.left: prof.left
                        text: "Trash"
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
                    visible: trash_browse.visible

                    Menu {

                        id: optionsMenu
                        x: parent.width - width
                        transformOrigin: Menu.TopRight


                        MenuItem {
                            text: "Empty Trash"
                            onTriggered: {
                                confirm_empty.visible = true
                            }
                            MessageYN {
                                id: confirm_empty
                                title: "Empty trash?"
                                text: "Delete ALL trash on ALL devices?"
                                informativeText: "No UNDO"

                                onYesClicked: {
                                    model.set_all_selected()
                                    model.obliterate_all_selected()
                                    close()
                                }
                                onNoClicked: {
                                    close()
                                }
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
                visible: {(HAS_VIDEO === 1 && HAS_SHORT_VIDEO === 0) && IS_QD === 0}
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
                //source: {PREVIEW_FILENAME !== "" ? ("file://" + PREVIEW_FILENAME) :
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
                            themsgview.is_trash = true
                            themsgview.msg_text = model.MSG_TEXT
                            themsgview.view_id = -1
                            themsgview.mid = model.mid
                            themsgview.uid = model.ASSOC_UID
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
