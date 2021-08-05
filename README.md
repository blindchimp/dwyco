
Dwyco Video Chat Client Components

CDC-X (stable)

Linux/Mac: [![Build Status](https://travis-ci.org/blindchimp/dwyco.svg?branch=cdcx-stable-travis)](https://travis-ci.org/blindchimp/dwyco)


This repo contains the source and build instructions for:

CDC-X: a video instant messenger using Qt5 (desktop only.)

Phoo: a video instant messenger using Qt5 (desktop and mobile.)

Rando: an anonymous picture exchange app (mobile, android only.)

Selfs: A quick video streamer app not intended for external use (desktop+android).

LH: Lingua Hacka interpreter used for the backend servers. 

LHC: Lingua Hacka compiler, compiles LH to C++ (only partially done.)

Misc: an assortment of helpers and bots used by the clients and servers.

Each component needs a slightly different setup in order to compile it.
You'll find details for building "cdcx" in "cdcxbld", "phoo" in "phoobld", etc.


Deployment status as of 2021:

* CDC-X has been deployed since the 2012, as a replacement for the long-running
CDC32.

* Rando is deployed in the Google play store as an all-ages app.

* Phoo is deployed in alpha for Android on the Google play store.

* Selfs is deployed in google play internal testing track, but I'll be a monkey's uncle if I can install it via Play Store. It is deployed currently in a local F-Droid repo instead.

* LH is only deployed internally at Dwyco

