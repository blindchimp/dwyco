
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import dwyco

Item {
    id: root
    property string friendUid: ""
    property real size: 12

    property string _status: "offline"
    property string _user_status: "none"
    property bool _is_tox: false

    width: size
    height: size
    visible: _is_tox && friendUid.length > 0

    Component.onCompleted: {
        for (var i = 0; i < ToxFriendModel.count; i++) {
            var f = ToxFriendModel.get(i);
            if (f.pubkey.substring(0, 20) === friendUid) {
                _is_tox = true
                _status = f.status;
                _user_status = f.user_status;
                break;
            }
        }
    }

    states: [
        State {
            name: "offline"
            when: _status === "offline"
            PropertyChanges { target: ring; visible: true }
            PropertyChanges { target: circle; visible: false }
            PropertyChanges { target: diamond; visible: false }
            PropertyChanges { target: crescent; visible: false }
        },
        State {
            name: "online_available"
            when: _status === "online" && _user_status === "none"
            PropertyChanges { target: circle; visible: true; color: "#4CAF50" }
            PropertyChanges { target: ring; visible: false }
            PropertyChanges { target: diamond; visible: false }
            PropertyChanges { target: crescent; visible: false }
        },
        State {
            name: "online_busy"
            when: _status === "online" && _user_status === "busy"
            PropertyChanges { target: diamond; visible: true }
            PropertyChanges { target: ring; visible: false }
            PropertyChanges { target: circle; visible: false }
            PropertyChanges { target: crescent; visible: false }
        },
        State {
            name: "online_away"
            when: _status === "online" && _user_status === "away"
            PropertyChanges { target: crescent; visible: true }
            PropertyChanges { target: ring; visible: false }
            PropertyChanges { target: circle; visible: false }
            PropertyChanges { target: diamond; visible: false }
        }
    ]

    Rectangle {
        id: ring
        anchors.fill: parent
        radius: width / 2
        border.width: 2
        border.color: "#9E9E9E"
        color: "transparent"
    }

    Rectangle {
        id: circle
        anchors.fill: parent
        radius: width / 2
        color: "#4CAF50"
    }

    Item {
        id: diamond
        anchors.fill: parent
        Rectangle {
            width: parent.width * 0.7
            height: parent.height * 0.7
            anchors.centerIn: parent
            rotation: 45
            color: "#F44336"
            antialiasing: true
        }
    }

    Canvas {
        id: crescent
        anchors.fill: parent
        visible: false
        antialiasing: true
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            var w = width;
            var h = height;
            var r = w / 2;
            ctx.beginPath();
            ctx.arc(w / 2, h / 2, r, 0, Math.PI * 2);
            ctx.fillStyle = "#FF9800";
            ctx.fill();
            ctx.globalCompositeOperation = "destination-out";
            ctx.beginPath();
            ctx.arc(w / 2 + r * 0.28, h / 2, r * 0.85, 0, Math.PI * 2);
            ctx.fill();
        }
    }
}
