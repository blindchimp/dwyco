#!/bin/sh
if [ "$TRAVIS_OS_NAME" = "osx" ]
then
	./cibuild-mac.sh
else
	./cibuild.sh
fi
