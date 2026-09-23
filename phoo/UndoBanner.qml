/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    visible: false
    anchors.fill: parent
    z: 99
    property string text: ""
    property var _pendingUndo: null

    function show(msg, undoFn) {
        text = msg
        _pendingUndo = undoFn
        visible = true
        undo_timer.restart()
        undo_timer.start()
    }

    function hideBanner() {
        visible = false
        _pendingUndo = null
        undo_timer.stop()
    }

    Timer {
        id: undo_timer
        interval: 5000
        onTriggered: root.hideBanner()
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        width: Math.min(parent.width - 24, banner_row.implicitWidth + 24)
        height: banner_row.implicitHeight + 12
        color: "black"
        opacity: 0.92
        radius: 4
        z: 2

        RowLayout {
            id: banner_row
            anchors.centerIn: parent
            spacing: 8
            Label {
                text: root.text
                color: "white"
                elide: Text.ElideRight
                Layout.maximumWidth: root.width - 140
            }
            Button {
                text: "Undo"
                onClicked: {
                    if(root._pendingUndo)
                        root._pendingUndo()
                    root.hideBanner()
                }
            }
            ToolButton {
                text: "\u2715"
                onClicked: root.hideBanner()
            }
        }
    }
}