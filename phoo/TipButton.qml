import QtQuick
import QtQuick.Controls

ToolButton {
    hoverEnabled: true
    // tooltips pop in after 1s of hover, which would show up as pixel churn
    // in a region-of-interest settle check. test_mode (set from main.cpp
    // only when --test-agent is passed) turns them off.
    ToolTip.visible: hovered && !test_mode
    ToolTip.timeout: 3000
    ToolTip.delay: 1000
}
