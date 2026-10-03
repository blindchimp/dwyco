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
        extras: Label {
            text: "Tox"
            color: "white"
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            verticalAlignment: Text.AlignVCenter
        }
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

    // the friend the delete dialog is about. the list can refresh while
    // the dialog is open, so the dialog must not read a live index.
    property var pendingFriend: null

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

    ScrollView {
        id: tox_page_scroll
        anchors.fill: parent
        anchors.margins: mm(2)
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: tox_page_scroll.availableWidth
            spacing: mm(1)

            // ---- connection status, sign in ----

            Label {
                text: "Status"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            Pane {
                Layout.fillWidth: true
                padding: mm(2)
                background: Rectangle {
                    color: "white"
                    radius: mm(1)
                    border.color: "#ddd"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: mm(1)

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: mm(1)

                        Rectangle {
                            width: 16
                            height: 16
                            radius: 8
                            color: tox_state.dotColor
                        }

                        Label {
                            text: connStatusText
                            font.bold: true
                            font.pixelSize: dp(14)
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                        }

                        Label {
                            text: "Status:"
                            visible: tox_state.state === "signedin"
                            enabled: tox_state.state === "signedin"
                        }

                        ComboBox {
                            id: userStatusCombo
                            model: ["Available", "Away", "Busy"]
                            visible: tox_state.state === "signedin"
                            enabled: tox_state.state === "signedin"
                            onActivated: {
                                var map = ["none", "away", "busy"]
                                core.tox_set_user_status(map[currentIndex])
                            }
                        }
                    }

                    Label {
                        visible: showSignInBanner
                        text: connBannerText
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: dp(14)
                    }

                    // the save on disk, shown while signed out.
                    ColumnLayout {
                        visible: showSignInBanner && tox_state.present
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
                        Layout.fillWidth: true
                        spacing: mm(2)

                        Button {
                            visible: showSignInBanner && tox_state.state !== "empty"
                            text: "Sign in"
                            onClicked: openToxSignIn()
                        }

                        Button {
                            visible: tox_state.state === "empty"
                            text: "Create or import a Tox save…"
                            onClicked: stack.push(tox_acct)
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Button {
                            visible: tox_state.state !== "empty"
                            text: "Tox save…"
                            onClicked: stack.push(tox_acct)
                        }
                    }

                    // the identity on this device is currently signed in on
                    // another device in the group. signing in here takes it over.
                    ColumnLayout {
                        visible: showSignInBanner && identityInUseElsewhere
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

                            Item { Layout.fillWidth: true }
                        }
                    }
                }
            }

            // ---- options ----

            Label {
                text: "Options"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            Pane {
                Layout.fillWidth: true
                padding: mm(2)
                background: Rectangle {
                    color: "white"
                    radius: mm(1)
                    border.color: "#ddd"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: mm(1)

                    RowLayout {
                        spacing: mm(1)

                        CheckBox {
                            id: toxAutoLoginCb
                            text: "Sign in to Tox automatically"
                            onCheckedChanged: {
                                core.set_local_setting("tox_auto_login", checked ? "1" : "0")
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }

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
                }
            }

            // ---- profile: name, status message, picture, tox id ----
            // note: publishing/unpublishing a shared save lives on the
            // tox save page (see ToxAcct.qml), next to the shared list
            // itself. this page only reports who is holding the identity.

            Label {
                text: "Profile"
                visible: tox_state.state === "signedin"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            Pane {
                visible: tox_state.state === "signedin"
                Layout.fillWidth: true
                padding: mm(2)
                background: Rectangle {
                    color: "white"
                    radius: mm(1)
                    border.color: "#ddd"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: mm(1)

                    RowLayout {
                        spacing: mm(1)

                        Image {
                            id: toxAvatarImg
                            fillMode: Image.PreserveAspectCrop
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: mm(12)
                            Layout.preferredHeight: mm(12)
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

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: mm(1)

                        Label {
                            text: "Name"
                            font.bold: true
                        }

                        TextFieldX {
                            id: toxNameInput
                            placeholder_text: "Enter client name..."
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: mm(1)

                        Label {
                            text: "Status message"
                            font.bold: true
                        }

                        TextFieldX {
                            id: toxStatusInput
                            placeholder_text: "Enter status message..."
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: mm(1)

                        Item {
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

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.topMargin: mm(1)
                        implicitHeight: mm(0.25)
                        color: "#ccc"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: mm(1)

                        Label {
                            text: "Tox ID"
                            font.bold: true
                        }

                        Text {
                            Layout.fillWidth: true
                            text: core.tox_self_address
                            font.family: "monospace"
                            font.pixelSize: dp(11)
                            // phones tap through to a dialog with the full id;
                            // desktops get the whole thing inline.
                            wrapMode: is_mobile ? Text.NoWrap : Text.WrapAnywhere
                            elide: is_mobile ? Text.ElideMiddle : Text.ElideNone
                            TapHandler {
                                enabled: is_mobile
                                onTapped: toxIdDialog.open()
                            }
                        }

                        Button {
                            text: "Copy"
                            onClicked: core.copy_to_clipboard(core.tox_self_address)
                        }
                    }
                }
            }

            // ---- friends ----

            Label {
                text: "Friends"
                enabled: tox_state.state === "signedin"
                visible: tox_state.state === "signedin"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            Pane {
                enabled: tox_state.state === "signedin"
                visible: tox_state.state === "signedin"
                Layout.fillWidth: true
                padding: mm(2)
                background: Rectangle {
                    color: "white"
                    radius: mm(1)
                    border.color: "#ddd"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: mm(1)

                    TextFieldX {
                        id: toxIdInput
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

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.topMargin: mm(1)
                        implicitHeight: mm(0.25)
                        color: "#ccc"
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.minimumHeight: mm(16)
                        Layout.preferredHeight: Math.min(friendList.contentHeight + mm(2), mm(80))

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

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: mm(1)
                                        spacing: mm(1)

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

                                        // deleting belongs to the row it acts on.
                                        ToolButton {
                                            Layout.alignment: Qt.AlignVCenter
                                            Layout.preferredWidth: mm(9)
                                            Layout.preferredHeight: mm(9)
                                            contentItem: Image {
                                                anchors.centerIn: parent
                                                source: mi("ic_delete_black_24dp.png")
                                            }
                                            onClicked: {
                                                var f = ToxFriendModel.get(index)
                                                if (f) {
                                                    pendingFriend = f
                                                    deleteFriendDialog.open()
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
            var f = pendingFriend
            if(f) {
                var nm = f.name
                if(nm === "")
                    nm = "(no name)"
                deleteFriendLabel.text = "Delete friend \"" + nm + "\" (" + f.pubkey.substring(0, 8) + ") and remove them from your contact list?"
            }
        }

        onAccepted: {
            var f = pendingFriend
            if(f) {
                if(trashMessagesCb.checked)
                    core.trash_messages(f.pubkey.substring(0, 20))
                core.tox_delete_friend(f.pubkey)
            }
            friendList.currentIndex = -1
            ToxFriendModel.load_friends()
            trashMessagesCb.checked = false
            pendingFriend = null
        }

        onRejected: {
            trashMessagesCb.checked = false
            pendingFriend = null
        }
    }

    // the full id, for touch screens where the row only shows a truncation.
    Dialog {
        id: toxIdDialog
        title: "Tox ID"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton
        width: Overlay.overlay
               ? Math.min(Overlay.overlay.width - mm(4), mm(90))
               : mm(80)

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            TextEdit {
                text: core.tox_self_address
                readOnly: true
                selectByMouse: true
                font.family: "monospace"
                font.pixelSize: dp(12)
                wrapMode: TextEdit.WrapAnywhere
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight
            }

            Label {
                text: "Anyone with this ID can add you as a contact."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#666"
                font.pixelSize: dp(11)
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Copy"
                    onClicked: core.copy_to_clipboard(core.tox_self_address)
                }

                Button {
                    text: "Close"
                    onClicked: toxIdDialog.close()
                }
            }
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
