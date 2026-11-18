/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// small title-bar control for the upload/download dock. it only appears
// while there is something to show, and it remembers nothing between
// transfers: the dock starts hidden every time.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import dwyco

ToolButton {
    id: button

    visible: send_progress_overlay.hasTransfers && !is_mobile
    checkable: false
    Layout.fillHeight: true

    background: Rectangle {
        radius: dp(4)
        color: send_progress_overlay.expanded ? Qt.rgba(1, 1, 1, 0.28) : "transparent"
    }

    contentItem: Image {
        anchors.centerIn: parent
        source: mi("ic_cloud_upload_black_24dp.png")
    }

    ToolTip.text: send_progress_overlay.expanded
                  ? "Hide uploads/downloads"
                  : "Show uploads/downloads"

    onClicked: {
        send_progress_overlay.expanded = !send_progress_overlay.expanded
    }
}
