
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import dwyco
import QtQuick.Controls.Material
import QtQuick.Layouts

Page {
    //anchors.fill: parent
    header: SimpleToolbar {

    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(3)
        spacing: mm(3)

        Label {
            text: "Click SEND to forward the message to Dwyco for special review."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Item {
            Layout.fillHeight: true
        }
        Button {
            text: "SEND for review"
            onClicked: {
                // note: don't save the message
                core.send_forward(the_man, "Abuse report", uid, mid, 0)
                stack.pop()
            }
            Layout.fillWidth: true
        }
        Item {
            Layout.fillHeight: true
        }
        Button {
            text: "Ooops, do NOT send."
            onClicked: {
                stack.pop()
            }
            Layout.fillWidth: true
        }


    }


}
