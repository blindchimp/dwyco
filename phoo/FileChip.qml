/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// a little "this is a file, not a picture" placeholder that is shown in
// place of a preview image for attachments we can't render. it's purely
// informational, the enclosing delegate still handles clicks.
import QtQuick
import QtQuick.Layouts

Item {
    id: chip

    // the original, sender-supplied name of the attachment
    property string fileName: ""
    // size of the local copy, in bytes. 0 means "we don't know".
    property double fileSize: 0
    // smaller type/icons, for use in the grid browse views
    property bool compact: false
    property color textColor: primary_text
    // extra line shown after the size. leave it empty when there's nothing
    // useful for the user to click on in this particular spot.
    property string hintText: "click to save"

    readonly property real padding: dp(6)
    readonly property real iconSize: dp(compact ? 18 : 24)
    readonly property real nameFontSize: dp(compact ? 11 : 13)
    readonly property real infoFontSize: dp(compact ? 9 : 11)

    readonly property string sizeText: fileSize > 0 ? fmtBytes(fileSize) : "unknown size"

    function fmtBytes(n) {
        var units = ["B", "KB", "MB", "GB", "TB"]
        var i = 0
        while(n >= 1024 && i < units.length - 1) {
            n = n / 1024
            i++
        }
        return (i === 0 ? String(Math.round(n)) : n.toFixed(1)) + " " + units[i]
    }

    // note: the sizes come from the RowLayout, and the layout gets its
    // width from us. do not try to compute the layout's width from
    // implicitWidth or the texts end up with no width at all.
    implicitWidth: Math.max(chip_row.implicitWidth, dp(48)) + padding * 2
    implicitHeight: Math.max(chip_row.implicitHeight, iconSize) + padding * 2

    RowLayout {
        id: chip_row
        anchors.fill: parent
        anchors.margins: chip.padding
        spacing: dp(6)

        Image {
            id: chip_icon
            source: mi("ic_attachment_black_24dp.png")
            Layout.preferredWidth: chip.iconSize
            Layout.preferredHeight: chip.iconSize
            Layout.minimumWidth: chip.iconSize
            Layout.minimumHeight: chip.iconSize
            Layout.alignment: Qt.AlignTop
            fillMode: Image.PreserveAspectFit
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Text {
                Layout.fillWidth: true
                // note: a zero minimum is what lets the name shrink and
                // elide when the chip is given less room than it wants.
                Layout.minimumWidth: 0
                text: chip.fileName.length > 0 ? chip.fileName : "(unnamed file)"
                color: chip.textColor
                font.pixelSize: chip.nameFontSize
                font.bold: true
                elide: Text.ElideMiddle
            }
            Text {
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                text: chip.hintText.length > 0 ? chip.sizeText + " · " + chip.hintText : chip.sizeText
                color: chip.textColor
                opacity: 0.75
                font.pixelSize: chip.infoFontSize
                elide: Text.ElideRight
            }
        }
    }
}