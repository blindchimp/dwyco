#include "dlli.h"
#include <stdlib.h>

// small background app for desktop, can either be invoked to
// finish sending queued messages then exit, or as a
// background "sync" process to act like a server to
// store all the messages for a given linked-device group.
// note: in sync-mode, you must set two environment variables
// DWYCO_GROUP, the name of the group (to join if not in one already.)
// DWYCO_GROUP_PW, the shared-key for the group.
// note that when the group change takes effect, the process will exit.
// on next restart, you will be in the group and sync normally in eager mode.
// if DWYCO_GROUP = "", then it leaves the current group and exits.
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
        dwyco_background_processing(port, 1, 0, 0, 0, 0);
    }
    else if(argc == 3)
    {
        dwyco_background_sync(port, 0, 0, 0, 0);
    }

    return 0;
}
