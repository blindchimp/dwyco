
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1

Item {
    id: picpv
    property alias source : preview.source
    property alias ok_vis: ok.visible
    property alias ok_text: ok.text
    signal closed(bool ok)

    Image {
        id: preview
        anchors.fill : parent
        fillMode: Image.PreserveAspectFit
        smooth: true
        autoTransform: true
    }

    RowLayout {
        id: button_row
        anchors {
            bottom: parent.bottom
            margins: dp(20)
            horizontalCenter: parent.horizontalCenter
        }
        width: parent.width
        spacing: mm(5)

        CameraButton {
            id: ok
            text: "Send"
            onClicked: {
                //source = ""
                closed(true)
            }
            Layout.fillWidth: true
        }

        CameraButton {
            id: cancel
            text: "Cancel"
            onClicked: {
                source = ""
                closed(false)
            }
            Layout.fillWidth: true
        }

    }

}
