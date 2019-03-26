
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Pane {
    //width: 400
    //height: 400
    property alias vid_preview_button: vid_preview_button
    property alias about_button: about_button
    property alias settings_button: settings_button
    //property alias pin_lock_button: pin_lock_button
    //property alias block_list_button: block_list_button
    property alias profile_button: profile_button
    property alias lock_and_exit_button: lock_and_exit_button
    property alias invisible_switch: invisible_switch
    property alias quiet_switch: quiet_switch
    property alias browse_tags_button: browse_tags_button
    property alias browse_hidden_button: browse_hidden_button
    property real ctrl_pad: 4
    property alias circularImage: circularImage
    property alias text1: text1
    property alias tech_uid: tech_uid
    padding: 6

    ColumnLayout {
        id: columnLayout
        spacing: 3
        anchors.fill: parent

        RowLayout {
            id: rowLayout
            //width: 100
            //height: 100
            Layout.fillWidth: true

            CircularImage {
                id: circularImage
                visible: false
                Layout.maximumHeight: 32
                Layout.maximumWidth: 32
                Layout.minimumHeight: 32
                Layout.minimumWidth: 32
                Layout.margins: ctrl_pad
            }

            Text {
                id: text1
                text: qsTr("Text")
                clip: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.margins: ctrl_pad
                font.pixelSize: 12
            }
        }

        Text {
            id: tech_uid
            text: qsTr("Text")
            clip: true
            Layout.fillWidth: true
            font.pixelSize: 12
        }

        ItemDelegate {
            id: lock_and_exit_button
            text: qsTr("Lock and exit")
            visible: false
            bottomPadding: ctrl_pad
            topPadding: ctrl_pad
            padding: ctrl_pad

            Layout.fillWidth: true
        }

        ItemDelegate {
            id: browse_tags_button
            text: qsTr("Browse Favs")
            padding: ctrl_pad
            Layout.fillWidth: true
        }

        ItemDelegate {
            id: browse_hidden_button
            text: qsTr("Browse Hidden")
            visible: false
            bottomPadding: ctrl_pad
            topPadding: ctrl_pad
            padding: ctrl_pad
            Layout.fillWidth: true
        }

        ItemDelegate {
            id: profile_button
            text: qsTr("Update profile...")
            visible: false
            bottomPadding: ctrl_pad
            topPadding: ctrl_pad
            padding: ctrl_pad
            Layout.fillWidth: true
        }

        Switch {
            id: quiet_switch
            text: qsTr("Quiet")
            visible: false
            bottomPadding: ctrl_pad
            topPadding: ctrl_pad
            padding: ctrl_pad
            Layout.fillWidth: true
        }

        Switch {
            id: invisible_switch
            text: qsTr("Invisible")
            visible: false
            bottomPadding: ctrl_pad
            topPadding: ctrl_pad
            padding: ctrl_pad
            Layout.fillWidth: true
        }

        //        ItemDelegate {
        //            id: block_list_button
        //            text: qsTr("Block List")
        //            Layout.fillWidth: true
        //        }

        //        ItemDelegate {
        //            id: pin_lock_button
        //            text: qsTr("PIN Lock Setup")
        //            Layout.fillWidth: true
        //        }
        ItemDelegate {
            id: settings_button
            text: qsTr("Settings")
            bottomPadding: ctrl_pad
            topPadding: ctrl_pad
            padding: ctrl_pad
            Layout.fillWidth: true
            visible: false
        }

        ItemDelegate {
            id: about_button
            text: qsTr("About")
            Layout.fillWidth: true
        }

        ItemDelegate {
            id: vid_preview_button
            text: qsTr("Preview")
            visible: false
            bottomPadding: ctrl_pad
            topPadding: ctrl_pad
            padding: ctrl_pad
            //visible: false
            Layout.fillWidth: true
        }
        Item {
            id: item1

            Layout.fillHeight: true
        }
    }
}
