import QtQuick
import QtQuick.Controls

Item {
    id: root
    property alias grid_checked: grid_tog.checked

    // Size to the checkbox. The wrapper used to be 0x0 and let the
    // checkbox overflow it, which meant anything anchoring to it -- and
    // any layout giving it room -- had nothing to work with.
    implicitWidth: grid_tog.implicitWidth
    implicitHeight: grid_tog.implicitHeight
    width: implicitWidth
    height: implicitHeight

    CheckBox {
        id: grid_tog
        anchors.fill: parent
        text: "Show Grid"
        checkable: true
        checked: true
        // no objectName here on purpose: this component is used by
        // ConvList AND by every SimpleToolbar, so naming it inside the
        // component gives every instance the same name and a lookup by
        // objectName lands on an arbitrary one of them. Name it at the
        // point of use instead (see ConvList.qml).
    }
}