
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

import QtQuick
import dwyco
import QtQuick.Layouts
import QtQuick.Controls

ToolBar {
    property Component extras
    property alias grid_checked: show_grid.grid_checked
    property bool hide_grid: true
    background: Rectangle {
        color: accent
    }

    implicitWidth: parent.width

    RowLayout {
        Item {
            Layout.minimumHeight: cm(1)
        }
        anchors.fill: parent
        spacing: mm(2)

        TipButton {
            id: back_button
            contentItem: Image {
                source: mi("ic_arrow_back_black_24dp.png")
                anchors.centerIn: parent
            }
            checkable: false
            onClicked: {
                stack.pop()
            }
            Layout.fillHeight: true
            ToolTip.text: "Go back"
        }

        GridToggle {
            id: show_grid
            visible: !hide_grid
            Layout.fillHeight: true
        }
        Item {
            Layout.fillWidth: true
        }

        ConvToolButton {
        }

        Loader {
            id: extras_loader
            sourceComponent: extras
        }
    }





}
