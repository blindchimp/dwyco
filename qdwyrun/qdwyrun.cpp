// note: this is a qt4 program that doesn't compile at the moment.
// i'm not going to fix it because it is only used by CDC-X installs, and
// i'm not sure i'm going to continue using it, as the functionality i put
// in it hasn't been used that much (different methods of performing updates
// which didn't work quite as well as i'd hoped, mainly due to virus checkers
// and whatnot on windows.) in addition, the mac version doesn't work at all, and
// the linux version has so few users it isn't worth it.)
//
// at some point, it might be useful once ported up to qt5 in situations where
// there is no "app store" available to perform updates for you.

#include <QFileInfo>
#include <QProcess>
#include <QDesktopWidget>
#include <stdio.h>
#include "qdwyrun.h"
#include "ui_qdwyrun.h"

#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#ifdef LINUX
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#endif
#ifdef _Windows
#include <windows.h>
#include <io.h>
#include <process.h>
#define F_OK 0
#endif

#include "vc.h"
#include "vccomp.h"
#include "vclhsys.h"
#include "dwstr.h"
#include "sha.h"
#include "vccrypt2.h"
#include "zcomp.h"
#include "vclh.h"

using namespace CryptoPP;

extern QString app_to_run;
extern QString update_app;
static int Did_simple_update;

qdwyrun::qdwyrun(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::qdwyrun)
{
    ui->setupUi(this);
    idle_timer.setInterval(2000);
    connect(&idle_timer, SIGNAL(timeout()), this, SLOT(idle()));
    idle_timer.setSingleShot(1);
    idle_timer.start();
    ui->done_button->setVisible(0);
}

qdwyrun::~qdwyrun()
{
    delete ui;
}

void qdwyrun::centerWidget(QWidget *w, bool useSizeHint)
{
    if(w->isFullScreen())
        return;

    QSize size;
    if(useSizeHint)
        size = w->sizeHint();
    else
        size = w->size();

    QDesktopWidget *d = QApplication::desktop();
    QRect r = d->screenGeometry();
    int ws = r.width();   // returns screen width
    int h = r.height();  // returns screen height
    int mw = size.width();
    int mh = size.height();
    int cw = (ws/2) - (mw/2);
    int ch = (h/2) - (mh/2);
    w->move(cw,ch);
}

// note: this function leaks a little bit, but it is only
// called once or twice, so no big deal
static int
check_staged_update(DwString fn)
{
    FILE *f = fopen(fn.c_str(), "rb");
    if(!f)
        return 0;

    SHA1 sha;
    byte *hash = new byte[sha.DigestSize()];
    while(!feof(f))
    {
        char buf[2048];
        size_t n;
        if((n = fread(buf, 1, sizeof(buf), f)) != sizeof(buf))
        {
            if(ferror(f))
            {
                fclose(f);
                unlink(fn.c_str());
                return 0;
            }
        }
        sha.Update((byte *)buf, n);
    }
    fclose(f);
    sha.Final(hash);
    DwString chk(fn);
    chk += ".chk";
    FILE *f2 = fopen(chk.c_str(), "rb");
    if(!f2)
        return 0;
    byte *hash2 = new byte[sha.DigestSize()];
    if(fread(hash2, 1, sha.DigestSize(), f2) != sha.DigestSize())
    {
        fclose(f2);
        unlink(chk.c_str());
        unlink(fn.c_str());
        return 0;
    }
    fclose(f2);
    if(memcmp(hash, hash2, sha.DigestSize()) != 0)
    {
        unlink(chk.c_str());
        unlink(fn.c_str());
        return 0;
    }

    DwString sigfn(fn);
    sigfn += ".sig";
    vc sz = vclh_file_size(sigfn.c_str());
    if(sz.type() != VC_INT)
    {
        unlink(chk.c_str());
        unlink(fn.c_str());
        return 0;
    }
    size_t s = (long)sz;
    if(s <= 0 || s > 2048)
    {
        unlink(chk.c_str());
        unlink(fn.c_str());
        return 0;
    }
    char *buf = new char[s];

    FILE *f3 = fopen(sigfn.c_str(), "rb");
    if(!f3)
    {
        unlink(chk.c_str());
        unlink(fn.c_str());
        return 0;
    }
    if(fread(buf, 1, s, f3) != s)
    {
        fclose(f3);
        unlink(chk.c_str());
        unlink(fn.c_str());
        unlink(sigfn.c_str());
        return 0;
    }
    fclose(f3);

    vclh_dsa_pub_init("dsadwyco.pub");
    vc sig(VC_BSTRING, buf, s);
    vc valid = vclh_dsa_verify(vc(VC_BSTRING, (const char *)hash, sha.DigestSize()), sig);
    if(valid.is_nil())
    {
        unlink(chk.c_str());
        unlink(fn.c_str());
        unlink(sigfn.c_str());
        return 0;
    }

    return 1;

}

