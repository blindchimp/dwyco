import QtQuick 2.0
import QtQml.Models 2.3
import QtQuick.Controls 2.2
import dwyco 1.0


Button {
    id: our_but
    property int model_index;
    property string but_name;

//    Connections {
//        target: chatbox_page
//        onTo_uidChanged: {
//            console.log("to uid changed ", to_uid)

//            //but.target = call_buttons_model.get(but_name)
//        }
//    }

    Binding {
        target: our_but
        property: "visible"
        value: call_buttons_model.get(but_name).visible
    }



//    Connections {
//        id: but
//        onVisibleChanged: {
//            our_but.visible = call_buttons_model.get(but_name).visible
//        }
//    }

    onClicked: {
        console.log("LINK CLICK ", but_name)
        call_buttons_model.get(but_name).clicked()

    }
    onPressed: {
        call_buttons_model.get(but_name).pressed()
    }
    onReleased: {
        call_buttons_model.get(but_name).released()
    }
//    onTriggered: {
//        model.get(index).triggered(state)
//    }
    onToggled: {
        call_buttons_model.get(but_name).toggled(state)
    }
}
