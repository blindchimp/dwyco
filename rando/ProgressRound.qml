//
// this was found on stackoverflow, but written for qtquick
// controls 1. this is just updated for qtquick.controls 2
//
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

ProgressBar
{
    id: control
    background: Item{}
   contentItem : Rectangle
   {
      color: "transparent"
      implicitWidth: img.width - cm(.5)
      implicitHeight: implicitWidth

      Rectangle
      {
         id: outerRing
         z: 0
         anchors.fill: parent
         radius: Math.max(width, height) / 2
         color: "transparent"
         border.color: "gray"
         border.width: 8
      }

      Rectangle
      {
         id: innerRing
         z: 1
         anchors.fill: parent
         anchors.margins: (outerRing.border.width - border.width) / 2
         radius: outerRing.radius
         color: "transparent"
         border.color: "darkgray"
         border.width: 4

         ConicalGradient
         {
            source: innerRing
            anchors.fill: parent
            gradient: Gradient
            {
               GradientStop { position: 0.00; color: "white" }
               GradientStop { position: control.visualPosition; color: "white" }
               GradientStop { position: control.visualPosition + 0.01; color: "transparent" }
               GradientStop { position: 1.00; color: "transparent" }
            }
         }
      }

      Text
      {
         id: progressLabel
         anchors.centerIn: parent
         color: amber_light
         style: Text.Outline
         styleColor: "black"

         text: control.indeterminate ? "" : (control.visualPosition * 100).toFixed() + "%"
      }
   }
}
