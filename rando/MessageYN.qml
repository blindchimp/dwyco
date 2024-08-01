import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material
//import QtQuick.Dialogs

Dialog {
    id: dia
    anchors.centerIn: parent
    signal noClicked
    signal yesClicked
    property string detailedText
    property string informativeText
    property string text
    background: Rectangle {
        color: "red"
        //implicitWidth: 300
        //implicitHeight: 200
        radius: 5
    }

    //standardButtons: Dialog.Yes | Dialog.No
    modal: true
    focus: true

    //width: 400
    //height: 300
    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Yes | DialogButtonBox.No
        //buttonLayout: DialogButtonBox.AndroidLayout
        background: Rectangle {
            color: "black"
            radius: 5

        }

    }

    ColumnLayout {
    anchors.fill: parent
    Label {
        id: lab
        text: dia.text
        //horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        Layout.fillWidth: true
    }
    Label {
        id: lab2
        text: dia.informativeText
        //horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        Layout.fillWidth: true
    }
    Item {
        Layout.fillHeight: true
    }
    }


    onAccepted: {
        console.log("YES")
        yesClicked()
    }
    onRejected:{
        console.log("NO")
        noClicked()
    }
}
