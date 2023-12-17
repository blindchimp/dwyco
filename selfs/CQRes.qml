
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
import dwyco 1.0

Page {
    id: cqres_top
    anchors.fill: parent

    property int query_in_progress
    property int query_succeeded
    property bool no_contacts: false

    signal uid_selected(string uid, string action)

    // onVisibleChanged: this doesn't seem to work except
    // when visible turns off. for some reason, the loader
    // that is encapsulating this. since the loader is
    // creating and destroying things, i used the "onCompleted"
    // handler below.

    Connections {
        target: core
        function onCq_results_received(succ) {
            if(succ) {
                cq_res_model.load_from_cq_file()
                query_succeeded = 1
                core.set_local_setting("cq-succeeded", "1")
            } else {
                cq_res_model.clear()
                query_succeeded = 0
                core.set_local_setting("cq-succeeded", "0")
            }
            query_in_progress = 0
            core.set_local_setting("cq-in-progress", "0")

        }

    }
    background: Rectangle {
        color: primary_dark
        gradient: Gradient {
            GradientStop { position: 0.0; color: primary_light }
            GradientStop { position: 1.0; color: primary_dark}
        }
    }

    DwycoSimpleContactModel {
        id: user_model
    }
    DwycoSimpleUserModel {
        id: cq_res_model
    }

    ColumnLayout {
        id: help_column
        visible: {cq_res_model.count === 0}
        z: 5
        spacing: mm(10)
        anchors.fill: parent

        Label {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            id: empty_help
            wrapMode: Text.Wrap
            text: qsTr("PRIVACY NOTE: We DO NOT keep a copy of your contact list. Click the PRIVACY button for more details.")
            horizontalAlignment: Text.AlignHCenter
        }
        TextFieldX {
            id: single_email
            placeholder_text: "Enter Email to search for"
            inputMethodHints: Qt.ImhEmailCharactersOnly
            visible: !is_mobile
        }

        Button {
            Layout.alignment: Qt.AlignCenter
            text: is_mobile ? "Send Email Contacts Securely" : "Search for email"
            onClicked: {
                core.delete_cq_results()
                if(is_mobile)
                {
                    if(core.load_contacts() === 0) {
                        // permission denied
                        return;
                    }

                    user_model.load_users_to_model()
                    if(user_model.count > 0) {
                        user_model.send_query()
                        query_in_progress = 1
                        query_succeeded = 0
                        core.set_local_setting("cq-in-progress", "1")
                        core.set_local_setting("cq-succeeded", "0")
                        no_contacts = false
                    } else {
                        query_in_progress = 0
                        query_succeeded = 0
                        core.set_local_setting("cq-in-progress", "0")
                        core.set_local_setting("cq-succeeded", "0")
                        no_contacts = true
                    }
                }
                else
                {
                    user_model.set_model_to_single_email(single_email.text_input)
                    user_model.send_query()
                    query_in_progress = 1
                    query_succeeded = 0
                    core.set_local_setting("cq-in-progress", "1")
                    core.set_local_setting("cq-succeeded", "0")
                    no_contacts = false
                }
            }
            enabled: query_in_progress === 0

        }
        Button {
            Layout.alignment: Qt.AlignCenter
            text: "Privacy notes"
            onClicked: {
                stack.push(privacy_details)
            }

        }
        Label {
            id: working
            text: "Query sent... check here later for results"
            font.bold: true
            visible: query_in_progress > 0
        }
        Label {
            id: sorry
            text: "Sorry, none of your contact emails were found."
            visible: query_in_progress === 0 && query_succeeded === 1
        }
        Label {
            id: sorry2
            text: "No contacts to find"
            visible: no_contacts
        }

        Item {
            Layout.fillHeight: true
        }
    }

    Label {
        id: privacy_details
        text: qsTr("When you click SEND, only the Email addresses in your contact list are securely sent to Dwyco. The servers check our database for those Email addresses, and return the results securely to the app. The servers then discard the information used to perform the check. We DO NOT send any emails to the contacts in your contact list.")
        visible: false
        anchors.fill: parent
        wrapMode: Text.Wrap
        anchors.margins: mm(2)
        background: Rectangle {
            color: primary_dark
            gradient: Gradient {
                GradientStop { position: 0.0; color: primary_light }
                GradientStop { position: 1.0; color: primary_dark}
            }
        }
        Button {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: mm(3)
            text: "Got it"
            onClicked: {
                stack.pop()
            }

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
                        core.delete_cq_results()
                        cq_res_model.load_from_cq_file()
                        if(is_mobile)
                        {
                            if(core.load_contacts() === 0) {
                                return
                            }

                            user_model.load_users_to_model()
                            if(user_model.count > 0) {
                                user_model.send_query()
                                query_in_progress = 1
                                query_succeeded = 0
                                core.set_local_setting("cq-in-progress", "1")
                                core.set_local_setting("cq-succeeded", "0")
                                no_contacts = false
                            } else {
                                query_in_progress = 0
                                query_succeeded = 0
                                core.set_local_setting("cq-in-progress", "0")
                                core.set_local_setting("cq-succeeded", "0")
                                no_contacts = true

                            }
                        }
                        else
                        {
                            single_email.text_input = ""
                            query_in_progress = 0
                            query_succeeded = 0
                            core.set_local_setting("cq-in-progress", "0")
                            core.set_local_setting("cq-succeeded", "0")
                        }
                    }
                }

            }
        }
    }


    Component {
        id: cqres_delegate
        Rectangle {
            width: ListView.view.width
            height: {uid.length > 0 ? vh(pct) : 0}
            border.width: 1

            color: primary_dark

            gradient: Gradient {
                GradientStop { position: 0.0; color: primary_light }
                GradientStop { position: 1.0; color: primary_dark}
            }

            visible: {uid.length > 0}

            RowLayout {
                id: drow33
                spacing: mm(1)
                anchors.fill: parent

                CircularImage {
                    id: preview
                    source: {core.uid_profile_regular(uid) ? core.uid_to_profile_preview(uid) : ""}
                    fillMode: Image.PreserveAspectCrop
                    Layout.minimumWidth: picht()
                    Layout.maximumWidth: picht()
                    Layout.minimumHeight: picht()
                    Layout.maximumHeight: picht()
                    Layout.margins: mm(.125)
                    Connections {
                        target: core
                        function onSys_uid_resolved(uid) {
                            if(uid === model.uid) {
                                preview.source = core.uid_profile_regular(uid) ? core.uid_to_profile_preview(uid) : ""
                            }
                        }

                    }
                }
                ColumnLayout {
                    id: cl
                    Layout.fillWidth: true

                    spacing: mm(2)
                    Text {
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        id: nm
                        text: {core.uid_profile_regular(uid) ? core.uid_to_profile_info(uid, DwycoCore.HANDLE) : "<<hidden>>"}
                        clip: true
                        font.bold: true
                        elide: Text.ElideRight
                        Connections {
                            target: core
                            function onSys_uid_resolved(uid) {
                                if(uid === model.uid) {
                                    nm.text = core.uid_profile_regular(uid) ? core.uid_to_profile_info(uid, DwycoCore.HANDLE) : "<<hidden>>"
                                }
                            }

                        }
                        font.pixelSize: applicationWindow1.font.pixelSize

                    }
                    Text {
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        text: email
                        clip: true
                        elide: Text.ElideRight
                        font.pixelSize: applicationWindow1.font.pixelSize
                    }

                    Text {
                        id: desc
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: {core.uid_profile_regular(uid) ? core.uid_to_profile_info(uid, DwycoCore.DESCRIPTION) : "<<hidden>>"}
                        clip: true
                        font.pixelSize: applicationWindow1.font.pixelSize
                        //width: parent.width - (nm.width + preview.width)

                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        Connections {
                            target: core
                            function onSys_uid_resolved(uid) {
                                if(uid === model.uid) {
                                    desc.text = core.uid_profile_regular(uid) ? core.uid_to_profile_info(uid, DwycoCore.DESCRIPTION) : "<<hidden>>"
                                }
                            }

                        }

                    }

                }


            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton|Qt.RightButton
                onClicked: {
                    console.log("cqres click ")
                    console.log(index)
                    listView1.currentIndex = index
                    if(mouse.button === Qt.LeftButton) {
                        uid_selected(uid, "clicked")
                    } else if(mouse.button === Qt.RightButton) {
                        uid_selected(uid, "hold")
                    }
                }
                onPressAndHold: {
                    console.log("cqres hold ")
                    console.log(index)
                    listView1.currentIndex = index
                    uid_selected(uid, "hold")
                }

            }
        }
    }

    Component.onCompleted: {
        cqres_top.uid_selected.connect(top_dispatch.uid_selected)
        var q = core.get_local_setting("cq-in-progress")
        if(q === "")
        {
            query_in_progress = 0
            core.set_local_setting("cq-in-progress", "0")
        }
        else
        {
            query_in_progress = parseInt(core.get_local_setting("cq-in-progress"))
        }
        q = core.get_local_setting("cq-succeeded")
        if(q === "")
        {
            query_succeeded = 0;
            core.set_local_setting("cq-succeeded", "0")
        }
        else
        {
            query_succeeded = parseInt(core.get_local_setting("cq-succeeded"))
        }
        cq_res_model.load_from_cq_file()
    }

    ListView {
        id: listView1
         anchors.fill: parent
         model: cq_res_model
         delegate: cqres_delegate
         clip:true
         ScrollBar.vertical: ScrollBar { }
         
    }

}
