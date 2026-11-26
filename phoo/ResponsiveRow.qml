/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
import QtQuick.Layouts

// a row of controls that wraps onto more lines when there is not enough
// width, instead of clipping or forcing a horizontal scroll. used by the
// tox pages so one layout works on a phone and a resized desktop window.
Flow {
    id: root

    Layout.fillWidth: true
    spacing: mm(1)
}
