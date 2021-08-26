
import QtQuick 2.12
import dwyco 1.0
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.12
Page {

    property bool group_active
    property bool show_failed
    property bool quitnow: false
    property string provisional_group

    group_active: core.active_group_name.length > 0 && core.group_status === 0 && core.group_private_key_valid === 1

    header: SimpleToolbar {

    }
    background: Rectangle {
        color: amber_light
    }

    Connections {
        target: core
        onJoin_result: {
            if(result === 1) {
                core.set_setting("group/alt_name", gname)
                provisional_group = gname
                quitnow = true
                header.visible = false
                //Qt.quit()
            } else {
                show_failed = true
                join_button.checked = false
            }
        }
    }

    RowLayout {
        id: quitit
        anchors.fill: parent
        anchors.margins: mm(2)
        visible: quitnow
        spacing: mm(3)
        Button {
            text: "Quit"
            onClicked: {
                Qt.quit()
            }
        }
        Label {
            text: "Success! " + provisional_group + " active. Click QUIT"
            Layout.fillWidth: true
        }

    }

    ColumnLayout {
        id: devcol
        anchors.fill: parent
        anchors.margins: mm(2)
        spacing: mm(1)
        visible: !quitnow
        Label {
            id: failed
            text: "Linking failed.\nTry using a different account name or PIN."
            visible: show_failed
            Layout.fillWidth: true
        }
        Label {
            id: unusable
            text: "NO Data connection available, linking not possible right now."
            visible: !group_active && !up_and_running
            Layout.fillWidth: true
        }

        Label {
            text: group_active ? "Active: " + core.active_group_name + " (" + core.percent_synced + "%)" : "Not Linked"
            font.bold: true
            font.pixelSize: 16
            visible: true
            Layout.fillWidth: true
        }
        RowLayout {
            id: requesting
            visible: core.group_status === 1
            Button {
                text: "Cancel"
                onClicked: {
                    if(core.start_gj2("", "") === 1) {
                        Qt.quit()
                    }
                }
                //Layout.maximumHeight: label1.height
                //Layout.maximumWidth: textInput1.height
                Layout.alignment: Qt.AlignVCenter
            }
            Label {
                id: label1
                text: "Requesting key for: " + core.active_group_name
                font.bold: true
                font.pixelSize: 16
                Layout.fillWidth: true
            }
            Layout.fillWidth: true
        }

        TextFieldX {
            id: group_name
            text_input: !enabled ? core.active_group_name : ""
            placeholder_text: "Account name (it can be anything you want)"
            visible: !group_active
            inputMethodHints: Qt.ImhNoPredictiveText|Qt.ImhLowercaseOnly
            Layout.fillWidth: true
            enabled: !requesting.visible
        }



        TextFieldX {
            id: group_pw
            text_input: ""
            placeholder_text: "Enter secret PIN (at least 4 digits)"
            //inputMethodHints: Qt.ImhDigitsOnly
            visible: !group_active && !show_pin_layout.visible
            Layout.fillWidth: true
        }

        RowLayout {
            id: show_pin_layout
            visible: group_active || requesting.visible
            Button {
                id: show_pin
                text: "Show PIN"
                checkable: true
                Layout.alignment: Qt.AlignVCenter
            }
            Label {
                id: label2
                text: show_pin.checked ? core.join_key : "####"
                font.bold: true
                font.pixelSize: 16
                Layout.fillWidth: true
            }
            Layout.fillWidth: true
        }

        Switch {
            id: join_button
            text: qsTr("Enable to link this device (requires restart)")
            onClicked: {
                core.start_gj2(group_name.text_input, group_pw.text_input)
                show_failed = false;
            }

            visible: !(group_active || requesting.visible)
            enabled: up_and_running //&& group_pw.text_input.length >= 3 && group_name.text_input.length > 4
            Layout.fillWidth: true
        }
        Switch {
            id: unjoin_button
            text: qsTr("Disable to UNLINK this device (requires restart)")
            onClicked: {
                if(core.start_gj2("", "") === 1) {
                    Qt.quit()
                }
            }
            checked: true
            visible: group_active
            Layout.fillWidth: true
        }
        Switch {
            id: server_mode
            text: qsTr("Server mode (download and store all messages to this device)")
            checked: core.eager_pull
            onClicked: {
                if(checked) {
                    core.set_setting("sync/eager", "1")
                } else {
                    core.set_setting("sync/eager", "0")
                }
            }

            visible: group_active
            Layout.fillWidth: true
        }

        ListView {
            id: synclist
            //anchors.fill: parent
            Layout.fillWidth: true
            Layout.fillHeight: true
            header: Component {
                RowLayout {
                    width: parent.width
                    height: implicitHeight
                    spacing: mm(1)
                    Label {
                        elide: Text.ElideRight
                        text: "Status"
                        Layout.preferredWidth: cm(2)
                    }
                    Label {
                        elide: Text.ElideRight
                        text: "Handle"
                        Layout.preferredWidth: cm(2)
                    }
//                    Label {
//                        text: "status"
//                        Layout.preferredWidth:  mm(5)
//                    }

//                    Label {
//                        text: "asserts"
//                        Layout.preferredWidth:  mm(15)
//                        horizontalAlignment: Text.AlignRight
//                    }
//                    Label {
//                        text: "q"
//                        Layout.preferredWidth:  mm(15)
//                        horizontalAlignment: Text.AlignRight
//                    }
//                    Label {
//                        text: "%"
//                        Layout.preferredWidth:  mm(10)
//                        horizontalAlignment: Text.AlignRight
//                    }
                    Label {
                        text: "ip"
                        //color: proxy ? "red" : "black"
                        //Layout.preferredWidth: cm(4)
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }

            }

            model: SyncDescModel
            delegate: Component {
                RowLayout {
                    width: parent.width
                    height: implicitHeight
                    spacing: mm(1)
                    Label {
                        text: status + " " + asserts + " " + sendq_count + " " + percent_synced
                        Layout.preferredWidth: cm(2)
                    }

                    Label {
                        elide: Text.ElideRight
                        text: handle
                        Layout.preferredWidth: cm(2)
                    }
//                    Label {
//                        text: status
//                        Layout.preferredWidth:  mm(5)
//                    }

//                    Label {
//                        text: asserts
//                        Layout.preferredWidth:  mm(15)
//                        horizontalAlignment: Text.AlignRight
//                    }
//                    Label {
//                        text: sendq_count
//                        Layout.preferredWidth:  mm(15)
//                        horizontalAlignment: Text.AlignRight
//                    }
//                    Label {
//                        text: percent_synced
//                        Layout.preferredWidth:  mm(10)
//                        horizontalAlignment: Text.AlignRight
//                    }
                    Label {
                        id: conn_ip
                        text: ip
                        color: proxy ? "red" : "green"
                        visible: ip.length > 0
                        //Layout.preferredWidth: cm(4)
                    }
                    Label {
                        id: advertised_ip
                        text: adv_ip
                        visible: adv_ip.length > 0 && !conn_ip.visible
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }

            clip: true
        }
    }
}
