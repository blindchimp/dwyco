import QtQuick 2.0
import dwyco 1.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1

Item {
    signal uid_selected(string uid, string action)

    Component {
        id: chatlist_delegate
        Rectangle {
            property int showit
            showit: (REVIEWED && REGULAR) || show_unreviewed
            height: showit ? picht() : 0
            width: parent.width
            //opacity: {multiselect_mode && selected ? 0.5 : 1.0}
            color: primary_dark
            border.width: 1
            gradient: Gradient {
                GradientStop { position: 0.0; color: amber_light }
                GradientStop { position: 1.0; color: amber_dark}
            }
            visible: showit
            RowLayout {
                id: drow
                spacing: mm(1)
                anchors.fill: parent

                CircularImage {
                    id: ppic
                    //width: dp(80)
                    //height: dp(60)
                    source : {
                        (!invalid && (show_unreviewed || (REVIEWED && REGULAR)) && resolved_counter > -1) ?
                                    core.uid_to_profile_preview(uid) :
                                    "qrc:/new/red32/icons/red-32x32/exclamation-32x32.png"
                    }
                    //anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectCrop
                    Layout.minimumWidth: picht()
                    Layout.maximumWidth: picht()
                    Layout.minimumHeight: picht()
                    Layout.maximumHeight: picht()
                    Layout.margins: mm(.125)

                    Image {
                        id: has_msgs
                        width: .3 * ppic.height
                        height: .3 * ppic.height
                        source: "qrc:/new/red32/icons/red-32x32/arrow right-32x32.png"
                        z:3
                        anchors.top:parent.top
                        anchors.left:parent.left
                        visible: {unseen_count > 0 ? true : false }
                    }
                }

                Text {
                    id: display_name
                    color: {active ? "red" : "black"}
                    font.italic: {active ? true : false}
                    text: display
                    //anchors.verticalCenter: parent.verticalCenter

                    elide: Text.ElideRight
                    clip: true
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true

                }
            }
            MouseArea {
                anchors.fill: drow
                onClicked: {
                    console.log("click")
                    console.log(index)
                    listView2.currentIndex = index

                    uid_selected(uid, "clicked")

                }
                onPressAndHold:  {
                    listView2.currentIndex = index
                    uid_selected(uid, "hold")
                }


            }

        }

    }

    Component.onCompleted: {
        uid_selected.connect(top_dispatch.uid_selected)

    }


    Connections {
        target: core
        onSys_invalidate_profile: {
            console.log("chatlist invalidate " + uid)
        }
    }

    ListView {
        id: listView2
        anchors.fill:parent

        model: ChatListModel
        delegate: chatlist_delegate
        clip: true
        //spacing: 5
        ScrollBar.vertical: ScrollBar { }

    }

}
