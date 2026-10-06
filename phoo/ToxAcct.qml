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
    property string selectedKey: ""
    property bool selectedEncrypted: false
    // the row a confirmation dialog is about. the list can refresh while a
    // dialog is open, so the dialog must not read a live list index.
    property var pendingRow: null

    function doRefresh() {
        tox_state.refresh()
        refreshAllSaves()
    }

    function refreshAllSaves() {
        allSavesList = core.tox_list_all_saves()
        // keep the current selection across a refresh if that row still exists,
        // otherwise fall back to the first row.
        if(selectedKey !== "" && indexOfKey(selectedKey) < 0)
            selectedKey = ""
        if(selectedKey === "" && allSavesList.length > 0)
            selectedKey = allSavesList[0].key
        syncSelected()
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

    function selectedRow() {
        var i = indexOfKey(selectedKey)
        return i < 0 ? null : allSavesList[i]
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

    // what picking this row will cost you. selecting anything other than the
    // local save overwrites the identity currently on disk, which used to be
    // completely invisible.
    function replaceWarning(row) {
        if(!row || row.local)
            return ""
        var cur = allSavesList.length > 0 ? allSavesList[0] : null
        if(!cur)
            return ""
        var who = nameOf(cur)
        if(who.length === 0 && cur.pubkey && cur.pubkey.length > 0)
            who = shortPub(cur.pubkey)
        else if(who.length === 0)
            who = "the save on this device"
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

    function doSignIn() {
        var row = selectedRow()
        if(!row) {
            showBanner("Pick a Tox save first.", true)
            return
        }
        // picking a shared identity overwrites the save on disk, so the
        // remembered name/address describe the identity being replaced. drop
        // them up front; if the sign in then fails the next refresh re-caches
        // nothing and the boxes correctly show blank.
        if(!row.local)
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
        showBanner("Imported. Select it in the list and sign in to use it.")
        doRefresh()
    }

    function doCreateNew() {
        createNewConfirmDlg.close()
        var ok = core.tox_reset_identity()
        if (ok) {
            // same as above: the remembered name/address belong to the old
            // identity, which reset backed up and replaced.
            tox_state.forgetCachedIdentity()
            showBanner("Created a new Tox save. Select it in the list and sign in to use it.")
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

            // ---- 1. sign in ----

            Label {
                text: "Sign In"
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

                    // unified list of all tox saves
                    ListView {
                        id: savesListView
                        model: allSavesList
                        visible: allSavesList.length > 0
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(savesListView.contentHeight, mm(80))
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        ScrollBar.vertical: ScrollBar { }

                        delegate: Item {
                            id: saveRow
                            width: ListView.view.width
                            height: saveRowCol.implicitHeight + mm(2)

                            Rectangle {
                                anchors.fill: parent
                                radius: mm(1)
                                color: modelData.key === selectedKey ? "#e8f4e8" : "transparent"
                                border.color: modelData.key === selectedKey ? "#4a4" : "#dedede"
                                border.width: 1
                            }

                            ColumnLayout {
                                id: saveRowCol
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.margins: mm(1)
                                spacing: mm(0.5)

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: mm(1)

                                    RadioButton {
                                        checked: modelData.key === selectedKey
                                        onClicked: {
                                            selectedKey = modelData.key
                                            syncSelected()
                                            pwInput.text = ""
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 0

                                        Text {
                                            Layout.fillWidth: true
                                            text: rowTitle(modelData)
                                            font.bold: true
                                            font.pixelSize: dp(13)
                                            // monospace only when we are showing
                                            // a raw pubkey, since that is the one
                                            // case where character shape helps.
                                            font.family: (!nameOf(modelData).length
                                                          && modelData.pubkey
                                                          && modelData.pubkey.length > 0)
                                                         ? "monospace" : "sans-serif"
                                            elide: Text.ElideRight
                                        }

                                        Text {
                                            Layout.fillWidth: true
                                            text: modelData.pubkey && modelData.pubkey.length > 0
                                                  ? shortPub(modelData.pubkey)
                                                  : ""
                                            font.family: "monospace"
                                            font.pixelSize: dp(10)
                                            color: "#666"
                                            visible: text.length > 0
                                            elide: Text.ElideRight
                                        }
                                    }

                                    Text {
                                        text: modelData.local ? "This device" : ""
                                        font.pixelSize: dp(10)
                                        font.bold: true
                                        color: accent
                                    }

                                    // unpublish. only on shared rows: the local
                                    // save isn't published, so there is nothing
                                    // to remove from the group list.
                                    ToolButton {
                                        visible: !modelData.local
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
                                        onClicked: openUnshareConfirm(modelData)
                                    }
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: rowSubLabel(modelData)
                                    font.pixelSize: dp(10)
                                    color: "#666"
                                    elide: Text.ElideRight
                                }

                                // say out loud what picking a shared identity
                                // costs, before the user commits to it.
                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.key === selectedKey
                                          ? replaceWarning(modelData) : ""
                                    font.pixelSize: dp(10)
                                    font.italic: true
                                    color: "#a06000"
                                    visible: text.length > 0
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }

                    Label {
                        visible: allSavesList.length === 0
                        text: "No Tox saves available. Import one or create a new one below."
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        color: "#666"
                        font.pixelSize: dp(11)
                    }

                    // password field, only when the selected save is actually encrypted
                    RowLayout {
                        visible: selectedEncrypted
                        Layout.fillWidth: true
                        spacing: mm(1)

                        TextField {
                            id: pwInput
                            echoMode: TextInput.Password
                            placeholderText: "Password"
                            Layout.fillWidth: true
                            onAccepted: doSignIn()
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: mm(1)

                        Button {
                            // a swap is not a sign in, it is an overwrite plus
                            // a sign in. say so before the click.
                            text: {
                                if(tox_state.state === "signedin")
                                    return "Sign out"
                                var r = selectedRow()
                                if(r && !r.local)
                                    return "Replace and sign in"
                                return "Sign in"
                            }
                            Layout.fillWidth: true
                            onClicked: {
                                if (tox_state.state === "signedin")
                                    signOut()
                                else
                                    doSignIn()
                            }
                        }
                    }
                }
            }

            // ---- 2. add / change save ----

            Label {
                text: "Add / Change Save"
                font.bold: true
                Layout.topMargin: mm(2)
            }

            ColumnLayout {
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

                Rectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: mm(1)
                    implicitHeight: mm(0.25)
                    color: "#ccc"
                }

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
                    text: "Deletes the Tox save on this device only. Your messages, "
                          + "contacts and Dwyco account are not affected. A copy is "
                          + "saved to disk first. This cannot be undone."
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    color: "#666"
                    font.pixelSize: dp(11)
                }
            }

            // ---- 3. share with devices ----

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: mm(2)
                Layout.bottomMargin: mm(0.5)
                spacing: mm(1)

                Label {
                    text: "Share With My Devices"
                    font.bold: true
                    Layout.fillWidth: true
                }

                Button {
                    text: "Share this save"
                    enabled: tox_state.state === "signedin"
                    onClicked: {
                        if (core.tox_publish_save())
                            showBanner("Shared with your other devices.")
                        else
                            showBanner("Could not share. Sign in to your Tox save first.", true)
                    }
                }
            }

            Label {
                text: "Share your Tox save with your other devices. Anyone in your "
                      + "group can use one, but only one device at a time."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#666"
                font.pixelSize: dp(11)
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
                text: "You will be signed out. The imported save appears in the "
                      + "list above, and you sign in to it when you are ready."
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
                text: "You will be signed out. The new save appears in the list "
                      + "above, and you sign in to it when you are ready."
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
