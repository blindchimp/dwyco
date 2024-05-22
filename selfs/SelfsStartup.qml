import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.12

Page {
    property alias watch: watch_button
    property alias capture: capture_button

    anchors.fill: parent
    header: Label {
        text: "Dwyco Selfie Stream " + core.buildtime + " " + (core.is_database_online === 1 ? "(online)" : "(offline)")
        font.bold: true
        color: "white"
        background: Rectangle {
            color: core.is_database_online === 1 ? "green" : "red"
        }
    }
    focusPolicy: Qt.StrongFocus
    Column {

        spacing: mm(5)
        anchors.fill: parent
        anchors.margins: mm(5)
        Button {
            id: watch_button
            width: parent.width
            height: parent.height / 2
            text: "Watch"
            focus: true
            Component.onCompleted: {
                forceActiveFocus()
            }
            KeyNavigation.down: capture_button

        }

        Button {
            id: capture_button
            width: parent.width
            height: parent.height / 2
            enabled: is_camera_available
            text: is_camera_available ? "Capture" : "(No cameras found)"

        }
    }
}
