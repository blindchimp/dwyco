
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
import QtGraphicalEffects 1.0
import QtQml 2.2
import QtQuick.Controls 2.2
import dwyco 1.0


TipButton {
    background: Image {
        // WTF: this id isn't referenced
        // anywhere, but in qt 5.12, when this is
        // defined, the image doesn't scale right.
        // in qt 5.10, and previous, it works fine
        //id: bgimage
        anchors.centerIn: parent
        source : mi("ic_home_black_24dp.png")
    }
    contentItem:  Rectangle {
        anchors.centerIn: parent
        visible: core.unread_count > 0
        color: "indigo"
        anchors.margins: mm(1)
        anchors.fill: parent
        radius: 3
        Text {
            anchors.centerIn: parent
            anchors.fill: parent
            text: core.unread_count > 9 ? "9+" : String(core.unread_count)
            color: "white"
            visible: core.unread_count > 0
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
    ToolTip.text: "Pop back to top"
    checkable: false
    onClicked: {
        // go back to top level
        stack.pop(null)
    }

}
