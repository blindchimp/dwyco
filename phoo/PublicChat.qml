
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQml.Models 2.12
import dwyco 1.0

Page {
    id: pchat
    anchors.fill: parent
    property alias model: listView1.model
    property int connect_server: 0
    
    background: Rectangle {
        color: amber_dark
        gradient: Gradient {
            GradientStop { position: 0.0; color: amber_light }
            GradientStop { position: 1.0; color: "white"}
        }
    }

    header: ToolBar {
        background: Rectangle {
            color: amber_accent
        }

        implicitWidth: parent.width


        RowLayout {
            Item {
                Layout.minimumHeight: cm(1)
            }
            anchors.fill: parent
            spacing: mm(2)

            ToolButton {
                id: back_button
                contentItem: Image {
                    anchors.centerIn: parent
                    source: mi("ic_arrow_back_black_24dp.png")
                }
                checkable: false
                onClicked: {
                    stack.pop()
                }
                Layout.fillHeight: true

            }
            ToolButton {
                id: chatlist_button
                text: "Players"

                background: Rectangle {
                    color: primary_dark
                    radius: 3
                }
                contentItem: Text {
                    x: parent.leftPadding
                    y: parent.topPadding
                    width: parent.availableWidth
                    height: parent.availableHeight

                    text: parent.text
                    font: parent.font
                    color: "white"
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                checkable: false
                visible: public_chat.visible
                onClicked: {
                    stack.push(chatlist)
                }
            }


            Item {

                Layout.fillWidth: true
            }

            ConvToolButton {

            }
        }
    }


    ListModel {
        id: chatmodel
        ListElement {
            name : "To play Competitive Trivia"
            text_msg: "Simply type in the answer, the bot will reward you if you are the first to get it right."
        }
        ListElement {
            name : "Dwyco Chat"
            text_msg: "Welcome, this is a public chat area, Wheaton Normal Form, please."
        }
    }

    // this just makes sure mouse events don't go down to other
    // components
    MouseArea {
        anchors.fill: parent
    }
    
    onVisibleChanged: {
        if(visible) {
            if(!core.is_chat_online) {
                core.switch_to_chat_server(chat_server.connect_server)
                chat_server.auto_connect = true
            }
        }

    }
    
    
    Connections {
        target: core

        onChat_event: {
            console.log("got chat event")
            console.log(cmd, sid)
            console.log(huid)
            console.log(sname)
            console.log(vdata)
            console.log(qid)
            console.log(extra_arg)
            if(cmd == DwycoCore.CHAT_CTX_RECV_DATA)
            {
                //public_chat.text = public_chat.text + "<br>" + vdata[0];
                var smname = sname.substring(0, 10)
                var smmsg = String(vdata[0]).substring(0, 500)
                model.insert(0, { "name" : smname,  "text_msg" : smmsg})
                if(model.count > 30) {
                    model.remove(30, model.count - 30)
                }
                listView1.positionViewAtBeginning()
            }
        }
    }
    

    ListView {
        id: listView1
        anchors.bottom: textField1.top
        anchors.bottomMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0
        delegate: msglist_delegate
        model: chatmodel
        spacing: dp(3)
        ScrollBar.vertical: ScrollBar { }
        verticalLayoutDirection: ListView.BottomToTop

        clip: true

    }

    Component {
        id: msglist_delegate
        
        Rectangle {
            id: ditem
            width: parent.width
            //border.width: 1
            //border.color: "gray"
            height: msg.implicitHeight
            color: Qt.rgba(1,1,1, 0)
            
            Text {
                id: msg
                text: { name + ": " + text_msg}
                anchors.left: ditem.left
                width: parent.width
                wrapMode: Text.Wrap
                opacity: 1.0
                color: primary_text
                
            }
            
        }
        
    }

    TextField {
        id: textField1

        anchors.right: toolButton1.left
        anchors.rightMargin: 1
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.bottom: parent.bottom
        //anchors.bottomMargin: 0
        placeholderText: qsTr("Type...")
        height: Math.max(implicitHeight * 1.3, contentHeight)
        wrapMode: TextInput.WordWrap
        background: Rectangle {
                    radius: 2
                    anchors.fill: parent
                    border.color: "#333"
                    border.width: 1
                    color: "lawngreen"
                }
            
        onLengthChanged: {
            //core.uid_keyboard_input(to_uid)
        }
        onInputMethodComposingChanged: {
            //core.uid_keyboard_input(to_uid)
        }

        onAccepted: {
            if(textField1.length > 0) {
                core.send_chat(textField1.text)
                textField1.text = ""
                listView1.positionViewAtBeginning()
            }

        }
        // this is here because on iOS, when you pop this
        // item, the focus stays in here somehow and the keyboard
        // will pop up on the previous screen, wtf.
        //focus: visible
        // qt 5.11 seems focus is handled a little differently
        onVisibleChanged: {
            if(Qt.platform.os == "android") {
            if(!visible)
                focus = false
            } else {
                focus = visible
            }
        }
    }
    
    Button {
        property int but_width
        property int but_height
        id: toolButton1

        height: but_height
        width: but_width
        
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 1
        anchors.bottomMargin: 1
        //anchors.verticalCenter: textField1.verticalCenter
        
        enabled: {textField1.inputMethodComposing || textField1.length > 0 || textField1.text.length > 0}
        
        background: Rectangle {
            id: bg
            color: "indigo"
            radius: 20
            anchors.fill: toolButton1
        }
        contentItem: Text {
            color: toolButton1.enabled ? "white" : "gray"
            text: toolButton1.text
            anchors.centerIn: bg
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
        
        text: "send"
        onClicked: {
            Qt.inputMethod.commit()
            Qt.inputMethod.reset()
            
            core.send_chat(textField1.text)
            textField1.text = ""
            listView1.positionViewAtBeginning()
            if(Qt.platform.os == "android") {
                notificationClient.set_user_property("triv_player", "t")
            }
        }

        Component.onCompleted: {
            but_height = textField1.height
            but_width = but_height
        }
        focusPolicy: Qt.NoFocus
    }
    
    BusyIndicator {
        id: busy1
        
        running: {!core.is_chat_online}
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }
}

