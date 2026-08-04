
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
    property string importFile: ""
    property string importPw: ""
    property bool pwTick: false

    function profileHasPassword() {
        return core.tox_has_profile_password()
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
        toxNameInput.text_input = core.tox_get_name()
        toxStatusInput.text_input = core.tox_get_status_message()
        origName = toxNameInput.text_input
        origStatus = toxStatusInput.text_input
        autoInitToxIdentity()
        pwTick = !pwTick
        ToxFriendModel.load_friends()
        var curStatus = core.tox_get_user_status()
        var statusIdx = ["none", "away", "busy"].indexOf(curStatus)
        if(statusIdx >= 0)
            userStatusCombo.currentIndex = statusIdx
    }

    function startImportConfirm(p, pw) {
        importFile = p
        importPw = pw
        importConfirmDialog.open()
    }

    function isInvisibleChar(c) {
        var code = c.charCodeAt(0)
        return code < 33 || /\s/.test(c)
    }

    function visiblePrefix(s, n) {
        var out = ""
        for (var i = 0; i < s.length && out.length < n; ++i) {
            var c = s.charAt(i)
            if (isInvisibleChar(c))
                continue
            out += c
        }
        return out
    }

    function sanitizeFilename(s) {
        var out = ""
        for (var i = 0; i < s.length; ++i) {
            var c = s.charAt(i)
            if (c === "/" || c === "\\" || c === ":" || c === "*" ||
                c === "?" || c === "\"" || c === "<" || c === ">" || c === "|")
                out += "_"
            else
                out += c
        }
        return out
    }

    function exportDefaultName() {
        var name8 = sanitizeFilename(visiblePrefix(core.tox_get_name(), 8))
        var id = core.tox_self_address
        if (name8.length === 0)
            return "tox-" + id.substring(0, 8) + ".tox"
        return name8 + id.substring(0, 4) + ".tox"
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
            toxAvatarImg.source = ""
            toxAvatarImg.source = core.uid_to_profile_preview(core.tox_self_address.substring(0,20))
        }
        // function onTox_self_addressChanged() {
        //     refreshToxAvatar()
        // }
    }

    Component.onCompleted: {
        var a = core.get_local_setting("tox_enabled")
        if(a === "" || a === "0") {
            enable_tox_cb.checked = false
        } else {
            enable_tox_cb.checked = true
        }
        ToxFriendModel.load_friends()
        toxNameInput.text_input = core.tox_get_name()
        toxStatusInput.text_input = core.tox_get_status_message()
        origName = toxNameInput.text_input
        origStatus = toxStatusInput.text_input
        autoInitToxIdentity()
        var curStatus = core.tox_get_user_status()
        var statusIdx = ["none", "away", "busy"].indexOf(curStatus)
        if(statusIdx >= 0)
            userStatusCombo.currentIndex = statusIdx

        if(core.tox_enabled && core.tox_needs_password()) {
            unlockPwInput.text = ""
            unlockError.text = ""
            unlockDialog.open()
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
        //refreshToxAvatar()
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

                CheckBox {
                    id: enable_tox_cb
                    text: "Enable Tox"
                    onCheckedChanged: {
                        core.set_local_setting("tox_enabled", checked ? "1" : "0")
                        if(checked) {
                            core.enable_tox()
                            toxNameInput.text_input = core.tox_get_name()
                            toxStatusInput.text_input = core.tox_get_status_message()
                            origName = toxNameInput.text_input
                            origStatus = toxStatusInput.text_input
                            autoInitToxIdentity()
                            if(core.tox_needs_password()) {
                                unlockPwInput.text = ""
                                unlockError.text = ""
                                unlockDialog.open()
                            }
                        } else {
                            core.disable_tox()
                        }
                    }
                }

                Rectangle {
                    id: statusIndicator
                    width: 16
                    height: 16
                    radius: 8
                    color: core.tox_connected ? "green" : "red"
                    enabled: core.tox_enabled
                }

                Label {
                    text: core.tox_connected ? "Connected" : "Not connected"
                    enabled: core.tox_enabled
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
                spacing: mm(1)

                Button {
                    text: "Copy Tox ID"
                    onClicked: core.copy_to_clipboard(core.tox_self_address)
                }

                Label {
                    id: toxIdField
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
                Layout.fillWidth: true
            }

            Button {
                id: addFriendButton
                text: "Add Friend"
                enabled: core.tox_enabled && toxIdInput.text_input.length > 0
                onClicked: {
                    core.tox_add_friend(toxIdInput.text_input, "Hello from Phoo!")
                    toxIdInput.text_input = ""
                }
                Layout.fillWidth: true
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

            Button {
                text: "Delete Friend"
                enabled: friendList.currentIndex >= 0
                Layout.fillWidth: true
                onClicked: deleteFriendDialog.open()
            }

            Label {
                text: "Profile"
                enabled: core.tox_enabled
                font.bold: true
                Layout.topMargin: mm(2)
            }

            RowLayout {
                enabled: core.tox_enabled
                visible: core.tox_enabled
                spacing: mm(1)


                Image {
                    id: toxAvatarImg
                    width: 32
                    height: 32
                    fillMode: Image.PreserveAspectCrop
                    Layout.alignment: Qt.AlignVCenter
                    source: core.uid_to_profile_preview(core.tox_self_address.substring(0,20))
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

            RowLayout {
                enabled: core.tox_enabled
                spacing: mm(1)

                Button {
                    text: "Import qTox Profile..."
                    onClicked: importFileDialog.open()
                }

                Button {
                    text: "Export Profile..."
                    onClicked: {
                        var locs = StandardPaths.standardLocations(StandardPaths.DocumentsLocation)
                        if (locs.length > 0) {
                            exportFileDialog.currentFolder = locs[0]
                            exportFileDialog.currentFile = locs[0].toString() + "/" + exportDefaultName()
                        }
                        exportFileDialog.open()
                    }
                }

                Button {
                    text: profileHasPassword() ? "Change Password" : "Set Password"
                    onClicked: {
                        setPwInput.text = ""
                        setPwConfirmInput.text = ""
                        setPwOldInput.text = ""
                        setPwError.text = ""
                        setPwDialog.open()
                    }
                }

                Button {
                    text: "Remove Password"
                    visible: profileHasPassword()
                    onClicked: {
                        removePwInput.text = ""
                        removePwError.text = ""
                        removePwDialog.open()
                    }
                }

                Item {
                    Layout.fillWidth: true
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
                text: "Delete this friend and remove them from your contact list?"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            CheckBox {
                id: trashMessagesCb
                text: "Also trash all messages with this friend"
                checked: false
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
        id: importFileDialog
        title: "Choose a qTox profile (.tox) to import"
        nameFilters: ["Tox profiles (*.tox)", "All files (*)"]
        onAccepted: {
            var p = core.url_to_filename(selectedFile)
            if(p === "") {
                importResultText.text = "Could not use that file."
                importResultDialog.open()
                return
            }
            if(core.tox_file_is_encrypted(p)) {
                importFile = p
                importPwInput.text = ""
                importPwError.text = ""
                importPwDialog.open()
            } else {
                startImportConfirm(p, "")
            }
        }
    }

    FileDialog {
        id: exportFileDialog
        title: "Export Tox profile"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Tox profiles (*.tox)"]
        onAccepted: {
            var p = core.url_to_filename(selectedFile)
            if(p === "") {
                exportResultText.text = "Could not use that location."
                exportResultDialog.open()
                return
            }
            if(p.toLowerCase().lastIndexOf(".tox") !== p.length - 4)
                p += ".tox"
            var err = core.tox_export_profile(p)
            if(err.length > 0)
                exportResultText.text = "Export failed: " + err
            else
                exportResultText.text = "Profile exported to " + p
            exportResultDialog.open()
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

    Dialog {
        id: importPwDialog
        title: "qTox profile is password-protected"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "This qTox profile was saved with a password. Enter it to import."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            TextField {
                id: importPwInput
                echoMode: TextInput.Password
                placeholderText: "qTox profile password"
                Layout.fillWidth: true
            }

            Label {
                id: importPwError
                text: ""
                color: "red"
                visible: text.length > 0
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: importPwDialog.close()
                }

                Button {
                    text: "Next"
                    onClicked: {
                        importPwError.text = ""
                        startImportConfirm(importFile, importPwInput.text)
                        importPwDialog.close()
                    }
                }
            }
        }
    }

    Dialog {
        id: importConfirmDialog
        title: "Import qTox Profile"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "Importing this qTox profile will replace your current Tox identity and friend list. Message history is not stored in tox profiles and will not be imported."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            CheckBox {
                id: noBackupCb
                text: "Don't save a backup of the current profile"
                checked: false
            }

            Label {
                id: importConfirmError
                text: ""
                color: "red"
                visible: text.length > 0
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: importConfirmDialog.close()
                }

                Button {
                    text: "Import"
                    onClicked: {
                        importConfirmError.text = ""
                        var err = core.tox_import_profile(importFile, importPw, !noBackupCb.checked)
                        if(err.length > 0) {
                            importConfirmError.text = "Import failed: " + err
                        } else {
                            importConfirmDialog.close()
                            importResultText.text = "Imported. Your new Tox identity is now in use."
                            importResultDialog.open()
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: importResultDialog
        title: "Import Complete"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok

        Label {
            id: importResultText
            text: ""
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    Dialog {
        id: exportResultDialog
        title: "Export Complete"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok

        Label {
            id: exportResultText
            text: ""
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    Dialog {
        id: unlockDialog
        title: "Tox profile is password-protected"
        modal: true
        closePolicy: Dialog.NoAutoClose
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton
        onRejected: enable_tox_cb.checked = false
        onOpened: unlockPwInput.forceActiveFocus()

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "Enter the password for this Tox profile to use it."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            TextField {
                id: unlockPwInput
                echoMode: TextInput.Password
                placeholderText: "Password"
                Layout.fillWidth: true
                onAccepted: unlockButton.clicked()
            }

            Label {
                id: unlockError
                text: ""
                color: "red"
                visible: text.length > 0
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: unlockDialog.reject()
                }

                Button {
                    id: unlockButton
                    text: "Unlock"
                    enabled: unlockPwInput.text.length > 0
                    onClicked: {
                        if(core.tox_unlock(unlockPwInput.text)) {
                            unlockDialog.close()
                            refreshToxIdentity()
                        } else {
                            unlockError.text = "Wrong password or corrupt profile. Try again."
                            unlockPwInput.text = ""
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: setPwDialog
        title: "Set Profile Password"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "Choose a password. You will need to enter it each time Tox is enabled."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            TextField {
                id: setPwOldInput
                echoMode: TextInput.Password
                placeholderText: "Current password"
                Layout.fillWidth: true
                visible: profileHasPassword()
            }

            TextField {
                id: setPwInput
                echoMode: TextInput.Password
                placeholderText: "New password"
                Layout.fillWidth: true
            }

            TextField {
                id: setPwConfirmInput
                echoMode: TextInput.Password
                placeholderText: "Confirm password"
                Layout.fillWidth: true
            }

            Label {
                id: setPwError
                text: ""
                color: "red"
                visible: text.length > 0
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: setPwDialog.close()
                }

                Button {
                    text: "Set Password"
                    enabled: setPwInput.text.length > 0
                    onClicked: {
                        setPwError.text = ""
                        if(profileHasPassword()) {
                            if(setPwOldInput.text.length === 0) {
                                setPwError.text = "Enter the current password."
                                return
                            }
                            if(!core.tox_check_password(setPwOldInput.text)) {
                                setPwError.text = "Current password is incorrect."
                                setPwOldInput.text = ""
                                return
                            }
                        }
                        if(setPwInput.text !== setPwConfirmInput.text) {
                            setPwError.text = "Passwords do not match."
                            return
                        }
                        if(core.tox_set_profile_password(setPwInput.text)) {
                            pwTick = !pwTick
                            setPwDialog.close()
                        } else {
                            setPwError.text = "Could not set password."
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: removePwDialog
        title: "Remove Profile Password"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "Remove the password from this Tox profile? The profile will be saved without encryption."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            TextField {
                id: removePwInput
                echoMode: TextInput.Password
                placeholderText: "Current password"
                Layout.fillWidth: true
            }

            Label {
                id: removePwError
                text: ""
                color: "red"
                visible: text.length > 0
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: removePwDialog.close()
                }

                Button {
                    text: "Remove Password"
                    enabled: removePwInput.text.length > 0
                    onClicked: {
                        removePwError.text = ""
                        if(!core.tox_check_password(removePwInput.text)) {
                            removePwError.text = "Current password is incorrect."
                            removePwInput.text = ""
                            return
                        }
                        if(core.tox_set_profile_password("")) {
                            pwTick = !pwTick
                            removePwDialog.close()
                        } else {
                            removePwError.text = "Could not remove password."
                        }
                    }
                }
            }
        }
    }
}
