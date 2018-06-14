import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

Item {
    property alias vid_incoming: vid_incoming
    property alias vid_preview: vid_preview
    anchors.fill: parent
    ColumnLayout {
        id: columnLayout
        anchors.fill: parent

        Image {
            id: vid_incoming
            fillMode: Image.PreserveAspectFit
            Layout.fillHeight: true
            Layout.fillWidth: true
            //source: "qrc:/qtquickplugin/images/template_image.png"
        }

        Image {
            id: vid_preview
            fillMode: Image.PreserveAspectFit
            Layout.fillHeight: true
            Layout.fillWidth: true
            //source: "qrc:/qtquickplugin/images/template_image.png"
        }
    }
}
