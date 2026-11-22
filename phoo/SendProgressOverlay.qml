/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// A small floating dock that shows outgoing attachment transfers. It lives
// outside the StackView so progress remains visible even when the user is
// looking at another conversation (or another page entirely.)
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import dwyco

Item {
    id: root

    property int maxHeight: cm(11)
    // the dock is off by default; the title bar button toggles this
    property bool expanded: false
    readonly property int transferCount: dock_model.count
    readonly property bool hasTransfers: transferCount > 0
    visible: expanded && hasTransfers && !is_mobile && pwdialog.allow_access === 1
    height: visible ? Math.min(dock_view.contentHeight, maxHeight) : 0

    onTransferCountChanged: {
        if(transferCount === 0)
            expanded = false
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

    function fmtSpeed(bps) {
        return fmtBytes(bps) + "/s"
    }

    function fmtEta(secs) {
        if(secs < 0)
            return ""
        if(secs < 1)
            return "almost done"
        if(secs < 60)
            return Math.round(secs) + "s left"
        var mins = Math.floor(secs / 60)
        var rem = Math.round(secs - mins * 60)
        return mins + "m " + rem + "s left"
    }

    function looksLikeImage(fn) {
        var a = String(fn).toLowerCase()
        return a.endsWith(".jpg") || a.endsWith(".jpeg") || a.endsWith(".png")
                || a.endsWith(".gif") || a.endsWith(".webp")
    }

    function onIncomingStart(xfer_id, sender, display_name) {
        var i = indexOf(xfer_id)
        if(i >= 0)
            return
        var has_name = display_name && display_name.length > 0
        var name = has_name ? display_name : "Incoming video/audio"
        dock_model.insert(0, {
            pers_id: xfer_id,
            recipient: sender,
            who: core.uid_to_name(sender),
            filename: name,
            file_size: 0,
            is_image: has_name && looksLikeImage(name),
            incoming: true,
            generic: !has_name,
            percent: 0,
            status: "receiving",
            status_text: "",
            speed: 0,
            eta: -1,
            last_pct: -1,
            last_ms: Date.now()
        })
    }

    function onIncomingProgress(xfer_id, percent_done) {
        var i = indexOf(xfer_id)
        if(i < 0)
            return
        var pct = Math.max(0, Math.min(100, percent_done))
        dock_model.setProperty(i, "percent", pct)
        dock_model.setProperty(i, "status", "receiving")
    }

    function indexOf(pid) {
        for(var i = 0; i < dock_model.count; ++i) {
            if(dock_model.get(i).pers_id === pid)
                return i
        }
        return -1
    }

    function removeTransfer(pid) {
        var i = indexOf(pid)
        if(i >= 0)
            dock_model.remove(i)
    }

    function onProgress(pid, recipient, msg, percent_done) {
        var i = indexOf(pid)
        if(i < 0) {
            var info = core.qd_msg_info(pid)
            if(!info || !info.has_file)
                return
            var now = Date.now()
            dock_model.insert(0, {
                pers_id: pid,
                recipient: recipient,
                who: core.uid_to_name(recipient),
                filename: info.file_name.length > 0 ? info.file_name : "(unnamed file)",
                file_size: info.file_size,
                is_image: info.is_image,
                incoming: false,
                generic: false,
                percent: 0,
                status: "sending",
                status_text: msg,
                speed: 0,
                eta: -1,
                last_pct: -1,
                last_ms: now
            })
            i = 0
        }

        var e = dock_model.get(i)
        var pct = Math.max(0, Math.min(100, percent_done))
        var now2 = Date.now()
        var dt = (now2 - e.last_ms) / 1000.0
        if(e.last_pct >= 0 && pct > e.last_pct && dt >= 0.2 && e.file_size > 0) {
            var inst = (pct - e.last_pct) / 100.0 * e.file_size / dt
            var speed = e.speed > 0 ? e.speed * 0.65 + inst * 0.35 : inst
            var eta = speed > 0 ? (100 - pct) / 100.0 * e.file_size / speed : -1
            dock_model.setProperty(i, "speed", speed)
            dock_model.setProperty(i, "eta", eta)
            dock_model.setProperty(i, "last_pct", pct)
            dock_model.setProperty(i, "last_ms", now2)
        } else if(e.last_pct < 0) {
            dock_model.setProperty(i, "last_pct", pct)
            dock_model.setProperty(i, "last_ms", now2)
        }
        dock_model.setProperty(i, "percent", pct)
        dock_model.setProperty(i, "status", "sending")
        dock_model.setProperty(i, "status_text", msg)
    }

    function finishTransfer(pid, state) {
        var i = indexOf(pid)
        if(i < 0)
            return
        if(state === "done") {
            dock_model.setProperty(i, "percent", 100)
            dock_model.setProperty(i, "status", "done")
        } else if(state === "failed") {
            dock_model.setProperty(i, "status", "failed")
        } else {
            dock_model.remove(i)
        }
    }

    function cancelTransfer(pid, recipient) {
        core.delete_message(recipient, pid)
        removeTransfer(pid)
    }

    ListModel {
        id: dock_model
    }

    ListView {
        id: dock_view
        anchors.fill: parent
        spacing: dp(8)
        clip: true
        interactive: contentHeight > height
        model: dock_model

        add: Transition {
            NumberAnimation {
                properties: "opacity,scale"
                from: 0
                to: 1
                duration: 260
                easing.type: Easing.OutBack
            }
        }
        remove: Transition {
            NumberAnimation {
                properties: "opacity,scale"
                to: 0
                duration: 180
                easing.type: Easing.InCubic
            }
        }
        displaced: Transition {
            NumberAnimation {
                properties: "y"
                duration: 180
                easing.type: Easing.OutCubic
            }
        }

        delegate: Rectangle {
            id: card
            width: dock_view.width
            height: card_layout.implicitHeight + dp(18)
            radius: dp(12)
            color: Qt.rgba(primary_dark.r, primary_dark.g, primary_dark.b, 0.94)
            border.width: 1
            border.color: {
                if(model.status === "done")
                    return "#66BB6A"
                if(model.status === "failed")
                    return "#EF5350"
                return Qt.rgba(1, 1, 1, 0.16)
            }
            clip: true

            readonly property string detailText: {
                if(model.status === "done")
                    return (model.incoming ? "Received" : "Sent")
                            + (model.file_size > 0 ? " · " + root.fmtBytes(model.file_size) : "")
                if(model.status === "failed")
                    return model.incoming ? "Failed to receive" : "Failed to send"
                var parts = [Math.round(model.percent) + "%"]
                if(model.speed > 0)
                    parts.push(root.fmtSpeed(model.speed))
                if(model.eta >= 0)
                    parts.push(root.fmtEta(model.eta))
                return parts.join(" · ")
            }

            ColumnLayout {
                id: card_layout
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: dp(10)
                spacing: dp(5)

                RowLayout {
                    Layout.fillWidth: true
                    spacing: dp(8)

                    Image {
                        source: model.is_image
                                ? mi("ic_add_a_photo_black_24dp.png")
                                : mi("ic_attachment_black_24dp.png")
                        Layout.preferredWidth: dp(20)
                        Layout.preferredHeight: dp(20)
                        Layout.alignment: Qt.AlignTop
                        fillMode: Image.PreserveAspectFit
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        spacing: 0

                        Text {
                            Layout.fillWidth: true
                            Layout.minimumWidth: 0
                            text: model.filename
                            textFormat: Text.PlainText
                            color: "white"
                            font.pixelSize: dp(13)
                            font.bold: true
                            elide: Text.ElideMiddle
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.minimumWidth: 0
                            text: model.generic
                                  ? "Receiving media from " + model.who
                                  : (model.incoming
                                     ? (model.is_image ? "Receiving photo from " : "Receiving file from ")
                                     : (model.is_image ? "Sending photo to " : "Sending file to ")) + model.who
                            textFormat: Text.PlainText
                            color: primary_light
                            opacity: 0.85
                            font.pixelSize: dp(10)
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        Layout.alignment: Qt.AlignTop
                        text: model.status === "done" ? "✓"
                              : model.status === "failed" ? "!"
                              : Math.round(model.percent) + "%"
                        color: model.status === "failed" ? "#EF5350"
                                : model.status === "done" ? "#66BB6A"
                                : "white"
                        font.pixelSize: (model.status === "sending" || model.status === "receiving") ? dp(15) : dp(18)
                        font.bold: true
                    }
                }

                Rectangle {
                    id: track
                    Layout.fillWidth: true
                    Layout.topMargin: dp(2)
                    height: dp(5)
                    radius: height / 2
                    color: Qt.rgba(1, 1, 1, 0.16)
                    clip: true

                    Rectangle {
                        id: fill
                        width: track.width * Math.max(0, Math.min(100, model.percent)) / 100
                        height: parent.height
                        radius: parent.radius
                        clip: true
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop {
                                position: 0.0
                                color: model.status === "failed" ? "#EF5350"
                                        : model.status === "done" ? "#66BB6A" : accent
                            }
                            GradientStop {
                                position: 1.0
                                color: model.status === "failed" ? "#EF5350"
                                        : model.status === "done" ? "#66BB6A" : primary_light
                            }
                        }
                        Behavior on width {
                            NumberAnimation { duration: 400; easing.type: Easing.OutCubic }
                        }

                        Rectangle {
                            width: Math.max(dp(18), fill.width * 0.25)
                            height: parent.height
                            color: "white"
                            opacity: model.status === "sending" ? 0.22 : 0
                            NumberAnimation on x {
                                running: model.status === "sending" && fill.width > 0
                                loops: Animation.Infinite
                                from: -width
                                to: fill.width
                                duration: 1300
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: dp(8)

                    Text {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        text: card.detailText
                        color: primary_light
                        opacity: 0.9
                        font.pixelSize: dp(10)
                        elide: Text.ElideRight
                    }

                    Text {
                        visible: model.status === "sending"
                        text: "Cancel"
                        color: "white"
                        opacity: cancel_area.pressed ? 1.0 : 0.7
                        font.pixelSize: dp(10)
                        font.underline: cancel_area.containsMouse

                        MouseArea {
                            id: cancel_area
                            anchors.fill: parent
                            anchors.margins: -dp(6)
                            hoverEnabled: true
                            onClicked: root.cancelTransfer(model.pers_id, model.recipient)
                        }
                    }
                }
            }

            Timer {
                running: model.status === "done" || model.status === "failed"
                interval: model.status === "done" ? 1300 : 4000
                onTriggered: root.removeTransfer(model.pers_id)
            }
        }
    }

    Connections {
        target: core

        function onMsg_progress(pers_id, recipient, msg, percent_done) {
            root.onProgress(pers_id, recipient, msg, percent_done)
        }

        function onMsg_send_status(status, recipient, pers_id) {
            if(status === DwycoCore.MSG_SEND_START)
                root.onProgress(pers_id, recipient, "", 0)
            else if(status === DwycoCore.MSG_SEND_SUCCESS)
                root.finishTransfer(pers_id, "done")
            else if(status === DwycoCore.MSG_SEND_FAIL)
                root.finishTransfer(pers_id, "failed")
            else if(status === DwycoCore.MSG_SEND_CANCELED)
                root.finishTransfer(pers_id, "cancel")
        }

        function onIncoming_attachment_status(status, sender, xfer_id, display_name, percent) {
            if(status === DwycoCore.INCOMING_ATTACHMENT_START)
                root.onIncomingStart(xfer_id, sender, display_name)
            else if(status === DwycoCore.INCOMING_ATTACHMENT_STATUS)
                root.onIncomingProgress(xfer_id, percent)
            else if(status === DwycoCore.INCOMING_ATTACHMENT_OK)
                root.finishTransfer(xfer_id, "done")
            else if(status === DwycoCore.INCOMING_ATTACHMENT_FAILED)
                root.finishTransfer(xfer_id, "failed")
        }
    }
}
