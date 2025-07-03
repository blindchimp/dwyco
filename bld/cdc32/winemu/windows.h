#ifdef _Windows
#include <windows.h>
#else
#ifndef DUMMY_WINDOWS
#define DUMMY_WINDOWS
#include <unistd.h>

typedef unsigned long HANDLE;
typedef unsigned long HWND;
typedef unsigned long DWORD;
typedef void *DWORD_PTR;
typedef unsigned int UINT;
typedef HANDLE HWAVEOUT;

#include <pthread.h>

typedef struct CRITICAL_SECTION {
	long LockCount;
	long RecursionCount;
	int is_owned;
	pthread_t OwningThread;
	pthread_mutexattr_t attr;
	pthread_mutex_t LockSemaphore;
} CRITICAL_SECTION, *LPCRITICAL_SECTION;


void InitializeCriticalSection(CRITICAL_SECTION *);
void EnterCriticalSection(CRITICAL_SECTION *);
void LeaveCriticalSection(CRITICAL_SECTION *);
void DeleteCriticalSection(CRITICAL_SECTION *);
unsigned long GetTickCount();
unsigned long timeGetTime();
int CopyFile(const char *s, const char *d, int fail_if_exists);
HANDLE CreateEvent(int, int, int, int);
void SetEvent(HANDLE);
int WaitForMultipleObjects(int num, HANDLE* objs, int wait_all, int ms_to_wait);
void CloseHandle(HANDLE);
DWORD GetCurrentThreadId();
#define WAIT_TIMEOUT 1
#define WAIT_OBJECT_0 2
#define WAIT_ERROR 3

struct foo_gibble
{
	char cFileName[256];
};


typedef struct foo_gibble WIN32_FIND_DATA;

struct RECT 
{
	int top;
	int bottom;
	int left;
	int right;
};

#define DeleteFile(p) (unlink(p) == 0)
#define RemoveDirectory(p) (rmdir(p) == 0)
#define MoveFile(p1, p2) (rename(p1, p2) == 0)

#include "miscemu.h"

#endif
#endif
