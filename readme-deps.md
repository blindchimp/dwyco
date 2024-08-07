Tue Jun  4 09:19:30 MST 2019
Notes on Ubuntu 18.04

You may need to install qt4 in order to get all the libGL stuff setup.
Probably easier way of doing it, but this will work:
sudo apt intall libqt4-dev

also may need:

sudo apt install libgl1-mesa-dev

if you get link errors for -lGL. Note that you may be able to avoid
some of the installation by providing the right symlink target for
libGL.so, but I didn't try this.

for v4l, use this

sudo apt install libv4l-dev
or
sudo apt install libv4l-0

WARNING: you will need to set the permissions on /dev/videoX, or put
your user into the "video" group on some systems.


for android codec builds, sadly, you'll need autoconf:

sudo apt install autoconf libtool


Thu Apr 18 09:37:39 PDT 2019

You'll need:

on the mac:
	homebrew
	ccache (brew install ccache)

NOTES: on a fresh mac, do yourself a favor and COMPLETELY INSTALL
xcode FIRST. then start xcode, and build a small test project to make sure
it is all set up with all the licenses accepted and developer account
signed in and all that monkey business.
I had problems with homebrew installing command
line tools and leaving things in an odd state such that qt-creator
couldn't find the iOS compiler. If you run xcodebuild or 
xcode-select from the command-line
and it gives you weird error messages, that could be your problem.

sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

seems to solve a lot of these problems.

THEN install homebrew, and lastly install Qt5 (5.12 as of this writing).

android:

brew install automake
brew install pkg-config

(This is for the android build of some of the old xiph codecs, so
get the hand-coded assembly performance improvements.)

The current sdkmanager from google doesn't work with jdk12.
Oracle is no longer providing previous versions of jdk.

I had to search for OpenJDK with a version of 8 and install that.
NOTE! at time of writing, using homebrew standard java8 install
failed mainly because it seems it was trying to access jdk files
at oracle that are missing. this worked a few days ago, but not anymore:
---- FAILS ---
brew tap caskroom/versions
brew cask install java8
---- END FAIL ----

OpenJDK like this seems to still work ok.

from https://dzone.com/articles/install-openjdk-versions-on-the-mac

brew tap AdoptOpenJDK/openjdk 
brew install adoptopenjdk/openjdk/adoptopenjdk-openjdk8 

If you are using qt-creator for android development, there is no
need to download the entire 1GB ide from google. Just download the
sdk tools (command line). Unpack into ~/android (results in
~/android/tools) and tell qt-creator to use sdk in ~/android.
It should pick up the sdkmanager and provided java8 is working,
should be able to download enough stuff to do android builds.

Get the latest NDK from google, and unpack it into ~/android
(results in ~/android/android-ndk-...) and point qt-creator at it.
NOTE: I have noticed in some circumstances the sdk takes over
management of the ndk, and performs updates to it automatically.
I'm not sure how this works, it just seemed to happen (moved the
NDK under the sdk directory automatically.) This is more
convenient for sure, but seems a bit mysterious.
