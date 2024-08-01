import QtQuick
import QtQuick.Controls.Material

ToolButton {
    hoverEnabled: true
    ToolTip.visible: hovered
    ToolTip.timeout: 3000
    ToolTip.delay: 1000
}
