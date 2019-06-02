
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

Page {
    anchors.fill: parent
    header: SimpleToolbar {

    }
    font.bold: true
    background: Rectangle {
        color: primary_light
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(3)
        //spacing: mm(3)
        //width: parent.width
        Label {
            text: "(1) You take nice pictures, and Rando uploads them."
            Layout.fillWidth: true
            wrapMode: Text.Wrap

        }
        Label {
            text: "(2) Your picture is sent to a random person in the world."
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }
        Label {
            text: "(3) You get a unique picture from a random person each time you send a picture."
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }

        Item {
            Layout.fillHeight: true
        }
        Label {
            text: "Tap on a round picture to see it in full detail. Use pinching to zoom and pan as usual."
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }
        Label {
            text: "You can mark a picture FAVORITE, or DELETE it in this mode too."
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            spacing: mm(3)
            Image {
                source: mi("ic_add_a_photo_black_24dp.png")
            }

            Label {
                text: "Starts your camera, take a picture of something interesting!"
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

        }
        RowLayout {
            spacing: mm(3)
            Image {
                source: mi("ic_language_white_24dp.png")
            }

            Label {
                text: "Tap to see where in the world the picture was created."
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

        }
        RowLayout {
            spacing: mm(3)
            Image {
                source: mi("ic_language_black_24dp.png")
            }

            Label {
                text: "Tap to see where in the world your picture was sent."
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

        }
        RowLayout {
            spacing: mm(3)
            Image {
                source: mi("ic_not_interested_black_24dp.png")
            }

            Label {
                text: "Picture wasn't too interesting or didn't pass review.\nYou won't get a return rando for this picture."
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

        }
        Item {
            Layout.fillHeight: true
        }

    }

}
