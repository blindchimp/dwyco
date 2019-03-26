
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick 2.6
Item {
    property var adj : [
        "exciting",
        "cuddly",
        "fiddly",
        "obedient",
        "wrathful",
        "happy",
        "melted",
        "lovely",
        "flaky",
        "makeshift",
        "first",
        "perfect",
        "difficult",
        "crazy",
        "smooth",
        "soupy",
        "shiny",
        "tangy",
        "special",
        "roasted",
        "eminent",
        "curt",
        "forgetful",
        "wealthy",
        "curved",
        "matte",
        "spotty",
        "sneaky",
        "creative",
        "overt",
        "chilly",
        "flashy",
        "nutritious",
        "grieving",
        "envious",
        "fresh",
        "meaty",
        "scarce",
        "tangible",
        "interesting",
        "wistful",
        "permissible",
        "impossible",
        "inexpensive",
        "brash",
        "hysterical",
        "brief",
        "best",
        "misty"
    ]
    property var noun : [
        "ear",
        "twig",
        "vessel",
        "rock",
        "boat",
        "knot",
        "gate",
        "bone",
        "observation",
        "desire",
        "activity",
        "group",
        "weight",
        "back",
        "iron",
        "mountain",
        "tin",
        "bells",
        "bed",
        "arithmetic",
        "force",
        "creature",
        "mailbox",
        "dog",
        "reading",
        "current",
        "oranges",
        "robin",
        "transport",
        "peace",
        "drain",
        "furniture",
        "thunder",
        "front",
        "coffee",
        "purpose",
        "aardvark",
        "friend",
        "holiday",
        "surprise",
        "plot",
        "fever",
        "level",
        "kitten",
        "fireman",
        "locket",
        "smoke",
        "potion"
    ]

    function cap(str) {
        return str.charAt(0).toUpperCase() + str.slice(1)
    }

    function rnoun() {
        var w = noun[Math.floor(Math.random() * (noun.length - 1))]
        return cap(w)
    }
    function radj() {
        var w = adj[Math.floor(Math.random() * (adj.length - 1))]
        return cap(w)
    }

    function fname() {
        return radj() + radj() + rnoun()

    }
}
