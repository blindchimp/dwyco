import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {

    property bool reindex_in_progress
    property bool reindex_done
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
        function onReindex_complete() {
            reindex_done = true
        }
    }

    ColumnLayout {
        spacing: mm(1)
        anchors.fill: parent

        Label {
            text: "This is a one-time Rando upgrade"
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }
        Label {
            text: "Please be patient,\n  it can take a minute."
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }
        Label {
            text: "Restart Rando to use normally."
            Layout.fillWidth: true
            Layout.leftMargin: mm(3)

        }

        Button {
            text: "Tap to start upgrade"
            Layout.fillWidth: true
            onClicked: {
                core.background_reindex()
                reindex_in_progress = true
            }
            visible: !reindex_in_progress && !reindex_done
        }

        Button {
            text: "Tap to finish"
            //Layout.horizontalCenter: true
            Layout.fillWidth: true
            onClicked: {
                core.set_local_setting("reindex1", "1")
                Qt.quit()
            }
            visible: reindex_done
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
            running: reindex_in_progress && !reindex_done
            Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter
        }
        Item {
            Layout.fillHeight: true
        }

    }

}
