
import QtQuick
import QtQuick.Controls.Material
import QtLocation
import QtPositioning

Page {
    id: randomap
    //property real lat: 59.91 //39.739200
    //property real lon: 10.75 //-104.984700

    property real lat: 39.739200
    property real lon: -104.984700
    property string placename: ""
    property int zoom
    property var center

    function reset_zoom(z) {
        mapz.reset_zoom(z)
    }

    //anchors.fill: parent
    Plugin {
        id: mapPlugin
        name: "osm"
        // the reason we do this is to force a non-ssl
        // connection, since qt has problems accessing the
        // encryption stuff (for https connections)
        // on android and other platforms.
        //PluginParameter { name: "osm.mapping.custom.host"; value: "https://tile.thunderforest.com/neighbourhood/%z/%x/%y.png?apikey=0698437986fb4f2999e18c53e5809e12" }
        PluginParameter { name: "osm.mapping.custom.host"; value: "https://tile.openstreetmap.org/" }
        PluginParameter { name: "osm.mapping.custom.mapcopyright"; value: "OpenStreetMap" }
        PluginParameter { name: "osm.mapping.custom.datacopyright"; value: "OpenStreetMap" }

    }

    MapView {
        id: mapz
        anchors.fill: parent
        map.plugin: mapPlugin
        map.center: QtPositioning.coordinate(lat, lon)

        // i guess this binding is undone by whatever is going on in the mapview gesture processing
        // so you need to call the reset_zoom thing to programmatically change the zoom.
        map.zoomLevel: randomap.zoom
        function reset_zoom(z) {
            map.zoomLevel = z
        }

        map.onZoomLevelChanged: (zoomLevel) => {
            console.log("ZOOM ", zoomLevel)
        }
        map.onCenterChanged: (center) => {
            console.log("center ", center)
            loc_circle.center = QtPositioning.coordinate(lat, lon)
        }

        // see note above regarding failing https stuff
        map.activeMapType: map.supportedMapTypes[map.supportedMapTypes.length - 1]
        Component.onCompleted: {
            map.addMapItem(loc_circle)
            // note: we had to change the tile server, but the cache doesn't
            // seem to invalidate itself properly. so do a one-off cache clear
            var a = core.get_local_setting("bugfix_clear_map_cache1")
            if(a === "") {
                map.clearData();
                core.set_local_setting("bugfix_clear_map_cache1", "1")
            }
        }
        map.onActiveMapTypeChanged: {
            console.log("MAP TYPE ", map.activeMapType)
        }


        MapCircle {
            id: loc_circle
            radius: 10000.0
            color: "red"
            border.width: 3
            opacity: .5
        }
    }

    PinchArea {
        id: pincher
        anchors.fill: parent
        pinch.dragAxis: Pinch.XAndYAxis

        // this works, but zooms too fast
        //property real start_zoom_level

        // onPinchStarted: {
        //     console.log("start pinch", mapz.map.zoomLevel)
        //     start_zoom_level = mapz.map.zoomLevel
        // }

        // onPinchUpdated: {
        //     console.log("pinch ", pinch.scale)
        //     var p
        //     p = pinch.scale
        //     mapz.map.zoomLevel = start_zoom_level * p
        // }

        property real oldZoom
        function calcZoomDelta(zoom, percent) {
            return zoom + Math.log(percent)/Math.log(2)
        }

        onPinchStarted: {
            oldZoom = mapz.map.zoomLevel
        }

        onPinchUpdated: (pinch) => {
            mapz.map.zoomLevel = calcZoomDelta(oldZoom, pinch.scale)
        }

        onPinchFinished: (pinch) => {
            mapz.map.zoomLevel = calcZoomDelta(oldZoom, pinch.scale)
        }

        MouseArea {
            id: dragArea
            hoverEnabled: true
            anchors.fill: parent
            //drag.target: mapz
            scrollGestureEnabled: false

            // onPressed: {
            //     dragging = true

            // }
            onClicked: {
                stack.pop()
            }
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
