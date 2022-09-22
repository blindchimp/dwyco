
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import dwyco 1.0
import QtQuick.Controls 2.12

Rectangle {
    id: userlist_top
   anchors.fill: parent

   signal uid_selected(string uid, string action)

   Component {
       id: userlist_delegate
       Item {
           height: drow.height
           width: ListView.view.width
           Row {
               id: drow
               spacing: dp(5)
                width:parent.width

               Image {
                   width: dp(80)
                   height: dp(60)
                   source : {
                       
                       core.uid_to_profile_preview(uid)
                    }
//                   source: {
//                       (PROFILE_REVIEWED == 1 && PROFILE_REGULAR == 1) ?
//                                   core.uid_to_profile_preview(uid) :
//                                    "qrc:/new/red32/icons/red-32x32/exclamation-32x32.png"
//                   }
                   anchors.verticalCenter: parent.verticalCenter
                   fillMode: Image.PreserveAspectFit
                   Image {
                       id: ggtiny
                       width: dp(32)
                       height: dp(32)
                       source: "qrc:/new/prefix1/icons/ggtiny.png"
                       anchors.top: parent.top
                       anchors.left: parent.left
                       visible: {IS_PAL == 1 ? true : false}
                       z: 2
                   }
                   Image {
                       id: has_msgs
                       width: dp(32)
                       height: dp(32)
                       source: "qrc:/new/red32/icons/red-32x32/arrow right-32x32.png"
                       z:2
                       anchors.top:parent.top
                       anchors.left:ggtiny.right
                       visible: {HAS_UNVIEWED == 1 ? true : false }

                       RotationAnimation on rotation {
                           id: cw
                           running: has_msgs.visible && !ccw.running
                           to: 20
                           duration: 200
                           loops: 1
                           direction: RotationAnimation.Clockwise
                           //easing {type: Easing.OutBack; }
                       }
                       RotationAnimation on rotation {
                           id: ccw
                           running: has_msgs.visible && !cw.running
                           from: 20
                           to: 340
                           duration: 200
                           loops: 1
                           direction: RotationAnimation.Counterclockwise
                           //easing {type: Easing.OutBack; }
                       }
//                       {
//                           id: animate_rotation
//                           target: has_msgs
//                           properties: "rotation"
//                           from: -10
//                           to: 10
//                           loops: Animation.Infinite
//                           easing {type: Easing.OutBack; overshoot: 500}
//                       }
                   }
               }

               Text {
                   id: display_name
                   color: {(IS_IN_LOBBY == 1 || IS_ONLINE == 1) ? "red" : "black"}
                   font.italic: {IS_ACTIVE ? true : false}
                   text: display
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

                   uid_selected(uid, "clicked")

               }
               onPressAndHold:  {
                   listView2.currentIndex = index
                   //core.set_pal(uid, core.get_pal(uid) == 0 ? 1 : 0)
                   uid_selected(uid, "hold")
               }


           }

       }

   }

   Component.onCompleted: {
       userlist_top.uid_selected.connect(top_dispatch.uid_selected)
   }

   ListView {
       id: listView2
        anchors.fill:parent

       model: UserListModel
       delegate: userlist_delegate
       clip: true
       ScrollBar.vertical: ScrollBar { }

   }

}
