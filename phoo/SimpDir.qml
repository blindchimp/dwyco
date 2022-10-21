
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// note: this page doesn't update its contents dynamically.
// it loads a static page from the server and displays it.
// to get a new page, you have to "refresh" the entire contents.
// this is different from the "conversation" model, where
// it listens for signals related to profile updates and
// can update its contents piecemeal
Page {
    id: simpdir_top
    anchors.fill: parent

    signal uid_selected(string uid, string action)

    onVisibleChanged: {
        if(visible && SimpleDirectoryList.count === 0)
            core.refresh_directory()
    }

    header: SimpleToolbar {
        id: toolbar
        extras: extras_button
        hide_grid: false
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
                        core.refresh_directory()
                    }
                }
            }
        }
    }


    Component {
        id: simpdir_delegate
        Rectangle {
            width: ListView.view.width
            height: has_preview ? vh(pct) : vh(pct) / 2
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
                    source: {has_preview ?
                                 (core.uid_profile_regular(uid) ?
                                     core.uid_to_http_profile_preview(uid) :
                                     "qrc:/new/red32/icons/red-32x32/exclamation-32x32.png") : ""}
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
                        text: core.uid_profile_regular(uid) ? handle : censor_name(handle)
                        clip: true
                        font.bold: true
                        elide: Text.ElideRight
                        font.pixelSize: applicationWindow1.font.pixelSize
                    }
                    Text {
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: core.uid_profile_regular(uid) ? description : censor_name(description)
                        clip: true
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        font: applicationWindow1.font

                    }
                }

            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton|Qt.RightButton
                onClicked: {
                    console.log("simpdir click ")
                    console.log(index)
                    listView1.currentIndex = index
                    if(mouse.button === Qt.LeftButton) {
                        uid_selected(uid, "clicked")
                    } else if(mouse.button === Qt.RightButton) {
                        uid_selected(uid, "hold")
                    }
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
         model: SimpleDirectoryList
         visible: !toolbar.grid_checked

         delegate: simpdir_delegate
         clip: true
         ScrollBar.vertical: ScrollBar { }
         
    }

    Component {
        id: simpdir_grid_delegate
        Rectangle {
            width: gridView1.cellWidth
            height: gridView1.cellHeight
            border.width: 1
            color: primary_dark

            gradient: Gradient {
                GradientStop { position: 0.0; color: primary_light }
                GradientStop { position: 1.0; color: primary_dark}
            }

            CircularImage {
                id: preview
                source: {has_preview ?
                             (core.uid_profile_regular(uid) ?
                                 core.uid_to_http_profile_preview(uid) :
                                 "qrc:/new/red32/icons/red-32x32/exclamation-32x32.png") : ""}
                fillMode: Image.PreserveAspectCrop
                height:parent.height
                width: parent.height
                visible: has_preview
            }
            Text {
                text: core.uid_profile_regular(uid) ? handle : censor_name(handle)
                elide: Text.ElideRight
                clip: true
                anchors.bottom: parent.bottom
                width: parent.width
                color: amber_light
                visible: has_preview
            }
            ColumnLayout {
                visible: !has_preview
                Layout.fillWidth: true
                anchors.fill: parent
                Layout.margins: 3

                spacing: mm(2)
                Text {
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true
                    id: nm
                    text: core.uid_profile_regular(uid) ? handle : censor_name(handle)
                    clip: true
                    font.bold: true
                    elide: Text.ElideRight

                }
                Text {
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: core.uid_profile_regular(uid) ? description : censor_name(description)
                    clip: true
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton|Qt.RightButton
                onClicked: {
                    console.log("simpdir click ")
                    console.log(index)
                    gridView1.currentIndex = index
                    if(mouse.button === Qt.LeftButton) {
                        uid_selected(uid, "clicked")
                    } else if(mouse.button === Qt.RightButton) {
                        uid_selected(uid, "hold")
                    }
                }
                onPressAndHold: {
                    console.log("simpdir hold ")
                    console.log(index)
                    gridView1.currentIndex = index
                    uid_selected(uid, "hold")
                }

            }

        }
    }

    GridView {
        id: gridView1
        anchors.fill:parent
        cellWidth: 160 ; cellHeight: 160

        visible: toolbar.grid_checked

        model: SimpleDirectoryList
        delegate: simpdir_grid_delegate
        clip: true
        //spacing: 5
        ScrollBar.vertical: ScrollBar {
            background: Rectangle {
                color: "green"
            }
        }
    }

    Connections {
        target: core
        function onIgnore_event() {
            if(simpdir_top.visible)
                core.refresh_directory()
        }
    }

    BusyIndicator {
        id: busy1

        running: {core.directory_fetching}
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }
}
