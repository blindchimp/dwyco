
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import dwyco 1.0
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.12
//import QtGraphicalEffects 1.0

Item {
    id: bare_conv_list
    //anchors.fill: parent
    property bool multiselect_mode : false
    property var model

   signal uid_selected(string uid, string action)

    function star_fun(b) {
        console.log("convlist star")
        ConvListModel.pal_all_selected(b)
    }

   Component {
       id: convlist_delegate

       Rectangle {
           height: 30 //vh(pct)
           width: ListView.view.width
           opacity: {multiselect_mode && selected ? 0.5 : 1.0}
           color: primary_dark
           border.width: 6
           border.color: primary_dark
//           gradient: Gradient {
//               GradientStop { position: 0.0; color: primary_light }
//               GradientStop { position: 1.0; color: primary_dark}
//           }

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
                   Layout.minimumWidth: 24
                   Layout.maximumWidth: 24
                   Layout.minimumHeight: 24
                   Layout.maximumHeight: 24
                   Layout.margins: mm(.125)

                   Image {
                       id: ggtiny
                       width: .3 * ppic.height
                       height: .3 * ppic.height
                       source: "qrc:/new/prefix1/icons/ggtiny.png"
                       anchors.top: parent.top
                       anchors.left: has_msgs.right
                       visible: selected
                       z: 3
                       opacity: 1.0
                   }
                   Image {
                       id: has_msgs
                       width: .3 * ppic.height
                       height: .3 * ppic.height
                       source: "qrc:/new/red32/icons/red-32x32/arrow right-32x32.png"
                       z:3
                       anchors.top:parent.top
                       anchors.left:parent.left
                       visible: any_unread
                   }
                   Rectangle {
                       id: ispal
                       width: .3 * ppic.height
                       height: .3 * ppic.height
                       anchors.top: parent.top
                       anchors.left: parent.left
                       visible: pal
                       z: 2
                       color: primary_light
                       radius: width / 2
                       Image {
                           anchors.fill: parent
                           anchors.margins: 2
                           source: mi("ic_star_black_24dp.png")
                       }
                   }
                   Rectangle {
                       id: blocked
                       width: .3 * ppic.height
                       height: .3 * ppic.height
                       anchors.top: parent.top
                       anchors.left: parent.left
                       visible: is_blocked
                       z: 2
                       color: primary_dark
                       radius: width / 2
                       Image {
                           anchors.fill: parent
                           anchors.margins: 2
                           source: mi("ic_visibility_off_white_24dp.png")
                       }
                   }
                   Rectangle {
                       id: hidden
                       width: .3 * ppic.height
                       height: .3 * ppic.height
                       anchors.right:parent.right
                       anchors.top:parent.top
                       visible: show_hidden && has_hidden
                       z: 2
                       radius: width
                       color: "orange"
                   }
               }

               Text {
                   text: display
                   elide: Text.ElideRight
                   clip: true
                   Layout.alignment: Qt.AlignLeft
                   Layout.fillWidth: true
                   color: "white"
                   font.weight: Font.Light
               }
           }
           MouseArea {
               anchors.fill: drow
               acceptedButtons: Qt.LeftButton|Qt.RightButton
               onClicked: {
                   console.log("click")
                   console.log(index)
                   listView2.currentIndex = index
                   if(multiselect_mode) {
                       listView2.model.toggle_selected(uid)
                       if(!listView2.model.at_least_one_selected())
                           multiselect_mode = false
                   }   else {                     
                       if(mouse.button === Qt.LeftButton) {
                           uid_selected(uid, "clicked-nopush")
                       } else if(mouse.button === Qt.RightButton) {
                           uid_selected(uid, "hold")
                       }
                   }

               }
               onPressAndHold:  {
                   listView2.currentIndex = index
                   multiselect_mode = true
                   listView2.model.toggle_selected(uid)
                   if(Qt.platform.os == "android") {
                   notificationClient.vibrate(50)
                   }
               }


           }

       }

   }
   
   onMultiselect_modeChanged: {
       listView2.model.set_all_selected(false)
   }

   Component.onCompleted: {
       model = ConvListModel
       bare_conv_list.uid_selected.connect(top_dispatch.uid_selected)
   }

   onVisibleChanged: {
       multiselect_mode = false
   }

   Connections {
       target: top_dispatch
       function onUid_selected(uid, action) {
           var i
           i = listView2.model.get_by_uid(uid)
           listView2.currentIndex = i
       }
   }
   
   Rectangle {
       color: "dimgray"
       anchors.fill: parent
   ListView {
       id: listView2
       anchors.fill:parent

       //visible: !show_grid.grid_checked

       model: ConvListModel
       delegate: convlist_delegate
       clip: true
       highlight: Rectangle { z:3 ; color: primary_light; opacity: .3}
       highlightMoveDuration: 200
       highlightMoveVelocity: -1
       //spacing: 3

       ScrollBar.vertical: ScrollBar { 
           background: Rectangle {
               color: "lightgreen"
           }

       }

   }
   }

   Label {
       id: empty_help
       anchors.fill: parent

       anchors.margins: mm(3)
       wrapMode: Text.WordWrap

       text: qsTr("(NO DM's yet)")
       z: 5
       visible: {ConvListModel.count === 0}
   }

}
