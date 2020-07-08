
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12

Item {
    property Item container;

    state: "start"
    states: [
        State {
            name: "start"
            AnchorChanges { target: container; anchors.verticalCenter: undefined }
            AnchorChanges { target: container; anchors.bottom: container.parent.top }
        },
        State {
            name: "center"
            AnchorChanges { target: container; anchors.bottom: undefined }
            AnchorChanges { target: container; anchors.verticalCenter: container.parent.verticalCenter }
        }
    ]

    transitions: Transition {
        AnchorAnimation { duration: 800; easing.type: Easing.InOutBack; easing.overshoot: 2 }
    }
}

