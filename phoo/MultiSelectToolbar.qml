
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
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3

ToolBar {
    property Component extras
    property alias delete_warning_text : confirm_delete.text
    property alias delete_warning_inf_text: confirm_delete.informativeText
    property url star_icon: mi("ic_star_black_24dp.png")
    property bool is_trash: false

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
                source: star_icon
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
            MessageDialog {
                id: confirm_delete
                title: "Bulk Trash"
                icon: StandardIcon.Question
                text: "Trash ALL messages from selected users?"
                informativeText: "This KEEPS FAVORITES, but TRASHES HIDDEN messages."
                standardButtons: StandardButton.Yes | StandardButton.No
                onYes: {
                    if(is_trash) {
                        model.obliterate_all_selected()
                    } else {
                        model.trash_all_selected()
                    }
                    model.invalidate_model_filter()
                    // ACK, this is a kluge. this just gets the model that
                    // might be loaded with a uid we happen to have modified
                    // during the trash operation.
                    themsglist.invalidate_model_filter()
                    multiselect_mode = false
                    close()
                }
                onNo: {
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
