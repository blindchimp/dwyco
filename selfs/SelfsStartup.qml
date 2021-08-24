import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.12

Page {
    property alias watch: watch_button
    property alias capture: capture_button

    anchors.fill: parent
    Column {
        anchors.fill: parent
        Button {
            id: watch_button
            width: parent.width
            height: parent.height / 2
            text: "Watch"

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
