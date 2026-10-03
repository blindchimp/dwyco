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
    // the tox status state is owned by tox_state in main.qml, which both
    // this page and ToxPage.qml read, so the two panels cannot disagree.
    // this page only owns the shared-saves list, which is its own thing.

    function passwordLabel() {
        return tox_state.encrypted ? "Password protected" : "Not set"
    }

    // re-sample the shared state and this page's list. used when the page
    // comes forward, and after any operation that changes the state
    // without emitting a core signal.
    function doRefresh() {
        tox_state.refresh()
        refreshShared()
    }

    // ---- sign in / out ----

    function signIn(pw) {
        if (!core.tox_enabled)
            core.enable_tox()
        tox_state.refresh()
        if (core.tox_needs_password()) {
            if (pw.length === 0)
                return false
            return core.tox_unlock(pw)
        }
        return true
    }

    function doSignIn(pw) {
        cardPwError.text = ""
        if (!signIn(pw)) {
            cardPwError.text = "Wrong password."
            cardPwInput.text = ""
            cardPwInput.forceActiveFocus()
            return false
        }
        core.set_local_setting("tox_enabled", "1")
        showBanner("Signed in.")
        doRefresh()
        return true
    }

    // the card has its own password field, so a locked save never needs
    // a dialog to sign in.
    function signInFromCard() {
        if (cardPwInput.text.length === 0)
            return
        if (doSignIn(cardPwInput.text))
            cardPwInput.text = ""
    }

    function signOut() {
        if (core.tox_enabled)
            core.disable_tox()
        core.set_local_setting("tox_enabled", "0")
        doRefresh()
        showBanner("Signed out.")
    }

    // ---- transient result banner ----
    // replaces the single "Tox Account" dialog that used to report both
    // successes and failures, which made the two indistinguishable.

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

    // ---- local save file operations ----

    function doImport(path, pw) {
        var err = core.tox_import_profile(path, pw, !importNoBackup.checked)
        if (err.length > 0) {
            importError.text = "Import failed: " + err
            return
        }
        importConfirmDlg.close()
        core.set_local_setting("tox_enabled", "1")
        showBanner("Imported. The new Tox save is in place.")
        doRefresh()
    }

    function doCreateNew() {
        createNewConfirmDlg.close()
        var ok = core.tox_reset_identity()
        if (ok) {
            core.set_local_setting("tox_enabled", "1")
            showBanner("Created a new Tox save.")
        } else {
            showBanner("Could not create a new Tox save.", true)
        }
        doRefresh()
    }

    function doDeleteSave() {
        deleteConfirmDlg.close()
        if (core.tox_enabled)
            core.disable_tox()
        var ok = core.tox_factory_reset()
        core.set_local_setting("tox_enabled", "0")
        if (ok) {
            // the save is gone, so the remembered name/address are stale.
            // clearing them through the shared owner is what stops the
            // tox page carrying on showing the deleted identity.
            tox_state.forgetCachedIdentity()
            showBanner("Tox save deleted. A copy was saved to disk first.")
        } else {
            showBanner("Delete failed. The Tox save was not removed.", true)
        }
        doRefresh()
    }

    // ---- tox saves shared with the rest of the group ----
    // one entry per save any group member has published. keys come from
    // DwycoCore::tox_list_saves(): mid, pubkey, when, size, encrypted,
    // is_current, holder, held_by_me. note mid and pubkey are the same
    // string, the hex pubkey of the save.

    property var sharedList: []
    // the row a confirmation dialog is about. the list can refresh while
    // a dialog is open, so the dialogs must not read a live index.
    property var pendingRow: null

    function refreshShared() {
        sharedList = core.tox_list_saves()
    }

    // load a shared save. this is a file level operation: it leaves tox
    // signed out, so the card drops to the "loaded" state where the
    // password field (or a plain sign in) takes over.
    function doLoad(row) {
        selectSharedDlg.close()
        var err = core.tox_select_save(row.mid)
        if (err.length > 0) {
            showBanner("Could not load that save: " + err, true)
            pendingRow = null
            return
        }
        showBanner("Loaded " + shortPub(row.pubkey)
                   + ". Sign in to use it on this device.")
        pendingRow = null
        doRefresh()
    }

    // drop a shared save from every group member's list. does not change
    // any client's tox state, and is a one shot removal: a device still
    // signed in with the save can publish it again.
    function doUnshare(row) {
        unshareConfirmDlg.close()
        var removed = core.tox_depublish_save(row.mid)
        pendingRow = null
        if (removed)
            showBanner(shortPub(row.pubkey)
                       + " is no longer shared with your devices.")
        else
            showBanner("That save was already not shared.")
        doRefresh()
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
        return m + " min ago"
    }

    function holderLabel(row) {
        if (row.held_by_me)
            return "In use here"
        if (row.holder && row.holder.length > 0)
            return "In use on " + shortPub(row.holder)
        return "Not in use"
    }

    function rowSubLabel(row) {
        var s = holderLabel(row) + " · " + sizeLabel(row.size)
        if (row.encrypted)
            s += " · Password protected"
        return s
    }

    function shareCurrent() {
        if (core.tox_publish_save())
            showBanner("Shared with your other devices.")
        else
            showBanner("Could not share. Sign in to your Tox save first.", true)
        refreshShared()
    }

    // ---- filename helpers (for default export name) ----

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
        // the tox status state itself is refreshed centrally in main.qml
        // (see tox_state), so only the shared-saves list is left to track
        // here. debounced upstream: another group member published,
        // claimed or dropped a save.
        function onTox_saves_changed() { refreshShared() }
        function onTox_disabled_by_remote(holder) { doRefresh() }
    }

    onVisibleChanged: {
        if (visible) {
            doRefresh()
            refreshShared()
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

            // ---- 1. this device ----

            Label {
                text: "This Device"
                font.bold: true
                Layout.topMargin: mm(1)
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
                            Layout.preferredWidth: mm(1.5)
                            Layout.preferredHeight: mm(1.5)
                            radius: mm(0.75)
                            color: tox_state.dotColor
                        }

                        Label {
                            text: tox_state.statusText
                            font.bold: true
                            font.pixelSize: dp(14)
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                        }
                    }

                    // no save yet: this is where you get one.
                    ColumnLayout {
                        visible: tox_state.state === "empty"
                        Layout.fillWidth: true
                        spacing: mm(1)

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: mm(1)

                            Button {
                                text: "Import from file…"
                                Layout.fillWidth: true
                                onClicked: {
                                    importPath = ""
                                    importPw = ""
                                    importFileDialog.open()
                                }
                            }

                            Button {
                                text: "Create new Tox save"
                                Layout.fillWidth: true
                                onClicked: createNewConfirmDlg.open()
                            }
                        }
                    }

                    // save present: identity and password, each with the
                    // action that changes it right next to the answer.
                    ColumnLayout {
                        visible: tox_state.present
                        Layout.fillWidth: true
                        spacing: mm(0.5)

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: mm(1)

                            Label {
                                text: "Tox ID"
                                font.bold: true
                            }

                            Label {
                                text: tox_state.displayId
                                font.family: "monospace"
                                font.pixelSize: dp(10)
                            }

                            Label {
                                visible: tox_state.displayIdCached
                                text: "(last used)"
                                color: "#666"
                                font.pixelSize: dp(10)
                            }

                            Item { Layout.fillWidth: true }

                            Button {
                                text: "Copy"
                                enabled: tox_state.displayId !== "-"
                                onClicked: {
                                    core.copy_to_clipboard(
                                        tox_state.state === "signedin"
                                        ? tox_state.selfAddress
                                        : tox_state.cachedAddress)
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: mm(1)

                            Label {
                                text: "Password"
                                font.bold: true
                            }

                            Label {
                                text: passwordLabel()
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                            }

                            Button {
                                visible: !tox_state.encrypted
                                text: "Set password…"
                                enabled: tox_state.signedOut
                                onClicked: {
                                    setPwInput.text = ""
                                    setPwConfirmInput.text = ""
                                    setPwError.text = ""
                                    setPwDialog.open()
                                }
                            }

                            Button {
                                visible: tox_state.encrypted
                                text: "Remove password…"
                                enabled: tox_state.signedOut
                                onClicked: openPwEntry(
                                    "removepw",
                                    "Enter the current password to remove the "
                                    + "password from this Tox save.")
                            }
                        }

                        Label {
                            visible: tox_state.state === "signedin"
                            text: "Sign out to change the password."
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            color: "#666"
                            font.pixelSize: dp(11)
                        }

                        // locked save: sign in right here, no dialog.
                        RowLayout {
                            visible: tox_state.state === "locked"
                            Layout.fillWidth: true
                            spacing: mm(1)

                            TextField {
                                id: cardPwInput
                                echoMode: TextInput.Password
                                placeholderText: "Password"
                                Layout.fillWidth: true
                                onAccepted: signInFromCard()
                            }

                            Button {
                                text: "Sign in"
                                enabled: cardPwInput.text.length > 0
                                onClicked: signInFromCard()
                            }
                        }

                        Label {
                            id: cardPwError
                            text: ""
                            color: "#a00"
                            visible: text.length > 0
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        Label {
                            visible: tox_state.encrypted
                            text: "Forgot it? You can import a different save "
                                  + "or delete this one — you don't need the "
                                  + "old password for either."
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            color: "#666"
                            font.pixelSize: dp(11)
                        }
                    }

                    Button {
                        id: signInOutButton
                        // hidden while a save is locked, because the card's
                        // own password field is the sign in for that state.
                        visible: tox_state.state !== "empty"
                                 && tox_state.state !== "locked"
                        text: tox_state.state === "signedin" ? "Sign out" : "Sign in"
                        Layout.fillWidth: true
                        onClicked: {
                            if (tox_state.state === "signedin")
                                signOut()
                            else
                                doSignIn("")
                        }
                    }
                }
            }

            // ---- 2. back up or replace the local save ----

            Label {
                visible: tox_state.present
                text: "Back Up or Replace"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            ColumnLayout {
                visible: tox_state.present
                Layout.fillWidth: true
                spacing: mm(1)

                RowLayout {
                    Layout.fillWidth: true
                    spacing: mm(1)

                    Button {
                        text: "Import from file…"
                        enabled: tox_state.state !== "signedin"
                        Layout.fillWidth: true
                        onClicked: {
                            importPath = ""
                            importPw = ""
                            importFileDialog.open()
                        }
                    }

                    Button {
                        text: "Create new Tox save"
                        enabled: tox_state.state !== "signedin"
                        Layout.fillWidth: true
                        onClicked: createNewConfirmDlg.open()
                    }
                }

                Label {
                    visible: tox_state.state === "signedin"
                    text: "Sign out to import or create a Tox save."
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    color: "#666"
                    font.pixelSize: dp(11)
                }

                Button {
                    text: "Export to file…"
                    Layout.fillWidth: true
                    onClicked: {
                        var locs = StandardPaths.standardLocations(StandardPaths.DocumentsLocation)
                        if (locs.length > 0) {
                            exportFileDialog.currentFolder = locs[0]
                            exportFileDialog.currentFile = locs[0].toString() + "/" + exportDefaultName()
                        }
                        exportFileDialog.open()
                    }
                }
            }

            // ---- 3. saves shared with the rest of the group ----

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: mm(2)
                Layout.bottomMargin: mm(0.5)
                spacing: mm(1)

                Label {
                    text: "Shared Saves"
                    font.bold: true
                    Layout.fillWidth: true
                }

                Button {
                    text: "Share this save"
                    enabled: tox_state.state === "signedin"
                    onClicked: shareCurrent()
                }
            }

            Label {
                text: "Tox saves shared with your other devices. Anyone in your "
                      + "group can use one, but only one device at a time."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#666"
                font.pixelSize: dp(11)
            }

            // one hint for the whole section, chosen by state: an empty
            // list, a signed out save, or nothing to show at all.
            Label {
                visible: text.length > 0
                text: {
                    if (sharedList.length === 0) {
                        if (tox_state.state === "empty")
                            return "No shared saves yet. Create or import a "
                                   + "Tox save to get started."
                        if (tox_state.state !== "signedin")
                            return "No shared saves yet. Sign in to your Tox save "
                                   + "and use \"Share this save\" to add one."
                        return "No shared saves yet. Use \"Share this save\" to add one."
                    }
                    if (tox_state.state !== "signedin" && tox_state.present)
                        return "Sign in to your Tox save to share it with your devices."
                    return ""
                }
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#666"
                font.pixelSize: dp(11)
            }

            ListView {
                id: sharedListView
                model: sharedList
                visible: sharedList.length > 0
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(sharedListView.contentHeight, mm(100))
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar { }

                delegate: Item {
                    id: sharedRow
                    width: ListView.view.width
                    height: sharedRowCol.implicitHeight + mm(2)

                    Rectangle {
                        anchors.fill: parent
                        radius: mm(1)
                        color: modelData.is_current ? "white" : "transparent"
                        border.color: modelData.is_current ? "#ccc" : "#dedede"
                        border.width: 1
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
                                visible: modelData.is_current
                                Layout.preferredWidth: mm(1.5)
                                Layout.preferredHeight: mm(1.5)
                                radius: mm(0.75)
                                color: accent
                            }

                            Text {
                                Layout.fillWidth: true
                                text: shortPub(modelData.pubkey)
                                font.family: "monospace"
                                font.pixelSize: dp(11)
                                elide: Text.ElideRight
                            }

                            Text {
                                text: modelData.is_current
                                      ? "This device"
                                      : ageLabel(Math.floor(Date.now() / 1000)
                                                 - modelData.when)
                                font.pixelSize: dp(10)
                                font.bold: modelData.is_current
                                color: modelData.is_current ? accent : "#666"
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: rowSubLabel(modelData)
                            font.pixelSize: dp(10)
                            color: modelData.held_by_me ? "#070" : "#666"
                            elide: Text.ElideRight
                        }

                        // actions belong to the row they act on, so
                        // there is no separate selection to get wrong.
                        RowLayout {
                            visible: !modelData.is_current
                            Layout.fillWidth: true
                            spacing: mm(1)

                            Button {
                                text: "Load"
                                onClicked: {
                                    pendingRow = modelData
                                    selectSharedDlg.open()
                                }
                            }

                            Button {
                                text: "Stop sharing"
                                onClicked: {
                                    pendingRow = modelData
                                    unshareConfirmDlg.open()
                                }
                            }

                            Item { Layout.fillWidth: true }
                        }
                    }
                }
            }

            // ---- 4. delete the local save (destructive, so it gets its
            // own card at the bottom, away from the routine operations) ----

            Label {
                visible: tox_state.present
                text: "Delete"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            Pane {
                visible: tox_state.present
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

                    Button {
                        text: "Delete this Tox save"
                        enabled: tox_state.state !== "signedin"
                        Layout.fillWidth: true
                        background: Rectangle {
                            color: tox_state.state === "signedin" ? "#999" : "#c00"
                            radius: mm(1)
                        }
                        contentItem: Label {
                            text: "Delete this Tox save"
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: deleteConfirmDlg.open()
                    }

                    Label {
                        visible: tox_state.state === "signedin"
                        text: "Sign out to delete this Tox save."
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        color: "#666"
                        font.pixelSize: dp(11)
                    }

                    Label {
                        text: "Deletes the Tox save on this device only. A copy is "
                              + "saved to disk first. Your messages, contacts and "
                              + "Dwyco account are not affected."
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        color: "#666"
                        font.pixelSize: dp(11)
                    }
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

    // ---- password entry, for import and remove-password only ----
    // signing in to a locked save is handled by the card's own field, so
    // this dialog never has to double as the sign in flow.

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

    Dialog {
        id: pwEntryDialog
        title: pwEntryPurpose === "import"
               ? "Password-protected Tox save" : "Remove password"
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
                    text: pwEntryPurpose === "import" ? "Next" : "Remove password"
                    enabled: pwEntryInput.text.length > 0
                    onClicked: {
                        pwEntryError.text = ""
                        if (pwEntryPurpose === "import") {
                            pwEntryDialog.close()
                            importPw = pwEntryInput.text
                            openImportConfirm()
                        } else {
                            var err = core.tox_set_save_password(pwEntryInput.text, "")
                            if (err.length === 0) {
                                pwEntryDialog.close()
                                showBanner("Password removed. This Tox save is "
                                           + "no longer protected.")
                                // tox_set_save_password emits no core
                                // signal, so the shared state has to be
                                // re-sampled by hand here.
                                doRefresh()
                            } else {
                                pwEntryError.text = "Could not remove password: " + err
                                pwEntryInput.text = ""
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- set a new password ----

    Dialog {
        id: setPwDialog
        title: "Set password"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "Choose a password. From now on you must enter it to sign "
                      + "in to this Tox save."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
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
                    onClicked: setPwDialog.close()
                }

                Button {
                    text: "Set password"
                    enabled: setPwInput.text.length > 0
                    onClicked: {
                        setPwError.text = ""
                        if (setPwInput.text !== setPwConfirmInput.text) {
                            setPwError.text = "Passwords do not match."
                            return
                        }
                        var err = core.tox_set_save_password("", setPwInput.text)
                        if (err.length === 0) {
                            setPwDialog.close()
                            showBanner("Password set. You will need it to sign in.")
                            // see above: no signal is emitted for this.
                            doRefresh()
                        } else {
                            setPwError.text = "Could not set password: " + err
                        }
                    }
                }
            }
        }
    }

    // ---- load confirmation ----

    Dialog {
        id: selectSharedDlg
        title: "Load this Tox save?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: pendingRow
                      ? "Make " + shortPub(pendingRow.pubkey) + " the Tox save on "
                        + "this device?"
                      : ""
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: pendingRow && pendingRow.holder && pendingRow.holder.length > 0
                      ? "It stays in use on " + shortPub(pendingRow.holder)
                        + " until you sign in here. Signing in is what takes it over."
                      : "You will be signed out. Sign in afterwards to start using it."
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
                        selectSharedDlg.close()
                    }
                }

                Button {
                    text: "Load"
                    onClicked: doLoad(pendingRow)
                }
            }
        }
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
                      ? "Remove " + shortPub(pendingRow.pubkey) + " from your "
                        + "devices' shared list?"
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
                visible: pendingRow && pendingRow.held_by_me
                text: "This save is in use on this device. It stays signed in "
                      + "here, and it can be shared again at any time."
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
                    text: "Create new Tox save"
                    onClicked: doCreateNew()
                }
            }
        }
    }

    // ---- delete save confirmation ----

    Dialog {
        id: deleteConfirmDlg
        title: "Delete Tox save"
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
                    text: "Delete this Tox save"
                    background: Rectangle {
                        color: "#c00"
                        radius: mm(1)
                    }
                    contentItem: Label {
                        text: "Delete this Tox save"
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
