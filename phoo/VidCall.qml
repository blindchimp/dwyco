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
}
