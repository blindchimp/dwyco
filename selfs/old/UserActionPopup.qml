
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12

Rectangle {
    id: menu_rect
    anchors.centerIn: parent
    //anchors.margins: 100
    width: 200
    height: 200
    z: 5

    property string uid
    signal user_action(string uid, string action)

    ListModel {
        id: actions
        ListElement {
            name: "View profile"
        }

        ListElement {
            name: "Clear"
        }
        ListElement {
            name: "Delete"
        }
        ListElement {
            name: "Block"
        }
    }

    Component {
        id: entry
        Rectangle {
            id: touchy
            //anchors.fill: parent
            border.color: "red"
            border.width: 3
            height:  menuitem.implicitHeight + 40
            width: parent.width

            Text {
                id: menuitem
                width: parent.width
                text: name
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    stack.pop()
                    user_action(uid, name)

                    //menu_rect.visible = false
                }


            }
        }
    }

    onUser_action: {
        if(action == "Clear") {
            core.clear_messages(uid)
            themsglist.reload_model()
        }
        else if(action == "Delete")
        {
            core.delete_user(uid)
            themsglist.reload_model()
            //core.power_clean()
        }
        else if(action == "View profile")
        {
            stack.push(theprofileview)
        }
        else if(action == "Block")
        {
            core.set_ignore(uid, 1)
            the_ignore_list.reload_model()

        }

    }

    ListView {
        model: actions
        delegate: entry
        anchors.fill: parent
    }


}

