#undef LOCAL_TEST
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#ifdef LINUX
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#ifdef __WIN32__
#include <io.h>
#include <process.h>
#define F_OK 0
#endif
#include "vc.h"
#include "vccomp.h"
#include "vcxstrm.h"
#include "vclhsys.h"
#include "vcsock.h"
#include "dwstr.h"
#include "sha.h"
#include "vccrypt2.h"

using namespace CryptoPP;
vc vclh_from_hex(vc);

vc Filename;
vc Filesize;

int Loose_file = 1;

static int
backout(vc *)
{
    return VC_SOCKET_BACKOUT;
}

static void
dolog(const char *s)
{
    DwString out;
    char a[4096];
    char b[4096];
    struct tm *t;
    time_t tt = time(0);
    t = gmtime(&tt);
    strftime(b, sizeof(b), "%Y/%m/%d %T - ", t);
    static int fd = open("xfer.log", O_CREAT|O_APPEND|O_WRONLY, 0666);
    sprintf(a, "(%d) ", getpid());

    out = b;
    out += a;
    DwString fn;
    if(Filename.len() > 100)
        fn = DwString((const char *)Filename, 0, 100);
    else
        fn = (const char *)Filename;
    out += fn;
    out += " ";
    out += s;
    out += "\n";

    (void)write(fd, out.c_str(), out.length());
}



static int
checkfn(vc s)
{
    if(Loose_file)
    {
        const char *a = (const char *)s;
        int n = s.len();
        // eliminates . and ..
        if(n <= 2 || n > 40)
            return 0;
        // only allow a-zA-Z0-9.-_
        for(int i = 0; i < n; ++i)
        {
            if((a[i] >= 'a' && a[i] <= 'z') ||
                    (a[i] >= 'A' && a[i] <= 'Z'))
                continue;
            if(a[i] >= '0' && a[i] <= '9')
                continue;
            if(a[i] == '.' || a[i] == '-' || a[i] == '_')
                continue;
            return 0;
        }
        return 1;
    }
    if(s.len() != 44 && s.len() != 24)
        return 0;
    const char *a = (const char *)s;
    int n = s.len() - 4;
    int i;
    for(i = 0; i < n; ++i)
    {
        if(a[i] >= '0' && a[i] <= '9')
            continue;
        if(a[i] >= 'a' && a[i] <= 'f')
            continue;
        break;
    }
    if(i != n)
        return 0;
    if(a[n] == '.' || a[n+1] == 'd' || a[n+2] == 'y' || a[n+3] == 'c')
        return 1;
    if(a[n] == '.' || a[n+1] == 'f' || a[n+2] == 'l' || a[n+3] == 'e')
    {
        return 1;
    }
    return 0;
}


int
send_vc(vc sock, vc v)
{
    vcxstream vcx(sock);
    if(!vcx.open(vcxstream::WRITEABLE))
        return 0;
    vc_composite::new_dfs();
    long len;
    if((len = v.xfer_out(vcx)) < 0)
        return 0;
    vcx.close();
    return 1;
}

int
recv_vc(vc sock, vc& out)
{
    vc req;
    vcxstream vcx(sock);
    int len;

    if(!vcx.open(vcxstream::READABLE))
        return 0;
    if((len = req.xfer_in(vcx)) < 0)
        return 0;
    out = req;
    return 1;
}

void
oopanic(char *s)
{
    dolog(s);
    exit(0);
}
void
oopanic(const char *s)
{
    dolog(s);
    exit(0);
}

void
got_alarm(int)
{
    dolog("alarm");
    _exit(0);
}

#ifdef __WIN32__

#include <windows.h>
#include <tchar.h>
// this thread stuff is just a poor-mans watchdog timer so we don't end up
// with these things lingering around if they get stuck for some reason.
static
DWORD ThreadProc (LPVOID lpdwThreadParam )
{
    Sleep(20 * 60 * 1000);
    ExitProcess(1);
}

static int ourmain(int argc, char **argv);
#ifdef __BORLANDC__
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#endif
{
    LPTSTR *av;
    int num;
    char **argv = 0;

    /*******************************************************
     * WIN32 command line parser function********************************************************/	int    argc, BuffSize, i;
    WCHAR  *wcCommandLine;
    LPWSTR *argw;		// Get a WCHAR version of the parsed commande line
    wcCommandLine = GetCommandLineW();
    argw = CommandLineToArgvW( wcCommandLine, &argc); 	// Create the first dimension of the double array
    argv = (char **)GlobalAlloc( LPTR, argc + 1 );		// convert eich line of wcCommandeLine to MultiByte and place them	// to the argv[] array
    for (i = 0; i < argc; i++) {
        BuffSize = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, argw[i], -1,
                                       NULL, 0, NULL, NULL);
        argv[i] = (char *)GlobalAlloc(LPTR, BuffSize);
        WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, argw[i],
                            BuffSize * sizeof(WCHAR), argv[i], BuffSize, NULL, NULL);
    }
    return ourmain(argc, argv);

}
static int
ourmain(int argc, char **argv)

