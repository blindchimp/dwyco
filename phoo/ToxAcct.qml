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

    // one layout that adapts to the width it is given, rather than a separate
    // phone layout. a narrow desktop window takes the same path as a phone.
    readonly property bool compact: width < 600

    // while a modal dialog is up, the page underneath must not react to a
    // drag that starts on the dialog (android otherwise still flicks it).
    readonly property bool dialog_open:
        unshareConfirmDlg.visible
        || pwEntryDialog.visible
        || savePwDlg.visible
        || importConfirmDlg.visible
        || createNewConfirmDlg.visible
        || deleteConfirmDlg.visible
        || useSaveConfirmDlg.visible

    // ---- state ----

    property var allSavesList: []
    // just the group-shared rows (the _tox_mid tag payloads). the local save
    // is not in here: it has its own hero card at the top of the page.
    property var sharedSaves: []
    // the row a confirmation dialog is about. the list can refresh while a
    // dialog is open, so the dialog must not read a live list index.
    property var pendingRow: null
    property var pendingUseRow: null

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

    // the comparison itself (state code, wording, per-difference sentences, the
    // plain-language summary, the bullet cap) lives on tox_state in main.qml,
    // because the tox page shows it too and the two must not disagree. read
    // tox_state.diffState / diffHeadline() / diffSummary() / diffLine() /
    // diffOverflow() there.

    function refreshAllSaves() {
        allSavesList = core.tox_list_all_saves()
        sharedSaves = allSavesList.filter(function(r) { return r.local !== true })
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
    // does not have to touch the shared list.
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

    // using a shared tox-id. the row is passed in because the list refreshes
    // out from under any live index while dialogs are open. the password comes
    // from the confirm dialog instead of an inline field, so the row and its
    // controls fit a phone.
    function openUseSave(row) {
        if(!row) {
            showBanner("Pick a Tox save first.", true)
            return
        }
        pendingUseRow = row
        usePwInput.text = ""
        usePwError.text = ""
        useSaveConfirmDlg.open()
        if(row.encrypted)
            usePwInput.forceActiveFocus()
    }

    function confirmUseSave() {
        if(!pendingUseRow)
            return
        usePwError.text = ""
        // picking a shared identity overwrites the save on disk, so the
        // remembered name/address describe the identity being replaced. drop
        // them up front; if the sign in then fails the next refresh re-caches
        // nothing and the boxes correctly show blank.
        tox_state.forgetCachedIdentity()
        var err = core.tox_sign_in(pendingUseRow.mid, usePwInput.text)
        if(err.length > 0) {
            usePwError.text = err
            usePwInput.text = ""
            usePwInput.forceActiveFocus()
            doRefresh()
            return
        }
        useSaveConfirmDlg.close()
        pendingUseRow = null
        usePwInput.text = ""
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
        anchors.margins: tox_acct.compact ? mm(1.5) : mm(2)
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        // ScrollView's contentItem is a Flickable, but typed as an Item here,
        // so freeze it with a Binding rather than a direct property write.
        Binding {
            target: acct_scroll.contentItem
            property: "interactive"
            value: !tox_acct.dialog_open
        }
        // android otherwise lets the whole page be dragged past its edges
        // (and sideways) and then snaps it back.
        Binding {
            target: acct_scroll.contentItem
            property: "boundsBehavior"
            value: Flickable.StopAtBounds
        }
        Binding {
            target: acct_scroll.contentItem
            property: "flickableDirection"
            value: Flickable.VerticalFlick
        }

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

                Label {
                    text: tox_acct.bannerText
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    font.pixelSize: dp(12)
                    color: tox_acct.bannerError ? "#a00" : "#060"
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
                                wrapMode: Text.WordWrap
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
                                wrapMode: Text.WrapAnywhere
                            }
                        }
                    }

                    ResponsiveRow {
                        Rectangle {
                            visible: tox_state.state !== "empty"
                            width: statusPill.implicitWidth + mm(2)
                            height: mm(5)
                            radius: height / 2
                            color: tox_state.dotColor

                            RowLayout {
                                id: statusPill
                                anchors.centerIn: parent
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

                        Button {
                            text: "Copy"
                            flat: true
                            visible: tox_state.state !== "empty"
                            onClicked: core.copy_to_clipboard(
                                          core.tox_self_address.length > 0
                                          ? core.tox_self_address
                                          : tox_acct.heroPubkey)
                        }

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

                        // the plain-language summary of the difference, so the
                        // user can tell at a glance whether this is 1 contact
                        // or 40 without counting the bullets under it.
                        Label {
                            visible: text !== ""
                            Layout.fillWidth: true
                            Layout.leftMargin: mm(2.5)
                            text: tox_state.diffSummary()
                            font.pixelSize: dp(10)
                            color: secondary_text
                            wrapMode: Text.WordWrap
                        }

                        // the differences, one per line. capped per-delegate rather
                        // than by slicing the model -- see diffBulletCap on
                        // tox_state. invisible rows cost a ColumnLayout no
                        // space, so the tail drops out cleanly.
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

                    // password entry, only when a save is loaded but locked
                    ColumnLayout {
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

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: mm(1)

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
            }

            // ---- 2. the ways to switch ----

            Label {
                text: "To use another tox-id, you can…"
                font.bold: true
                font.pixelSize: dp(13)
                color: primary_text
                Layout.topMargin: mm(1)
            }

            // one compact list of actions instead of a stack of wide cards.
            // each one opens the existing confirmation or file picker, so the
            // page stays short on a phone and still reads like settings.
            Pane {
                Layout.fillWidth: true
                padding: mm(1)
                background: Rectangle {
                    color: "white"
                    radius: mm(1)
                    border.color: "#e6ddc6"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 0

                    ItemDelegate {
                        Layout.fillWidth: true
                        text: "Import from a .tox file…"
                        enabled: tox_state.state !== "signedin"
                        onClicked: {
                            importPath = ""
                            importPw = ""
                            importFileDialog.open()
                        }
                    }

                    ItemDelegate {
                        Layout.fillWidth: true
                        text: "Create a new tox-id…"
                        enabled: tox_state.state !== "signedin"
                        onClicked: createNewConfirmDlg.open()
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 1
                        color: "#eee6d2"
                    }

                    ItemDelegate {
                        Layout.fillWidth: true
                        text: tox_state.state === "signedin"
                              ? "Share with my devices"
                              : "Share with my devices (sign in first)"
                        enabled: tox_state.state === "signedin"
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

                    ItemDelegate {
                        Layout.fillWidth: true
                        text: "Export to file…"
                        onClicked: {
                            var locs = StandardPaths.standardLocations(StandardPaths.DocumentsLocation)
                            if (locs.length > 0) {
                                exportFileDialog.currentFolder = locs[0]
                                exportFileDialog.currentFile = locs[0].toString() + "/" + exportDefaultName()
                            }
                            exportFileDialog.open()
                        }
                    }

                    ItemDelegate {
                        Layout.fillWidth: true
                        text: tox_state.state === "signedin"
                              ? (tox_state.encrypted ? "Remove password… (sign out first)"
                                                     : "Add a password… (sign out first)")
                              : (tox_state.encrypted ? "Remove password…"
                                                     : "Add a password…")
                        enabled: tox_state.state !== "signedin"
                                  && tox_state.state !== "empty"
                        onClicked: tox_state.encrypted
                                  ? openRemoveSavePassword()
                                  : openAddSavePassword()
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 1
                        color: "#eee6d2"
                    }

                    ItemDelegate {
                        Layout.fillWidth: true
                        enabled: tox_state.state !== "signedin"
                        onClicked: deleteConfirmDlg.open()

                        contentItem: Label {
                            text: tox_state.state === "signedin"
                                  ? "Delete this tox-id (sign out first)"
                                  : "Delete this tox-id"
                            color: tox_state.state === "signedin" ? "#c88" : "#c00"
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
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

                    // a short list, so it lives in the page's one scroll
                    // surface instead of being a nested flickable.
                    Repeater {
                        model: tox_acct.sharedSaves

                        delegate: Rectangle {
                            id: sharedRow
                            Layout.fillWidth: true
                            implicitHeight: sharedRowCol.implicitHeight + mm(2)
                            radius: mm(1)
                            color: "#faf8f1"
                            border.color: "#e9e3d1"
                            border.width: 1

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

                                    ToolButton {
                                        icon.source: mi("ic_more_vert_black_24dp.png")
                                        icon.width: dp(20)
                                        icon.height: dp(20)
                                        Layout.preferredWidth: dp(40)
                                        Layout.preferredHeight: dp(40)
                                        onClicked: sharedRowMenu.popup()
                                    }
                                }

                                ResponsiveRow {
                                    visible: modelData.is_current === true

                                    Rectangle {
                                        visible: modelData.is_current === true
                                        width: thisBadge.implicitWidth + mm(1.5)
                                        height: mm(4.5)
                                        radius: mm(2.25)
                                        color: "#e6f0e6"

                                        Text {
                                            id: thisBadge
                                            anchors.centerIn: parent
                                            text: "this device"
                                            color: "#2a6"
                                            font.pixelSize: dp(10)
                                        }
                                    }

                                    Rectangle {
                                        visible: modelData.is_current === true
                                                 && tox_state.diffState === tox_state.diffDifferent
                                        width: diffBadge.implicitWidth + mm(1.5)
                                        height: mm(4.5)
                                        radius: mm(2.25)
                                        color: "#fdf0dc"

                                        Text {
                                            id: diffBadge
                                            anchors.centerIn: parent
                                            text: "differs from this device"
                                            color: "#a60"
                                            font.pixelSize: dp(10)
                                        }
                                    }
                                }
                            }

                            Menu {
                                id: sharedRowMenu

                                MenuItem {
                                    text: "Use this tox-id"
                                    onTriggered: tox_acct.openUseSave(modelData)
                                }

                                MenuItem {
                                    text: "Stop sharing"
                                    onTriggered: tox_acct.openUnshareConfirm(modelData)
                                }
                            }
                        }
                    }

                    Label {
                        visible: tox_acct.sharedSaves.length === 0
                        text: "Nothing here yet. Share a tox-id with your devices and "
                              + "it shows up here on this device."
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        color: secondary_text
                        font.pixelSize: dp(11)
                    }

                    // spell out the direction of both copy actions. they write
                    // the same file in opposite directions, and the labels
                    // alone don't make that obvious.
                    Label {
                        visible: tox_acct.sharedSaves.length > 0
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

            CollapsibleSection {
                Layout.fillWidth: true
                title: "About saves and sharing"
                subtitle: "Export, sharing, passwords and deleting"

                Label {
                    Layout.fillWidth: true
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
                    color: secondary_text
                    font.pixelSize: dp(10)
                }
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
        width: Math.min(Math.max(implicitWidth, mm(70)), Overlay.overlay.width - mm(4))

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: tox_acct.pendingRow
                      ? "Remove " + tox_acct.rowTitle(tox_acct.pendingRow) + " from your devices' "
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
        width: Math.min(Math.max(implicitWidth, mm(70)), Overlay.overlay.width - mm(4))

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: tox_acct.pwEntryIntro
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
        width: Math.min(Math.max(implicitWidth, mm(70)), Overlay.overlay.width - mm(4))

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
        width: Math.min(Math.max(implicitWidth, mm(70)), Overlay.overlay.width - mm(4))

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
        width: Math.min(Math.max(implicitWidth, mm(70)), Overlay.overlay.width - mm(4))

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
        width: Math.min(Math.max(implicitWidth, mm(70)), Overlay.overlay.width - mm(4))

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

    // ---- use a shared tox-id confirmation ----

    Dialog {
        id: useSaveConfirmDlg
        title: "Use this tox-id?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton
        width: Math.min(Math.max(implicitWidth, mm(70)), Overlay.overlay.width - mm(4))

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: tox_acct.pendingUseRow
                      ? "Use " + tox_acct.rowTitle(tox_acct.pendingUseRow) + "?"
                      : ""
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                font.bold: true
            }

            Label {
                visible: text.length > 0
                text: tox_acct.pendingUseRow
                      ? tox_acct.replaceWarning(tox_acct.pendingUseRow)
                      : ""
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: amber_dark
            }

            Label {
                text: "This copies the shared save over the save on this device. "
                      + "The current save is backed up first."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            TextField {
                id: usePwInput
                visible: tox_acct.pendingUseRow
                         && tox_acct.pendingUseRow.encrypted === true
                echoMode: TextInput.Password
                placeholderText: "Password"
                Layout.fillWidth: true
                onAccepted: confirmUseSave()
            }

            Label {
                id: usePwError
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
                        pendingUseRow = null
                        usePwInput.text = ""
                        usePwError.text = ""
                        useSaveConfirmDlg.close()
                    }
                }

                Button {
                    text: "Use this tox-id"
                    onClicked: confirmUseSave()
                }
            }
        }
    }
}
