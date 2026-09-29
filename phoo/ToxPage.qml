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

    // the tox status state is owned by tox_state in main.qml, which the
    // tox save page reads too, so the two status panels cannot disagree.
    // the name/address shown while signed out come from the same place,
    // which is what stops this page carrying on showing an identity the
    // save page has already deleted.

    // group-shared identity state. the identity on disk can be claimed by
    // another device in the group, in which case tox is stopped here and the
    // user can take it over by signing in again.
    property var sharedList: []
    property bool disabledByRemote: false
    property var remoteRow: null

    // one wording, shared with the tox save page's card.
    readonly property string connStatusText: tox_state.statusText

    // the extra guidance this page adds on top of the shared headline.
    readonly property string connBannerText: {
        if (tox_state.state === "locked")
            return "This Tox save is password protected. Sign in to unlock it."
        if (tox_state.state === "signedin")
            return ""
        if (tox_state.state === "empty")
            return "There is no Tox save on this device. Import one or create "
                   + "a new one on the Tox save page."
        return "You are not signed in to this Tox save."
    }

    // the banner is only for states where signing in is the thing to do.
    readonly property bool showSignInBanner:
        tox_state.state === "locked" || tox_state.state === "signedout"
        || tox_state.state === "empty"

    function shortHex(s) {
        if (!s || s.length < 12)
            return "?"
        return s.substring(0, 12) + "..."
    }

    function refreshShared() {
        sharedList = core.tox_list_saves()
        // find the identity currently on disk, if the group knows about it
        var pk = core.tox_get_self_public_key()
        remoteRow = null
        if (pk.length > 0) {
            for (var i = 0; i < sharedList.length; ++i) {
                if (sharedList[i].mid === pk && !sharedList[i].held_by_me) {
                    remoteRow = sharedList[i]
                    break
                }
            }
        }
    }

    // true when the identity on disk is signed in somewhere else, so the
    // banner can offer to take it over.
    property bool identityInUseElsewhere: remoteRow !== null

    function takeOverIdentity() {
        if (!remoteRow)
            return
        // selecting is a file level operation; it leaves tox stopped and
        // claims nothing. the sign in that follows is what takes the identity
        // over from the other device.
        var err = core.tox_select_save(remoteRow.mid)
        if (err.length > 0) {
            takeOverError.text = "Could not load that save: " + err
            return
        }
        takeOverError.text = ""
        // one sign in path, shared with the banner button and the tox save
        // page, so all of them do the same thing.
        openToxSignIn()
        disabledByRemote = false
        remoteRow = null
        refreshToxIdentity()
        refreshShared()
    }

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

    // status text and the sign in banner are now bound to tox_state, so
    // there is nothing to re-derive here.

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
        // only meaningful while tox is actually running.
        if (tox_state.state !== "signedin")
            return
        toxNameInput.text_input = core.tox_get_name()
        toxStatusInput.text_input = core.tox_get_status_message()
        // note: the cached name/address that survive a sign out are owned
        // and written by tox_state now, so both pages read the same values.
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
        // the tox status state is refreshed centrally in main.qml (see
        // tox_state), so only this page's own widgets are handled here.
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
            refreshToxIdentity()
        }
        function onTox_saves_changed() {
            refreshShared()
        }
        function onTox_disabled_by_remote(holder) {
            // another device claimed the identity we were running
            disabledByRemote = true
            refreshShared()
            refreshToxIdentity()
        }
    }

    onVisibleChanged: {
        if(visible) {
            // pick up anything that changed while this page was hidden,
            // and drop the "was turned off" wording: that describes a
            // one-off event, not a standing condition, so it must not
            // outlive the visit it happened during.
            tox_state.refresh()
            disabledByRemote = false
            refreshToxAvatar()
            refreshToxIdentity()
            refreshShared()
        }
    }

    Component.onCompleted: {
        tox_state.refresh()
        ToxFriendModel.load_friends()
        refreshToxIdentity()
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
        refreshShared()
        refreshToxAvatar()
    }

    Timer {
        interval: 5000
        running: tox_state.state === "signedin"
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
                color: tox_state.dotColor
            }

            Label {
                text: connStatusText
            }

            Label {
                text: "Status:"
                enabled: tox_state.state === "signedin"
            }

            ComboBox {
                id: userStatusCombo
                model: ["Available", "Away", "Busy"]
                enabled: tox_state.state === "signedin"
                onActivated: {
                    var map = ["none", "away", "busy"]
                    core.tox_set_user_status(map[currentIndex])
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "Tox save…"
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
                    visible: tox_state.present
                    spacing: mm(0.5)
                    Layout.fillWidth: true

                    RowLayout {
                        spacing: mm(1)
                        Layout.fillWidth: true

                        Label {
                            text: "Tox save"
                            font.bold: true
                        }

                        Label {
                            visible: tox_state.encrypted
                            text: "Password protected"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        Label {
                            visible: !tox_state.encrypted
                            text: {
                                if (tox_state.cachedName !== "")
                                    return tox_state.cachedName
                                var name = core.tox_get_name()
                                if (name !== "")
                                    return name
                                return "Unnamed"
                            }
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                        }

                        Label {
                            visible: !tox_state.encrypted
                                     && tox_state.cachedAddress.length > 0
                            font.family: "monospace"
                            font.pixelSize: 10
                            text: tox_state.displayId
                            verticalAlignment: Text.AlignVCenter
                        }

                        Button {
                            visible: !tox_state.encrypted
                                     && tox_state.cachedAddress.length > 0
                            text: "Copy"
                            onClicked: core.copy_to_clipboard(tox_state.cachedAddress)
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: mm(2)

                    Button {
                        text: "Sign in"
                        onClicked: openToxSignIn()
                    }

                    Button {
                        text: "Go to Tox save…"
                        onClicked: stack.push(tox_acct)
                    }
                }

                // the identity on this device is currently signed in on
                // another device in the group. signing in here takes it over.
                ColumnLayout {
                    visible: identityInUseElsewhere
                    Layout.fillWidth: true
                    spacing: mm(0.5)

                    Label {
                        text: disabledByRemote
                              ? "Tox was turned off here because this Tox save is now in use on another device."
                              : "This Tox save is in use on another device."
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        color: "#a00"
                        font.pixelSize: dp(12)
                    }

                    Label {
                        text: remoteRow
                              ? "Tox save " + shortHex(remoteRow.pubkey) + " is in use on device "
                                + shortHex(remoteRow.holder) + "."
                              : ""
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        color: "#666"
                        font.pixelSize: dp(11)
                    }

                    Label {
                        id: takeOverError
                        text: ""
                        color: "red"
                        visible: text.length > 0
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: dp(11)
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Button {
                            text: "Sign in here instead"
                            onClicked: takeOverIdentity()
                        }

                        Button {
                            text: "Go to Tox save…"
                            onClicked: stack.push(tox_acct)
                        }

                        Item { Layout.fillWidth: true }
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

        // note: publishing/unpublishing a shared identity lives on the
        // account page (see ToxAcct.qml), next to the shared list itself.
        // this page only reports who is holding the identity.

        RowLayout {
            enabled: tox_state.state === "signedin"
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
                enabled: tox_state.state === "signedin" && autoAwayCb.checked
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
            enabled: tox_state.state === "signedin"
            visible: tox_state.state === "signedin"
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
                enabled: tox_state.state === "signedin" && (toxNameInput.text_input !== origName || toxStatusInput.text_input !== origStatus)
                onClicked: {
                    core.tox_set_name(toxNameInput.text_input)
                    core.tox_set_status_message(toxStatusInput.text_input)
                    origName = toxNameInput.text_input
                    origStatus = toxStatusInput.text_input
                }
            }
        }

        RowLayout {
            enabled: tox_state.state === "signedin"
            visible: tox_state.state === "signedin"
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
            enabled: tox_state.state === "signedin"
            font.bold: true
            Layout.topMargin: mm(2)
        }

        TextFieldX {
            id: toxIdInput
            enabled: tox_state.state === "signedin"
            placeholder_text: "Paste Tox ID here..."
            validator: RegularExpressionValidator { regularExpression: /[0-9a-fA-F]{0,76}/ }
            Layout.fillWidth: true
        }

        Button {
            id: addFriendButton
            text: "Add Friend"
            enabled: tox_state.state === "signedin" && isValidToxId(toxIdInput.text_input)
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
            enabled: tox_state.state === "signedin"
            visible: tox_state.state === "signedin"
            font.bold: true
            Layout.topMargin: mm(2)
        }

        RowLayout {
            enabled: tox_state.state === "signedin"
            visible: tox_state.state === "signedin"
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
            enabled: tox_state.state === "signedin"
            font.bold: true
            Layout.topMargin: mm(2)
            visible: tox_state.state === "signedin"
        }

        Item {
            enabled: tox_state.state === "signedin"
            visible: tox_state.state === "signedin"
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