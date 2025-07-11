import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {

    property bool migration_in_progress
    property bool migration_done
    z: 5
    color: primary_light
//    MouseArea {
//        anchors.fill: parent
//        onClicked: {
//            Qt.quit()
//        }
//    }
    Connections {
        target: core
        function onMigration_complete() {
            migration_done = true
        }
    }

    ColumnLayout {
        spacing: mm(1)
        anchors.fill: parent

        Label {
            text: "Phoo files must be copied."
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }
        Label {
            text: "Phoo will close when the copy is complete."
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }
        Label {
            text: "Restart Phoo to use normally."
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }
        Label {
            text: "Please SAVE pictures you want\n  to keep before uninstalling Phoo"
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }
        Button {
            text: "Tap to start copy"
            Layout.fillWidth: true
            onClicked: {
                core.background_migrate()
                migration_in_progress = true
            }
            visible: !migration_in_progress && !migration_done
        }

        Button {
            text: "Tap to finish"
            //Layout.horizontalCenter: true
            Layout.fillWidth: true
            onClicked: {
                // do directory swap, and exit immediately
                core.directory_swap();
                Qt.quit()
            }
            visible: migration_done
        }
        Button {
            text: "Stop"
            //Layout.horizontalCenter: true
            Layout.fillWidth: true
            onClicked: {
                Qt.quit()
            }
            visible: busy.running
        }
        Item {
            Layout.fillHeight: true
        }

    }
    BusyIndicator {
        id: busy
        running: migration_in_progress && !migration_done
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

}
