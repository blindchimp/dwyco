import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Rectangle {
    z: 5
    color: primary_light
//    MouseArea {
//        anchors.fill: parent
//        onClicked: {
//            Qt.quit()
//        }
//    }

    ColumnLayout {
        spacing: mm(1)
        anchors.fill: parent

        Label {
            text: "Doing Android file migration, please be patient..."
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }
        Label {
            text: "Rando will close when the copy is complete."
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }
        Label {
            text: "Restart Rando to use normally."
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }
        Label {
            text: "Remember: SAVE Pictures you want to keep before uninstalling Rando"
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }

        Button {
            text: "Tap to finish migraton"
            //Layout.horizontalCenter: true
            Layout.fillWidth: true
            onClicked: {
                // do directory swap, and exit immediately
                Qt.quit()
            }
            visible: !busy.running
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

        BusyIndicator {
            id: busy
            running: parent.visible
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
        Item {
            Layout.fillHeight: true
        }

    }

}
