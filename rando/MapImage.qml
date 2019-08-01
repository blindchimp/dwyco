
import QtQuick 2.6
import QtQuick.Controls 2.2
import QtLocation 5.12
import QtPositioning 5.12

Page {
    //property real lat: 59.91 //39.739200
    //property real lon: 10.75 //-104.984700

    property real lat: 39.739200
    property real lon: -104.984700
    property string placename: ""

    anchors.fill: parent
    Plugin {
        id: mapPlugin
        name: "osm"
        // the reason we do this is to force a non-ssl
        // connection, since qt has problems accessing the
        // encryption stuff (for https connections)
        // on android and other platforms.
        PluginParameter { name: "osm.mapping.custom.host"; value: "http://a.tile.openstreetmap.org/" }
    }

    Map {
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(lat, lon)
        zoomLevel: 10
        // see note above regarding failing https stuff
        activeMapType: supportedMapTypes[supportedMapTypes.length - 1]
    }

    Component {
        id: extras_button
        Label {
            text: placename
            color: "white"
            rightPadding: mm(1)
        }

//        ToolButton {
//            contentItem: Image {
//                anchors.centerIn: parent
//                source: mi("ic_action_overflow.png")
//            }
//            onClicked: optionsMenu.open()

//            Menu {
//                id: optionsMenu
//                x: parent.width - width
//                transformOrigin: Menu.TopRight
//                MenuItem {
//                    text: "Show City Name"
//                    onTriggered: {

//                    }
//                }
//            }
//        }
    }

    header: SimpleToolbar {
        extras: extras_button

    }
}
