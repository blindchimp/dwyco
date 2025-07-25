import QtQuick
import QtQml.Models
import QtQuick.Controls
import dwyco


TipButton {
    id: our_but
    property int model_index;
    property string but_name;

    visible: call_buttons_model.get(but_name).visible
    enabled: call_buttons_model.get(but_name).enabled
    checkable: call_buttons_model.get(but_name).checkable
    checked: call_buttons_model.get(but_name).checked

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
        call_buttons_model.get(but_name).toggle()
    }
}
