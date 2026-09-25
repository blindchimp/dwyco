
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// The bar that replaces the normal toolbar while things are multi-selected.
//
// The two operations are injected rather than picked out of the air: the
// hosting page passes ops.* functions in. They used to be found implicitly by
// reaching for whatever "model" and "star_fun" happened to be visible from the
// context chain, which silently did nothing on the conversation list, where
// there is no message model at all.

import QtQuick
import dwyco
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ToolBar {
    property Component extras
    property var bulk_trash_op
    property var bulk_star_op
    property alias delete_warning_text : confirm_delete.text
    property alias delete_warning_inf_text: confirm_delete.informativeText
    property url star_icon: mi("ic_star_black_24dp.png")
    property string star_tooltip: qsTr("Favorite selected")
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
            ToolTip.text: star_tooltip

            onClicked: {
                if(typeof bulk_star_op === "function")
                    bulk_star_op()
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

            Layout.fillHeight: true
            ToolTip.text: is_trash ? qsTr("Delete forever") : qsTr("Trash")

            onClicked: {
                confirm_delete.visible = true
            }
            MessageYN {
                id: confirm_delete
                title: is_trash ? qsTr("Delete forever?") : qsTr("Bulk Trash?")

                onYesClicked: {
                    if(typeof bulk_trash_op === "function")
                        bulk_trash_op()
                    multiselect_mode = false
                    close()
                }
                onNoClicked: {
                    close()
                }
            }
        }


        Loader {
            id: extras_loader
            sourceComponent: extras
        }

    }



}
