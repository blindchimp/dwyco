/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// this is just a list of users currently in the chat room.
// on mobile, this is presented as one page. on desktop, it can be
// attached to a chatbox or something as a side-bar, since you have more
// room.

//import QtQml
import QtQuick
import dwyco
import QtQuick.Layouts
import QtQuick.Controls

Item {
    signal uid_selected(string uid, string action)

    Component {
        id: chatlist_delegate
        Rectangle {
            property bool censor_it
            censor_it: censor && !regular_profile(REVIEWED, REGULAR)
            height: picht()
            width: ListView.view.width
            //opacity: {multiselect_mode && selected ? 0.5 : 1.0}
            color: primary_dark
            border.width: 1
            gradient: Gradient {
                GradientStop { position: 0.0; color: amber_light }
                GradientStop { position: 1.0; color: amber_dark}
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
                        (!invalid && (!censor_it && resolved_counter > -1)) ?
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
                        id: has_msgs
                        width: .3 * ppic.height
                        height: .3 * ppic.height
                        source: "qrc:/new/red32/icons/red-32x32/arrow right-32x32.png"
                        z:3
                        anchors.top:parent.top
                        anchors.left:parent.left
                        visible: any_unviewed
                    }
                }

                Text {
                    id: display_name
                    color: {active ? "red" : "black"}
                    font.italic: {active ? true : false}
                    text: censor_it ? censor_name(display) : display
                    //anchors.verticalCenter: parent.verticalCenter

                    elide: Text.ElideRight
                    clip: true
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true
                    font.pixelSize: applicationWindow1.font.pixelSize

                }
            }
            MouseArea {
                anchors.fill: drow
                acceptedButtons: Qt.LeftButton|Qt.RightButton
                onClicked: (mouse) => {
                    console.log("click")
                    console.log(index)
                    listView2.currentIndex = index
                    if(mouse.button === Qt.LeftButton) {
                        uid_selected(uid, "clicked")
                    } else if(mouse.button === Qt.RightButton) {
                        uid_selected(uid, "hold")
                    }
                }
                onPressAndHold:  {
                    listView2.currentIndex = index
                    uid_selected(uid, "hold")
                }
            }

        }

    }

    Component.onCompleted: {
        uid_selected.connect(top_dispatch.uid_selected)

    }


    Connections {
        target: Core
        function onSys_invalidate_profile(uid) {
            console.log("chatlist invalidate " + uid)
        }
    }

    ListView {
        id: listView2
        anchors.fill:parent

        model: ChatListModel
        delegate: chatlist_delegate
        clip: true
        //spacing: 5
        ScrollBar.vertical: ScrollBar { }

    }

}
