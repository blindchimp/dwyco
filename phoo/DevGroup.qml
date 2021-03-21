
import QtQuick 2.12
import dwyco 1.0
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.12
Page {

    property bool group_active

    group_active: core.active_group_name.length > 0 && core.group_status === 0

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
                core.set_setting("group/join_key", "foo")
                Qt.quit()
            }
        }
    }



    ColumnLayout {
        id: devcol
        anchors.fill: parent
        anchors.margins: mm(2)
        spacing: mm(1)
        Label {
            text: "Active: " + core.active_group_name + " (" + core.percent_synced + "%)"
            font.bold: true
            font.pixelSize: 16
            visible: group_active
            Layout.fillWidth: true
        }

        Label {
            text: "Requesting key for: " + core.active_group_name
            font.bold: true
            font.pixelSize: 16
            visible: core.group_status !== 0
            Layout.fillWidth: true
        }

        TextFieldX {
            id: group_name
            text_input: ""
            placeholder_text: "Enter group name (you can change it later)"
            visible: !group_active
            Layout.fillWidth: true
        }

        TextFieldX {
            id: group_pw
            text_input: ""
            placeholder_text: "Enter group password"
            Layout.fillWidth: true
        }

        Switch {
            id: join_button
            text: qsTr("Enable to link this device (requires restart)")
            onClicked: {
                core.start_gj2(group_name.text_input, group_pw.text_input)
            }

            visible: !group_active
            Layout.fillWidth: true
        }
        Switch {
            id: unjoin_button
            text: qsTr("Disable to UNlink this device (requires restart)")
            onClicked: {
                if(core.start_gj2("", "") === 1) {
                    Qt.quit()
                }
            }
            checked: true
            visible: group_active
            Layout.fillWidth: true
        }


        Label {
            text: "Status: unlinked"
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
                        text: "Handle"
                        Layout.preferredWidth: cm(3)
                    }
                    Label {
                        text: "status"
                        Layout.preferredWidth:  mm(5)
                    }

                    Label {
                        text: "asserts"
                        Layout.preferredWidth:  mm(15)
                        horizontalAlignment: Text.AlignRight
                    }
                    Label {
                        text: "q"
                        Layout.preferredWidth:  mm(15)
                        horizontalAlignment: Text.AlignRight
                    }
                    Label {
                        text: "%"
                        Layout.preferredWidth:  mm(10)
                        horizontalAlignment: Text.AlignRight
                    }
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
                        elide: Text.ElideRight
                        text: core.uid_to_name(uid)
                        Layout.preferredWidth: cm(3)
                    }
                    Label {
                        text: status
                        Layout.preferredWidth:  mm(5)
                    }

                    Label {
                        text: asserts
                        Layout.preferredWidth:  mm(15)
                        horizontalAlignment: Text.AlignRight
                    }
                    Label {
                        text: sendq_count
                        Layout.preferredWidth:  mm(15)
                        horizontalAlignment: Text.AlignRight
                    }
                    Label {
                        text: percent_synced
                        Layout.preferredWidth:  mm(10)
                        horizontalAlignment: Text.AlignRight
                    }
                    Label {
                        text: ip
                        color: proxy ? "red" : "black"
                        //Layout.preferredWidth: cm(4)
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
