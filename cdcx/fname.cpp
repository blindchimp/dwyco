
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QByteArray>
#include <stdlib.h>


static const char *adj[] = {
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
};
#define NADJ (sizeof(adj) / sizeof(adj[0]))
static const char *noun[] = {
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
};
#define NNOUN (sizeof(noun) / sizeof(noun[0]))

static
QByteArray
cap(QByteArray str)
{
    return QByteArray(1, str[0]).toUpper() + str.mid(1);
}

static
QByteArray
rnoun()
{
    return cap(noun[rand() % NNOUN]);
}
static
QByteArray
radj()
{
    return cap(adj[rand() % NADJ]);
}

QByteArray
fname()
{
    return radj() + rnoun() + QByteArray::number(rand() % 1000);
}


#ifdef TEST_FNAME
#include <stdio.h>
#include <QCoreApplication>
main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);
    srand(time(0));
    for(int i = 0; i < 100; ++i)
        printf("%s\n", fname().constData());
}

#endif
