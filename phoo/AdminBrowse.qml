
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.12
import Qt.labs.folderlistmodel 2.12
import QtQml 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Rectangle {
    property alias folder: folderModel.folder

    GridView {
        //width: 200; height: 400
        id: grid
        anchors.fill: parent
        cellWidth: 160 ; cellHeight: 120

        FolderListModel {
            id: folderModel
            nameFilters: ["*.ppm", "*.jpg"]
        }

        Component {
            id: fileDelegate
            Item {
                width: grid.cellWidth; height: grid.cellHeight
                ColumnLayout {
                    anchors.fill: parent
                    Image {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        source: fileURL
                        fillMode: Image.PreserveAspectFit
                    }
                    Text {
                        Layout.fillWidth: true
                        text: fileName.substr(0,6)
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var filename = filePath.replace(".ppm.jpg", "")

                        //filename = filePath + "/" + filename
                        themsgview.view_id = -1
                        themsgview.view_source = ""
                        var vid = core.make_zap_view_file(filename)
                        themsgview.view_id = vid
                        themsgview.mid = ""
                        //core.play_zap_view(vid)
                        stack.push(themsgview)
                    }
                }
            }



        }

        model: folderModel
        delegate: fileDelegate
    }
}