#else
int
main(int argc, char **argv)
#endif
{
    if(argc < 4)
        exit(1);
#ifdef LINUX
    signal(SIGALRM, got_alarm);
    alarm(60);
#endif
#if defined(_Windows) || defined(_WIN32)
    int i = 0;
    DWORD dwThreadId;
    CreateThread(NULL, //Choose default security
                 0, //Default stack size
                 (LPTHREAD_START_ROUTINE)ThreadProc, //Routine to execute
                 (LPVOID) &i, //Thread parameter
                 0, //Immediately run the thread
                 &dwThreadId //Thread Id
                );
#endif
    vc::non_lh_init();

    vc vcsock(VC_SOCKET_STREAM);
    if(vcsock.socket_init("any:any", 0, 1).is_nil())
        exit(1);
    if(vcsock.socket_connect(argv[1]).is_nil())
    {
        dolog("connect failed");
        exit(1);
    }
    vc req(VC_VECTOR);
    req[0] = argv[2];
    if(!checkfn(req[0]))
        exit(1);
#if defined(_Windows) || defined(_WIN32)
    // if there is anyone around even thinking of using the file, punt
    DwString mn("dwyco-autoupdate-");
    mn += argv[2];
    if(CreateMutex(NULL, TRUE, mn.c_str()) == NULL)
        exit(1);
    if(GetLastError() == ERROR_ALREADY_EXISTS)
        exit(1);
#endif

    // offset, might want to compute this ourselves from the existing file size
    const char *mode = "w+b";
    if(access(argv[2], F_OK) == 0)
    {
        req[1] = vclh_file_size(argv[2]);
        mode = "a+b";
    }
    else
        req[1] = 0;
    if(!send_vc(vcsock, req))
        exit(1);

    vc v;
    if(!recv_vc(vcsock, v))
        exit(1);

    if(v.type() != VC_VECTOR)
        exit(1);
    if(v[0] != vc("sfile"))
        exit(1);
    if(v[1].type() != VC_VECTOR)
        exit(1);
    if(v[1][0] != req[0])
        exit(1);
    if(v[1][1].type() != VC_INT)
        exit(1);
    int expected_size = (long)v[1][1];

    if(!send_vc(vcsock, "fileok"))
        exit(1);

    FILE *f = fopen(argv[2], mode);
    if(!f)
        exit(1);
#ifdef LINUX
    if(flock(fileno(f), LOCK_EX|LOCK_NB) == -1)
        exit(1);
    if(fseeko(f, (off_t)(long) req[1], SEEK_SET) != 0)
        exit(1);
#else
    if(fseek(f, (off_t)(long) req[1], SEEK_SET) != 0)
        exit(1);
#endif

    while(1)
    {
#ifdef LINUX
        alarm(60);
#endif
        if(!recv_vc(vcsock, v))
            exit(1);
        if(v.is_nil())
            break;
        if(v.type() != VC_STRING)
            exit(1);
        if(fwrite((const char *)v, 1, v.len(), f) != v.len())
            exit(1);

    }
    if(fflush(f) != 0)
        exit(1);
    if(!send_vc(vcsock, "frok"))
        exit(1);
#ifdef LINUX
    alarm(60);
#endif

#ifdef LINUX
    if(fchmod(fileno(f), 0755) != 0)
        exit(1);
#endif

    rewind(f);
    SHA1 sha;
    byte *hash = new byte[sha.DigestSize()];
    while(!feof(f))
    {
        char buf[2048];
        size_t n;
        if((n = fread(buf, 1, sizeof(buf), f)) != sizeof(buf))
        {
            if(ferror(f))
                exit(1);
        }
        sha.Update((byte *)buf, n);
    }
    sha.Final(hash);
    if(fclose(f) != 0)
        exit(1);
    DwString chk(argv[2]);
    chk += ".chk";
    FILE *f2 = fopen(chk.c_str(), "wb");
    if(!f2)
        exit(1);
    if(fwrite(hash, 1, sha.DigestSize(), f2) != sha.DigestSize())
        exit(1);
    if(fclose(f2) != 0)
        exit(1);

    vc sig = vclh_from_hex(argv[3]);
    vclh_dsa_pub_init("dsadwyco.pub");

    vc valid = vclh_dsa_verify(vc(VC_BSTRING, (const char *)hash, sha.DigestSize()), sig);
    if(valid.is_nil())
    {
        unlink(chk.c_str());
        unlink(argv[2]);
        exit(1);
    }

    DwString sigfn(argv[2]);
    sigfn += ".sig";
    FILE *f3 = fopen(sigfn.c_str(), "wb");
    if(!f3)
        exit(1);

    if(fwrite((const char *)sig, 1, sig.len(), f3) != sig.len())
    {
        fclose(f3);
        exit(1);
    }
    if(fclose(f3) != 0)
        exit(1);

    exit(0);
}
