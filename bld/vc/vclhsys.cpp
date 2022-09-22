
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//#ifndef ANDROID
//
// stuff related to the system LH is running in
//
#ifdef _MSC_VER
#define _Windows
#define __WIN32__
#undef _UNICODE
#undef UNICODE
#endif

#if defined(_Windows)
#include <windows.h>
#include <io.h>
#include <dos.h>
#include <sys\stat.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#ifndef _MSC_VER
#include <dirent.h>
#endif
#include "vc.h"
#include "vclhsys.h"
#include "vcmap.h"
#include "vcwsock.h"
#include "dwstr.h"

//static char Rcsid[] = "$Header: /e/linux/home/dwight/repo/vc/rcs/vclhsys.cpp 1.45 1999/03/17 14:57:04 dwight Exp $";

vc
vclh_getenv(vc name)
{
	if(name.type() != VC_STRING)
		USER_BOMB("first arg to getenv must be string", vcnil);
	char *a = getenv(name);
	if(a == 0)
		return vcnil;
	return vc(a);
}

vc
vclh_putenv(vc name, vc value)
{
	static char puttmp[2048];
	const char *put;
	
	if(name.type() != VC_STRING)
		USER_BOMB("first arg to putenv must be string", vcnil);
	if(!value.is_nil() && value.type() != VC_STRING)
		USER_BOMB("second arg to putenv must be a string", vcnil);
	if(value.is_nil())
		put = "";
	else
		put = value;
	sprintf(puttmp, "%s=%s", (const char *)name, put);
	char *putstr = strdup(puttmp);
	if(putenv(putstr) != 0)
		return vcnil;
	return vctrue;
}

vc
vclh_dir2map(vc dir)
{
#ifdef _MSC_VER
	USER_BOMB("sorry, msvc version doesn't support dir2map yet", vcnil);
#else
	if(dir.type() != VC_STRING)
		USER_BOMB("first arg to dir2map must be string dir name", vcnil);
	DIR *dp = opendir((const char *)dir);
	if(dp == 0)
		return vcnil;
	vc ret(VC_MAP, "", 31);
	struct dirent *dep;
	while((dep = readdir(dp)) != 0)
	{
		ret.add(dep->d_name);
	}
	closedir(dp);
	return ret;
#endif	
}

vc
vclh_file_remove(vc file)
{
	if(file.type() != VC_STRING)
		USER_BOMB("first arg to remove must be string filename", vcnil);
	if(unlink(file) == 0)
		return vctrue;
	return vcnil;
}

vc
vclh_file_exists(vc file)
{
	if(file.type() != VC_STRING)
		USER_BOMB("first arg to exists must be string filename", vcnil);
#if defined(_Windows)
	if(access(file, 0) == 0)
#else
	if(access(file, F_OK) == 0)
#endif
		return vctrue;
	return vcnil;
}

vc
vclh_file_access(vc file, vc how)
{
	if(file.type() != VC_STRING)
		USER_BOMB("first arg to access must be string filename", vcnil);
	if(how.type() != VC_STRING)
		USER_BOMB("second arg to access must be string from rwx", vcnil);
		
#if defined(_Windows)
	int amode = 0;
	if(strchr(how, 'r'))
		amode |= 4;
	if(strchr(how, 'w'))
		amode |= 2;
	if(access(file, amode) == 0)
#else
	if(access(file, F_OK) == 0)
#endif
		return vctrue;
	return vcnil;
}

// note: on linux, this provides an atomic rename operation
// on windows, i have no idea about the detailed properties of
// the operation.
vc
vclh_file_rename(vc from, vc to)
{
	if(from.type() != VC_STRING)
		USER_BOMB("first arg to rename must be string filename", vcnil);
	if(to.type() != VC_STRING)
		USER_BOMB("second arg to access must be string filename", vcnil);
	if(rename(from, to) == -1)
		return vcnil;
	return vctrue;
}

vc
vclh_file_size(vc file)
{
	if(file.type() != VC_STRING)
		USER_BOMB("first arg to size must be string filename", vcnil);
		
	struct stat s;
	if(stat((const char *)file, &s) == -1)
		return vcnil;
	return vc((long)s.st_size);
}

