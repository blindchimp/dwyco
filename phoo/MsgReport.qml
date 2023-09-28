
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import dwyco
import QtQuick.Controls
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
            text: "Click SEND to forward the message to Dwyco. We will review the message within 24 hours and contact you if more details are needed. The user DWYCO will appear in your contact list to help with future messages regarding this issue."
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
                core.send_forward(the_man, "Abuse report", mid, 1)
                stack.pop()
            }
            Layout.fillWidth: true
        }
        Button {
            text: "SEND for review AND BLOCK User"
            onClicked: {
                core.send_forward(the_man, "Abuse report", mid, 1)
                core.set_ignore(uid, 1)
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
