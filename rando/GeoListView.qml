import QtQuick
import QtQuick.Controls.Material
import QtPositioning

Page {
    id: geolist

    property string hash: ""

    background: Rectangle {
        gradient: Gradient {
            GradientStop { position: 1.0; color: amber_light}
            GradientStop { position: 0.0; color: amber_dark}
        }
    }

    onHashChanged: {
        GeoSprayListModel.load_hash_to_model(hash)
    }

    header: SimpleToolbar {
        //extras: extras_button

    }

    Component {
        id: geodel
        ItemDelegate {
            anchors.left: ListView.view.contentItem.left
            anchors.right: ListView.view.contentItem.right
            text: model.display
            display: AbstractButton.TextBesideIcon
            icon.source: model.lat.length === 0 ? mi("ic_not_interested_black_24dp.png") : mi("ic_language_black_24dp.png")
            onClicked: {
                if(model.lat.length > 0) {
                    mapimage.lat = parseFloat(model.lat)
                    mapimage.lon = parseFloat(model.lon)
                    mapimage.center = QtPositioning.coordinate(parseFloat(model.lat), parseFloat(model.lon))
                    mapimage.placename = model.display
                    //mapimage.zoom = default_map_zoom
                    mapimage.reset_zoom(default_map_zoom)
                    stack.push(mapimage)
                }
            }
        }
    }

    ListView {
        id: geolistview
        anchors.fill: parent
        anchors.margins: mm(2)
        model: GeoSprayListModel
        delegate: geodel
        spacing: mm(2)
    }
}
