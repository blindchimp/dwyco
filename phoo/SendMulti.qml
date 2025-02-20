
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml
import QtQuick
import dwyco
import QtQuick.Layouts
import QtQuick.Controls

Page {
    id: send_multi_page
    //anchors.fill: parent
    property bool multiselect_mode : true
    property string mid_to_forward

    header: SimpleToolbar {

    }
    // note: this is needed since visible doesn't seem to
    // propagated if you use this via a loader
    Component.onCompleted: {
        user_model.load_users_to_model()
        multiselect_mode = true
    }

    Component {
        id: forward_list_delegate
        Rectangle {
            height: vh(pct)
            width: ListView.view.width
            opacity: {multiselect_mode && selected ? 0.5 : 1.0}
            color: primary_dark
            border.width: 1
            gradient: Gradient {
                GradientStop { position: 0.0; color: "deeppink" }
                GradientStop { position: 1.0; color: "lightgrey"}
            }

            RowLayout {
                id: drow
                spacing: mm(1)
                anchors.fill: parent

                CircularImage2 {
                    id: ppic
                    //width: dp(80)
                    //height: dp(60)
                    source : {
                        (!invalid && ((REVIEWED && REGULAR) || show_unreviewed) && resolved_counter > -1) ?
                                    Core.uid_to_profile_preview(uid) :
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
                    text: display
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
            user_model.load_users_to_model()
            multiselect_mode = true
        } else {
            multiselect_mode = false
            mid_to_forward = ""
        }
    }

    ListView {
        id: listView2
        anchors.fill: parent
        model: user_model
        delegate: forward_list_delegate
        clip: true
        //spacing: 5
        ScrollBar.vertical: ScrollBar {
            background: Rectangle {
                color: "green"
            }
        }
    }

    Button {
        id:send_button
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

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
        text: send_button.enabled ? "Send" : "Select Recipients"

        onClicked: {
            console.log("MULTI COUNT ", user_model.count)
            for(var i = 0; i < user_model.count; ++i) {
                if(user_model.at(i).selected) {
                    var recip_uid = user_model.at(i).uid
                    Core.send_report(recip_uid)
                    console.log(" selected ", recip_uid)
                }
            }
            themsglist.reload_model()
            //chatbox.listview.positionViewAtBeginning()
            //
            stack.pop()
        }

    }


}
