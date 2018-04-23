Fri Oct 21 09:37:12 MST 2011
WARNING

You will need the latest cdc32 source code to compile this.
The xcode project has header includes that point to the
cdc32 source, like this:
/Users/dwight/depot/dwycore/bld/cdc32/winemu
/Users/dwight/depot/dwycore/bld/cdc32
/Users/dwight/depot/dwycore/bld/dwcls

The reason for this is that these drivers need access to some
of the internal audio and video capture interfaces that aren't 
normally exported. This is part of the internal core, rather than
a separate module. This could use some improvement, but for now, it
is ok.

PRECOMPILED
libmacdrv.a
Compiled with xcode 7.2 on OSX 10.11 against libc++ and SDK for 10.11
The reason for this is that the API's this driver uses have been
deprecated as of 2018 and xcode9 will not compile them anymore.
The driver still works ok at runtime though, and linking it
on xcode9 seems to be ok.

The entrypoint for the driver is:
void init_mac_drivers();

That will install the drivers for video capture, audio capture and
audio playback. If you need to use a different driver for one of these
functions, you can call set_external_*_callbacks after the call to
init_mac_drivers to install your own driver.




