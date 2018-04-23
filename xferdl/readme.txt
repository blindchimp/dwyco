REMEMBER TO COMPILE VC LIBRARY with BUFFER_SOCKETS undefined.
that makes the default to avoid buffering, which is ok since we are
using blocking i/o on everything.

this uses the protocol from xfer to download a file. used as an
async downloader for autoupdates

