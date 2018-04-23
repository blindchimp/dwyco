
#include "vcuvsock.h"
#include "vccrypt2.h"
#include "libuv/uv.h"
#include <signal.h>

class DwAllocator;
DwAllocator *Default_alloc;
int Continue_on_alarm;
int Sz;

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

int do_send;
vc N2;

static
void
idle_cb(uv_idle_t *, int)
{
    if(do_send)
    {
        for(int k = 0; k < 1; ++k)
        {
        FILE *f = fopen("/dev/urandom", "rb");
        int len = rand() % Sz;
        char *buf = new char[len];
        fread(buf, len, 1, f);
        fclose(f);
#if 1
        vc v(VC_VECTOR);
        v[0] = vc(VC_BSTRING, buf, len);
        v[1] = vclh_md5(v[0]);
        vc dummy;
        if(N2.socket_put_obj(v, dummy, 0).is_nil())
        {
            fprintf(stderr, "put fail\n");
            exit(1);
        }
#else
        vc v(VC_VECTOR);
        v[0] = vc(VC_BSTRING, buf, len);
		delete [] buf;
        v[1] = vclh_md5(v[0]);
        vc dummy;
        if(N2.socket_put_obj(v[0], dummy, 1).is_nil())
        {
            fprintf(stderr, "put fail\n");
            exit(1);
        }
        if(N2.socket_put_obj(v[1], dummy, 1).is_nil())
        {
            fprintf(stderr, "put fail\n");
            exit(1);
        }
#endif
        printf("put %d\n", len);
        }

    }
}

int
main(int argc, char **argv)
{
	if(argc > 1)
		Sz = atoi(argv[1]);
	else
		Sz = 1000000;
    signal(SIGPIPE, SIG_IGN);
    DwVP::init_dvp();
    vc_uvsocket::init_uvs_loop();

    vc n(VC_UVSOCKET_STREAM); 
    n.socket_init("any:any", 0, 0);
    n.socket_connect("127.0.0.1:9000" );
    N2 = n;
    srand(1);
	int i = 0;
    //int do_send = 0;
    uv_idle_t *idle = new uv_idle_t;
    uv_idle_init(vc_uvsocket::uvs_loop, idle);
    uv_idle_start(idle, idle_cb);

    for(int i = 0; i < 1000; ++i)
    {
        vc_uvsocket::run_loop_once();
        int avail;
        vc dummy;
		do
		{
            vc v = n.socket_get_obj(avail, dummy);
            if(avail)
            {
                if(v[0] == vc("connect"))
				{
					if(v[1].is_nil())
					{
						v.print_top(VcError);
						exit(1);
					}
					v.print_top(VcError);
                    do_send = 1;
				}
                else if(v[0] == vc("write"))
                {
                    v.print_top(VcError);
                    exit(1);
                }
                else

                    v.print_top(VcError);
            }
		}
		while(avail);
#if 0
        if(do_send)
        {
            for(int k = 0; k < 20; ++k)
            {
            FILE *f = fopen("/dev/urandom", "rb");
            int len = rand() % 1000000;
            char buf[1000000];
            fread(buf, len, 1, f);
            fclose(f);
            vc v(VC_VECTOR);
            v[0] = vc(VC_BSTRING, buf, len);
            v[1] = vclh_md5(v[0]);
            if(n.socket_put_obj(v, dummy).is_nil())
            {
                fprintf(stderr, "put fail\n");
                exit(1);
            }
            printf("put %d\n", len);
            }

        }
#endif
    }



}

