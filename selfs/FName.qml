
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
import QtQuick
Item {
    property var adj : [
        "exciting",
        "cuddly",
        "fiddly",
        "happy",
        "melted",
        "lovely",
        "first",
        "perfect",
        "crazy",
        "smooth",
        "soupy",
        "shiny",
        "tangy",
        "special",
        "roasted",
        "eminent",
        "curt",
        "wealthy",
        "curved",
        "matte",
        "spotty",
        "sneaky",
        "overt",
        "chilly",
        "flashy",
        "fresh",
        "meaty",
        "brash",
        "brief",
        "best",
        "misty",

	"tall",
	"huge",
	"nice",
	"cute",
	"hot",
	"rare",
	"strong",
	"pure",
	"friendly"
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
        "desire",
        "group",
        "back",
        "iron",
        "tin",
        "bells",
        "bed",
        "force",
        "mailbox",
        "dog",
        "oranges",
        "robin",
        "peace",
        "drain",
        "thunder",
        "front",
        "coffee",
        "aardvark",
        "friend",
        "holiday",
        "plot",
        "fever",
        "level",
        "kitten",
        "fireman",
        "locket",
        "smoke",
        "potion",
	"lake",
	"shirt",
	"thing",
	"poet",
	"bird",
	"skill",
	"hair",
	"cheek"
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
        return radj() + rnoun() + Math.floor(Math.random() * 1000).toString()

    }
}
