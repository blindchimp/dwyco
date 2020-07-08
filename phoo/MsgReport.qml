
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
import dwyco 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

Page {
    anchors.fill: parent
    header: SimpleToolbar {

    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(3)
        spacing: mm(3)

        Label {
            text: "Click SEND to forward the message to Dwyco. We will review the message and contact you if more details are needed. The user DWYCO will appear in your contact list to help with future messages regarding this issue."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Label {
            text: "Please use the BLOCK feature to stop future contact from this user if you feel you are being harassed."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.italic: true
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
