Thu Feb 25 12:11:34 MST 2016
dwight melcher

this is a tiny swig interface file to allow android to run a small
message server as an android service, invoked from java.

swig -java -package com.dwyco.phoo -o dwybg_wrap.c dwybg.i

note: the package should match whatever the eventual package you
are going to be including the java file into... in the case of Qt, there
is a src/com/dwyco/phoo directory with java files used to implement the
java service.

NOTE NOTE!
This will not work if you statically link the dwyco libraries into the
qt static lib. this is some kind of limitation on the java jni stuff.
statically linking is useful for debugging sometimes, but it will silently
ignore calls trying to get in via java.

i *might* be able to fix this by dynamically linking the tiny wrapper
library, and loading that, and keeping the rest statically linked,
but it is probably too much trouble.  the best way is to just create a dynamic dwyco lib.
