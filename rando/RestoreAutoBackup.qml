
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material
import dwyco

Rectangle {
    anchors.fill: parent
    // this just makes sure mouse events don't go down to other
    // components
    MouseArea {
        anchors.fill: parent
    }
    gradient: Gradient {
        GradientStop { position: 0.0; color: primary_light }
        GradientStop { position: 1.0; color: primary_dark}
    }
    ColumnLayout {

        anchors.fill:parent
        anchors.margins: mm(3)


        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Label {
            id: backup_available
            text: qsTr("This is an Android auto-backup from a previous app installation. It has a partial backup of your randos.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true

        }
//        Label {
//            id: restore_note1
//            text: qsTr("NOTE: Loading does not change your profile, device linking, or other account related things.")
//            wrapMode: Text.WordWrap
//            Layout.fillWidth: true

//        }
        Label {
            id: restore_note2
            text: qsTr("No messages are deleted when you load this backup.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true

        }
        Button {
            id: restore_backup
            text: qsTr("Add previous randos?\n(quits now, requires restart)")
            onClicked: {
                core.load_backup()
                core.set_local_setting("reindex1", "")
                //core.set_local_setting("restore-prompt", "1")
                core.exit()
                hard_close = true
                Qt.quit()
            }
            Layout.fillWidth: true
        }


        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Button {
            id: not_now
            text: qsTr("Not now")
            onClicked: {
                //core.set_local_setting("restore-prompt", "1")
                stack.pop()
            }
            Layout.fillWidth: true
        }

    }

}