#ifdef WIN32
static int
move_replace(const DwString& s, const DwString& d)
{
    return MoveFileEx(s.c_str(), d.c_str(), MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
}
#else
static int
move_replace(const DwString& s, const DwString& d)
{
#ifdef TEST
    if(s.eq("a.out"))
    {
        errno = EPERM;
        return 0;
    }
#endif
    return rename(s.c_str(), d.c_str()) == 0;
}
#endif

static QList<QByteArray> bo;
static QList<QByteArray> dels;
static
int
simple_update(DwString fn)
{
    vc arch;
    {
        vc ctx = vclh_decompression_open();
        vc file = doopenfile(fn.c_str(), "rb");
        vc str = docontents_of(file);

        if(vclh_decompress_xfer(ctx, str, arch).is_nil())
            return 0;
    }

    // should be vector(name0 contents0 name1 contents1 ...
    for(int i = 0; i < arch.num_elems(); i += 2)
    {
        vc name = arch[i];
        vc cont = arch[i + 1];
        try
        {
            FILE *f = fopen("__tmp.tmp", "wb");
            if(!f)
                throw -1;
            if(fwrite((const char *)cont, cont.len(), 1, f) != 1)
                throw -1;
            if(fclose(f) != 0)
                throw -1;
            QByteArray nmo((const char *)name, name.len());
            nmo += "-old";
#ifndef _Windows
            mode_t mode = 0664;
            if(access(name, X_OK) == 0)
                mode = 0775;
#endif
            if(move_replace((const char *)name, nmo.constData()))
                bo.append(nmo);
            else
            {
                // this allows you to add a file that doesn't exist
                // without an error
#ifdef _Windows
                if(GetLastError() != ERROR_FILE_NOT_FOUND)
#else
                if(errno != ENOENT)
#endif
                    throw -1;
                else
                    dels.append((const char *)name);
            }
            if(!move_replace("__tmp.tmp", (const char *)name))
                throw -1;
#ifndef _Windows
            // note: preserve mode of new file
            chmod(name, mode);
#endif
        }
        catch(...)
        {
            for(int i = bo.count() - 1; i >= 0; --i)
            {
                QByteArray nm = bo[i];
                // remove the -old from the end
                nm.resize(nm.length() - 4);
                // if this doesn't work, we've screwed something
                // royally, but there isn't much to do except
                // indicate error
                move_replace(bo[i].constData(), nm.constData());
            }
            for(int i = dels.count() - 1; i >= 0; --i)
                QFile::remove(dels[i].constData());
            QFile::remove("__tmp.tmp");
            return 0;
        }

    }
    return 1;
}

static
void
undo_update()
{
    for(int i = bo.count() - 1; i >= 0; --i)
    {
        QByteArray nm = bo[i];
        // remove the -old from the end
        nm.resize(nm.length() - 4);
        // if this doesn't work, we've screwed something
        // royally, but there isn't much to do
        move_replace(bo[i].constData(), nm.constData());
    }
    for(int i = dels.count() - 1; i >= 0; --i)
        QFile::remove(dels[i].constData());
    QFile::remove("__tmp.tmp");

}


void
qdwyrun::run_proc(QString cmd, QStringList args)
{
    proc->start(cmd, args, QIODevice::NotOpen);
}

void
qdwyrun::run_app()
{
    static int been_here;
    if(been_here)
        return;
    been_here = 1;
    QStringList args;
    int n = qApp->arguments().count();
    for(int i = 3; i < n; ++i)
    {
        args.append(qApp->arguments().at(i));
    }
    proc = new QProcess;
    connect(proc, SIGNAL(started()), this, SLOT(app_started()));
    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(too_quick(int, QProcess::ExitStatus)));
    connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(proc_error(QProcess::ProcessError)));
    run_proc(app_to_run, args);
}

void
qdwyrun::too_quick(int code, QProcess::ExitStatus e)
{
    if(idle_timer.isActive())
    {
        idle_timer.stop();
        proc_error(QProcess::Crashed);
    }

}

