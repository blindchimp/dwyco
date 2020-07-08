
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import dwyco 1.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12

Page {
    id: forward_list
    //anchors.fill: parent
    property bool multiselect_mode : true
    property string mid_to_forward
    property string uid_folder
    property int limited_forward: 0

    header: SimpleToolbar {

    }

    Component {
        id: forward_list_delegate
        Rectangle {
            height: vh(pct)
            width: parent.width
            opacity: {multiselect_mode && selected ? 0.5 : 1.0}
            color: primary_dark
            border.width: 1
            gradient: Gradient {
                GradientStop { position: 0.0; color: "lightgreen" }
                GradientStop { position: 1.0; color: "lightgrey"}
            }

            RowLayout {
                id: drow
                spacing: mm(1)
                anchors.fill: parent

                CircularImage {
                    id: ppic
                    //width: dp(80)
                    //height: dp(60)
                    source : {
                        (!invalid && ((REVIEWED && REGULAR) || show_unreviewed) && resolved_counter > -1) ?
                                    core.uid_to_profile_preview(uid) :
                                    "qrc:/new/red32/icons/red-32x32/exclamation-32x32.png"
                    }
                    //anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectCrop
                    Layout.minimumWidth: picht()
                    Layout.maximumWidth: picht()
                    Layout.minimumHeight: picht()
                    Layout.maximumHeight: picht()
                    Layout.margins: mm(.125)

                    Image {
                        id: ggtiny
                        width: .3 * ppic.height
                        height: .3 * ppic.height
                        source: "qrc:/new/prefix1/icons/ggtiny.png"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        visible: selected
                        z: 3
                        opacity: 1.0
                    }

                }

                Text {
                    //id: display_name
                    //color: {(IS_IN_LOBBY == 1 || IS_ONLINE == 1) ? "red" : "black"}
                    //font.italic: {IS_ACTIVE ? true : false}
                    text: display
                    //anchors.verticalCenter: parent.verticalCenter

                    elide: Text.ElideRight
                    clip: true
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true

                }
            }
            MouseArea {
                anchors.fill: drow
                onClicked: {
                    console.log("click")
                    console.log(index)
                    listView2.currentIndex = index
                    if(multiselect_mode) {
                        listView2.model.toggle_selected(uid)

                    }
                }
            }

        }

    }

    onMultiselect_modeChanged: {
        listView2.model.set_all_selected(false)
    }

    DwycoSimpleUserModel {
        id: user_model

    }

    onVisibleChanged: {
        if(visible) {
            limited_forward = core.flim(uid_folder, mid_to_forward)
            if(limited_forward) {
                user_model.load_admin_users_to_model()
            } else {
                user_model.load_users_to_model()
            }
            multiselect_mode = true
        } else {
            multiselect_mode = false
            uid_folder = ""
            mid_to_forward = ""
        }
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: mm(2)

        ListView {
            id: listView2

            model: user_model
            delegate: forward_list_delegate
            clip: true
            //spacing: 5
            ScrollBar.vertical: ScrollBar {
                background: Rectangle {
                    color: "green"
                }
            }
            Layout.fillHeight: true
            Layout.fillWidth: true

        }

        Button {
            id:send_button
            Layout.fillWidth: true
            Layout.margins: mm(5)
            enabled: {user_model.selected_count > 0 ? true : false}

            background: Rectangle {
                id: bg
                color: "indigo"
                radius: 20
            }
            contentItem: Text {
                color: send_button.enabled ? "white" : "gray"
                text: send_button.text
                anchors.centerIn: bg
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
            text: send_button.enabled ? "Send" : (limited_forward ? "Select Admin" : "Select Recipients")

            onClicked: {
                //            for(var i = 0; i < user_model.count; ++i) {
                //                if(user_model.get(i).selected) {
                //                    var recip_uid = user_model.get(i).uid
                //                    core.send_forward(recip_uid, uid_folder, mid_to_forward)
                //                }
                //            }
                // for some reason, when user_model is a subclass
                // of sortproxymodel, the javascript above doesn't
                // work. it displays properly and whatnot, but the count
                // doesn't work, and i don't get a error. oh well, the
                // sort proxy doesn't play well since you have to forward stuff
                // out to the source model which is tedious.
                // instead, i just implement the send in c++ in the model
                user_model.send_forward_selected(uid_folder, mid_to_forward)
                themsglist.reload_model()
                chatbox.listview.positionViewAtBeginning()
                //
                stack.pop()
            }

        }
    }

}
