
import QtQuick 2.6
import QtQuick.Controls 2.2
import QtLocation 5.12
import QtPositioning 5.12

Rectangle {
    //property real lat: 59.91 //39.739200
    //property real lon: 10.75 //-104.984700
    property real lon: 39.739200
    property real lat: -104.984700

    anchors.fill: parent
    Plugin {
        id: mapPlugin
        name: "osm"
        PluginParameter { name: "osm.mapping.host"; value: "http://a.tile.openstreetmap.org/" }
    }

    Map {
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(-27.5, 153.1)//QtPositioning.coordinate(lat, lon)
        zoomLevel: 10
    }
}
