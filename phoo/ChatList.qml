
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

Page {
    id: chatlist_top
    //anchors.fill: parent

    header: SimpleToolbar {
        background: Rectangle {
            color: amber_accent
        }
    }

   BareChatList {
       anchors.fill: parent
   }

   onVisibleChanged: {
       if(visible) {
           Qt.inputMethod.hide()
           if(!core.is_chat_online) {
               core.switch_to_chat_server(chat_server.connect_server)
               chat_server.auto_connect = true
           }
       }
   }

   BusyIndicator {
       id: busy1

       running: {!core.is_chat_online}
       anchors.horizontalCenter: parent.horizontalCenter
       anchors.verticalCenter: parent.verticalCenter
   }

}
