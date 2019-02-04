import QtQuick 2.9
import QtQuick.Controls 2.2
import Qt.labs.folderlistmodel 2.1
import QtQuick.Layouts 1.3

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Scroll")

    ScrollView {
        anchors.fill: parent

        GridView {

            id: grid
            anchors.fill: parent
            cellWidth: 160 ; cellHeight: 120
            model: FolderListModel {
                nameFilters: ["*.jpg", "*.fle"]
                showFiles: true
                showDirs: false
                folder: "file:///home/dwight/Downloads/bots/botfiles"

            }


            Component {
                id: fileDelegate
                Item {
                    width: grid.cellWidth; height: grid.cellHeight
                    ColumnLayout {
                        anchors.fill: parent
                        Image {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            source: fileURL
                            fillMode: Image.PreserveAspectFit
                            sourceSize.width: 320
                        }
                        Text {
                            Layout.fillWidth: true
                            text: fileName
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {

                        }
                    }
                }
            }

            delegate: fileDelegate
        }
    }
}
