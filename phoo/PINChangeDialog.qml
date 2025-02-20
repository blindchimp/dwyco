
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import dwyco

Pane {
    anchors.fill: parent

    PINDialog {
        id: confirm_old_pw
        visible: {password_ok === 0}
        title: "Enter Old PIN"

        exit_button.onClicked: {
            reset()
            load_cpw()
            stack.pop()
        }
        Component.onCompleted: {
            load_cpw()
        }
    }


    PINDialog {
        id: enter_new_pin
        visible: confirm_old_pw.password_ok === 1
        show_set_pin_button: true
        set_pin_button_text: {(pw.length > 0) ? qsTr("Set PIN") :
                                 qsTr("Turn off PIN")}
        title: "Enter New PIN"
        exit_button.onClicked: {
            confirm_old_pw.reset()
            confirm_old_pw.load_cpw()
            reset()
            stack.pop()

        }
        set_pin_button.onClicked: {
            if(pw.length > 0) {
                var salt = Core.random_string(10)
                var total = salt + pw
                var hash = Qt.md5(total)
                Core.set_local_setting("salt", salt)
                Core.set_local_setting("pw", hash)
                //Core.set_local_setting("cpw", pw)
            } else {
                Core.set_local_setting("salt", "")
                Core.set_local_setting("pw", "")
                //Core.set_local_setting("cpw", "")
            }

            confirm_old_pw.reset()
            confirm_old_pw.load_cpw()
            reset()
            Core.set_local_setting("pin_duration", "0")
            var expire = pin_expire()
            Core.set_local_setting("pin_expire", expire.toString())
            pwdialog.pw_expire_time = expire
            pwdialog.reset()
            pwdialog.load_cpw()
            stack.pop()
        }
    }

}

