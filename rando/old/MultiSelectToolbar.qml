
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import dwyco
import QtQuick.Controls.Material
import QtQuick.Layouts
//import QtQuick.Dialogs
//import Qt.labs.platform

ToolBar {
    property Component extras
    property alias delete_warning_text : confirm_delete.text
    property alias delete_warning_inf_text: confirm_delete.informativeText

    background: Rectangle {
        color: primary_light
    }

    implicitWidth: parent.width


    RowLayout {

        Layout.margins: mm(5)
        Item {
            Layout.minimumHeight: cm(1)
        }
        anchors.fill: parent
        spacing: mm(2)

        ToolButton {
            id: back_button
            contentItem: Image {
                anchors.centerIn: parent
                source: mi("ic_arrow_back_black_24dp.png")
            }
            checkable: false
            onClicked: {
                //stack.pop()
                // exit multiselect
                multiselect_mode = false
            }
            Layout.fillHeight: true

        }


        Item {

            Layout.fillWidth: true
        }

        ToolButton {
            contentItem: Image {
                anchors.centerIn: parent
                source: mi("ic_star_black_24dp.png")
            }

            Layout.fillHeight: true

            onClicked: {
                // add all to favorites somehow
                star_fun(true)
                multiselect_mode = false
            }
        }

        Item {

            Layout.fillWidth: true
        }

        ToolButton {
            contentItem: Image {
                anchors.centerIn: parent
                source: mi("ic_delete_black_24dp.png")
            }

            onClicked: {
                // remove whatever is selected
                confirm_delete.visible = true
            }
            MessageYN {
                id: confirm_delete
                title: "Bulk delete?"
                //icon: StandardIcon.Question
                text: "Delete ALL messages from selected users?"
                informativeText: "This REMOVES FAVORITE messages too."
                detailedText: "This is what you use to completely obliterate all messages from selected users, including favorite and hidden messages."
                //detailedText: informativeText
                //buttons: MessageDialog.Yes | MessageDialog.No
//                onButtonClicked: (button, role)=> {
//                                     switch(button) {
//                                         case MessageDialog.Yes: {
//                                             model.delete_all_selected()
//                                             multiselect_mode = false
//                                             close()
//                                             break
//                                         }
//                                         case MessageDialog.No: {
//                                             close()
//                                             break
//                                         }
//                                     }
//                                 }

                onYesClicked: {
                    model.delete_all_selected()
                    multiselect_mode = false
                    close()
                }
                onNoClicked: {
                    close()
                }
            }

            Layout.fillHeight: true
        }


        Loader {
            id: extras_loader
            sourceComponent: extras
        }

    }



}