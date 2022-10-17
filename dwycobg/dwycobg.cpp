#include "dlli.h"
#include <stdlib.h>
#include <string.h>

// small background app for desktop, can either be invoked to
// finish sending queued messages then exit, or as a
// background "sync" process to act like a server to
// store all the messages for a given linked-device group.
//
// if invoked like this:
// dwycobg <port>
// the background process just runs sending and receiving messages
// in the background, performing sync services if the uid is in a group,
// and attempting to perform desktop notifications if new messages arrive.
//
// dwycobg <port> exit-outq-empty
// this is identical to the above mode, except, as soon as the output
// message queue is empty, the process exits.
//
// if invoked like this:
// dwycobg <port> <group-name> <group-pw>
// the process enters "group" mode. this mode is intended to allow the
// manipulation of the group state of the uid.
//
// if "group-name" is "", the background process LEAVES the existing group, then exits.
//
// if "group-name" is >0 length, and the process is NOT in a group, it tries
// to enter the group using the provided group-pw (which must have length > 0).
// the process will hang around waiting to complete the group-enter protocol
// with some other client that has the proper key. if the protocol completes
// successfully, the process exits. on next run, the process will be in
// eager sync mode, finding and downloading messages from other clients
// in the group, while providing sync services. NOTE: the process also
// will answer queries for other members trying to enter the group, so the
// group-pw MUST be non-zero length. this is the password that other clients
// must provide in order to enter the group. note: it is possible to give a
// bogus password as "group-pw" AFTER it has entered the group, in which
// case, this process will never dole out the group private key info.
//
// if the "group-name" provided doesn't match the group name currently used,
// the process exits without any processing.
// to change groups, you must first LEAVE the current group, then attempt to
// enter the new group.
//
// while the the daemon is running, it processes messages normally whether it
// is in the group or not.
// this process *must* be run in the working directory where the messages
// are stored (for desktop, this probably ought to be fixed and turned into
// more of a daemon rather than a side-show.)
//

int
main(int argc, char **argv)
{
    int port = 0;
    if(argc < 2)
        return 1;

    port = atoi(argv[1]);
    if(port < 1024 || port > 65535)
        return 1;

    if(argc == 2)
    {
        dwyco_background_processing(port, 0, "./", "./", "./tmp/", 0);
    }
    else if(argc == 3 && strcmp(argv[2], "exit-outq-empty") == 0)
    {
        dwyco_background_processing(port, 1, 0, 0, 0, 0);
    }
    else if(argc == 4)
    {
        dwyco_background_sync(port, 0, 0, 0, 0, argv[2], argv[3]);
    }

    return 0;
}
