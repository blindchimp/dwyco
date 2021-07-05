/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "libgen.h"
#include "dlli.h"
#include "dwyco_new_msg.h"
#include <QByteArray>

static
void
DWYCOCALLCONV
dwyco_db_login_result(const char *str, int what)
{

    if(what != 0)
        dwyco_switch_to_chat_server(0);
    else
        exit(1);
}

struct simple_scoped
{
    DWYCO_LIST value;
    simple_scoped(DWYCO_LIST v) {value = v;}
    ~simple_scoped() {dwyco_list_release(value);}
    operator DWYCO_LIST() {return value;}
};

int
main(int argc, char *argv[])
{
    if(access("stop", F_OK) == 0)
        exit(0);
    if(argc < 4)
        exit(1);
    signal(SIGPIPE, SIG_IGN);
	alarm(3600);
    int fd = open("/dev/urandom", O_RDONLY);
    unsigned int foo;
    read(fd, (void *)&foo, sizeof(foo));
    close(fd);

    srand(foo);

    const char *name = argv[1];
    const char *desc = argv[2];
    char *cmd = strdup(argv[0]);
    cmd = basename(cmd);

    dwyco_set_login_result_callback(dwyco_db_login_result);

    dwyco_init();

    dwyco_set_setting("net/listen", "0");
    dwyco_inhibit_sac(1);

    if(dwyco_get_create_new_account())
    {
        dwyco_create_bootstrap_profile(name, strlen(name), desc, strlen(desc), "", 0, "", 0);

    }


    dwyco_set_local_auth(1);
    dwyco_finish_startup();
	int i = 0;
	int r = 20 * 60 + (rand() % 30) * 60;
    time_t start = time(0);


    int was_online = 0;
    unlink("stopped");

    while(1)
    {
        int spin;
        dwyco_service_channels(&spin);
        usleep(100 * 1000);
        ++i;
        if(time(0) - start >= r || access("stop", F_OK) == 0)
        {
            dwyco_power_clean_safe();
            dwyco_empty_trash();
            dwyco_exit();
            creat("stopped", 0666);
            exit(0);
        }
        if(dwyco_chat_online() == 0)
        {
            if(was_online)
                exit(0);
            else
                continue;
        }

        was_online = 1;

        if(dwyco_get_rescan_messages())
        {
            dwyco_set_rescan_messages(0);
            process_remote_msgs();
        }

        QByteArray uid;
        QByteArray txt;
        int dummy;
        QByteArray mid;
        int has_att;

        if(dwyco_new_msg(uid, txt, dummy, mid, has_att))
        {
            processed_msg(mid);
            dwyco_delete_saved_message(uid.constData(), uid.length(), mid.constData());

        }

    }

}
