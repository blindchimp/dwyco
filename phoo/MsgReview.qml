
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

    property string rando_bot: "13404a7fc7664a943a20"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(3)
        spacing: mm(3)


        Button {
            text: "Ok"
            onClicked: {
                // note: don't save the message
                Core.send_forward(rando_bot, "Ok", mid, 1)
                Core.set_tag_message(mid, "_hid")
                stack.pop()
            }
            Layout.fillWidth: true
        }
        Item {
            Layout.fillHeight: true
        }
        Button {
            text: "Nope"
            onClicked: {
                Core.simple_send(uid, msg_text.text)
                stack.pop()
                Core.set_tag_message(mid, "_hid")
            }
            Layout.fillWidth: true
        }


    }


}
