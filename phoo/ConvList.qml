
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
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls
//import Qt.labs.platform as Mumble

Page {
    id: convlist_top
    //anchors.fill: parent
    property bool multiselect_mode : false
    property var model

   signal uid_selected(string uid, string action)

    function star_fun(b) {
        console.log("convlist star")
        ConvListModel.pal_all_selected(b)
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
                //transformOrigin: Menu.TopRight
                MenuItem {
                    text: "Unfavorite"
                    onTriggered: {
                        star_fun(false)
                        multiselect_mode = false
                    }
                }

                MenuItem {
                    text: "Select All"
                    onTriggered: {
                        ConvListModel.set_all_selected(true)
                    }
                }

                MenuItem {
                    text: "Block"
                    onTriggered: {
                        ConvListModel.block_all_selected()
                        multiselect_mode = false

                    }
                }

                MenuItem {
                    text: "Block and delete"
                    onTriggered: {
                        confirm_delete.visible = true
                    }
                    MessageYN {
                        id: confirm_delete
                        title: "Block and Bulk delete?"
                        //icon: StandardIcon.Question
                        text: "Delete ALL messages from selected users?"
                        informativeText: "This removes FAVORITE messages too."
                        //buttons: Mumble.MessageDialog.Yes | Mumble.MessageDialog.No
                        onYesClicked: {
                            ConvListModel.block_all_selected()
                            ConvListModel.delete_all_selected()
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
        width: parent.width
        MultiSelectToolbar {
            id: multi_toolbar
            visible: multiselect_mode
            extras: extras_button
        }
        ToolBar {
            id: regular_toolbar
            background: Rectangle {
                color: accent
            }
            visible: !multiselect_mode

            implicitWidth: parent.width

            RowLayout {
                Item {
                    Layout.minimumHeight: cm(1)
                }
                anchors.fill: parent
                //spacing: mm(5)

                ToolButton {
                    contentItem: Image {
                        source: mi("ic_menu_black_24dp.png")
                        anchors.centerIn: parent
                    }
                    onClicked: drawer.open()
                    visible: stack.depth === 1
                    Layout.fillHeight: true
                }
                Item {

                    Layout.fillWidth: true
                }

                GridToggle {
                    id: show_grid
                    Layout.fillHeight: true
                }
                Item {

                    Layout.fillWidth: true
                }


                ToolButton {

                    id: trivia
                    text: "Trivia"
                    background: Rectangle {
                        color: primary_dark
                        radius: 3
                        border.color: core.is_chat_online === 0 ? "black" : "limegreen"
                        border.width: core.is_chat_online === 0 ? 0 : 2
                    }
                    contentItem: Text {
                        x: parent.leftPadding
                        y: parent.topPadding
                        width: parent.availableWidth
                        height: parent.availableHeight

                        text: parent.text
                        font: parent.font
                        color: "white"
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    checkable: false

                    onClicked: {
                        stack.push(public_chat)
                    }

                }
                Item {

                    Layout.fillWidth: true
                }

                ToolButton {

                    id: dir_button
                    contentItem: Image {
                        source:  mi("ic_public_black_24dp.png")
                        anchors.centerIn: parent
                    }
                    checkable: false

                    onClicked: {
                        stack.push(simpdir_rect)
                    }
                }
                Item {

                    Layout.fillWidth: true
                }
                ToolButton {

                    id: clist_button
                    contentItem: Image {
                        source:  mi("ic_people_black_24dp.png")
                        anchors.centerIn: parent
                    }
                    checkable: false

                    onClicked: {
                        stack.push(cqres)
                    }
                }

            }
        }
    }


   Component {
       id: convlist_delegate

       Rectangle {
           height: vh(pct)
           width: ListView.view.width
           opacity: {multiselect_mode && selected ? 0.5 : 1.0}
           color: primary_dark
           border.width: 1
           gradient: Gradient {
               GradientStop { position: 0.0; color: primary_light }
               GradientStop { position: 1.0; color: primary_dark}
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
                       (!invalid && !is_blocked && ((REVIEWED && REGULAR) || show_unreviewed) && resolved_counter > -1) ?
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
                       width: 16
                       height: 16
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
                   font: applicationWindow1.font
                   Layout.alignment: Qt.AlignLeft
                   Layout.fillWidth: true

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
                           uid_selected(uid, "clicked")
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
       convlist_top.uid_selected.connect(top_dispatch.uid_selected)
   }

   onVisibleChanged: {
       if(visible) {
           Qt.inputMethod.hide()
       }
       multiselect_mode = false
   }
   
   
   ListView {
       id: listView2
       anchors.fill:parent

       visible: !show_grid.grid_checked

       model: ConvListModel
       delegate: convlist_delegate
       clip: true
       //spacing: 5
       ScrollBar.vertical: ScrollBar { 
           background: Rectangle {
               color: "green"
           }

       }

   }

   Component {
       id: convgrid_delegate

       Rectangle {
           id: bgrec
           height: gridView1.cellHeight
           width: gridView1.cellWidth

           opacity: {multiselect_mode && selected ? 0.5 : 1.0}
           color: primary_dark
           //scale: .5
           border.width: 1
           gradient: Gradient {
               GradientStop { position: 0.0; color: primary_light }
               GradientStop { position: 1.0; color: primary_dark}
           }

           CircularImage {
               id: ppic
               anchors.centerIn: parent
               source : {
                   (!invalid && !is_blocked && ((REVIEWED && REGULAR) || show_unreviewed) && resolved_counter > -1) ?
                               core.uid_to_profile_preview(uid) :
                               "qrc:/new/red32/icons/red-32x32/exclamation-32x32.png"
               }
               //anchors.verticalCenter: parent.verticalCenter
               fillMode: Image.PreserveAspectCrop
               height:parent.height
               width: parent.height
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
                   id: ghidden
                   width: 16
                   height: 16
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
               anchors.bottom: parent.bottom
               width: parent.width
               color: amber_light
           }

           MouseArea {
               anchors.fill: parent
               acceptedButtons: Qt.LeftButton|Qt.RightButton
               onClicked: (mouse)=> {
                   console.log("click")
                   console.log(index)
                   gridView1.currentIndex = index
                   if(multiselect_mode) {
                       gridView1.model.toggle_selected(uid)
                       if(!gridView1.model.at_least_one_selected())
                           multiselect_mode = false
                   }   else {
                       if(mouse.button === Qt.LeftButton) {
                           uid_selected(uid, "clicked")
                       } else if(mouse.button === Qt.RightButton) {
                           uid_selected(uid, "hold")
                       }
                   }

               }
               onPressAndHold:  {
                   gridView1.currentIndex = index
                   multiselect_mode = true
                   gridView1.model.toggle_selected(uid)
                   if(Qt.platform.os == "android") {
                       notificationClient.vibrate(50)
                   }
               }
           }
       }
   }

   GridView {
       id: gridView1
       anchors.fill:parent
       cellWidth: 80 ; cellHeight: 80

       visible: show_grid.grid_checked

       model: ConvListModel
       delegate: convgrid_delegate
       clip: true
       //spacing: 5
       ScrollBar.vertical: ScrollBar {
           background: Rectangle {
               color: "green"
           }

       }
       // i can't get any of this to work on linux for some reason
//       move: Transition {
//                 NumberAnimation { properties: "x,y"; duration: 5000 }
//             }
//       moveDisplaced: Transition {
//                 NumberAnimation { properties: "x,y"; duration: 5000 }
//             }
//       populate: Transition {
//                 NumberAnimation { properties: "x,y"; duration: 1000 }
//             }

   }

   Label {
       id: empty_help
       anchors.fill: parent

       anchors.margins: mm(3)
       wrapMode: Text.WordWrap

       text: qsTr("(This is your list of people you have exchanged messages with... and it is empty! Click icons above to find more people.)")
       z: 5
       visible: {ConvListModel.count === 0}
   }

}
