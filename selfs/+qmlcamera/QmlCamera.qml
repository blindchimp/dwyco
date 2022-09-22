import QtQuick 2.12
import QtMultimedia 5.12

Camera {
    id: preview_cam
    objectName: "qrCameraQML"
    viewfinder {
        resolution: Qt.size(320, 240)
        maximumFrameRate: 20
    }
    position: Camera.FrontFace
    captureMode: Camera.CaptureViewfinder
    onCameraStateChanged: {
        //if(state === Camera.ActiveState) {
            var res = preview_cam.supportedViewfinderResolutions();
            console.log("RESOLUTIONS ")
        for(var i = 0; i < res.length; i++) {
            console.log(res[i].width)
            console.log(res[i].height)
        }
        //}
    }
    onCameraStatusChanged: {
        //if(state === Camera.ActiveState) {
            var res = preview_cam.supportedViewfinderResolutions();
            console.log("RESOLUTIONS ")
        for(var i = 0; i < res.length; i++) {
            console.log(res[i].width)
            console.log(res[i].height)
        }

        //}
    }
}
