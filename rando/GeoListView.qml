import QtQuick 2.6
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

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
        RowLayout {
            width: parent.width
            Image {
                source: mi("ic_language_black_24dp.png")
            }

            Label {
                Layout.fillWidth: true
                text: model.display
            }

            Item {
                Layout.fillWidth: true
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
