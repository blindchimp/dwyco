
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// Non-visual undo stack, instantiated once in main.qml as "undo_hub".
//
// Every reversible operation in the app funnels through ops.qml, which pushes
// a record here. The banner (UndoBar.qml) is only the discoverable affordance
// -- Ctrl+Z keeps working after the banner has auto-dismissed, which means a
// destructive mistake is never lost just because you looked away.

import QtQuick

Item {
    id: hub
    visible: false
    enabled: false

    // deep enough that a burst of bulk operations is all recoverable, shallow
    // enough that we never hold onto a huge pile of closures
    property int max_depth: 25
    property var entries: []
    readonly property bool can_undo: entries.length > 0
    readonly property string last_label: entries.length > 0 ? entries[entries.length - 1].label : ""

    // emitted when a record lands on the stack (drives the banner)
    signal notify(string label)
    // emitted when the stack is emptied by an undo or a clear
    signal dismissed()

    function push(label, fn) {
        if(typeof fn !== "function")
            return
        // copy rather than push in place, otherwise the property assignment
        // below sees the same array and no one is notified
        var e = entries.slice(0)
        e.push({label: label, fn: fn})
        while(e.length > max_depth)
            e.shift()
        entries = e
        notify(label)
    }

    function undo() {
        if(entries.length === 0)
            return
        var e = entries.slice(0)
        var rec = e.pop()
        entries = e
        try {
            rec.fn()
        } catch(err) {
            console.warn("undo of '" + rec.label + "' failed: " + err)
        }
        dismissed()
    }

    function clear() {
        if(entries.length === 0)
            return
        entries = []
        dismissed()
    }
}
