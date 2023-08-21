import QtQuick
import QtQuick.Controls

ToolButton {
    hoverEnabled: false
    ToolTip.visible: hovered
    ToolTip.timeout: 3000
    ToolTip.delay: 1000
}
