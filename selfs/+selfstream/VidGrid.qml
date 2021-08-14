/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.12
import dwyco 1.0
import QtQuick.Layouts 1.3

Page {
    anchors.fill: parent

    header:
        Label {
            text: "Dwyco Selfie Stream Viewer " + core.buildtime + " " + (core.is_database_online === 1 ? "(online)" : "(offline)")
            font.bold: true
            color: "white"
            background: Rectangle {
                color: core.is_database_online === 1 ? "green" : "red"
            }
    }



    Component {
        id: video_delegate

        ColumnLayout {
            property int connected: 0
            width: gview.cellWidth
            height: gview.cellHeight
            RowLayout {
                Layout.fillWidth: true
                ToolButton {
                    text: "X"
                    onClicked: {
                        console.log("delete ", model.uid)
                    }
                    Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter
                }
                Label {
                    text: model.display
                    Layout.alignment: Qt.AlignLeft|Qt.AlignVCenter
                    background: Rectangle {
                        color: model.online ? "green" : "white"
                    }
                }
                Label {
                    text: model.ip
                }

                Item {
                    Layout.fillWidth: true
                }

            }
            Button {
                text: "connect"
                Layout.fillHeight: true
                Layout.fillWidth: true
                visible: connected === 0 && model.online
            }

            VidCall {
                id: vc
                visible: connected !== 0
                Layout.fillHeight: true

            }
        }
    }

    GridView {
        id: gview
        anchors.fill: parent
        cellHeight: parent.height / 2
        cellWidth: parent.width / 2

        model: DiscoverList
        delegate: video_delegate

    }

}
