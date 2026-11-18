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
    // shown while this attachment is being uploaded or downloaded
    property bool sending: false
    // 0..100 when we know the percentage, negative when it is indeterminate
    property real progress: -1
    property string sendingWord: "Sending"

    readonly property real padding: dp(6)
    readonly property real iconSize: dp(compact ? 18 : 24)
    readonly property real nameFontSize: dp(compact ? 11 : 13)
    readonly property real infoFontSize: dp(compact ? 9 : 11)

    readonly property string sizeText: fileSize > 0 ? fmtBytes(fileSize) : "unknown size"
    readonly property string infoText: {
        if(sending) {
            var pct = progress >= 0 ? " · " + Math.round(progress) + "%" : ""
            return sendingWord + "…" + pct
        }
        return hintText.length > 0 ? sizeText + " · " + hintText : sizeText
    }

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
    implicitHeight: Math.max(chip_row.implicitHeight, iconSize) + padding * 2 +
                    (sending ? progress_track.height + dp(4) : 0)

    RowLayout {
        id: chip_row
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: sending ? progress_track.top : parent.bottom
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
                text: chip.infoText
                color: chip.textColor
                opacity: 0.75
                font.pixelSize: chip.infoFontSize
                elide: Text.ElideRight
            }
        }
    }

    Rectangle {
        id: progress_track
        visible: sending
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: chip.padding
        anchors.rightMargin: chip.padding
        anchors.bottomMargin: chip.padding
        height: dp(3)
        radius: height / 2
        color: Qt.rgba(0, 0, 0, 0.15)
        clip: true

        Rectangle {
            id: progress_fill
            width: chip.progress >= 0
                   ? progress_track.width * Math.min(100, Math.max(0, chip.progress)) / 100
                   : progress_track.width * 0.3
            height: parent.height
            radius: parent.radius
            clip: true
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: accent }
                GradientStop { position: 1.0; color: primary_light }
            }
            Behavior on width {
                NumberAnimation { duration: 350; easing.type: Easing.OutCubic }
            }

            Rectangle {
                id: progress_shimmer
                width: Math.max(dp(16), progress_fill.width * 0.28)
                height: parent.height
                color: "white"
                opacity: 0.28
                NumberAnimation on x {
                    running: chip.sending && progress_fill.width > 0
                    loops: Animation.Infinite
                    from: -progress_shimmer.width
                    to: progress_fill.width
                    duration: 1200
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}