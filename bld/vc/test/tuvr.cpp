
#include "vcuvsock.h"
#include "vccrypt2.h"
#include <signal.h>

class DwAllocator;
DwAllocator *Default_alloc;
int Continue_on_alarm;

void
oopanic(const char *s)
{
    ::abort();
}

void
oopanic(char *s)
{
    ::abort();
}

int
main(int, char **)
{
    signal(SIGPIPE, SIG_IGN);
    DwVP::init_dvp();
    vc_uvsocket::init_uvs_loop();

    vc n(VC_UVSOCKET_STREAM); 
#if 1
    n.socket_init("any:9000", 1, 1);
#else
    n.socket_init("any:9000", 1, 1, 1);
#endif
    //n.socket_connect("127.0.0.1:9000" );
    srand(1);
	int i = 0;
    int do_send = 0;
	int state = 0;
	vc chk;
    while(1)
    {
        vc_uvsocket::run_loop_once();
        int avail;
        vc dummy;
		do
		{
            vc v = n.socket_get_obj(avail, dummy);
            if(avail)
            {
                if(v[0] == vc("listen-accept"))
                {
					if(v[1].is_nil())
					{
						v.print_top(VcError);
					}
					else
					{
						v.print_top(VcError);
						n = v[2];
					}
                }
                else if(v[0] == vc("data"))
                {
#if 1
                    chk = vclh_md5(v[2][0]);
                    if(chk != v[2][1])
                    {
                        fprintf(stderr, "chksum err\n");
                    }
                    printf("recv %d\n", v[2][0].len());
#else
					if(state == 0)
					{
						chk = vclh_md5(v[2]);
					}
					else
					{
						if(chk != v[2])
						{
							printf("chksum err\n");
							chk.print_top(VcOutput);
							printf("\n");
							//v[2].print_top(VcOutput);
						}
					}
					state = !state;
                    printf("recv %d\n", v[2].len());
#endif
                }
                else if(v[0] == vc("eof"))
                {
                    v.print_top(VcError);
                    exit(0);
                }
                else
				{
                    v.print_top(VcError);
					if(v[0] == vc("error"))
						n.socket_close(0);
				}
            }
		}
		while(avail);
    }



}

