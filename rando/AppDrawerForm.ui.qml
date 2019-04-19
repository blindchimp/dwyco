
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
    property alias about_button: about_button

    property real ctrl_pad: 4
    property alias circularImage: circularImage
    property alias text1: text1
    property alias tech_uid: tech_uid
    property alias clear_nonfav: clear_nonfav
    property alias delete_all: delete_all
    focusPolicy: Qt.NoFocus
    padding: 6

    ColumnLayout {
        id: columnLayout
        spacing: 3
        anchors.fill: parent

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: primary_dark

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: mm(1)
                Text {
                    text: "Dwyco Rando"
                    font.bold: true
                    font.pixelSize: 20
                    color: "white"
                }

        RowLayout {
            id: rowLayout
            //width: 100
            //height: 100
            //Layout.fillWidth: true
            //anchors.fill: parent
            //anchors.margins: 10
            Layout.fillHeight: true
            Layout.fillWidth: true

            CircularImage {
                id: circularImage

                Layout.maximumHeight: 32
                Layout.maximumWidth: 32
                Layout.minimumHeight: 32
                Layout.minimumWidth: 32
                Layout.margins: ctrl_pad
                source: "qrc:/icons/greenguy.png"
            }
            ColumnLayout {
                Text {
                    id: text1
                    text: qsTr("Text")
                    clip: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.fillWidth: true
                    Layout.margins: ctrl_pad
                    font.pixelSize: 12
                    color: "white"
                }
                Text {
                    id: tech_uid
                    text: qsTr("Text")
                    clip: true
                    Layout.fillWidth: true
                    font.pixelSize: 12
                    color: "white"
                }
            }
        }
            }
            }



        Item {

            Layout.fillHeight: true
        }
        ItemDelegate {
            id: clear_nonfav
            text: "Delete All...\n(except favorites)"
            font.bold: true
            icon.source: mi("ic_delete_black_24dp.png")
        }

        ItemDelegate {
            id: delete_all
            text: qsTr("Delete All...")
            font.bold: true
            icon.source: mi("ic_delete_black_24dp.png")
        }
        Item {

            Layout.fillHeight: true
        }

        ItemDelegate {
            id: about_button
            text: qsTr("About")
            Layout.fillWidth: true
        }
    }
}




/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
