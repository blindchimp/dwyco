
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// Transient result message ("Saved to foo.png", "Nothing to copy"). In the
// window Overlay so it is positioned the same way no matter which page asked
// for it. Replaces the one-off save_toast that used to live in MsgView.

import QtQuick
import QtQuick.Controls
import dwyco

Popup {
    id: toast
    parent: Overlay.overlay
    visible: false
    modal: false
    dim: false
    padding: 0
    closePolicy: Popup.NoAutoClose

    property string text: ""

    x: Math.max(mm(1), ((toast.parent ? toast.parent.width : 0) - toast.width) / 2)
    y: Math.max(mm(1), (toast.parent ? toast.parent.height : 0) * 0.4)

    background: Rectangle {
        radius: 3
        color: "black"
    }

    contentItem: Label {
        text: toast.text
        color: "white"
        wrapMode: Text.WordWrap
    }

    implicitWidth: contentItem.implicitWidth + mm(2)
    implicitHeight: contentItem.implicitHeight + mm(1)

    Timer {
        id: toast_timer
        interval: 4000
        onTriggered: toast.close()
    }

    function show(msg) {
        if(!msg)
            return
        text = msg
        open()
        toast_timer.restart()
    }

    onClosed: {
        toast_timer.stop()
    }
}
