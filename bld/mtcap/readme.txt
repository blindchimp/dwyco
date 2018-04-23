08/26/2016 

notes on using this dll from a program compiled with MS tools.
in the past, this lib was delay-linked into cdcdll8.dll using the
borland tools, so there wasn't much to do except compile cdcdll8.dll
using the normal borland ide.

i want to eliminate the borland dependency, but i don't want to
redo this video stuff for the umpteenth time, since it works
passably well using the third-party stuff i have now.

this dll is built using borland rad studio xe, and it incorporates some
third party components (basic video, from mitov.com). it is statically
linked with the borland rtl. IT MUST BE DELAY LOADED INTO whatever
program is using it, since it gets weird init crashes on some computers
unless you do this (i don't know why.) 

creating a lib file for the MS linker involves running the "lib" command
from the MS dev tools you are using:

lib/def:mtcapxe.def

and using the resulting mtcapxe.lib file. the def file was modified by
hand to remove "_", since that seems to be what the MS linker wants.
the interface must be declared extern "C" and you must not do anything
to the memory (except copy its contents) returned from the dll, since it
is using a different runtime.

NOTE: the vgexp.h file i changed to extern "C", which will probably end up
producing a dll that WILL NOT LINK with old versions of cdcdll8.dll. but
that is ok, since we aren't going to modify cdcdll8.dll/mtcapxe.dll pair
after switching to MS. note that mtcapxe.dll functionality is completely
unchanged in either case.

i would have included the borland impdef.exe and implib.exe tools just in
case they are needed to interpret the borland-produced dll's again. but they
are obviously not redistributable.
