
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml 2.12
import QtQuick 2.12
import dwyco 1.0
import QtQuick.Controls 2.12

Page {
    id: profview
    property string uid
    property alias preview_source : preview.source
    property alias preview_text: handle.text
    property alias preview_desc: desc.text
    property bool dragging

    anchors.fill:parent
    header: SimpleToolbar {

    }

    function update_profile(uid) {
        preview_source = core.uid_to_profile_preview(uid)
        preview_text = core.uid_to_name(uid)
        preview_desc = core.uid_to_profile_info(uid, DwycoCore.DESCRIPTION)

    }

    Connections {
        target: core
        onSys_uid_resolved: {
            if(uid === profview.uid) {
                update_profile(uid)
            }
        }
        onSys_invalidate_profile: {
            if(uid === profview.uid) {
                preview_source = ""
                preview_text = ""
                preview_desc = ""
                update_profile(uid)
            }

        }
    }

    
    onVisibleChanged: {
        if(visible) {
            update_profile(uid)
        }
    }

    Image {
        id: preview
        anchors.top: dragging ? undefined : handle.bottom
        anchors.right: dragging ? undefined : parent.right
        anchors.left: dragging ? undefined : parent.left
        anchors.bottom: dragging ? undefined : desc.top
        //anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        cache: false
        onSourceChanged: {
            dragging = false
            scale = 1.0
        }

    }

    Text {
        id: handle
        //height: 16
        //anchors.horizontalCenter: parent.horizontalCenter
        anchors.top : parent.top
        anchors.left:parent.left
        anchors.right:parent.right
        anchors.margins: 10
        font.bold: true
        font.pointSize: 20
        clip: true
        wrapMode: Text.WordWrap
    }

    Text {
        id: desc
        //height: 16
        //anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom : parent.bottom
        anchors.left:parent.left
        anchors.right:parent.right
        anchors.margins: 10
        font.bold: false
        //font.pointSize: 20
        clip: true
        wrapMode: Text.WordWrap
    }

    PinchArea {
        anchors.fill: parent
        pinch.target: preview
        pinch.minimumScale: 0.1
        pinch.maximumScale: 10
        pinch.dragAxis: Pinch.XAndYAxis

        MouseArea {
            id: dragArea
            hoverEnabled: true
            anchors.fill: parent
            drag.target: preview
            scrollGestureEnabled: false

            onPressed: {
                dragging = true

            }
            onClicked: {
                stack.pop()
            }
        }
    }
}