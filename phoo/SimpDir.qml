
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
import QtQuick.Controls 2.1
import QtQuick.XmlListModel 2.0
import QtQuick.Layouts 1.3

Page {
    id: simpdir_top
    anchors.fill: parent

    signal uid_selected(string uid, string action)
    
    XmlListModel {
         id: xmlModel
         //source: {"http://profiles.dwyco.org/cgi-bin/mksimpxmldir.sh"}
         source: simpdir_rect.xml_url
         query: "/directory/entry"

         XmlRole { name: "uid"; query: "uid/string()" }
         XmlRole { name: "name"; query: "name/string()" }
         XmlRole { name: "description"; query: "description/string()" }
         XmlRole { name: "has_preview"; query: "has_preview/number()" }
         onStatusChanged: {
             console.log("XML")
             console.log(status)
             console.log(xmlModel.errorString())
         }
     }

    header: SimpleToolbar {
        extras: extras_button
    }

    Component {
        id: extras_button

        ToolButton {
            contentItem: Image {
                anchors.centerIn: parent
                source: mi("ic_action_overflow.png")
            }
            onClicked: optionsMenu.open()

            Menu {
                id: optionsMenu
                x: parent.width - width
                transformOrigin: Menu.TopRight

                MenuItem {
                    text: "Refresh"
                    onTriggered: {
                        xmlModel.reload()
                    }
                }
            }
        }
    }


    Component {
        id: simpdir_delegate
        Rectangle {
            width: parent.width
            height: has_preview == 1 ? vh(pct) : vh(pct) / 2
            border.width: 1

            color: primary_dark

            gradient: Gradient {
                GradientStop { position: 0.0; color: primary_light }
                GradientStop { position: 1.0; color: primary_dark}
            }

            RowLayout {
                id: drow33
                spacing: 5
                anchors.fill: parent
                CircularImage {
                    id: preview
                    source: {has_preview == 1 ? core.uid_to_http_profile_preview(uid) : ""}
                    fillMode: Image.PreserveAspectCrop
                    Layout.minimumWidth: picht()
                    Layout.maximumWidth: picht()
                    Layout.minimumHeight: picht()
                    Layout.maximumHeight: picht()
                    Layout.margins: mm(.125)
                    visible: has_preview
                }
                Item {
                    Layout.minimumWidth: mm(.125)
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: mm(2)
                    Text {
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        id: nm
                        text: name
                        clip: true
                        font.bold: true
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: description
                        clip: true
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                    }
                }

            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log("simpdir click ")
                    console.log(index)
                    listView1.currentIndex = index
                    uid_selected(uid, "clicked")
                }
                onPressAndHold: {
                    console.log("simpdir hold ")
                    console.log(index)
                    listView1.currentIndex = index
                    uid_selected(uid, "hold")
                }

            }
        }
    }

    Component.onCompleted: {
        simpdir_top.uid_selected.connect(top_dispatch.uid_selected)
    }

    ListView {
        id: listView1
         anchors.fill: parent
         model: xmlModel

         delegate: simpdir_delegate
         clip: true
         ScrollBar.vertical: ScrollBar { }
         
    }
    BusyIndicator {
        id: busy1

        running: {xmlModel.status == XmlListModel.Loading}
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }
}
