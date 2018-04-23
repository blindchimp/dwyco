#!/bin/sh

if [ $TRAVIS_OS_NAME = linux ]
then
sudo apt-get -qq update
sudo apt-get install -y libqt4-dev
sudo apt-get install -y libsdl1.2-dev
sudo apt-get install -y libesd0-dev
sudo apt-get install -y libsqlite3-dev
sudo apt-get install -y libv4l-dev
else
brew tap cartr/qt4
brew tap-pin cartr/qt4
brew install qt@4
brew install ccache
brew install qt-webkit@2.3
fi
