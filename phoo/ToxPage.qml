/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQml
import QtQuick
import dwyco
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

Page {
    anchors.fill: parent
    header: SimpleToolbar {

    }
    background: Rectangle {
        color: amber_light
    }

    property string origName: ""
    property string origStatus: ""
    property string connStatusText: "Not signed in"
    property string connBannerText: "You are not signed in to Tox."
    property bool showSignInBanner: true
    property string cachedName: core.get_local_setting("cached_tox_name")
    property string cachedAddress: core.get_local_setting("cached_tox_address")

    function isValidToxId(s) {
        if (s.length !== 76)
            return false
        if (!/^[0-9a-fA-F]{76}$/.test(s))
            return false
        var xorEven = 0, xorOdd = 0
        for (var i = 0; i < 36; i++) {
            var b = parseInt(s.substr(i * 2, 2), 16)
            if (i % 2 === 0)
                xorEven ^= b
            else
                xorOdd ^= b
        }
        var ck = parseInt(s.substr(72, 2), 16)
        var co = parseInt(s.substr(74, 2), 16)
        return xorEven === ck && xorOdd === co
    }

    function refreshStatus() {
        if (!core.tox_enabled) {
            connStatusText = "Not signed in"
            connBannerText = "You are not signed in to Tox."
            showSignInBanner = true
        } else if (core.tox_needs_password()) {
            connStatusText = "Pending password"
            connBannerText = "Your Tox profile is password protected. Sign in to unlock it."
            showSignInBanner = true
        } else {
            connStatusText = core.tox_connected ? "Connected" : "Connecting..."
            showSignInBanner = false
        }
    }

    function autoInitToxIdentity() {
        if (core.tox_get_name() === "" && core.tox_get_status_message() === "") {
            var genName = fname.fname()
            core.tox_set_name(genName)
            core.tox_set_status_message("toxing with DTox!")
            toxNameInput.text_input = genName
            toxStatusInput.text_input = "toxing with DTox!"
            origName = genName
            origStatus = "toxing with DTox!"
        }
    }

    function refreshToxIdentity() {
        if (!core.tox_enabled || core.tox_needs_password())
            return
        toxNameInput.text_input = core.tox_get_name()
        toxStatusInput.text_input = core.tox_get_status_message()
        core.set_local_setting("cached_tox_name", core.tox_get_name())
        core.set_local_setting("cached_tox_address", core.tox_self_address)
        cachedName = core.tox_get_name()
        cachedAddress = core.tox_self_address
        origName = toxNameInput.text_input
        origStatus = toxStatusInput.text_input
        autoInitToxIdentity()
        ToxFriendModel.load_friends()
        var curStatus = core.tox_get_user_status()
        var statusIdx = ["none", "away", "busy"].indexOf(curStatus)
        if(statusIdx >= 0)
            userStatusCombo.currentIndex = statusIdx
    }

    function toxSelfPseudoUid() {
        return core.tox_get_self_public_key().substring(0, 20)
    }

    function refreshToxAvatar() {
        var pseudo = toxSelfPseudoUid()
        if (pseudo.length === 0) {
            toxAvatarImg.source = ""
            return
        }
        toxAvatarImg.source = core.uid_to_profile_preview(pseudo)
    }

    Connections {
        target: core
        function onAuto_away_state_changed(isAway) {
            if (isAway)
                userStatusCombo.currentIndex = 1
            else {
                var curStatus = core.tox_get_user_status()
                var statusIdx = ["none", "away", "busy"].indexOf(curStatus)
                if(statusIdx >= 0)
                    userStatusCombo.currentIndex = statusIdx
            }
        }
        function onTox_user_status_changed(status) {
            var statusIdx = ["none", "away", "busy"].indexOf(status)
            if(statusIdx >= 0)
                userStatusCombo.currentIndex = statusIdx
        }
        function onTox_import_finished() {
            refreshToxIdentity()
        }
        function onTox_avatar_changed() {
            refreshToxAvatar()
        }
        function onTox_self_addressChanged() {
            refreshToxAvatar()
            refreshToxIdentity()
        }
        function onTox_self_nameChanged() {
            refreshToxIdentity()
        }
        function onTox_enabledChanged() {
            refreshToxAvatar()
            refreshStatus()
            refreshToxIdentity()
        }
        function onTox_connection_status_changed(connected) {
            refreshStatus()
        }
    }

    onVisibleChanged: {
        if(visible) {
            refreshToxAvatar()
            refreshStatus()
            refreshToxIdentity()
        }
    }

    Component.onCompleted: {
        ToxFriendModel.load_friends()
        if (core.tox_enabled && !core.tox_needs_password()) {
            toxNameInput.text_input = core.tox_get_name()
            toxStatusInput.text_input = core.tox_get_status_message()
            origName = toxNameInput.text_input
            origStatus = toxStatusInput.text_input
            autoInitToxIdentity()
            var curStatus = core.tox_get_user_status()
            var statusIdx = ["none", "away", "busy"].indexOf(curStatus)
            if(statusIdx >= 0)
                userStatusCombo.currentIndex = statusIdx
        }
        var aaEnabled = core.get_local_setting("auto_away_enabled")
        autoAwayCb.checked = (aaEnabled === "1")
        var aaTimeout = core.get_local_setting("auto_away_timeout")
        if(aaTimeout !== "") {
            var timeoutValues = [60, 120, 300, 600, 900, 1800]
            var tidx = timeoutValues.indexOf(parseInt(aaTimeout))
            if(tidx >= 0)
                autoAwayTimeout.currentIndex = tidx
        }
        var toxAutoLogin = core.get_local_setting("tox_auto_login")
        toxAutoLoginCb.checked = (toxAutoLogin === "1")
        refreshToxAvatar()
        refreshStatus()
    }

    Timer {
        interval: 5000
        running: core.tox_enabled
        repeat: true
        onTriggered: ToxFriendModel.load_friends()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mm(2)
        spacing: mm(1)

        RowLayout {
            spacing: mm(1)

            Rectangle {
                id: statusIndicator
                width: 16
                height: 16
                radius: 8
                color: core.tox_enabled ? (core.tox_connected ? "green" : "orange") : "red"
            }

            Label {
                text: connStatusText
            }

            Label {
                text: "Status:"
                enabled: core.tox_enabled
            }

            ComboBox {
                id: userStatusCombo
                model: ["Available", "Away", "Busy"]
                enabled: core.tox_enabled
                onActivated: {
                    var map = ["none", "away", "busy"]
                    core.tox_set_user_status(map[currentIndex])
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "Account..."
                onClicked: stack.push(tox_acct)
            }
        }

        Pane {
            visible: showSignInBanner
            Layout.fillWidth: true
            padding: mm(2)
            background: Rectangle {
                color: "white"
                radius: mm(1)
                border.color: "#ddd"
            }
            Layout.topMargin: mm(2)

            ColumnLayout {
                width: parent.width
                spacing: mm(1)

                Label {
                    text: connBannerText
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    font.pixelSize: dp(14)
                }

                ColumnLayout {
                    visible: core.tox_save_exists()
                    spacing: mm(0.5)
                    Layout.fillWidth: true

                    RowLayout {
                        spacing: mm(1)
                        Layout.fillWidth: true

                        Label {
                            text: "Identity:"
                            font.bold: true
                        }

                        Label {
                            visible: core.tox_save_is_encrypted()
                            text: "Encrypted profile"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        Label {
                            visible: !core.tox_save_is_encrypted()
                            text: {
                                if (cachedName !== "")
                                    return cachedName
                                var name = core.tox_get_name()
                                if (name !== "")
                                    return name
                                return "Unnamed"
                            }
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                        }

                        Label {
                            visible: !core.tox_save_is_encrypted() && cachedAddress.length > 0
                            font.family: "monospace"
                            font.pixelSize: 10
                            text: cachedAddress.substring(0, 8)
                            verticalAlignment: Text.AlignVCenter
                        }

                        Button {
                            visible: !core.tox_save_is_encrypted() && cachedAddress.length > 0
                            text: "Copy"
                            onClicked: core.copy_to_clipboard(cachedAddress)
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: mm(2)

                    Button {
                        text: "Sign In"
                        onClicked: openToxSignIn()
                    }

                    Button {
                        text: "Go to Account..."
                        onClicked: stack.push(tox_acct)
                    }
                }
            }
        }

        RowLayout {
            spacing: mm(1)

            CheckBox {
                id: toxAutoLoginCb
                text: "Login to tox automatically"
                onCheckedChanged: {
                    core.set_local_setting("tox_auto_login", checked ? "1" : "0")
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        RowLayout {
            enabled: core.tox_enabled
            spacing: mm(1)

            CheckBox {
                id: autoAwayCb
                text: "Auto-away on inactivity"
                checked: core.auto_away_enabled
                onCheckedChanged: {
                    core.auto_away_enabled = checked
                    core.set_local_setting("auto_away_enabled", checked ? "1" : "0")
                    if (checked)
                        core.start_auto_away()
                    else
                        core.stop_auto_away()
                }
            }

            ComboBox {
                id: autoAwayTimeout
                model: ["1 min", "2 min", "5 min", "10 min", "15 min", "30 min"]
                enabled: core.tox_enabled && autoAwayCb.checked
                onActivated: {
                    var values = [60, 120, 300, 600, 900, 1800]
                    core.auto_away_timeout = values[currentIndex]
                    core.set_local_setting("auto_away_timeout", values[currentIndex].toString())
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        RowLayout {
            enabled: core.tox_enabled
            visible: core.tox_enabled
            spacing: mm(1)

            TextFieldX {
                id: toxNameInput
                placeholder_text: "Enter client name..."
                Layout.fillWidth: true
            }

            TextFieldX {
                id: toxStatusInput
                placeholder_text: "Enter status message..."
                Layout.fillWidth: true
            }

            Button {
                text: "Update"
                enabled: core.tox_enabled && (toxNameInput.text_input !== origName || toxStatusInput.text_input !== origStatus)
                onClicked: {
                    core.tox_set_name(toxNameInput.text_input)
                    core.tox_set_status_message(toxStatusInput.text_input)
                    origName = toxNameInput.text_input
                    origStatus = toxStatusInput.text_input
                }
            }
        }

        RowLayout {
            enabled: core.tox_enabled
            visible: core.tox_enabled
            spacing: mm(1)

            Button {
                text: "Copy Tox ID"
                onClicked: core.copy_to_clipboard(core.tox_self_address)
            }

            Label {
                text: core.tox_self_address.substring(0, 8)
                font.family: "monospace"
                font.pixelSize: 10
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
            }
        }

        Label {
            text: "Add Friend"
            enabled: core.tox_enabled
            font.bold: true
            Layout.topMargin: mm(2)
        }

        TextFieldX {
            id: toxIdInput
            enabled: core.tox_enabled
            placeholder_text: "Paste Tox ID here..."
            validator: RegularExpressionValidator { regularExpression: /[0-9a-fA-F]{0,76}/ }
            Layout.fillWidth: true
        }

        Button {
            id: addFriendButton
            text: "Add Friend"
            enabled: core.tox_enabled && isValidToxId(toxIdInput.text_input)
            onClicked: {
                core.tox_add_friend(toxIdInput.text_input, "Hello from Phoo!")
                toxIdInput.text_input = ""
            }
            Layout.fillWidth: true
        }

        Button {
            text: "Delete Friend"
            enabled: friendList.currentIndex >= 0
            Layout.fillWidth: true
            onClicked: deleteFriendDialog.open()
        }

        Label {
            text: "Profile"
            enabled: core.tox_enabled
            visible: core.tox_enabled
            font.bold: true
            Layout.topMargin: mm(2)
        }

        RowLayout {
            enabled: core.tox_enabled
            visible: core.tox_enabled
            spacing: mm(1)

            Image {
                id: toxAvatarImg
                fillMode: Image.PreserveAspectCrop
                Layout.alignment: Qt.AlignVCenter
                Layout.minimumWidth: parent.height
                Layout.maximumWidth: parent.height
                Layout.minimumHeight: parent.height
                Layout.maximumHeight: parent.height
                sourceSize.width: 256
                sourceSize.height: 256
            }

            Button {
                text: "Set Picture..."
                onClicked: avatarFileDialog.open()
                Layout.alignment: Qt.AlignVCenter
            }

            Button {
                text: "Remove Picture"
                onClicked: core.tox_clear_avatar()
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Label {
            text: "Friends"
            enabled: core.tox_enabled
            font.bold: true
            Layout.topMargin: mm(2)
            visible: core.tox_enabled
        }

        Item {
            enabled: core.tox_enabled
            visible: core.tox_enabled
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: mm(40)

            ListView {
                id: friendList
                anchors.fill: parent
                clip: true
                spacing: mm(1)
                model: ToxFriendModel
                currentIndex: -1
                highlight: Rectangle {
                    color: amber_accent
                    opacity: 0.3
                }
                highlightMoveDuration: 200
                ScrollBar.vertical: ScrollBar { }

                delegate: Item {
                    width: ListView.view.width
                    height: mm(9)

                    MouseArea {
                        anchors.fill: parent
                        onClicked: friendList.currentIndex = index
                        onDoubleClicked: {
                            var pseudo_uid = pubkey.substring(0, 20)
                            top_dispatch.uid_selected(pseudo_uid, "clicked")
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 2
                        color: "transparent"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: mm(1)
                            spacing: mm(0.5)

                            RowLayout {
                                spacing: mm(1)
                                Layout.fillWidth: true

                                ToxBadge {
                                    friendUid: pubkey.substring(0, 20)
                                    width: 14
                                    height: 14
                                    Layout.alignment: Qt.AlignTop
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: mm(0.3)

                                    Text {
                                        text: name
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }

                                    Text {
                                        text: pubkey.substring(0, 8)
                                        font.family: "monospace"
                                        font.pixelSize: 9
                                        color: "#666"
                                        Layout.fillWidth: true
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: ToxFriendModel.count === 0
                text: qsTr("(No friends yet. Add one above.)")
            }
        }
    }

    Dialog {
        id: deleteFriendDialog
        title: "Delete Friend"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        anchors.centerIn: Overlay.overlay

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                id: deleteFriendLabel
                text: "Delete this friend and remove them from your contact list?"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            CheckBox {
                id: trashMessagesCb
                text: "Also trash all messages with this friend"
                checked: false
            }

            Label {
                text: "This will also TRASH all messages with this friend."
                color: "red"
                visible: trashMessagesCb.checked
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        onOpened: {
            var f = ToxFriendModel.get(friendList.currentIndex)
            if(f) {
                var nm = f.name
                if(nm === "")
                    nm = "(no name)"
                deleteFriendLabel.text = "Delete friend \"" + nm + "\" (" + f.pubkey.substring(0, 8) + ") and remove them from your contact list?"
            }
        }

        onAccepted: {
            var f = ToxFriendModel.get(friendList.currentIndex)
            if(trashMessagesCb.checked)
                core.trash_messages(f.pubkey.substring(0, 20))
            core.tox_delete_friend(f.pubkey)
            friendList.currentIndex = -1
            ToxFriendModel.load_friends()
            trashMessagesCb.checked = false
        }

        onRejected: {
            trashMessagesCb.checked = false
        }
    }

    FileDialog {
        id: avatarFileDialog
        title: "Choose a profile picture"
        nameFilters: ["Images (*.png *.jpg *.jpeg *.bmp *.gif)", "All files (*)"]
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        onAccepted: {
            var p = core.url_to_filename(selectedFile)
            if(p === "")
                return
            core.tox_set_avatar(p)
        }
    }
}