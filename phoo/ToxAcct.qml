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
    id: tox_acct

    anchors.fill: parent
    header: SimpleToolbar {
        extras: Label {
            text: "Tox Save"
            color: "white"
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
    background: Rectangle {
        color: amber_light
    }

    // ---- state ----

    property var allSavesList: []
    // just the group-shared rows (the _tox_mid tag payloads). the local save
    // is not in here: it has its own hero card at the top of the page.
    property var sharedSaves: []
    property string selectedKey: ""
    property bool selectedEncrypted: false
    // the row a confirmation dialog is about. the list can refresh while a
    // dialog is open, so the dialog must not read a live list index.
    property var pendingRow: null

    // pubkey of the save on this device, for the hero's avatar tint and the
    // fallback copy. the local row carries it even when signed out or locked,
    // where core's live lookups come back empty.
    readonly property string heroPubkey: {
        for(var i = 0; i < allSavesList.length; ++i) {
            var r = allSavesList[i]
            if(r.local && r.pubkey && r.pubkey.length > 0)
                return r.pubkey
        }
        return core.tox_get_self_public_key()
    }

    function doRefresh() {
        tox_state.refresh()
        refreshAllSaves()
    }

    function indexOfKey(k) {
        for(var i = 0; i < allSavesList.length; ++i) {
            if(allSavesList[i].key === k)
                return i
        }
        return -1
    }

    // derive the password-field state from the selected row, so it can't
    // drift out of sync with what is actually selected.
    function syncSelected() {
        var i = indexOfKey(selectedKey)
        if(i < 0) {
            selectedEncrypted = false
            return
        }
        selectedEncrypted = allSavesList[i].encrypted === true
    }

    // the selected shared row, or null when the selection is the local save
    // (or nothing). the hero card owns the local save, so the shared list's
    // action strip only ever acts on a shared row.
    function selectedSharedRow() {
        var i = indexOfKey(selectedKey)
        if(i < 0 || allSavesList[i].local === true)
            return null
        return allSavesList[i]
    }

    function shortPub(pk) {
        if (!pk || pk.length < 12)
            return "?"
        return pk.substring(0, 12) + "…"
    }

    function sizeLabel(n) {
        if (n < 1024)
            return n + " B"
        if (n < 1024 * 1024)
            return Math.round(n / 1024) + " KB"
        return (Math.round((n / (1024 * 1024)) * 10) / 10) + " MB"
    }

    function ageLabel(secs) {
        if (!secs || secs <= 0)
            return ""
        var d = Math.floor(secs / 86400)
        if (d >= 1)
            return d + (d === 1 ? " day ago" : " days ago")
        var h = Math.floor(secs / 3600)
        if (h >= 1)
            return h + (h === 1 ? " hour ago" : " hours ago")
        var m = Math.max(1, Math.floor(secs / 60))
        return m + (m === 1 ? " min ago" : " min ago")
    }

    // the comparison itself (state code, wording, per-difference sentences) lives
    // on tox_state in main.qml, because the tox page shows it too and the two
    // must not disagree. read tox_state.diffState / diffHeadline() /
    // diffLine() there.

    function refreshAllSaves() {
        allSavesList = core.tox_list_all_saves()
        sharedSaves = allSavesList.filter(function(r) { return r.local !== true })
        // keep the current selection across a refresh if that row still exists,
        // otherwise fall back to the first row.
        if(selectedKey !== "" && indexOfKey(selectedKey) < 0)
            selectedKey = ""
        if(selectedKey === "" && allSavesList.length > 0)
            selectedKey = allSavesList[0].key
        syncSelected()
    }

    // small helper for the local save's title. no row object exists for it,
    // so its bits live on the page.
    function localSafeName() {
        return tox_state.selfName.length > 0 ? tox_state.selfName
                                             : tox_state.cachedName
    }

    // name we know for an identity through dwyco, else "".
    function nameOf(row) {
        return row && row.name ? row.name : ""
    }

    // primary label: the dwyco name when we have one, else the short pubkey.
    // an encrypted save with no known name falls back to describing the state,
    // since there is genuinely no identifier we can show for it.
    function rowTitle(row) {
        var n = nameOf(row)
        if(n.length > 0)
            return n
        if(row.pubkey && row.pubkey.length > 0)
            return shortPub(row.pubkey)
        if(row.encrypted)
            return "Password protected save"
        return "Tox save"
    }

    function rowSubLabel(row) {
        var bits = []
        if(row.local)
            bits.push("The save on this device")
        else
            bits.push("Shared by your devices")
        if(row.encrypted)
            bits.push("Password protected")
        else if(row.size > 0)
            bits.push(sizeLabel(row.size))
        if(!row.local && row.when > 0)
            bits.push(ageLabel(Math.floor(Date.now() / 1000) - row.when))
        return bits.join(" · ")
    }

    // stable tint for an identity's monogram, derived from its pubkey so the
    // same tox-id always gets the same color.
    function avatarTint(pk) {
        if(!pk || pk.length < 6)
            return "#b0a58a"
        // spread from the first, middle and last bytes so similar keys do not
        // end up nearly adjacent on the wheel.
        var h = (parseInt(pk.substring(0, 2), 16) * 7
                 + parseInt(pk.substring(Math.floor(pk.length / 2),
                                         Math.floor(pk.length / 2) + 2), 16) * 3
                 + parseInt(pk.substring(pk.length - 2), 16) * 11) % 360
        return "hsl(" + h + ", 55%, 42%)"
    }

    function avatarInitial(name) {
        if(name && name.length > 0)
            return name.substring(0, 1).toUpperCase()
        return "T"
    }

    // monogram letter for a shared row. without a dwyco name there is
    // nothing personable to use, so fall back to the tox "T".
    function rowInitial(row) {
        return avatarInitial(nameOf(row))
    }

    // what picking this row will cost you. selecting anything other than the
    // local save overwrites the identity currently on disk, which used to be
    // completely invisible. with nothing on this device there is nothing to
    // replace, and the first list row is a shared save, not the local one --
    // so find the local row rather than assuming position.
    function replaceWarning(row) {
        if(!row || row.local)
            return ""
        var cur = null
        for(var i = 0; i < allSavesList.length; ++i) {
            if(allSavesList[i].local === true) {
                cur = allSavesList[i]
                break
            }
        }
        if(!cur)
            return ""
        var who = nameOf(cur)
        if(who.length === 0 && cur.pubkey && cur.pubkey.length > 0)
            who = shortPub(cur.pubkey)
        else if(who.length === 0)
            // no name and no pubkey for the local save (a freshly created
            // identity has neither yet): the phrase is complete on its own,
            // so it must not get the "on this device" suffix again.
            return "Replaces the save on this device"
        return "Replaces " + who + " on this device"
    }

    // ---- transient result banner ----

    property string bannerText: ""
    property bool bannerError: false

    Timer {
        id: bannerTimer
        interval: 4000
        onTriggered: tox_acct.bannerText = ""
    }

    function showBanner(text, isError) {
        bannerText = text
        bannerError = isError === true
        bannerTimer.restart()
    }

    // ---- sign in / out ----

    // signing in to the local save: the mid is empty, same as the local row
    // used to be. kept separate from the shared-row flow so the hero's button
    // does not have to touch the shared list's selection.
    function signInLocal() {
        var err = core.tox_sign_in("", heroPwInput.text)
        if(err.length > 0) {
            showBanner(err, true)
            heroPwInput.text = ""
            heroPwInput.forceActiveFocus()
            doRefresh()
            return
        }
        heroPwInput.text = ""
        showBanner("Signed in.")
        doRefresh()
    }

    // sign in to a shared tox-id. the row is passed in because the list
    // refreshes out from under any live index while dialogs are open.
    function doSignIn(row) {
        if(!row) {
            showBanner("Pick a Tox save first.", true)
            return
        }
        // picking a shared identity overwrites the save on disk, so the
        // remembered name/address describe the identity being replaced. drop
        // them up front; if the sign in then fails the next refresh re-caches
        // nothing and the boxes correctly show blank.
        tox_state.forgetCachedIdentity()
        var err = core.tox_sign_in(row.mid, pwInput.text)
        if(err.length > 0) {
            showBanner(err, true)
            pwInput.text = ""
            pwInput.forceActiveFocus()
            doRefresh()
            return
        }
        pwInput.text = ""
        showBanner("Signed in.")
        doRefresh()
    }

    function signOut() {
        if (core.tox_enabled)
            core.disable_tox()
        core.set_local_setting("tox_enabled", "0")
        doRefresh()
        showBanner("Signed out.")
    }

    // ---- file-level password on the local save ----
    //
    // the same core call covers all three cases: an empty old password with a
    // new one adds encryption, the current password with an empty new one
    // removes it. tox has to be stopped for it to touch the file on disk, and
    // nothing is emitted when it succeeds, so refresh by hand.

    property string savePwMode: "add"

    function openAddSavePassword() {
        savePwMode = "add"
        savePwError.text = ""
        savePwOld.text = ""
        savePwNew.text = ""
        savePwConfirm.text = ""
        savePwDlg.open()
        savePwNew.forceActiveFocus()
    }

    function openRemoveSavePassword() {
        savePwMode = "remove"
        savePwError.text = ""
        savePwOld.text = ""
        savePwNew.text = ""
        savePwConfirm.text = ""
        savePwDlg.open()
        savePwOld.forceActiveFocus()
    }

    function applySavePassword() {
        if(savePwMode === "add") {
            if(savePwNew.text.length === 0) {
                savePwError.text = "Enter a password."
                return
            }
            if(savePwNew.text !== savePwConfirm.text) {
                savePwError.text = "The passwords do not match."
                return
            }
        }
        var err = core.tox_set_save_password(savePwOld.text, savePwNew.text)
        if(err.length > 0) {
            savePwError.text = err
            return
        }
        savePwDlg.close()
        if(savePwMode === "add")
            showBanner("Password added. This save now needs it to sign in.")
        else
            showBanner("Password removed.")
        doRefresh()
    }

    // ---- local save file operations ----

    function doImport(path, pw) {
        var err = core.tox_import_profile(path, pw, !importNoBackup.checked)
        if (err.length > 0) {
            importError.text = "Import failed: " + err
            return
        }
        importConfirmDlg.close()
        // no auto sign in. the save is on disk but tox is stopped, so the
        // row shows up in the list and the user signs in explicitly -- same
        // as every other save-manipulating action.
        // the cached name/address described the identity we just replaced, so
        // drop them; otherwise the tox page would attribute them to the
        // imported save.
        tox_state.forgetCachedIdentity()
        showBanner("Imported. Sign in to it above to use it.")
        doRefresh()
    }

    function doCreateNew() {
        createNewConfirmDlg.close()
        var ok = core.tox_reset_identity()
        if (ok) {
            // same as above: the remembered name/address belong to the old
            // identity, which reset backed up and replaced.
            tox_state.forgetCachedIdentity()
            showBanner("Created a new tox-id. Sign in to it above to use it.")
        } else {
            showBanner("Could not create a new Tox save.", true)
        }
        doRefresh()
    }

    // drop a shared identity from every group member's list of shared saves.
// this does NOT change any client's tox state: nobody gets signed out, and a
// device still signed in with that save can publish it again, which puts it
// back in the list.
function doUnshare(row) {
        unshareConfirmDlg.close()
        pendingRow = null
        if(!row)
            return
        var removed = core.tox_depublish_save(row.mid)
        if(removed) {
            // the row we are about to remove may be the selected one, so let
            // the refresh fall back to the local save rather than leaving the
            // selection pointing at something that no longer exists.
            if(selectedKey === row.key)
                selectedKey = ""
            showBanner(rowTitle(row) + " is no longer shared with your devices.")
        } else {
            showBanner("That save was already not shared.")
        }
        doRefresh()
    }

    function openUnshareConfirm(row) {
        pendingRow = row
        unshareConfirmDlg.open()
    }

    function doDeleteSave() {
        deleteConfirmDlg.close()
        if (core.tox_enabled)
            core.disable_tox()
        var ok = core.tox_factory_reset()
        core.set_local_setting("tox_enabled", "0")
        if (ok) {
            tox_state.forgetCachedIdentity()
            showBanner("Tox save deleted. A copy was saved to disk first.")
        } else {
            showBanner("Delete failed. The Tox save was not removed.", true)
        }
        doRefresh()
    }

    // ---- filename helpers ----

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
        if (id.length === 0)
            id = core.tox_get_self_public_key()
        if (name8.length === 0)
            return "tox-" + (id.length > 8 ? id.substring(0, 8) : id) + ".tox"
        return name8 + (id.length > 4 ? id.substring(0, 4) : id) + ".tox"
    }

    Connections {
        target: core
        function onTox_saves_changed() { refreshAllSaves() }
        function onTox_disabled_by_remote(holder) { doRefresh() }
    }

    onVisibleChanged: {
        if (visible) {
            doRefresh()
        }
    }

    Component.onCompleted: {
        doRefresh()
    }

    ScrollView {
        id: acct_scroll
        anchors.fill: parent
        anchors.margins: mm(2)
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: acct_scroll.availableWidth
            spacing: mm(1)

            // ---- transient result / error banner ----

            Pane {
                id: banner
                visible: tox_acct.bannerText.length > 0
                Layout.fillWidth: true
                padding: mm(1)
                background: Rectangle {
                    color: tox_acct.bannerError ? "#fdecec" : "#eaf6ec"
                    radius: mm(1)
                    border.color: tox_acct.bannerError ? "#e3b0b0" : "#b3d6b8"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 0

                    Label {
                        text: tox_acct.bannerText
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: dp(12)
                        color: tox_acct.bannerError ? "#a00" : "#060"
                    }
                }
            }

            // ---- 1. the tox save being used right now ----

            Label {
                text: "THE TOX SAVE BEING USED NOW"
                font.pixelSize: dp(10)
                font.bold: true
                font.letterSpacing: dp(1)
                color: amber_dark
                Layout.topMargin: mm(1)
            }

            Pane {
                Layout.fillWidth: true
                padding: mm(2)
                background: Rectangle {
                    color: "white"
                    radius: mm(1.5)
                    border.color: "#ecd9a0"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: mm(1)

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: mm(1.5)

                        Rectangle {
                            Layout.preferredWidth: mm(12)
                            Layout.preferredHeight: mm(12)
                            radius: width / 2
                            color: tox_state.state === "empty"
                                  ? "#d5cdb8"
                                  : tox_acct.avatarTint(tox_acct.heroPubkey)

                            Text {
                                anchors.centerIn: parent
                                text: tox_state.state === "empty"
                                      ? "T"
                                      : tox_acct.avatarInitial(tox_acct.localSafeName())
                                color: "white"
                                font.bold: true
                                font.pixelSize: dp(24)
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0

                            Label {
                                Layout.fillWidth: true
                                text: tox_state.state === "empty"
                                      ? "No tox-id yet"
                                      : (tox_acct.localSafeName().length > 0
                                         ? tox_acct.localSafeName()
                                         : "Tox save on this device")
                                font.bold: true
                                font.pixelSize: dp(17)
                                color: primary_text
                                elide: Text.ElideRight
                            }

                            Label {
                                Layout.fillWidth: true
                                visible: tox_state.state === "empty"
                                text: "This device has no Tox save yet."
                                font.pixelSize: dp(11)
                                color: secondary_text
                                elide: Text.ElideRight
                            }

                            Label {
                                Layout.fillWidth: true
                                visible: tox_state.state !== "empty"
                                text: tox_state.displayIdCached
                                      ? tox_state.displayId + " (last used here)"
                                      : tox_state.displayId
                                font.family: "monospace"
                                font.pixelSize: dp(11)
                                color: secondary_text
                                elide: Text.ElideRight
                            }
                        }

                        Button {
                            text: "Copy"
                            flat: true
                            visible: tox_state.state !== "empty"
                            onClicked: core.copy_to_clipboard(
                                          core.tox_self_address.length > 0
                                          ? core.tox_self_address
                                          : tox_acct.heroPubkey)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: mm(1)

                        Pane {
                            visible: tox_state.state !== "empty"
                            padding: mm(0.75)
                            background: Rectangle {
                                color: tox_state.dotColor
                                radius: mm(3.5)
                            }
                            RowLayout {
                                spacing: mm(1)
                                Rectangle {
                                    Layout.preferredWidth: mm(1.75)
                                    Layout.preferredHeight: mm(1.75)
                                    radius: width / 2
                                    color: "white"
                                }
                                Label {
                                    text: tox_state.statusText
                                    color: "white"
                                    font.bold: true
                                    font.pixelSize: dp(11)
                                }
                            }
                        }

                        RowLayout {
                            visible: tox_state.encrypted
                            spacing: mm(0.5)
                            Image {
                                source: mi("ic_lock_black_24dp.png")
                                sourceSize.width: mm(3.5)
                                sourceSize.height: mm(3.5)
                            }
                            Label {
                                text: "Password protected"
                                font.pixelSize: dp(10)
                                color: secondary_text
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Button {
                            text: "Sign out"
                            visible: tox_state.state === "signedin"
                            onClicked: signOut()
                        }

                        Button {
                            text: "Sign in"
                            visible: tox_state.state === "signedout"
                            onClicked: signInLocal()
                        }
                    }

                    // how this device's save actually differs from the copy on the
                    // other devices. this lists the real differences rather
                    // than inferring which copy is newer, so the user can see
                    // what they'd be giving up either way before picking one
                    // of the two copy buttons.
                    ColumnLayout {
                        visible: tox_state.state !== "empty"
                              && tox_state.diffHeadline(tox_state.diffState) !== ""
                        Layout.fillWidth: true
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
                            }
                        }

                        // the differences, one per line. only shown when
                        // there are any, since the headline already says
                        // "same" when there aren't.
                        Repeater {
                            model: tox_state.differences
                            delegate: Label {
                                required property var modelData
                                Layout.fillWidth: true
                                Layout.leftMargin: mm(2.5)
                                text: "• " + tox_state.diffLine(modelData)
                                font.pixelSize: dp(10)
                                color: secondary_text
                                wrapMode: Text.WordWrap
                            }
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

                    // password entry, only when a save is loaded but locked
                    RowLayout {
                        visible: tox_state.state === "locked"
                        Layout.fillWidth: true
                        spacing: mm(1)

                        Label {
                            text: "Enter the password to sign in to this save."
                            font.pixelSize: dp(11)
                            color: secondary_text
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: heroPwInput
                            echoMode: TextInput.Password
                            placeholderText: "Password"
                            Layout.fillWidth: true
                            onAccepted: signInLocal()
                        }

                        Button {
                            text: "Unlock"
                            onClicked: signInLocal()
                        }
                    }
                }
            }

            // ---- 2. the ways to switch ----

            Label {
                text: "To use another tox-id, you can…"
                font.bold: true
                font.pixelSize: dp(13)
                color: primary_text
                Layout.topMargin: mm(1)
            }

            // import from a file
            Pane {
                Layout.fillWidth: true
                padding: mm(2)
                background: Rectangle {
                    color: "white"
                    radius: mm(1)
                    border.color: "#e6ddc6"
                }

                RowLayout {
                    width: parent.width
                    spacing: mm(1.5)

                    Rectangle {
                        Layout.preferredWidth: mm(9)
                        Layout.preferredHeight: mm(9)
                        radius: width / 2
                        color: amber_light

                        Image {
                            anchors.centerIn: parent
                            source: mi("ic_cloud_download_black_24dp.png")
                            sourceSize.width: mm(5)
                            sourceSize.height: mm(5)
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Label {
                            text: "Import from a file…"
                            font.bold: true
                            font.pixelSize: dp(13)
                        }

                        Label {
                            Layout.fillWidth: true
                            text: "Pick a .tox file saved on this device. Your current "
                                  + "save is backed up first."
                            font.pixelSize: dp(10)
                            color: secondary_text
                            wrapMode: Text.WordWrap
                        }
                    }

                    Button {
                        text: "Choose…"
                        enabled: tox_state.state !== "signedin"
                        onClicked: {
                            importPath = ""
                            importPw = ""
                            importFileDialog.open()
                        }
                    }
                }
            }

            // brand new identity
            Pane {
                Layout.fillWidth: true
                padding: mm(2)
                background: Rectangle {
                    color: "white"
                    radius: mm(1)
                    border.color: "#e6ddc6"
                }

                RowLayout {
                    width: parent.width
                    spacing: mm(1.5)

                    Rectangle {
                        Layout.preferredWidth: mm(9)
                        Layout.preferredHeight: mm(9)
                        radius: width / 2
                        color: amber_light

                        Image {
                            anchors.centerIn: parent
                            source: mi("ic_create_black_24dp.png")
                            sourceSize.width: mm(5)
                            sourceSize.height: mm(5)
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Label {
                            text: "Create a brand new tox-id…"
                            font.bold: true
                            font.pixelSize: dp(13)
                        }

                        Label {
                            Layout.fillWidth: true
                            text: "Start over with a fresh identity and friend list."
                            font.pixelSize: dp(10)
                            color: secondary_text
                            wrapMode: Text.WordWrap
                        }
                    }

                    Button {
                        text: "Create…"
                        enabled: tox_state.state !== "signedin"
                        onClicked: createNewConfirmDlg.open()
                    }
                }
            }

            // ---- 3. shared tox-ids (from the group tag payload) ----

            Label {
                text: "…or… import one of your shared tox-id"
                font.bold: true
                font.pixelSize: dp(13)
                color: primary_text
                Layout.topMargin: mm(1)
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

                    ListView {
                        id: sharedSavesListView
                        model: tox_acct.sharedSaves
                        visible: sharedSavesListView.count > 0
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(sharedSavesListView.contentHeight, mm(72))
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        spacing: mm(1)
                        ScrollBar.vertical: ScrollBar { }

                        delegate: Item {
                            id: sharedRow
                            width: ListView.view.width
                            height: sharedRowCol.implicitHeight + mm(2)

                            Rectangle {
                                anchors.fill: parent
                                radius: mm(1)
                                color: modelData.key === selectedKey ? "#fdf3d8" : "#faf8f1"
                                border.color: modelData.key === selectedKey ? amber_dark : "#e9e3d1"
                                border.width: 1
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if(selectedKey !== modelData.key) {
                                        selectedKey = modelData.key
                                        syncSelected()
                                        pwInput.text = ""
                                    }
                                }
                            }

                            ColumnLayout {
                                id: sharedRowCol
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.margins: mm(1)
                                spacing: mm(0.5)

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: mm(1)

                                    Rectangle {
                                        Layout.preferredWidth: mm(8)
                                        Layout.preferredHeight: mm(8)
                                        radius: width / 2
                                        color: tox_acct.avatarTint(modelData.pubkey)

                                        Text {
                                            anchors.centerIn: parent
                                            text: tox_acct.rowInitial(modelData)
                                            color: "white"
                                            font.bold: true
                                            font.pixelSize: dp(14)
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 0

                                        Text {
                                            Layout.fillWidth: true
                                            text: tox_acct.rowTitle(modelData)
                                            font.bold: true
                                            font.pixelSize: dp(13)
                                            font.family: (!tox_acct.nameOf(modelData).length
                                                          && modelData.pubkey
                                                          && modelData.pubkey.length > 0)
                                                         ? "monospace" : "sans-serif"
                                            elide: Text.ElideRight
                                        }

                                        Text {
                                            Layout.fillWidth: true
                                            text: tox_acct.rowSubLabel(modelData)
                                            font.pixelSize: dp(10)
                                            color: secondary_text
                                            elide: Text.ElideRight
                                        }
                                    }

                                    Image {
                                        visible: modelData.encrypted === true
                                        source: mi("ic_lock_outline_black_24dp.png")
                                        sourceSize.width: mm(4)
                                        sourceSize.height: mm(4)
                                    }

                                    // the identity on this device shows up in
                                    // this list too (once it's been shared), so
                                    // it needs saying: without this the same
                                    // tox-id appears twice with nothing to
                                    // connect the two.
                                    Rectangle {
                                        visible: modelData.is_current === true
                                        Layout.preferredWidth: badgeRow.implicitWidth + mm(1.5)
                                        Layout.preferredHeight: mm(4.5)
                                        radius: mm(2.25)
                                        color: "#e6f0e6"

                                        RowLayout {
                                            id: badgeRow
                                            anchors.centerIn: parent
                                            spacing: mm(0.5)

                                            Rectangle {
                                                Layout.preferredWidth: mm(1.5)
                                                Layout.preferredHeight: mm(1.5)
                                                radius: width / 2
                                                color: "#3a8"
                                            }

                                            Text {
                                                text: "this device"
                                                color: "#2a6"
                                                font.pixelSize: dp(10)
                                            }
                                        }

                                        ToolTip.visible: badgeHover.hovered
                                        ToolTip.text: "This is the tox-id on this device"
                                        HoverHandler { id: badgeHover }
                                    }

                                    // this row is the published copy of the identity that
                                    // is on this device, and the two genuinely
                                    // differ. flag it here as well as in the
                                    // hero card, or the row looks like just
                                    // another shared identity.
                                    Rectangle {
                                        visible: modelData.is_current === true
                                                 && tox_state.diffState === 1
                                        Layout.preferredWidth: newerRow.implicitWidth + mm(1.5)
                                        Layout.preferredHeight: mm(4.5)
                                        radius: mm(2.25)
                                        color: "#fdf0dc"

                                        RowLayout {
                                            id: newerRow
                                            anchors.centerIn: parent
                                            spacing: mm(0.5)

                                            Rectangle {
                                                Layout.preferredWidth: mm(1.5)
                                                Layout.preferredHeight: mm(1.5)
                                                radius: width / 2
                                                color: "#c80"
                                            }

                                            Text {
                                                text: "differs"
                                                color: "#a60"
                                                font.pixelSize: dp(10)
                                            }
                                        }

                                        ToolTip.visible: newerHover.hovered
                                        ToolTip.text: "This shared copy differs from "
                                                      + "the save on this device"
                                        HoverHandler { id: newerHover }
                                    }

                                    // unpublish. drop this tox-id from every
                                    // device's shared list; nobody is signed
                                    // out and no settings change.
                                    ToolButton {
                                        Layout.preferredWidth: mm(6)
                                        Layout.preferredHeight: mm(6)
                                        padding: 0
                                        contentItem: Image {
                                            anchors.centerIn: parent
                                            source: mi("ic_delete_black_24dp.png")
                                            sourceSize.width: mm(4)
                                            sourceSize.height: mm(4)
                                        }
                                        ToolTip.visible: hovered
                                        ToolTip.text: "Stop sharing this save"
                                        onClicked: tox_acct.openUnshareConfirm(modelData)
                                    }
                                }

                                // selected: say what it costs, right on the
                                // row, before the user commits to it.
                                Label {
                                    Layout.fillWidth: true
                                    Layout.leftMargin: mm(9)
                                    text: tox_acct.replaceWarning(modelData)
                                    font.pixelSize: dp(10)
                                    font.italic: true
                                    color: amber_dark
                                    wrapMode: Text.WordWrap
                                    visible: modelData.key === selectedKey
                                            && text.length > 0
                                }
                            }
                        }
                    }

                    Label {
                        visible: sharedSavesListView.count === 0
                        text: "Nothing here yet. Share a tox-id with your devices and "
                              + "it shows up here on this device."
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        color: secondary_text
                        font.pixelSize: dp(11)
                    }

                    // the selected row's action strip. picking a shared tox-id
                    // replaces the save on this device, so the confirm lives
                    // here, next to the warning above the row, instead of in a
                    // modal.
                    RowLayout {
                        visible: sharedSavesListView.count > 0
                        Layout.fillWidth: true
                        spacing: mm(1)

                        Label {
                            Layout.fillWidth: true
                            text: tox_acct.selectedSharedRow()
                                  ? "Use " + tox_acct.rowTitle(tox_acct.selectedSharedRow()) + "?"
                                  : "Pick one above to use it."
                            font.pixelSize: dp(10)
                            color: secondary_text
                            wrapMode: Text.WordWrap
                        }

                        TextField {
                            id: pwInput
                            visible: tox_acct.selectedSharedRow() !== null
                                     && tox_acct.selectedEncrypted
                            echoMode: TextInput.Password
                            placeholderText: "Password"
                            Layout.preferredWidth: mm(28)
                            onAccepted: tox_acct.doSignIn(tox_acct.selectedSharedRow())
                        }

                        Button {
                            text: "Use this tox-id"
                            enabled: tox_acct.selectedSharedRow() !== null
                            ToolTip.visible: hovered
                            ToolTip.text: "Copies this shared save over the save "
                                          + "on this device"
                            onClicked: tox_acct.doSignIn(tox_acct.selectedSharedRow())
                        }
                    }

                    // spell out the direction of both copy buttons. they write
                    // the same file in opposite directions, and the labels
                    // alone don't make that obvious.
                    Label {
                        visible: sharedSavesListView.count > 0
                        Layout.fillWidth: true
                        Layout.topMargin: mm(0.5)
                        text: tox_state.copyDirectionNote()
                        wrapMode: Text.WordWrap
                        font.pixelSize: dp(10)
                        color: secondary_text
                    }
                }
            }

            // ---- 4. quiet manage footer ----

            Rectangle {
                Layout.fillWidth: true
                Layout.topMargin: mm(1)
                implicitHeight: 1
                color: "#ddd3b2"
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: mm(1)

                Button {
                    text: "Export to file…"
                    flat: true
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
                    text: "Share with my devices"
                    flat: true
                    enabled: tox_state.state === "signedin"
                    ToolTip.visible: hovered
                    ToolTip.text: "Copies this device's Tox save (tox_save.tox) "
                                  + "out to your other devices"
                    onClicked: {
                        if (core.tox_publish_save()) {
                            showBanner("Shared with your other devices.")
                            // publishing is what changes the sync state, so the
                            // indicator has to be re-read or it still reads
                            // "this device is newer" right after it stopped
                            // being true.
                            doRefresh()
                        }
                        else
                            showBanner("Could not share. Sign in to your Tox save first.", true)
                    }
                }

                // add or remove the file-level password on the local save.
                // the bridge refuses while tox is running, so this is only
                // offered when signed out, like the other file operations.
                Button {
                    text: tox_state.encrypted ? "Remove password…" : "Add a password…"
                    flat: true
                    enabled: tox_state.state !== "signedin"
                              && tox_state.state !== "empty"
                    ToolTip.visible: hovered
                    ToolTip.text: tox_state.state === "signedin"
                                  ? "Sign out first to change the save's password"
                                  : (tox_state.encrypted
                                     ? "Take the password off this device's tox save"
                                     : "Password protect this device's tox save")
                    onClicked: tox_state.encrypted
                              ? openRemoveSavePassword()
                              : openAddSavePassword()
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Delete this tox-id"
                    flat: true
                    enabled: tox_state.state !== "signedin"
                    opacity: tox_state.state === "signedin" ? 0.4 : 1
                    contentItem: Label {
                        text: "Delete this tox-id"
                        color: "#c00"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    onClicked: deleteConfirmDlg.open()
                }
            }

            Label {
                text: "Export writes a .tox file you can import on another device. "
                      + "Sharing copies this device's tox_save.tox out to the "
                      + "other devices in your group; using a shared tox-id copies "
                      + "one back in over tox_save.tox here. Neither happens on its "
                      + "own, so the save on this device is always the one that runs. "
                      + "Password protecting the save also protects every .tox "
                      + "file you export from it. Deleting removes it from this device "
                      + "only — your messages, contacts and Dwyco account are not "
                      + "affected, and a copy is saved to disk first."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: secondary_text
                font.pixelSize: dp(10)
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }

    // ---- import / export file pickers ----

    property string importPath: ""
    property string importPw: ""

    FileDialog {
        id: importFileDialog
        title: "Choose a Tox save (.tox) to import"
        nameFilters: ["Tox saves (*.tox)", "All files (*)"]
        onRejected: {
            importPath = ""
            importPw = ""
        }
        onAccepted: {
            var p = core.url_to_filename(selectedFile)
            if (p === "") {
                showBanner("Could not use that file.", true)
                return
            }
            importPath = p
            if (core.tox_file_is_encrypted(p)) {
                openPwEntry("import",
                            "This save is password protected. Enter its "
                            + "password to import it.")
            } else {
                importPw = ""
                openImportConfirm()
            }
        }
    }

    FileDialog {
        id: exportFileDialog
        title: "Export Tox save"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Tox saves (*.tox)"]
        onAccepted: {
            var p = core.url_to_filename(selectedFile)
            if (p === "") {
                showBanner("Could not use that location.", true)
                return
            }
            if (p.toLowerCase().lastIndexOf(".tox") !== p.length - 4)
                p += ".tox"
            var err = core.tox_export_profile(p)
            if (err.length > 0)
                showBanner("Export failed: " + err, true)
            else
                showBanner("Exported to " + p + ".")
        }
    }

    function openImportConfirm() {
        importError.text = ""
        importNoBackup.checked = false
        importConfirmDlg.open()
    }

    // ---- password entry for import ----

    property string pwEntryPurpose: "import"
    property string pwEntryIntro: ""

    function openPwEntry(purpose, intro) {
        pwEntryPurpose = purpose
        pwEntryIntro = intro
        pwEntryInput.text = ""
        pwEntryError.text = ""
        pwEntryDialog.open()
        pwEntryInput.forceActiveFocus()
    }

    // ---- stop sharing confirmation ----

Dialog {
        id: unshareConfirmDlg
        title: "Stop sharing?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: pendingRow
                      ? "Remove " + rowTitle(pendingRow) + " from your devices' "
                        + "shared list?"
                      : ""
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "This removes it for every device in your group. Nobody is "
                      + "signed out and no one's settings change."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                visible: pendingRow && pendingRow.key === selectedKey
                text: "This is the save you have selected. Removing it does not "
                      + "change the save on this device."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#a00"
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: {
                        pendingRow = null
                        unshareConfirmDlg.close()
                    }
                }

                Button {
                    text: "Stop sharing"
                    onClicked: doUnshare(pendingRow)
                }
            }
        }
    }

    Dialog {
        id: pwEntryDialog
        title: "Password-protected Tox save"
        modal: true
        closePolicy: Dialog.NoAutoClose
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: pwEntryIntro
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            TextField {
                id: pwEntryInput
                echoMode: TextInput.Password
                placeholderText: "Password"
                Layout.fillWidth: true
                onAccepted: pwEntryOkButton.clicked()
            }

            Label {
                id: pwEntryError
                text: ""
                color: "#a00"
                visible: text.length > 0
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: pwEntryDialog.close()
                }

                Button {
                    id: pwEntryOkButton
                    text: "Next"
                    enabled: pwEntryInput.text.length > 0
                    onClicked: {
                        pwEntryError.text = ""
                        pwEntryDialog.close()
                        importPw = pwEntryInput.text
                        openImportConfirm()
                    }
                }
            }
        }
    }

    // ---- add / remove the save's password ----

    Dialog {
        id: savePwDlg
        title: tox_acct.savePwMode === "add"
               ? "Add a password" : "Remove the password"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                visible: tox_acct.savePwMode === "add"
                text: "Password protect the Tox save on this device. You will "
                      + "need the password every time this device signs in, "
                      + "and any .tox file exported from it will be protected "
                      + "too."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                visible: tox_acct.savePwMode === "remove"
                text: "Take the password off the Tox save on this device. It "
                      + "will no longer ask for one to sign in here. Enter the "
                      + "current password to confirm."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            // current password, only when taking one off
            TextField {
                id: savePwOld
                visible: tox_acct.savePwMode === "remove"
                echoMode: TextInput.Password
                placeholderText: "Current password"
                Layout.fillWidth: true
                onAccepted: applySavePassword()
            }

            ColumnLayout {
                visible: tox_acct.savePwMode === "add"
                Layout.fillWidth: true
                spacing: mm(1)

                TextField {
                    id: savePwNew
                    echoMode: TextInput.Password
                    placeholderText: "New password"
                    Layout.fillWidth: true
                }

                TextField {
                    id: savePwConfirm
                    echoMode: TextInput.Password
                    placeholderText: "New password again"
                    Layout.fillWidth: true
                    onAccepted: applySavePassword()
                }
            }

            Label {
                id: savePwError
                text: ""
                color: "#a00"
                visible: text.length > 0
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: savePwDlg.close()
                }

                Button {
                    text: tox_acct.savePwMode === "add" ? "Add password"
                                                       : "Remove password"
                    onClicked: applySavePassword()
                }
            }
        }
    }

    // ---- import confirmation ----

    Dialog {
        id: importConfirmDlg
        title: "Import Tox save"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "Importing this save replaces the Tox save on this device, "
                      + "along with its friend list. Message history is not stored "
                      + "in a Tox save and will not be imported."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "You will be signed out. The imported tox-id becomes the "
                      + "save on this device, and you sign in to it when you "
                      + "are ready."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            CheckBox {
                id: importNoBackup
                text: "Don't save a backup of the current save"
                checked: false
            }

            Label {
                id: importError
                text: ""
                color: "#a00"
                visible: text.length > 0
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: {
                        importPath = ""
                        importPw = ""
                        importConfirmDlg.close()
                    }
                }

                Button {
                    text: "Import"
                    onClicked: doImport(importPath, importPw)
                }
            }
        }
    }

    // ---- create new save confirmation ----

    Dialog {
        id: createNewConfirmDlg
        title: "Create new Tox save"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "This replaces the Tox save on this device with a brand new "
                      + "identity. The old save (including your Tox ID and friend "
                      + "list) is backed up to a file named replaced_tox_save.tox."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "You will be signed out. The new tox-id becomes the save "
                      + "on this device, and you sign in to it when you are ready."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "This cannot be undone."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                font.bold: true
                color: "#a00"
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: createNewConfirmDlg.close()
                }

                Button {
                    text: "Create new tox-id"
                    onClicked: doCreateNew()
                }
            }
        }
    }

    // ---- delete save confirmation ----

    Dialog {
        id: deleteConfirmDlg
        title: "Delete this tox-id"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "This deletes the Tox save on this device. Your Tox ID and "
                      + "friend list are gone. A copy of the current save is saved "
                      + "to disk first."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "Your messages, contacts and Dwyco account are not affected."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "This cannot be undone."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                font.bold: true
                color: "#a00"
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: deleteConfirmDlg.close()
                }

                Button {
                    text: "Delete this tox-id"
                    background: Rectangle {
                        color: "#c00"
                        radius: mm(1)
                    }
                    contentItem: Label {
                        text: "Delete this tox-id"
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: doDeleteSave()
                }
            }
        }
    }
}
