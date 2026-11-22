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
    id: tox_page

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

    // one layout that adapts to the width it is given, rather than a separate
    // phone layout. a narrow desktop window takes the same path as a phone.
    readonly property bool compact: width < 600

    // while a modal dialog is up, the page underneath must not react to a
    // drag that starts on the dialog (android otherwise still flicks it).
    readonly property bool dialog_open: deleteFriendDialog.visible

    // friend rows open a menu on tap-and-hold or on the trailing button, so
    // deleting a friend does not depend on a selection highlight.
    property string pendingFriendName: ""
    property string pendingFriendPubkey: ""

    property bool showFullToxId: false

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
        // the identity on disk, if the group knows about it and someone else
        // is the one running it.
        //
        // note this asks the list's own is_current rather than re-deriving the
        // pubkey here. is_current is computed against the save on disk rather
        // than the running instance, which is what makes this work while
        // signed out -- and that is exactly when this banner is relevant.
        remoteRow = null
        for (var i = 0; i < sharedList.length; ++i) {
            if (sharedList[i].is_current === true && !sharedList[i].held_by_me) {
                remoteRow = sharedList[i]
                break
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

    function openDeleteFriend(nm, pk) {
        pendingFriendName = nm
        pendingFriendPubkey = pk
        deleteFriendDialog.open()
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
        // not signed in. there is no live identity to read the profile from,
        // and whatever was last typed into these boxes belongs to the identity
        // that was running before, not to the save sitting on disk now. falling
        // back to the cached name is safe because the save-changing paths call
        // tox_state.forgetCachedIdentity(), so a brand new or newly adopted
        // identity has nothing cached and shows blank rather than inheriting
        // the previous identity's name.
        if (tox_state.state !== "signedin") {
            toxNameInput.text_input = tox_state.cachedName
            // the status message is not cached, so there is nothing honest to
            // fall back to here.
            toxStatusInput.text_input = ""
            origName = toxNameInput.text_input
            origStatus = ""
            return
        }
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
        function onTox_friend_name_changed(pseudo_uid) {
            ToxFriendModel.load_friends()
        }
        function onTox_friend_status_message_changed(pseudo_uid) {
            ToxFriendModel.load_friends()
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
            // the save on disk may have just been replaced, created or
            // deleted, so the profile boxes can be about a different identity
            // than the one they were filled in for.
            refreshToxIdentity()
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

    // one scroll surface: status, settings and the friend list all live in
    // the same ColumnLayout. keeping the controls here rather than in a
    // ListView header also keeps them in the page's own id scope, which the
    // profile refresh code relies on.
    ScrollView {
        id: tox_scroll
        anchors.fill: parent
        anchors.margins: tox_page.compact ? mm(1.5) : mm(2)
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        // ScrollView's contentItem is a Flickable, but typed as an Item here,
        // so freeze it with a Binding rather than a direct property write.
        Binding {
            target: tox_scroll.contentItem
            property: "interactive"
            value: !tox_page.dialog_open
        }
        // android otherwise lets the whole page be dragged past its edges
        // (and sideways) and then snaps it back.
        Binding {
            target: tox_scroll.contentItem
            property: "boundsBehavior"
            value: Flickable.StopAtBounds
        }
        Binding {
            target: tox_scroll.contentItem
            property: "flickableDirection"
            value: Flickable.VerticalFlick
        }

        ColumnLayout {
            width: tox_scroll.availableWidth
            spacing: mm(1)

            Pane {
                Layout.fillWidth: true
                padding: mm(1.5)
                background: Rectangle {
                    color: "white"
                    radius: mm(1)
                    border.color: "#e6ddc6"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: mm(1)

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: mm(1)

                        Rectangle {
                            id: statusIndicator
                            Layout.preferredWidth: dp(12)
                            Layout.preferredHeight: dp(12)
                            radius: width / 2
                            color: tox_state.dotColor
                        }

                        Label {
                            Layout.fillWidth: true
                            text: tox_page.connStatusText
                            font.bold: true
                            font.pixelSize: dp(13)
                            color: primary_text
                            wrapMode: Text.WordWrap
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: tox_page.compact ? 1 : 3
                        columnSpacing: mm(1)
                        rowSpacing: mm(1)

                        Label {
                            text: "Status:"
                            enabled: tox_state.state === "signedin"
                            color: secondary_text
                            font.pixelSize: dp(11)
                        }

                        ComboBox {
                            id: userStatusCombo
                            Layout.fillWidth: true
                            model: ["Available", "Away", "Busy"]
                            enabled: tox_state.state === "signedin"
                            onActivated: {
                                var map = ["none", "away", "busy"]
                                core.tox_set_user_status(map[currentIndex])
                            }
                        }

                        Button {
                            text: "Tox save…"
                            Layout.fillWidth: tox_page.compact
                            onClicked: stack.push(tox_acct)
                        }
                    }
                }
            }

            Pane {
                visible: tox_page.showSignInBanner
                Layout.fillWidth: true
                padding: mm(2)
                background: Rectangle {
                    color: "white"
                    radius: mm(1.5)
                    border.color: "#e6ddc6"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: mm(1)

                    Label {
                        text: tox_page.connBannerText
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: dp(14)
                        color: primary_text
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
                                color: primary_text
                            }

                            Label {
                                visible: tox_state.encrypted
                                text: "Password protected"
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                                color: secondary_text
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
                                color: primary_text
                            }
                        }

                        RowLayout {
                            visible: !tox_state.encrypted
                                     && tox_state.cachedAddress.length > 0
                            spacing: mm(1)
                            Layout.fillWidth: true

                            Label {
                                Layout.fillWidth: true
                                font.family: "monospace"
                                font.pixelSize: dp(10)
                                text: tox_state.displayId
                                wrapMode: Text.WrapAnywhere
                                verticalAlignment: Text.AlignVCenter
                                color: secondary_text
                            }

                            Button {
                                text: "Copy"
                                onClicked: core.copy_to_clipboard(tox_state.cachedAddress)
                            }
                        }
                    }

                    ResponsiveRow {
                        Button {
                            text: "Sign in"
                            onClicked: openToxSignIn()
                        }

                        Button {
                            text: "Go to Tox save…"
                            onClicked: stack.push(tox_acct)
                        }
                    }

                    ColumnLayout {
                        visible: tox_page.identityInUseElsewhere
                        Layout.fillWidth: true
                        spacing: mm(0.5)

                        Label {
                            text: tox_page.disabledByRemote
                                  ? "Tox was turned off here because this Tox save is now in use on another device."
                                  : "This Tox save is in use on another device."
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            color: "#a00"
                            font.pixelSize: dp(12)
                        }

                        Label {
                            text: tox_page.remoteRow
                                  ? "Tox save " + tox_page.shortHex(tox_page.remoteRow.pubkey) + " is in use on device "
                                    + tox_page.shortHex(tox_page.remoteRow.holder) + "."
                                  : ""
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            color: secondary_text
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

                        ColumnLayout {
                            visible: tox_state.diffHeadline(tox_state.diffState) !== ""
                            Layout.fillWidth: true
                            Layout.leftMargin: mm(1)
                            spacing: mm(0.25)

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: mm(0.75)

                                Rectangle {
                                    Layout.preferredWidth: mm(1.75)
                                    Layout.preferredHeight: mm(1.75)
                                    radius: width / 2
                                    color: tox_state.diffColor(tox_state.diffState)
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: tox_state.diffHeadline(tox_state.diffState)
                                    font.pixelSize: dp(11)
                                    font.bold: true
                                    elide: Text.ElideRight
                                    color: primary_text
                                }
                            }

                            Label {
                                visible: text !== ""
                                Layout.fillWidth: true
                                Layout.leftMargin: mm(2.5)
                                text: tox_state.diffSummary()
                                font.pixelSize: dp(10)
                                color: secondary_text
                                wrapMode: Text.WordWrap
                            }

                            Repeater {
                                model: tox_state.differences
                                delegate: Label {
                                    required property var modelData
                                    required property int index
                                    visible: index < tox_state.diffBulletCap
                                    Layout.fillWidth: true
                                    Layout.leftMargin: mm(2.5)
                                    text: "• " + tox_state.diffLine(modelData)
                                    font.pixelSize: dp(10)
                                    color: secondary_text
                                    wrapMode: Text.WordWrap
                                }
                            }

                            Label {
                                visible: tox_state.diffOverflow !== ""
                                Layout.fillWidth: true
                                Layout.leftMargin: mm(2.5)
                                text: tox_state.diffOverflow
                                font.pixelSize: dp(10)
                                color: secondary_text
                                wrapMode: Text.WordWrap
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.leftMargin: mm(2.5)
                                text: tox_state.diffDetail(tox_state.diffState)
                                font.pixelSize: dp(10)
                                color: secondary_text
                                wrapMode: Text.WordWrap
                            }
                        }

                        ResponsiveRow {
                            Button {
                                text: "Sign in here instead"
                                onClicked: tox_page.takeOverIdentity()
                            }

                            Button {
                                text: "Go to Tox save…"
                                onClicked: stack.push(tox_acct)
                            }
                        }
                    }
                }
            }

            Pane {
                Layout.fillWidth: true
                padding: mm(1.5)
                background: Rectangle {
                    color: "white"
                    radius: mm(1)
                    border.color: "#e6ddc6"
                }

                CheckBox {
                    id: toxAutoLoginCb
                    width: parent.width
                    text: "Login to tox automatically"
                    onCheckedChanged: {
                        core.set_local_setting("tox_auto_login", checked ? "1" : "0")
                    }
                }
            }

            // secondary settings live behind one disclosure, so the default
            // view is the status line and the friend list.
            CollapsibleSection {
                Layout.fillWidth: true
                visible: tox_state.state === "signedin"
                title: "Profile & settings"
                subtitle: "Name, status message, picture, auto-away, Tox ID"

                Pane {
                    Layout.fillWidth: true
                    padding: mm(1.5)
                    visible: tox_state.state === "signedin"
                    background: Rectangle {
                        color: "white"
                        radius: mm(1)
                        border.color: "#e6ddc6"
                    }

                    GridLayout {
                        width: parent.width
                        columns: tox_page.compact ? 1 : 3
                        columnSpacing: mm(1)
                        rowSpacing: mm(1)

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
                            Layout.fillWidth: tox_page.compact
                            enabled: tox_state.state === "signedin" && (toxNameInput.text_input !== tox_page.origName || toxStatusInput.text_input !== tox_page.origStatus)
                            onClicked: {
                                core.tox_set_name(toxNameInput.text_input)
                                core.tox_set_status_message(toxStatusInput.text_input)
                                tox_page.origName = toxNameInput.text_input
                                tox_page.origStatus = toxStatusInput.text_input
                            }
                        }
                    }
                }

                Pane {
                    Layout.fillWidth: true
                    padding: mm(1.5)
                    visible: tox_state.state === "signedin"
                    background: Rectangle {
                        color: "white"
                        radius: mm(1)
                        border.color: "#e6ddc6"
                    }

                    ColumnLayout {
                        width: parent.width
                        spacing: mm(1)

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: mm(1)

                            Label {
                                text: "Tox ID"
                                font.bold: true
                                color: primary_text
                            }

                            Item { Layout.fillWidth: true }

                            Button {
                                text: tox_page.showFullToxId ? "Hide" : "Show"
                                onClicked: tox_page.showFullToxId = !tox_page.showFullToxId
                            }

                            Button {
                                text: "Copy"
                                onClicked: core.copy_to_clipboard(core.tox_self_address)
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            text: tox_page.showFullToxId
                                  ? core.tox_self_address
                                  : core.tox_self_address.substring(0, 8) + "…"
                            font.family: "monospace"
                            font.pixelSize: dp(11)
                            color: secondary_text
                            wrapMode: tox_page.showFullToxId
                                       ? Text.WrapAnywhere : Text.NoWrap
                            elide: tox_page.showFullToxId
                                   ? Text.ElideNone : Text.ElideRight
                        }
                    }
                }

                Pane {
                    Layout.fillWidth: true
                    padding: mm(1.5)
                    visible: tox_state.state === "signedin"
                    background: Rectangle {
                        color: "white"
                        radius: mm(1)
                        border.color: "#e6ddc6"
                    }

                    ColumnLayout {
                        width: parent.width
                        spacing: mm(1)

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: mm(1)

                            Rectangle {
                                Layout.preferredWidth: mm(16)
                                Layout.preferredHeight: mm(16)
                                radius: width / 2
                                color: amber_light
                                clip: true

                                Image {
                                    id: toxAvatarImg
                                    anchors.fill: parent
                                    fillMode: Image.PreserveAspectCrop
                                    sourceSize.width: 256
                                    sourceSize.height: 256
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 0

                                Label {
                                    text: "Profile picture"
                                    font.bold: true
                                    color: primary_text
                                }

                                Label {
                                    text: "Choose an image to show to your friends."
                                    color: secondary_text
                                    font.pixelSize: dp(10)
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        ResponsiveRow {
                            Button {
                                text: "Set…"
                                onClicked: avatarFileDialog.open()
                            }

                            Button {
                                text: "Remove"
                                onClicked: core.tox_clear_avatar()
                            }
                        }
                    }
                }

                Pane {
                    Layout.fillWidth: true
                    padding: mm(1.5)
                    visible: tox_state.state === "signedin"
                    background: Rectangle {
                        color: "white"
                        radius: mm(1)
                        border.color: "#e6ddc6"
                    }

                    GridLayout {
                        width: parent.width
                        columns: tox_page.compact ? 1 : 2
                        columnSpacing: mm(1)
                        rowSpacing: mm(1)

                        CheckBox {
                            id: autoAwayCb
                            Layout.fillWidth: true
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
                            Layout.fillWidth: true
                            model: ["1 min", "2 min", "5 min", "10 min", "15 min", "30 min"]
                            enabled: tox_state.state === "signedin" && autoAwayCb.checked
                            onActivated: {
                                var values = [60, 120, 300, 600, 900, 1800]
                                core.auto_away_timeout = values[currentIndex]
                                core.set_local_setting("auto_away_timeout", values[currentIndex].toString())
                            }
                        }
                    }
                }
            }

            Pane {
                Layout.fillWidth: true
                padding: mm(1.5)
                background: Rectangle {
                    color: "white"
                    radius: mm(1)
                    border.color: "#e6ddc6"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: mm(1)

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: mm(1)

                        TextFieldX {
                            id: toxIdInput
                            enabled: tox_state.state === "signedin"
                            placeholder_text: "Paste Tox ID here..."
                            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                            validator: RegularExpressionValidator { regularExpression: /[0-9a-fA-F]{0,76}/ }
                            Layout.fillWidth: true
                        }

                        Button {
                            id: addFriendButton
                            text: "Add"
                            enabled: tox_state.state === "signedin" && tox_page.isValidToxId(toxIdInput.text_input)
                            onClicked: {
                                core.tox_add_friend(toxIdInput.text_input, "Hello from Phoo!")
                                toxIdInput.text_input = ""
                            }
                        }
                    }
                }
            }

            Label {
                text: "FRIENDS"
                font.pixelSize: dp(10)
                font.bold: true
                font.letterSpacing: dp(1)
                color: amber_dark
                Layout.topMargin: mm(1)
            }

            // friends, as plain rows in the same scroll surface. friend lists
            // are small, so a Repeater avoids introducing a nested flickable.
            Repeater {
                model: ToxFriendModel

                delegate: Rectangle {
                    id: friendRow
                    Layout.fillWidth: true
                    implicitHeight: Math.max(mm(12), friendRowLayout.implicitHeight + mm(2))
                    radius: mm(1)
                    color: friendMouse.containsMouse ? "#fdf3d8" : "transparent"

                    MouseArea {
                        id: friendMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            var pseudo_uid = pubkey.substring(0, 20)
                            top_dispatch.uid_selected(pseudo_uid, "clicked")
                        }
                        onPressAndHold: friendMenu.popup()
                    }

                    RowLayout {
                        id: friendRowLayout
                        anchors.fill: parent
                        anchors.margins: mm(1)
                        spacing: mm(1)

                        ToxBadge {
                            friendUid: pubkey.substring(0, 20)
                            Layout.preferredWidth: dp(16)
                            Layout.preferredHeight: dp(16)
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: mm(0.2)

                            Text {
                                text: name
                                font.bold: true
                                font.pixelSize: dp(12)
                                color: primary_text
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            Text {
                                text: status_message
                                visible: status_message.length > 0
                                font.pixelSize: dp(10)
                                color: secondary_text
                                elide: Text.ElideRight
                                maximumLineCount: 1
                                Layout.fillWidth: true
                            }
                        }

                        ToolButton {
                            icon.source: mi("ic_more_vert_black_24dp.png")
                            icon.width: dp(20)
                            icon.height: dp(20)
                            Layout.preferredWidth: dp(40)
                            Layout.preferredHeight: dp(40)
                            onClicked: friendMenu.popup()
                        }
                    }

                    Menu {
                        id: friendMenu

                        MenuItem {
                            text: "Open chat"
                            onTriggered: {
                                var pseudo_uid = pubkey.substring(0, 20)
                                top_dispatch.uid_selected(pseudo_uid, "clicked")
                            }
                        }

                        MenuItem {
                            text: "Copy Tox ID"
                            onTriggered: core.copy_to_clipboard(pubkey)
                        }

                        MenuItem {
                            text: "Delete friend…"
                            onTriggered: tox_page.openDeleteFriend(name, pubkey)
                        }
                    }
                }
            }

            Label {
                visible: ToxFriendModel.count === 0
                text: qsTr("(No friends yet. Add one above.)")
                color: secondary_text
                font.pixelSize: dp(11)
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }

    Dialog {
        id: deleteFriendDialog
        title: "Delete Friend"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        anchors.centerIn: Overlay.overlay
        width: Math.min(Math.max(implicitWidth, mm(70)), Overlay.overlay.width - mm(4))

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
            var nm = tox_page.pendingFriendName
            if(nm === "")
                nm = "(no name)"
            deleteFriendLabel.text = "Delete friend \"" + nm + "\" (" + tox_page.pendingFriendPubkey.substring(0, 8) + ") and remove them from your contact list?"
        }

        onAccepted: {
            if(trashMessagesCb.checked)
                core.trash_messages(tox_page.pendingFriendPubkey.substring(0, 20))
            core.tox_delete_friend(tox_page.pendingFriendPubkey)
            tox_page.pendingFriendName = ""
            tox_page.pendingFriendPubkey = ""
            ToxFriendModel.load_friends()
            trashMessagesCb.checked = false
        }

        onRejected: {
            tox_page.pendingFriendName = ""
            tox_page.pendingFriendPubkey = ""
            trashMessagesCb.checked = false
        }
    }

    FileDialog {
        id: avatarFileDialog
        title: "Choose a profile picture"
        nameFilters: ["Images (*.png *.jpg *.jpeg *.bmp *.gif *.ppm *.pgm *.pbm *.ico *.tif *.tiff *.webp *.xbm *.xpm *.svg)", "All files (*)"]
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        onAccepted: {
            var p = core.url_to_filename(selectedFile)
            if(p === "")
                return
            core.tox_set_avatar(p)
        }
    }
}
