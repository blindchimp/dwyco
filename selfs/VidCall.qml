import QtQml 2.12
import QtQuick 2.12
import dwyco 1.0

VidCallForm {
    property bool paused: false
    property int a_ui_id: -1
    //property string a_uid: ""

    Connections {
        target: core

        function onVideo_display(ui_id, frame_number, img_path) {
            if(ui_id != a_ui_id)
                return
            if(!paused) {
                vid_incoming.source = img_path
            }
        }

        function onVideo_capture_preview(img_path) {
            vid_preview.source = img_path
        }

//        onSc_rem_pause: {
//            console.log("pause");
//            paused = true
//            vid_incoming.source = mi("ic_pause_circle_outline_white_24dp.png")

//        }

//        onSc_rem_unpause: {
//            console.log("unpause");
//            paused = false

//        }

    }
//    vid_pause_button.onClicked: {
//        console.log("okok")
//    }
//    vid_pause_button.contentItem:  Image {
//            anchors.centerIn: vid_pause_button
//            width: Math.min(vid_preview.paintedHeight, vid_preview.paintedWidth)
//            height: Math.min(vid_preview.paintedHeight, vid_preview.paintedWidth)
//            source: mi("ic_pause_circle_outline_white_24dp.png")
//        }

}