vc
vclh_file_stat(vc file)
{
	if(file.type() != VC_STRING)
		USER_BOMB("first arg to size must be string filename", vcnil);
		
	struct stat s;
	if(stat((const char *)file, &s) == -1)
		return vcnil;
	vc ret(VC_TREE);
	ret.add_kv("st_dev", (long)s.st_dev);
	ret.add_kv("st_ino", (long)s.st_ino);
	ret.add_kv("st_mode", (long)s.st_mode);
	ret.add_kv("st_nlink", (long)s.st_nlink);
	ret.add_kv("st_uid", (long)s.st_uid);
	ret.add_kv("st_gid", (long)s.st_gid);
	ret.add_kv("st_rdev", (long)s.st_rdev);
	ret.add_kv("st_size", (long)s.st_size);
#ifndef _Windows
	ret.add_kv("st_blksize", (long)s.st_blksize);
	ret.add_kv("st_blocks", (long)s.st_blocks);
#endif
	ret.add_kv("st_atime", (long)s.st_atime);
	ret.add_kv("st_mtime", (long)s.st_mtime);
	ret.add_kv("st_ctime", (long)s.st_ctime);
	return ret;
}

vc
vclh_strftime(vc time, vc format)
{
	if(time.type() != VC_INT)
		USER_BOMB("first arg to strftime must be integer time", vcnil);
	if(format.type() != VC_STRING)
		USER_BOMB("second arg to strftime must be format specifier string", vcnil);
		
#ifdef _MSC_VER		
	time_t tt = (long)time;
#else	
	time_t tt = time;
#endif	
	struct tm *t = gmtime(&tt);
	char s[255];
	if(strftime(s, sizeof(s) - 1, format, t) == 0)
		USER_BOMB("strftime format result too long (must be < 255 chars)", vcnil);
	return vc(s);
}

vc
vclh_strftime_hp(vc format)
{
#ifdef UNIX
	if(format.type() != VC_STRING)
		USER_BOMB("second arg to strftime must be format specifier string", vcnil);
	struct timeval tv;
	if(gettimeofday(&tv, 0) == -1)
	{
		memset(&tv, 0, sizeof(tv));
	}
	struct tm *t = gmtime(&tv.tv_sec);
	char s[455];
	if(strftime(s, 255, format, t) == 0)
		USER_BOMB("strftime format result too long (must be < 255 chars)", vcnil);
	char us[100];
	sprintf(us, ".%06ld", tv.tv_usec);
	strcat(s, us);
	return vc(s);
#else
	USER_BOMB("sorry, no hi precision time under windows", vcnil);
#endif
}

vc
vclh_time(void)
{
	return vc((long)time(0));
}

vc
vclh_time_hp(void)
{
#ifdef UNIX
	struct timeval tv;
	if(gettimeofday(&tv, 0) == -1)
	{
		memset(&tv, 0, sizeof(tv));
	}
	vc ret(VC_VECTOR);
	ret[0] = tv.tv_sec;
	ret[1] = tv.tv_usec;
	return ret;
#else
	USER_BOMB("sorry, no high precision time under windows", vcnil);
#endif	
}

vc
vclh_time_hp2(void)
{
#ifdef UNIX
	struct timeval tv;
	if(gettimeofday(&tv, 0) == -1)
	{
		memset(&tv, 0, sizeof(tv));
	}
#if defined(__LP64__)
	return vc(tv.tv_sec * 1000000 + tv.tv_usec);
#else
	USER_BOMB("sorry, no high precision integer time in 32bits-ville", vcnil);
#endif
#else
	USER_BOMB("sorry, no high precision time under windows", vcnil);
#endif
}


vc
vclh_system(vc cmd)
{
#ifdef DWYCO_IOS
	return vcnil;
#else
	if(cmd.type() != VC_STRING)
		USER_BOMB("arg to system must be a string", vcnil);
	const char *s = (const char *)cmd;
	int i = system(s);
	return vc(i);
#endif
}

#if defined(_Windows) && !defined(__WIN32__)

