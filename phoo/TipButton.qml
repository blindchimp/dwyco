import QtQuick 2.0
import QtQuick.Controls 2.2

ToolButton {
    hoverEnabled: false
    ToolTip.visible: hovered
    ToolTip.timeout: 3000
    ToolTip.delay: 1000
}
