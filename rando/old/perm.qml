import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQml 2.12
import QtQuick.Layouts 1.3

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    Component.onCompleted: {
        AndroidPerms.load()
        if(!(AndroidPerms.camera_permission && AndroidPerms.external_storage_permission))
            lo.active = true
    }
    Timer {
        interval: 30
        running: true
        onTriggered: {
            if(lo.active === false)
                Qt.exit(0)
        }
    }

    Loader {
        id:lo
        active: false
        visible: true
        anchors.fill: parent

        sourceComponent: aw

        Component {
            id: aw
            Rectangle {
                color: "red"
                visible: true
                width: 800
                height: 600

                property bool die

                Component.onCompleted: {

                    die = AndroidPerms.camera_permission && AndroidPerms.external_storage_permission

                }

                //die: AndroidPerms.camera_permission && AndroidPerms.external_storage_permission

                        onVisibleChanged: {
                            if(die)
                                Qt.exit(0)
                        }

                ColumnLayout {
                    anchors.centerIn: parent
                    Button {

                        onClicked: {
                            //Qt.exit(0)
                            AndroidPerms.toggle()
                        }
                        text: AndroidPerms.camera_permission ? "cam ok" : "cam not ok"
                    }

                    Button {

                        onClicked: {
                            Qt.exit(0)
                        }
                        text: AndroidPerms.external_storage_permission ? "store ok" : "store not ok"
                    }
                }
            }
        }
    }
}
