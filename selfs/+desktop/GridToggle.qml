import QtQuick
import QtQuick.Controls

Item {
    property alias grid_checked: grid_tog.checked
    CheckBox {
        id: grid_tog
        text: "Show Grid"
        checkable: true
        checked: true
    }

}
