
import QtQuick 2.12
import dwyco 1.0
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.12
Page {

    header: SimpleToolbar {

    }
    background: Rectangle {
        color: amber_light
    }

    ColumnLayout {
        id: devcol
        anchors.fill: parent
        anchors.margins: mm(2)
        spacing: mm(1)

        TextFieldX {
            id: group_name
            text_input: ""
            placeholder_text: "Enter group name (you can change it later)"
            Layout.fillWidth: true
        }
        TextFieldX {
            id: group_pw
            text_input: ""
            placeholder_text: "Enter group password"
            Layout.fillWidth: true
        }

        ItemDelegate {
            id: join_button
            text: qsTr("Click to link this device to group")
            onClicked: {

            }
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

            model: SyncDescModel
            delegate: Component {
                RowLayout {
                    width: parent.width
                    height: implicitHeight
                    spacing: mm(1)
                    Label {
                        text: uid
                    }
                    Label {
                        text: status
                    }
                    Label {
                        text: ip
                        color: proxy ? "red" : "black"
                    }
                    Label {
                        text: asserts
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
