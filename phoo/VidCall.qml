import QtQuick 2.9
import dwyco 1.0

VidCallForm {
    Connections {
        target: core

        onVideo_display: {
            vid_incoming.source = img_path
        }

        onVideo_capture_preview: {
            vid_preview.source = img_path
        }

    }
    vid_pause_button.onClicked: {
        console.log("okok")
    }
    vid_pause_button.contentItem:  Image {
            anchors.centerIn: vid_pause_button
            source: mi("ic_pause_circle_outline_white_24dp.png")
        }

}
