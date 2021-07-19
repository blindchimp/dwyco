
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
    anchors.fill: parent

    header: SimpleToolbar {
        extras: extra_button
    }

    Component {
        id: extra_button


        ToolButton {
            contentItem: Image {
                anchors.centerIn: parent
                source: mi("ic_action_overflow.png")
            }
            onClicked: optionsMenu.open()
            visible: parent.visible

            Menu {

                id: optionsMenu
                x: parent.width - width
                transformOrigin: Menu.TopRight

                MenuItem {
                    text: "Unblock all"
                    onTriggered: {
                        console.log("clear")
                        core.clear_ignore_list()
                    }
                }
            }
        }
    }
   
    Component {
        id: iglist_delegate
        Rectangle {
            id: drow
            width: parent.width
            height: mm(9)
            //border.width: 1
            anchors.margins: 2
            Text {
                text: display
                clip: true
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                elide: Text.ElideRight
                anchors.leftMargin: mm(1)
            }
            Button {
                //height: implicitWidth
                text: "x"
                onClicked: {
                    console.log("unignore " + uid)
                    core.set_ignore(uid, 0)
                }
                anchors.right: drow.right
                //anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 2
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: height
                background: Rectangle {
                    color: "red"
                    radius: 3
                }
                
            }
        }

    }

    ListView {
        id: listView1
        anchors.fill: parent
        delegate: iglist_delegate
        clip: true
        spacing: mm(1)
        model: IgnoreListModel
        ScrollBar.vertical: ScrollBar { }
    }

    Label {
        id: empty_help
        anchors.fill: parent

        anchors.margins: mm(3)
        wrapMode: Text.WordWrap

        text: qsTr("(Your block list is empty.)")
        z: 5
        visible: {IgnoreListModel.count === 0}


    }


}

