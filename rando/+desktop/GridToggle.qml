import QtQuick 2.0
import QtQuick.Controls.Material

Item {
    property alias grid_checked: grid_tog.checked
    CheckBox {
        id: grid_tog
        text: "Show Grid"
        checkable: true
    }

}
