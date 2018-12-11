
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
import QtQuick.Controls 2.1
import dwyco 1.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Page {
    id: rectangle1

    property int profile_sent : 0
    property alias handle : handle.text_input
    property alias email : email.text_input
    property alias desc : desc.text
    property alias img_preview_source: img_preview.source
    property string img_filename : ""
    property int remove_img : 0
    property bool preview_existing : false
    //property alias pick_pic: picture_picker.visible
    property bool android_hack : false
    property string android_img_filename
    property int inh_profile_warning: 1

    Component.onCompleted: {
        if(core.get_local_setting("inh_profile_warning") === "")
            inh_profile_warning = 0
        else
            inh_profile_warning = 1

    }

    background: Rectangle {
        color: primary_dark
        gradient: Gradient {
            GradientStop { position: 0.0; color: primary_light }
            GradientStop { position: 1.0; color: primary_dark}
        }
    }

    Warning {
        id: warn
        visible: {inh_profile_warning === 0}
        z: 3
        warning: "Dwyco reviews all profiles. It is best if your profile contains no overtly adult language. If your profile has a picture, please be sure it shows a face, and the person is clothed and wearing a shirt. Other profiles are accepted, but are not visible to users that do not wish to view those types of profiles. Thanks."
        inhibit_key: "inh_profile_warning"
    }

    header:ToolBar {
            background: Rectangle {
                color: accent
            }

            implicitWidth: parent.width

            RowLayout {
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
                        stack.pop()
                    }
                    Layout.fillHeight: true

                }
                Item {

                    Layout.fillWidth: true
                }
                ToolButton {
                    id: cam_button
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_add_a_photo_black_24dp.png")
                    }
                    checkable: false
                    visible: !cam.visible
                    onClicked: {
                            stack.push(cam, {"next_state" : "StopAndPop", "ok_text" : "Use"})
                    }

                }
                ToolButton {
                    id: pic_button
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: mi("ic_attachment_black_24dp.png")
                    }
                    checkable: false
                    visible: !cam.visible
                    onClicked: {
                        if(Qt.platform.os == "android") {
                            // ugh, what a hack
                            android_img_pick_hack = 0
                            android_img_pick_hack = 1
                            notificationClient.open_image()
                        } else {
                            picture_picker.visible = true

                        }
                    }

                }

            }

        }

    
    onVisibleChanged: {
        if(visible && preview_existing) {
            img_filename = ""
            img_preview_source = ""
            handle.text_input = core.uid_to_name(core.get_my_uid())
            desc.text = core.uid_to_profile_info(core.get_my_uid(), DwycoCore.DESCRIPTION)
            email.text_input = core.uid_to_profile_info(core.get_my_uid(), DwycoCore.EMAIL)
            img_preview.source = core.uid_to_profile_preview(core.get_my_uid())
            img_filename = core.uid_to_profile_image_filename(core.get_my_uid())
            preview_existing = false

        }
    }

    onAndroid_hackChanged: {
        if(android_hack) {
            console.log(android_img_filename)

            prof_pic_preview.source = "file://" + android_img_filename
            prof_pic_preview.ok_vis = true
            prof_pic_preview.ok_text = "Ok"
            stack.push(prof_pic_preview)
            android_hack = false
        }
    }

    Loader {
        id: picture_picker

        visible: false
        active: visible
        sourceComponent: FileDialog {
                title: "Pick a picture"
                folder: shortcuts.pictures
                onAccepted: {
                    console.log(fileUrl)
                    console.log(Qt.resolvedUrl(fileUrl))
                    //img_preview.source = fileUrl
                    prof_pic_preview.source = fileUrl
                    prof_pic_preview.ok_vis = true
                    prof_pic_preview.ok_text = "Ok"
                    stack.push(prof_pic_preview, {"ok_text":"Use"})

                }
                onRejected: {picture_picker.visible = false}
                Component.onCompleted: {visible = true}
            }

    }

    PicPreview {
        id: prof_pic_preview
        onClosed: {
            if(ok) {
                img_preview.source = source
                img_filename = core.url_to_filename(source)
                picture_picker.visible = false
            }
            else
            {
                img_preview.source = core.uid_to_profile_preview(core.get_my_uid())
                img_filename = ""
            }
            source = ""

            stack.pop()
        }

    }

    
    
    Connections {
        target: top_dispatch

        onProfile_updated: {
            if(success === 1) {
                profile_sent = 0
                img_filename = ""
                img_preview.source = ""
                stack.pop()
            } else {
                profile_sent = 0
                animateOpacity.start()
            }
        }
    }

    function snapshot(filename) {
        console.log("CAMERA profile SNAP2", filename)
        img_filename = filename
        img_preview.source = "file:///" + String(filename)

    }

    // this just makes sure mouse events don't go down to other
    // components
    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: failed_msg
        text: "Update failed... "
        anchors.centerIn: parent

        opacity: 0.0
        NumberAnimation {
               id: animateOpacity
               target: failed_msg
               properties: "opacity"
               from: 1.0
               to: 0.0
               duration: 3000


          }
    }
    
    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: 3

        TextFieldX {
            id: handle
            placeholder_text:  qsTr("Nick name...")
            Layout.fillWidth: true
        }

        TextFieldX {
            id: email
            placeholder_text: qsTr("Email (optional, not displayed to others)")
            Layout.fillWidth: true
        }
        TextArea {
            id: desc
            //width: parent.width
            placeholderText: qsTr("Brief description...")
            //Layout.fillHeight: true
            selectByKeyboard: true
            selectByMouse: true
            Layout.fillWidth: true
        }
        Image {
            id: img_preview
            Layout.fillWidth: true
            Layout.fillHeight: true

            //sourceSize.height: 256
            //sourceSize.width: 256

            fillMode: Image.PreserveAspectFit
            // we'll perform the strip of exif and re-orientation in the
            // backend, but for display purposes, we'll have qml do it
            // for us here.
            autoTransform: true

            Image {
                id: clearit
                anchors.top: parent.top
                x: (parent.width / 2) + parent.paintedWidth / 2 - width
                width: dp(32)
                height: dp(32)
                source: "qrc:/new/red32/icons/red-32x32/cancel-32x32.png"
                visible: img_preview.source != "" && img_preview.status == Image.Ready
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        img_filename = ""
                        img_preview.source = ""
                    }
                }


            }

        }

        RowLayout {
            Layout.fillWidth: true
            //visible: {profile_sent === 0}
            Button {
                id: done_button
                Layout.fillWidth: true
                text: qsTr("Update")
                enabled: {profile_sent === 0}
                onClicked: {
                    Qt.inputMethod.commit()
                    if(core.set_simple_profile(handle.text_input, email.text_input, desc.text, img_filename) === 1) {
                        profile_sent = 1
                    }
                    else
                    {
                        animateOpacity.start()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: cancel_button
                Layout.fillWidth: true
                text: qsTr("Cancel")
                enabled: {profile_sent === 0}
                onClicked: {
                    img_filename = ""
                    img_preview_source = ""
                    stack.pop()
                }
            }
        }
    }

    BusyIndicator {
        id: busy1
        
        running: {profile_sent === 1}
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    Rectangle {
        id: offline_help
        anchors.fill: parent
        z: 5
        visible: {core.is_database_online !== 1}
        Label {
            anchors.fill: parent
            anchors.margins: mm(3)
            wrapMode: Text.WordWrap

            text: qsTr("(Can't update profile without data connection.)")
            z: 5
        }
        // this just makes sure mouse events don't go down to other
        // components
        MouseArea {
            anchors.fill: parent
        }
    }



}

