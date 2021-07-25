
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml 2.12
import QtQuick 2.12
import dwyco 1.0
import QtQuick.Controls 2.12

Page {
    id: contact_list
    //anchors.fill: parent
    property bool multiselect_mode : true

    header: SimpleToolbar {

    }

    Component {
        id: contact_list_delegate
        Item {
            height: drow.height
            width: parent.width
            Row {
                id: drow
                spacing: dp(5)
                width:parent.width

                Image {
                    width: dp(80)
                    height: dp(60)
                    source : {
                        "qrc:/new/red32/icons/red-32x32/exclamation-32x32.png"
                    }
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                    Image {
                        id: ggtiny
                        width: dp(32)
                        height: dp(32)
                        source: "qrc:/new/prefix1/icons/ggtiny.png"
                        anchors.top: parent.top
                        anchors.left: parent.right
                        visible: selected
                        z: 2
                    }

                }

                Text {
                    text: { display + ": " + phone + " : " + email }
                    anchors.verticalCenter: parent.verticalCenter
                    elide: Text.ElideRight
                    clip: true
                }
            }
            MouseArea {
                anchors.fill: drow
                onClicked: {
                    console.log("click")
                    console.log(index)
                    listView2.currentIndex = index
                    if(multiselect_mode) {
                        listView2.model.toggle_selected(index)

                    }
                }
            }

        }

    }

    onMultiselect_modeChanged: {
        listView2.model.set_all_selected(false)
    }

    DwycoSimpleContactModel {
        id: user_model

    }

    onVisibleChanged: {
        if(visible) {
            user_model.load_users_to_model()
        }
    }

    ListView {
        id: listView2
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right:parent.right
        anchors.bottom: send_button.top

        model: user_model
        delegate: contact_list_delegate
        clip: true
        spacing: 5
        ScrollBar.vertical: ScrollBar {
            background: Rectangle {
                color: "green"
            }
        }

    }
    Button {
        id:send_button
        //anchors.top: listView2.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: implicitHeight
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
        text: send_button.enabled ? "Query" : "Select Contact(s)"

        onClicked: {
            for(var i = 0; i < user_model.count; ++i) {
                if(user_model.get(i).selected) {
                    var recip_uid = user_model.get(i).email
                    console.log("selected " + recip_uid)
                    //core.send_forward(recip_uid, uid_folder, mid_to_forward)
                }
            }
            user_model.send_query()
            // for some reason, when user_model is a subclass
            // of sortproxymodel, the javascript above doesn't
            // work. it displays properly and whatnot, but the count
            // doesn't work, and i don't get a error. oh well, the
            // sort proxy doesn't play well since you have to forward stuff
            // out to the source model which is tedious.
            // instead, i just implement the send in c++ in the model
            //user_model.send_forward_selected(uid_folder, mid_to_forward)
            //
            stack.pop()
        }

    }

}
