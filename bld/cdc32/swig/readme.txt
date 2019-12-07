Thu Feb 25 12:11:34 MST 2016
dwight melcher

this is a tiny swig interface file to allow android to run a small
message server as an android service, invoked from java.

swig -java -package com.dwyco.cdc32 -o dwybg_wrap.c dwybg.i

note: the resulting java file would need to be put into a
....<some java source dir>..../com/dwyco/cdc32
i think.  for qt/qml, it is androidinst/src/com/dwyco/cdc32 .

your other java files will need to import:
import com.dwyco.cdc32.dwybg;
 (or com.dwyco.cdc32.*, not sure.)

NOTE NOTE!
This will not work if you statically link the dwyco libraries into the
qt static lib. this is some kind of limitation on the java jni stuff.
statically linking is useful for debugging sometimes, but it will silently
ignore calls trying to get in via java.

i *might* be able to fix this by dynamically linking the tiny wrapper
library, and loading that, and keeping the rest statically linked,
but it is probably too much trouble.  the best way is to just create a dynamic dwyco lib.
