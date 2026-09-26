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
            text: "Tox Account"
            color: "white"
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
    background: Rectangle {
        color: amber_light
    }

    // derived state, refreshed by doRefresh() after every operation.
    // stored as plain properties so QML bindings stay reactive.
    property string stateName: "empty"
    property string stateText: ""
    property string encText: ""
    property bool effPresent: false
    property bool effEncrypted: false
    property bool effRunning: false
    property string toxShortId: "-"

    function oml() // out of main: refresh all state
    {
        effPresent = core.tox_save_exists()
        effEncrypted = core.tox_save_is_encrypted()
        effRunning = core.tox_enabled && !core.tox_needs_password()
    }

    function doRefresh() {
        oml()
        if (!effPresent) {
            stateName = "empty"
            stateText = "No tox save yet.\nImport a profile or create a new identity to get started."
            encText = "None"
            toxShortId = "-"
        } else if (effRunning) {
            stateName = effEncrypted ? "active_unlocked" : "active_clear"
            stateText = effEncrypted ? "Signed in (password protected)." : "Signed in."
            encText = effEncrypted ? "Password protected" : "None"
            var addr = core.tox_self_address
            if (addr.length >= 8)
                toxShortId = addr.substring(0, 8) + "..."
            else {
                var pk = core.tox_get_self_public_key()
                toxShortId = pk.length >= 8 ? pk.substring(0, 8) + "..." : "-"
            }
        } else {
            stateName = effEncrypted ? "loaded_encrypted" : "loaded_clear"
            stateText = effEncrypted ? "Tox save is password protected.\nEnter the password to sign in, or export/remove it."
                                      : "Tox save exists. Not signed in."
            encText = effEncrypted ? "Password protected" : "None"
            toxShortId = "-"
        }
    }

    function signIn(pw) {
        if (!core.tox_enabled)
            core.enable_tox()
        if (core.tox_needs_password()) {
            if (pw.length === 0)
                return false
            return core.tox_unlock(pw)
        }
        return true
    }

    function doSignIn(pw) {
        if (signIn(pw)) {
            core.set_local_setting("tox_enabled", "1")
            resultText.text = "Signed in to tox."
            resultDialog.open()
            doRefresh()
        } else {
            pwEntryError.text = "Wrong password or corrupt profile. Try again."
            pwEntryInput.text = ""
            pwEntryInput.forceActiveFocus()
        }
    }

    function signOut() {
        if (core.tox_enabled)
            core.disable_tox()
        core.set_local_setting("tox_enabled", "0")
        doRefresh()
    }

    function doImport(path, pw) {
        var err = core.tox_import_profile(path, pw, !importNoBackup.checked)
        if (err.length > 0) {
            importError.text = "Import failed: " + err
        } else {
            importConfirmDlg.close()
            core.set_local_setting("tox_enabled", "1")
            resultText.text = "Imported. Your new Tox identity is now in use."
            resultDialog.open()
            doRefresh()
        }
    }

    function doCreateNew() {
        createNewConfirmDlg.close()
        var ok = core.tox_reset_identity()
        if (ok) {
            core.set_local_setting("tox_enabled", "1")
            resultText.text = "A new Tox identity was created."
            resultDialog.open()
        } else {
            resultText.text = "Could not create a new Tox identity."
            resultDialog.open()
        }
        doRefresh()
    }

    function doReset() {
        resetConfirmDlg.close()
        if (core.tox_enabled)
            core.disable_tox()
        var ok = core.tox_factory_reset()
        core.set_local_setting("tox_enabled", "0")
        core.set_local_setting("cached_tox_name", "")
        core.set_local_setting("cached_tox_address", "")
        if (ok) {
            resultText.text = "Tox was reset. This is a fresh start."
            resultDialog.open()
        } else {
            resultText.text = "Reset failed. The tox save was not removed."
            resultDialog.open()
        }
        doRefresh()
        refreshShared()
    }

    // ---- group-shared tox identities ----

    // one entry per identity the group has published. keys come from
    // DwycoCore::tox_list_saves(): mid, pubkey, when, size, encrypted,
    // is_current, holder, held_by_me
    property var sharedList: []
    property int sharedSel: -1
    property var sharedSelRow: sharedSel >= 0 && sharedSel < sharedList.length
                              ? sharedList[sharedSel] : null

    function refreshShared() {
        var keepMid = sharedSelRow ? sharedSelRow.mid : ""
        sharedList = core.tox_list_saves()
        // keep the selection across a refresh if we can find it again
        sharedSel = -1
        if (keepMid.length > 0) {
            for (var i = 0; i < sharedList.length; ++i) {
                if (sharedList[i].mid === keepMid) {
                    sharedSel = i
                    break
                }
            }
        }
    }

    // adopt the selected shared identity. leaves tox stopped, so the ui
    // drops to the "loaded, not signed in" state where Sign In lives.
    function doSelectShared() {
        if (!sharedSelRow)
            return
        var row = sharedSelRow
        selectSharedDlg.close()
        var err = core.tox_select_save(row.mid)
        if (err.length > 0) {
            resultText.text = "Could not use that identity: " + err
            resultDialog.open()
            return
        }
        resultText.text = "Identity " + shortPub(row.pubkey) + " loaded. Sign in to start using it here."
        resultDialog.open()
        doRefresh()
        refreshShared()
    }

    // drop a shared identity from every group member's list. does not change
    // any client's tox state, and is a one shot removal: a device still
    // signed in with the identity can publish it again.
    function doUnshare() {
        if (!sharedSelRow)
            return
        var row = sharedSelRow
        unshareConfirmDlg.close()
        if (core.tox_depublish_save(row.mid)) {
            resultText.text = "Identity " + shortPub(row.pubkey)
                            + " is no longer shared with the group."
            resultDialog.open()
        } else {
            // nothing to remove -- another member already unpublished it.
            resultText.text = "That identity was already not shared."
            resultDialog.open()
        }
        doRefresh()
        refreshShared()
    }

    function shortPub(pk) {
        if (!pk || pk.length < 12)
            return "?"
        return pk.substring(0, 12) + "..."
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

    function holderLabel(row) {
        if (row.held_by_me)
            return "in use here"
        if (row.holder && row.holder.length > 0)
            return "in use on " + shortPub(row.holder)
        return "not in use"
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
        function onTox_enabledChanged() { doRefresh() }
        function onTox_connection_status_changed(connected) { doRefresh() }
        function onTox_import_finished() { doRefresh() }
        // debounced upstream: the set of shared identities (or who holds one)
        // changed, eg. another group member published or claimed one.
        function onTox_saves_changed() { refreshShared() }
        function onTox_disabled_by_remote(holder) { doRefresh(); refreshShared() }
    }

    onVisibleChanged: {
        if (visible) {
            doRefresh()
            refreshShared()
        }
    }

    Component.onCompleted: {
        doRefresh()
        refreshShared()
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

            Label {
                text: "Status"
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
                    spacing: mm(0.5)

                    Label {
                        text: stateText
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: dp(14)
                    }

                    RowLayout {
                        spacing: mm(1)
                        Layout.fillWidth: true

                        Label {
                            text: "Encryption:"
                            font.bold: true
                        }
                        Label {
                            text: encText
                        }
                    }

                    RowLayout {
                        spacing: mm(1)
                        Layout.fillWidth: true

                        Label {
                            text: "Tox ID:"
                            font.bold: true
                        }
                        Label {
                            font.family: "monospace"
                            font.pixelSize: 10
                            text: toxShortId
                        }
                        Item { Layout.fillWidth: true }
                        Button {
                            text: "Copy"
                            visible: effRunning
                            onClicked: core.copy_to_clipboard(core.tox_self_address)
                        }
                    }
                }
            }

            Label {
                text: "Setup"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            Button {
                text: "Import Profile..."
                enabled: !effRunning && effPresent || stateName === "empty"
                Layout.fillWidth: true
                onClicked: {
                    importPath = ""
                    importPw = ""
                    importFileDialog.open()
                }
            }

            Button {
                text: "Create New Identity"
                enabled: !effRunning
                Layout.fillWidth: true
                onClicked: createNewConfirmDlg.open()
            }

            Label {
                text: "Shared Identities"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            Label {
                text: "Tox identities shared with your group. Anyone in the group can run one, but only one device at a time. Picking one here loads it; sign in to start using it. Sharing only happens when you press Publish, and stops when you press Stop Sharing."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#666"
                font.pixelSize: dp(11)
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: mm(1)

                Button {
                    text: "Refresh"
                    onClicked: refreshShared()
                }

                Button {
                    text: "Publish Mine"
                    enabled: core.tox_enabled
                    onClicked: {
                        if (core.tox_publish_save())
                            resultText.text = "Your tox identity was published to the group."
                        else
                            resultText.text = "Could not publish. Sign in to tox first."
                        resultDialog.open()
                        refreshShared()
                    }
                }

                Item { Layout.fillWidth: true }
            }

            ListView {
                id: sharedListView
                model: sharedList
                Layout.fillWidth: true
                Layout.preferredHeight: sharedList.length === 0 ? 0 : mm(9 * Math.min(sharedList.length, 4) + 1)
                clip: true
                spacing: mm(0.5)
                currentIndex: sharedSel
                highlight: Rectangle { color: amber_accent; opacity: 0.3 }
                ScrollBar.vertical: ScrollBar { }

                delegate: Item {
                    width: ListView.view.width
                    height: mm(9)

                    MouseArea {
                        anchors.fill: parent
                        onClicked: sharedSel = index
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 1
                        color: "transparent"
                        border.color: modelData.is_current ? amber_accent : "transparent"
                        border.width: 1
                        radius: 2

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: mm(1)
                            spacing: mm(1)

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 0

                                Text {
                                    text: shortPub(modelData.pubkey)
                                    font.family: "monospace"
                                    font.pixelSize: dp(11)
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                                Text {
                                    text: holderLabel(modelData) + " · "
                                          + sizeLabel(modelData.size) + " · "
                                          + ageLabel(Math.floor(Date.now() / 1000) - modelData.when)
                                    font.pixelSize: dp(10)
                                    color: modelData.held_by_me ? "#070" : "#666"
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }

                            Rectangle {
                                visible: modelData.encrypted
                                Layout.preferredWidth: lockedLabel.implicitWidth + mm(1)
                                Layout.preferredHeight: lockedLabel.implicitHeight + mm(0.5)
                                radius: 2
                                color: "#eee"
                                Text {
                                    id: lockedLabel
                                    anchors.centerIn: parent
                                    text: "locked"
                                    font.pixelSize: dp(9)
                                    color: "#444"
                                }
                            }
                        }
                    }
                }
            }

            Label {
                visible: sharedList.length === 0
                text: "(No shared identities yet. Sign in to tox and use \"Publish Mine\" to add one.)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#666"
                font.pixelSize: dp(11)
            }

            Button {
                text: "Use This Identity"
                enabled: sharedSelRow !== null && !sharedSelRow.is_current
                Layout.fillWidth: true
                // note: if the identity is password protected we don't ask
                // for it here. loading it puts the page into the normal
                // "loaded_encrypted" state, where the existing Sign In button
                // asks for the password.
                onClicked: selectSharedDlg.open()
            }

            Button {
                text: "Stop Sharing This Identity"
                enabled: sharedSelRow !== null
                Layout.fillWidth: true
                onClicked: {
                    if (sharedSelRow)
                        unshareConfirmDlg.open()
                }
            }

            Label {
                visible: sharedSelRow !== null && sharedSelRow !== null && sharedSelRow.held_by_me
                text: "This identity is currently in use on this device. Signing in with it elsewhere will turn it off here."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#a00"
                font.pixelSize: dp(11)
            }

            Label {
                text: "Account"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            Button {
                text: "Export Profile..."
                enabled: effPresent
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

            Button {
                id: signInOutButton
                text: {
                    if (stateName === "active_clear" || stateName === "active_unlocked")
                        return "Sign Out"
                    if (stateName === "loaded_encrypted")
                        return "Sign In with Password..."
                    return "Sign In"
                }
                enabled: stateName !== "empty"
                Layout.fillWidth: true
                onClicked: {
                    if (stateName === "active_clear" || stateName === "active_unlocked") {
                        signOut()
                    } else if (stateName === "loaded_encrypted") {
                        pwEntryPurpose = "signin"
                        openPwEntry("Enter the password for this Tox profile to sign in.")
                    } else if (stateName === "loaded_clear") {
                        doSignIn("")
                    }
                }
            }

            Label {
                visible: stateName === "loaded_encrypted"
                text: "This profile is password protected. You must enter its password to sign in."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#a00"
            }

            Label {
                text: "Encryption"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            Button {
                text: "Add Encryption..."
                visible: stateName === "loaded_clear"
                enabled: stateName === "loaded_clear" && !effRunning
                Layout.fillWidth: true
                onClicked: {
                    setPwInput.text = ""
                    setPwConfirmInput.text = ""
                    setPwError.text = ""
                    setPwDialog.open()
                }
            }

            Button {
                text: "Remove Encryption..."
                visible: stateName === "loaded_encrypted"
                enabled: stateName === "loaded_encrypted" && !effRunning
                Layout.fillWidth: true
                onClicked: {
                    pwEntryPurpose = "removeenc"
                    openPwEntry("Enter the current password to remove encryption from this profile.")
                }
            }

            Label {
                text: "Sign out to change encryption."
                color: "#a00"
                visible: stateName === "active_clear" || stateName === "active_unlocked"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.topMargin: mm(2)
                implicitHeight: mm(1)
                color: "#ccc"
            }

            Label {
                text: "Danger Zone"
                font.bold: true
                color: "#a00"
                Layout.topMargin: mm(1)
            }

            Button {
                text: "Reset to Factory"
                enabled: stateName !== "empty" && !effRunning
                Layout.fillWidth: true
                background: Rectangle {
                    color: stateName !== "empty" && !effRunning ? "#c00" : "#999"
                    radius: mm(1)
                }
                contentItem: Label {
                    text: "Reset to Factory"
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: resetConfirmDlg.open()
            }

            Label {
                text: "Reset deletes the current Tox save and returns to the state before Tox was first used. A copy is saved to disk first."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#666"
            }

            Label {
                text: "Forgot your password?"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            Label {
                text: "If you forgot the password for a protected profile, you can still import a different profile, create a new identity, or reset to factory above. You don't need the old password for those."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#666"
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
        title: "Choose a Tox profile (.tox) to import"
        nameFilters: ["Tox profiles (*.tox)", "All files (*)"]
        onRejected: {
            importPath = ""
            importPw = ""
        }
        onAccepted: {
            var p = core.url_to_filename(selectedFile)
            if (p === "") {
                resultText.text = "Could not use that file."
                resultDialog.open()
                return
            }
            importPath = p
            if (core.tox_file_is_encrypted(p)) {
                pwEntryPurpose = "import"
                openPwEntry("This profile was saved with a password. Enter it to import.")
            } else {
                importPw = ""
                openImportConfirm()
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
            if (p === "") {
                resultText.text = "Could not use that location."
                resultDialog.open()
                return
            }
            if (p.toLowerCase().lastIndexOf(".tox") !== p.length - 4)
                p += ".tox"
            var err = core.tox_export_profile(p)
            if (err.length > 0)
                resultText.text = "Export failed: " + err
            else
                resultText.text = "Profile exported to " + p
            resultDialog.open()
        }
    }

    function openImportConfirm() {
        importError.text = ""
        importNoBackup.checked = false
        importConfirmDlg.open()
    }

    // ---- reusable password entry dialog ----

    property string pwEntryPurpose: "signin"
    property string pwEntryIntro: ""

    function openPwEntry(intro) {
        pwEntryIntro = intro
        pwEntryInput.text = ""
        pwEntryError.text = ""
        pwEntryDialog.open()
        pwEntryInput.forceActiveFocus()
    }

    Dialog {
        id: pwEntryDialog
        title: {
            if (pwEntryPurpose === "import")
                return "Password-Protected Profile"
            if (pwEntryPurpose === "removeenc")
                return "Remove Encryption"
            return "Sign In"
        }
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
                    onClicked: {
                        // if tox is "pending" a password (enabled but not
                        // running), cancel should back out of the sign-in.
                        if (pwEntryPurpose === "signin" && core.tox_enabled && core.tox_needs_password())
                            core.disable_tox()
                        pwEntryDialog.close()
                    }
                }

                Button {
                    id: pwEntryOkButton
                    text: {
                        if (pwEntryPurpose === "import")
                            return "Next"
                        if (pwEntryPurpose === "removeenc")
                            return "Remove Encryption"
                        return "Sign In"
                    }
                    enabled: pwEntryInput.text.length > 0
                    onClicked: {
                        pwEntryError.text = ""
                        if (pwEntryPurpose === "signin") {
                            doSignIn(pwEntryInput.text)
                            if (pwEntryError.text.length === 0)
                                pwEntryDialog.close()
                        } else if (pwEntryPurpose === "import") {
                            pwEntryDialog.close()
                            importPw = pwEntryInput.text
                            openImportConfirm()
                        } else if (pwEntryPurpose === "removeenc") {
                            var err = core.tox_set_save_password(pwEntryInput.text, "")
                            if (err.length === 0) {
                                pwEntryDialog.close()
                                resultText.text = "Encryption removed. The profile is no longer password protected."
                                resultDialog.open()
                                doRefresh()
                            } else {
                                pwEntryError.text = "Could not remove encryption: " + err
                                pwEntryInput.text = ""
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- add encryption (set new password) dialog ----

    Dialog {
        id: setPwDialog
        title: "Add Encryption"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "Choose a password. From now on you must enter it to sign in to Tox."
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
                    text: "Add Encryption"
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
                            resultText.text = "Encryption added. You will need the password to sign in."
                            resultDialog.open()
                            doRefresh()
                        } else {
                            setPwError.text = "Could not add encryption: " + err
                        }
                    }
                }
            }
        }
    }

    // ---- shared identity confirmation ----

    Dialog {
        id: selectSharedDlg
        title: "Use This Identity?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: sharedSelRow
                      ? "Load the shared identity " + shortPub(sharedSelRow.pubkey) + "?"
                      : ""
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "This replaces the Tox profile currently on this device (a backup is kept). Tox stays signed out until you press Sign In -- signing in is what claims the identity and turns it off on whichever device is currently using it."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: selectSharedDlg.close()
                }

                Button {
                    text: "Load"
                    onClicked: doSelectShared()
                }
            }
        }
    }

    // ---- stop sharing confirmation ----

    Dialog {
        id: unshareConfirmDlg
        title: "Stop Sharing?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: sharedSelRow
                      ? "Remove the shared identity " + shortPub(sharedSelRow.pubkey)
                        + " from your group's list?"
                      : ""
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "This removes it for everyone in the group. Nobody is signed out and no one's tox settings change."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                visible: sharedSelRow !== null && sharedSelRow !== null
                         && (sharedSelRow.held_by_me || sharedSelRow.holder.length > 0)
                text: sharedSelRow && sharedSelRow.held_by_me
                      ? "This identity is in use on this device. It stays signed in here, and it can be published again at any time."
                      : "This identity is currently in use on " + (sharedSelRow ? shortPub(sharedSelRow.holder) : "") + ". That device keeps running it and can publish it again at any time."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#a00"
            }

            RowLayout {
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: unshareConfirmDlg.close()
                }

                Button {
                    text: "Stop Sharing"
                    onClicked: doUnshare()
                }
            }
        }
    }

    // ---- import confirmation ----

    Dialog {
        id: importConfirmDlg
        title: "Import Tox Profile"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "Importing this profile will replace your current Tox identity and friend list. Message history is not stored in Tox profiles and will not be imported."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            CheckBox {
                id: importNoBackup
                text: "Don't save a backup of the current profile"
                checked: false
            }

            Label {
                id: importError
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
                    onClicked: importConfirmDlg.close()
                }

                Button {
                    text: "Import"
                    onClicked: doImport(importPath, importPw)
                }
            }
        }
    }

    // ---- create new identity confirmation ----

    Dialog {
        id: createNewConfirmDlg
        title: "Create New Identity"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "Your current Tox profile will be replaced with a brand new identity. The old profile (including your Tox ID and friend list) is backed up to a file named replaced_tox_save.tox."
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
                    text: "Create New Identity"
                    onClicked: doCreateNew()
                }
            }
        }
    }

    // ---- reset to factory confirmation ----

    Dialog {
        id: resetConfirmDlg
        title: "Reset to Factory"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.NoButton

        ColumnLayout {
            spacing: mm(1)
            width: parent.width

            Label {
                text: "This will delete your current Tox save. Your Tox ID and friend list will be gone. A copy of the current profile is saved to disk first."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "Tox will return to the state it was in before you first used it."
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
                    onClicked: resetConfirmDlg.close()
                }

                Button {
                    text: "Reset to Factory"
                    background: Rectangle {
                        color: "#c00"
                        radius: mm(1)
                    }
                    contentItem: Label {
                        text: "Reset to Factory"
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: doReset()
                }
            }
        }
    }

    // ---- generic result dialog ----

    Dialog {
        id: resultDialog
        title: "Tox Account"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok

        Label {
            id: resultText
            text: ""
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}