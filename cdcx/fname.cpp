
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
    return radj() + radj() + rnoun();
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
