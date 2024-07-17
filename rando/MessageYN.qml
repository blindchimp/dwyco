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
    // background: Rectangle {
    //     color: "red"
    //     implicitWidth: 300
    //     implicitHeight: 200
    // }

    standardButtons: Dialog.Yes | Dialog.No
    modal: true
    focus: true
    width: 300
    height: 200

    ColumnLayout {

    Label {
        id: lab
        text: dia.text
        Layout.fillWidth: true
    }
    Label {
        id: lab2
        text: dia.informativeText
        Layout.fillWidth: true
    }
    }


    onAccepted: {
        console.log("YES")
        // FIXME
        noClicked()
    }
    onRejected:{
        console.log("NO")
        noClicked()
    }
}