void
qdwyrun::done()
{
    exit(0);
}

void
qdwyrun::app_started()
{
    disconnect(&idle_timer, SIGNAL(timeout()), this, SLOT(idle()));
    connect(&idle_timer, SIGNAL(timeout()), this, SLOT(done()));
    idle_timer.start();

}

void
qdwyrun::proc_error(QProcess::ProcessError)
{
    ui->label_2->setText("Launch failed...");
    ui->done_button->setVisible(1);
    if(Did_simple_update)
    {
        ui->label_2->setText("Launch failed... undoing update, try again.");
        undo_update();
    }
}

void
qdwyrun::update_finished(int exitcode, QProcess::ExitStatus estatus)
{
    if(exitcode == 0 && estatus == QProcess::NormalExit)
    {
        ui->label_2->setText("Update done");
    }
    else
        ui->label_2->setText("Update failed");

    QFile::remove("run-update");
    QFile::remove(update_app);
    QString chkfn = update_app + ".chk";
    QString sigfn = update_app + ".sig";
    QFile::remove(chkfn);
    QFile::remove(sigfn);

    delete proc;
    proc = 0;
    run_app();
}

void
qdwyrun::update_error(QProcess::ProcessError)
{
    ui->label_2->setText("Update failed");
    delete proc;
    proc = 0;
    run_app();
}

void
qdwyrun::run_update(QString fn)
{
    static int been_here;
    if(been_here)
        return;
    been_here = 1;
    QStringList args;
    args.append("/silent");

#ifdef LINUX
    QString a("./");
#else
    QString a(".\\");
#endif
    a += fn;
    chmod(fn.toLatin1().constData(), 0755);

    ui->label_2->setText("Updating CDC-X...");

    proc = new QProcess;
    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(update_finished(int, QProcess::ExitStatus)));
    connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(update_error(QProcess::ProcessError)));
    run_proc(a, args);
}

static int
check_and_do_simple_update()
{
    FILE *f = 0;
    try
    {
        f = fopen("simple-update", "rb");
        if(!f)
        {
            throw -1;
        }
#if defined(_Windows) || defined(_WIN32)
        // note: on windows, the mutex is protecting us, not the fd, so close it
        // now so we can delete the file later after the update is done.
        fclose(f);
#endif

#ifdef LINUX
        if(flock(fileno(f), LOCK_EX|LOCK_NB) == -1)
        {
            throw -1;
        }
#endif

        if(check_staged_update("simple-update"))
        {
            int ret = simple_update("simple-update");
            // whether it worked or not, remove the update
            QFile::remove("simple-update");
            QFile::remove("simple-update.chk");
            QFile::remove("simple-update.sig");
            QFile::remove("run-update");
            if(ret)
                Did_simple_update = 1;
            else
                throw -1;
        }
        else
            throw -1;
    }
    catch(...)
    {
        if(f) fclose(f);
        return 0;
    }
    return 1;
}

void
qdwyrun::idle()
{
    QFileInfo runup("run-update");

    if(runup.exists())
    {
#if defined(_Windows) || defined(_WIN32)
        // if there is anyone around even thinking of using the file, punt
        QString mn("dwyco-autoupdate-");
        mn += update_app;
        HANDLE h;
        if((h = CreateMutex(NULL, TRUE, mn.toLatin1())) == NULL)
        {
            run_app();
            return;
        }
        if(GetLastError() == ERROR_ALREADY_EXISTS)
        {
            run_app();
            return;
        }
#endif
        if(check_and_do_simple_update())
        {
            run_app();
            return;
        }

        FILE *f = fopen(update_app.toLatin1().constData(), "r");
        if(!f)
        {
            run_app();
            return;
        }
#if defined(_Windows) || defined(_WIN32)
        // note: on windows, the mutex is protecting us, not the fd, so close it
        // now so we can delete the file later after the update is done.
        fclose(f);
#endif

#ifdef LINUX
        if(flock(fileno(f), LOCK_EX|LOCK_NB) == -1)
        {
            run_app();
            return;
        }
#endif

        if(check_staged_update(update_app.toLatin1().constData()))
        {
            run_update(update_app);
        }
        else
            run_app();
        // note: with any luck, the OS will clean up the locks when we exit.
    }
    else
    {
        run_app();
    }
}

void qdwyrun::on_done_button_clicked()
{
    exit(1);
}
