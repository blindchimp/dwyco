
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Controls

Menu {
    title: "Action"
    property string uid
    x: parent.width / 2
    y: parent.height / 2
    
    MenuItem {
        text: "View profile"
        onTriggered: {
            stack.push(theprofileview)            
        }
    }
    
//    MenuItem {
//        text: "Clear msgs"
//        onTriggered: {
//            Core.clear_messages(uid)
//            themsglist.reload_model()
//        }
//    }
    
//    MenuItem {
//        text: "Delete user"
//        onTriggered: {
//            Core.delete_user(uid)
//            themsglist.reload_model()
//        }
//    }
   MenuSeparator {
        
    }

    MenuItem {
        text: "Block user"
        onTriggered: {
            Core.set_ignore(uid, 1)            
        }
    }
//    MenuItem {
//        text: "Block and Delete user"
//        onTriggered: {
//            Core.set_ignore(uid, 1)
//            Core.delete_user(uid)
//            themsglist.reload_model()
            
//        }
//    }
    
}
