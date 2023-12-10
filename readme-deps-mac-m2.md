Sat Jan 28 08:26:48 MST 2023

This gives some details regarding various dependencies you'll need
when setting up a fresh M2 MAC for Qt5 development for Desktop,
iOS, and Android

on the mac:
	xcode (as of this writing, xcode 14 is what is being used.)
	homebrew (from https://brew.sh/)

(1) On a fresh M2 mac, do yourself a favor and COMPLETELY INSTALL
xcode FIRST.  DO THIS BEFORE INSTALLING HOMEBREW OR QT OR ANYTHING
ELSE. Trying to build, sign, notarize, manage certificates, etc.
from a command-line only installation is likely to be frustrating.

Start xcode, and build a small test project to make sure it is all
set up with all the licenses accepted and developer account signed
in and all that monkey business.

I had problems with homebrew installing command line tools and
leaving things in an odd state such that QtCreator couldn't find
the iOS compiler.

If you run xcodebuild or xcode-select from the command-line and it
gives you weird error messages, that could be your problem.

sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

seems to solve a lot of these problems.

This page has some good information if your Qt-based builds are not
working and you need to get the right xcode selected:
https://doc.qt.io/qt-5/macos.html

(2) Install homebrew
(3) Install Qt5 (5.15 as of this writing). QtCreator is pretty much required, so just use the Qt Maintenance tool to get what you need.

DESKTOP builds
--------------

brew install ccache

(NOTE: QtCreator won't find ccache unless you put /opt/homebrew/bin
in your PATH. You can also just put /opt/homebrew/bin in the QtCreator
"build environment" if putting it in your PATH will cause problems.)

Optional but useful:

fd
pigz
ripgrep
db-browser-for-sqlite
tabby

NOTE NOTE! In order for QtCreator to access various system items
(like the Java JDK you'll be installing below, and various debugging
APIs) you'll need to grant it "full file access" AND "developer
tools" in the MacOS permissions system. I'm not sure what the minimal
set of permissions is, but builds and debugging will just randomly
fail if QtCreator can't access system things.

At this point, DESKTOP builds should work for Qt5 (though what is
produced by the builds is x86 binaries... Qt5 doesn't have native
apple silicon support built in.)

iOS builds
----------
There are some bugs in the Qt5 distribution if you are using a circa
2023 version of MacOS and XCode. You'll need to apply this small
patch to your Qt5 installation (change "python" references to
"python3", and remove a couple of lines from a settings file to
allow new-style builds.)

After applying this patch, deploying to the iOS simulator appears
to work ok.

--------------------------------
diff -r Qt/5.15.2/android/mkspecs/features/uikit/devices.py 5.15.2/android/mkspecs/features/uikit/devices.py
1c1
< #!/usr/bin/python
---
> #!/usr/bin/python3
diff -r Qt/5.15.2/clang_64/mkspecs/features/uikit/devices.py 5.15.2/clang_64/mkspecs/features/uikit/devices.py
1c1
< #!/usr/bin/python
---
> #!/usr/bin/python3
diff -r Qt/5.15.2/ios/mkspecs/features/uikit/devices.py 5.15.2/ios/mkspecs/features/uikit/devices.py
1c1
< #!/usr/bin/python
---
> #!/usr/bin/python3
diff -r Qt/5.15.2/ios/mkspecs/macx-xcode/WorkspaceSettings.xcsettings 5.15.2/ios/mkspecs/macx-xcode/WorkspaceSettings.xcsettings
5,6d4
< 	<key>BuildSystemType</key>
< 	<string>Original</string>
------------------------------------


Android builds using QtCreator
------------------------------
(1) Install the latest full Android-Studio from Google. Yes, it is
overkill, but in recent versions of QtCreator, they have removed
various android SDK and AVD manager interfaces. You can use android
studio to do these tasks (I suspect you might be able to do it all
from the command-line too, but I didn't try it.)

(1.5) SPECIAL NOTE! SAVE SOME HEADACHES!
You will save yourself a fair amount of headache if you can tell
qt-creator to use whatever java is shipped with Android studio. This
is true on both Linux and MAC. Usually the folder is something like
"android-studio/jbr". On the mac, you have to find the Application folder
and drill into it until you find the "jbr/Contents/Home" folder.  Note:
this is needed for deployment since some java stuff needs to be executed
to create android applications. Unless you need java for something
else, I think you can avoid the java installs mentioned below.


(2) Find and install a JDK.  Select a JDK to download with aarch64
architecture, version 11.  I've experimented in the past with getting
it from homebrew and other sources, but as of this writing, the
easiest is to use AdoptOpen (I think Adoptium.net now?)


(3) Start QtCreator and use the "devices" area to check for Android
setup. You'll need to tell it where the JDK is, and it will probably
download some versions of the NDK. The version is usually wrong.
Qt5.15 needs NDK 21.3xxx. If QtCreator is happy with your android
setup, but has the wrong NDK version, use android-studio SDK manager
to find and install the 21.3xxx NDK. Then return to QtCreator and
select the 21.3xxx version as the default for builds.

(4) If you are using ndk-build, you will have to edit the script distributed with the older NDK like this:

vi ~/Library/Android/sdk/ndk/21.3xxx/ndk-build

CHANGE

DIR="$(cd "$(dirname "$0")" && pwd)"
$DIR/build/ndk-build "$@"

TO

DIR="$(cd "$(dirname "$0")" && pwd)"
arch -x86_64 /bin/bash $DIR/build/ndk-build "$@"

(5) The following is Dwyco specific for building the Android shared lib for Dwyco apps

brew install automake pkg-config libtool autoconf

(This is for the android build of some of the old xiph codecs, to
get the hand-coded assembly performance improvements.)

Do the usual build process in phoobld/jni, you may have to use the NDK environment variable to tell it where the NDK is located.

Use android studio to create android emulators using aarch64.

Tell QtCreator to just build the ARM based variants, and try to deploy it to the emulator.
