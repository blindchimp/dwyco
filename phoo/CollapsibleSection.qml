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

// a header that opens and closes a block of secondary controls. the point is
// to keep the default view of the tox pages short: the things most people
// touch stay visible, the rest is one tap away. same layout everywhere.
ColumnLayout {
    id: root

    property string title: ""
    property string subtitle: ""
    property bool expanded: false

    default property alias content: body.data

    Layout.fillWidth: true
    spacing: mm(1)

    ItemDelegate {
        id: header
        Layout.fillWidth: true
        padding: mm(1)
        onClicked: root.expanded = !root.expanded

        contentItem: RowLayout {
            spacing: mm(1)

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Label {
                    Layout.fillWidth: true
                    text: root.title
                    font.bold: true
                    color: primary_text
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    visible: root.subtitle.length > 0
                    text: root.subtitle
                    font.pixelSize: dp(10)
                    color: secondary_text
                    elide: Text.ElideRight
                }
            }

            Label {
                text: root.expanded ? "\u25be" : "\u25b8"
                color: secondary_text
                font.pixelSize: dp(16)
            }
        }
    }

    ColumnLayout {
        id: body
        Layout.fillWidth: true
        visible: root.expanded
        spacing: mm(1)
    }
}
