
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
import QtQuick.Controls 2.12
import dwyco 1.0

Row {
    property alias model: rep.model

    signal button_click(int index)
    signal button_pressed(int index)
    signal button_released(int index)
    signal button_triggered(int index, bool state)
    signal button_toggled(int index, bool state)

    spacing: mm(1)
    ListModel {
        id: col
        ListElement {
            oname: "mute_button"
            color: "red"
        }
        ListElement {
            oname: "mute_button"
            color: "red"
        }
        ListElement {
            oname: "mute_button"
            color: "red"
        }
        ListElement {
            oname: "mute_button"
            color: "red"
        }
        ListElement {
            oname: "mute_button"
            color: "red"
        }
        ListElement {
            oname: "mute_button"
            color: "red"
        }
        ListElement {
            oname: "mute_button"
            color: "red"
        }
        ListElement {
            oname: "mute_button"
            color: "red"
        }
        ListElement {
            oname: "mute_button"
            color: "red"
        }
        ListElement {
            oname: "mute_button"
            color: "green"
        }
        ListElement {
            oname: "mute_button"
            color: "red"
        }
        ListElement {
            oname: "mute_button"
            color: "red"
        }
    }

    Repeater {
        id: rep
        delegate : Component {
            Button {

                id: but
                contentItem: Text {
                    anchors.centerIn: parent
                    text: model.text
                }
                background: Rectangle {
                    id: bg
                    // don't do this, docs say should not use
                    // value in property bindings
                    //color: col.get(index).color
                    opacity: .3
//                    Component.onCompleted: {
//                        color = col.get(index).color
//                    }
                    color: model.backgroundColor
                }

                visible: model.visible
                enabled: model.enabled
                checkable: model.checkable
                checked: model.checked
                onClicked: {
                    button_click(index)
                }
                onPressed: {
                    button_pressed(index)
                }
                onReleased: {
                    button_released(index)
                }
            }
        }
    }
    Component.onDestruction: {
        model = undefined
    }

    onModelChanged: {
//        var i
//        for(i = 0; i < col.count; i++) {
//            var but = col.get(i).oname
//            var iof = rep.model.indexOf(but)
//            var clr = col.get(i).color

//            rep.itemAt(iof).children[0].bg = clr
//        }
    }
}