vc
vclh_sleep(vc seconds)
{
	if(seconds.type() != VC_INT)
		USER_BOMB("first arg to sleep must be integer number of seconds", vcnil);
	DWORD now = GetTickCount();
	MSG msg;

	while(1)
	{
		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if((GetTickCount() - now) >= (long)seconds * 1000L)
			return vcnil;
	}
	if(msg.message == WM_QUIT)
		USER_BOMB("quit during sleep", vcnil);
	return vcnil;
}
#else
vc
vclh_sleep(vc seconds)
{
	if(seconds.type() != VC_INT)
		USER_BOMB("first arg to sleep must be integer number of seconds", vcnil);
#ifdef _MSC_VER
	Sleep((long)seconds * 1000);
#else		
	sleep((long)seconds);
#endif	
	return vcnil;
}
#endif

vc
vclh_process_create(VCArglist *va)
{
	int fd_tosave = (int)(*va)[0];
	va->del(0);
#ifdef __WIN32__
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	DwString argbuf;
	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	si.dwFlags = 0;

	// createprocess wants them all glommed together
	// unlike the unix exec, which wants them
	// separately
	int n = va->num_elems();
	for(int i = 0; i < n; ++i)
	{
		argbuf += (const char *)(*va)[i];
		argbuf += " ";
	}
	char *a = new char[argbuf.length() + 1];
	strcpy(a, argbuf.c_str());
	
    VcError << a
            << "\n";
	if (!CreateProcess(NULL,a,NULL,NULL,
               0, //TRUE, // inherit handles
               0,NULL,NULL,&si,&pi) ){
	    delete [] a;
	    int i = GetLastError();
	    VcError << "create fail " << i << "\n";
		return vcnil;
	}
	delete [] a;
	return vc((int)pi.dwProcessId);
	
#else
// unix here
	int n = va->num_elems();
	char **abuf = new char *[n + 1];
	for(int i = 0; i < n; ++i)
	{
		abuf[i] = (char *)(const char *)(*va)[i];
	}
	abuf[n] = 0;
	
	int pid;
	if((pid = fork()) == 0)
	{
		// child
		if(fd_tosave != -1)
			vc_winsock::close_all_but(fd_tosave);
		execv(abuf[0], abuf);
		printf("exec FAILED %d\n", errno);
		exit(128);
	}
	else
	{
		if(pid == -1)
		{
			delete [] abuf;
			return vcnil;
		}
	}
	delete [] abuf;
	return vc((int)pid);
	
#endif
}

vc
vclh_clean_zombies()
{
#ifndef __WIN32__
	int a;
	a = waitpid(-1, 0, WNOHANG);
	return vc(a);
#else
	USER_BOMB("no zombies in windows", vcnil);
	return vcnil;
#endif
}

vc
vclh_clean_zombies2(VCArglist *al)
{
#ifndef __WIN32__
	if((*al)[0].type() != VC_STRING)
		USER_BOMB("first arg to clean_zombies2 must be string to bind", vcnil);
	int a;
	int status;
	vc ret(VC_VECTOR);
	a = waitpid(-1, &status, WNOHANG);
	if(a > 0)
	{
		if(WIFEXITED(status))
		{
			ret[0] = vctrue;
			ret[1] = WEXITSTATUS(status);
		}
		else if(WIFSIGNALED(status))
		{
			ret[0] = "signal";
			ret[1] = WTERMSIG(status);
		}
		else
		{
			ret[0] = vcnil;
			ret[1] = vcnil;
		}
		(*al)[0].local_bind(ret);
	}
	return vc(a);
#else
	USER_BOMB("no zombies in windows", vcnil);
	return vcnil;
#endif
}

vc
vclh_alarm(vc sec)
{
#ifndef __WIN32__
	if(sec.type() != VC_INT)
	{
		USER_BOMB("arg to alarm must be int", vcnil);
	}
	return vc((int)alarm((long)sec));
#else
		USER_BOMB("alarm not implemented", vcnil);
#endif
}

vc
vclh_kill(vc pid, vc sig)
{
#ifndef __WIN32__
	if(pid.type() != VC_INT || sig.type() != VC_INT)
	{
		USER_BOMB("usage: kill pid sig", vcnil);
	}
	if(kill((long)pid, (long)sig) == -1)
		return vcnil;
	return vctrue;
#else
		USER_BOMB("kill not implemented", vcnil);
#endif
}

