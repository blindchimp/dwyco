
import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

Page {
    //property real lat: 59.91 //39.739200
    //property real lon: 10.75 //-104.984700

    property real lat: 39.739200
    property real lon: -104.984700
    property string placename: ""
    property alias zoom: mapz.zoomLevel
    property alias center: mapz.center

    //anchors.fill: parent
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
        id: mapz
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(lat, lon)
        //gesture.acceptedGestures: MapGestureArea.PanGesture|MapGestureArea.PinchGesture|MapGestureArea.FlickGesture
        zoomLevel: default_map_zoom
        onZoomLevelChanged: {
            console.log("ZOOM ", zoomLevel)
        }
        onCenterChanged: {
            console.log("center ", center)
            loc_circle.center = QtPositioning.coordinate(lat, lon)
        }

        // see note above regarding failing https stuff
        activeMapType: supportedMapTypes[supportedMapTypes.length - 1]

        MapCircle {
            id: loc_circle
            radius: 10000.0
            color: "red"
            border.width: 3
            opacity: .5
        }
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
