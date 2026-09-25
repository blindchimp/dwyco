
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// The undo banner. Lives in the window's Overlay rather than in any page, so
// it stays put no matter how deep the StackView has gone, and so an undo
// offered on one page is still there after you navigate.
//
// The banner is a convenience, not the mechanism: undo_hub keeps the stack
// after this closes, and Ctrl+Z works either way.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import dwyco

Popup {
    id: bar
    parent: Overlay.overlay
    visible: false
    modal: false
    dim: false
    padding: 0
    // the timer owns dismissal. auto-closing on an outside click would race
    // with the click that triggered the very operation we are announcing.
    closePolicy: Popup.NoAutoClose

    property string text: ""

    x: {
        var w = bar.parent ? bar.parent.width : 0
        return Math.max(mm(1), (w - bar.width) / 2)
    }
    y: {
        var h = bar.parent ? bar.parent.height : 0
        return Math.max(mm(1), h - bar.height - mm(2))
    }

    background: Rectangle {
        radius: 4
        color: primary_text
        border.width: 1
        border.color: divider
    }

    contentItem: RowLayout {
        spacing: mm(3)

        Label {
            id: msg_label
            text: bar.text
            color: icons
            elide: Text.ElideRight
            Layout.maximumWidth: Math.max(mm(20), (bar.parent ? bar.parent.width : 0) / 2)
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("UNDO")
            onClicked: undo_hub.undo()
            ToolTip.text: qsTr("Ctrl+Z")
        }
    }

    implicitWidth: contentItem.implicitWidth + mm(3)
    implicitHeight: contentItem.implicitHeight + mm(2)

    Timer {
        id: dismiss_timer
        interval: 6000
        onTriggered: bar.close()
    }

    // let people read it, and keep the button clickable, if they are reaching
    // for the mouse
    HoverHandler {
        onHoveredChanged: {
            if(!hovered)
                dismiss_timer.restart()
        }
    }

    onOpened: {
        msg_label.text = bar.text
    }

    onClosed: {
        dismiss_timer.stop()
    }

    Connections {
        target: undo_hub
        function onNotify(label) {
            bar.text = label
            bar.open()
            dismiss_timer.restart()
        }
        function onDismissed() {
            if(bar.visible)
                bar.close()
        }
    }
}
