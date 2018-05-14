/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.6
import QtMultimedia 5.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1
import dwyco 1.0

Item {
    property Camera camera
    property bool previewAvailable : false
    property int  cur_cam: 0

    id : captureControls
    anchors {
        bottom: parent.bottom
        margins: 8
        horizontalCenter: parent.horizontalCenter
    }
    width: parent.width
    
    RowLayout {
        spacing: 8
        anchors {
            bottom: parent.bottom
            margins: 8
            horizontalCenter: parent.horizontalCenter

        }
        width: parent.width
        z:20
        CameraButton {
            id: quitButton
            text: "Done"
            onClicked: {
                stack.pop()
            }
            visible: true
            Layout.fillWidth: true
        }
        CameraButton {
            text: "Capture"
            visible: camera.imageCapture.ready
            onClicked: {
                file_captured = ""
                //camera.imageCapture.capture()
                camera.imageCapture.captureToLocation(core.tmp_dir)
            }
            Layout.fillWidth: true
        }
        
        
        CameraButton {
            text: "Swap"
            visible: QtMultimedia.availableCameras.length > 1
            onClicked: {
                cur_cam = (cur_cam == 0 ? 1 : 0)
                captureControls.camera.deviceId = QtMultimedia.availableCameras[cur_cam].deviceId
            }
            Layout.fillWidth: true
        }
        
    }

}
