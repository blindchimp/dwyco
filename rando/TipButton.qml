import QtQuick 2.12
import QtQuick.Controls 2.12

ToolButton {
    hoverEnabled: true
    ToolTip.visible: hovered
    ToolTip.timeout: 3000
    ToolTip.delay: 1000
}
