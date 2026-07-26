param(
    [Parameter(Mandatory)] [string] $ToxcoreRoot
)

$compatDir = Join-Path $ToxcoreRoot "compat"
if (-not (Test-Path $compatDir)) {
    New-Item -ItemType Directory -Path $compatDir -Force | Out-Null
}

$content = @'
#ifndef COMPAT_PTHREAD_H
#define COMPAT_PTHREAD_H

#include <windows.h>

#define PTHREAD_MUTEX_RECURSIVE 1

typedef CRITICAL_SECTION pthread_mutex_t;
typedef int pthread_mutexattr_t;

static inline int pthread_mutexattr_init(pthread_mutexattr_t *a) { (void)a; return 0; }
static inline int pthread_mutexattr_settype(pthread_mutexattr_t *a, int t) { (void)a; (void)t; return 0; }
static inline int pthread_mutexattr_destroy(pthread_mutexattr_t *a) { (void)a; return 0; }

static inline int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *a) {
    (void)a;
    InitializeCriticalSection(m);
    return 0;
}
static inline int pthread_mutex_lock(pthread_mutex_t *m) { EnterCriticalSection(m); return 0; }
static inline int pthread_mutex_unlock(pthread_mutex_t *m) { LeaveCriticalSection(m); return 0; }
static inline int pthread_mutex_destroy(pthread_mutex_t *m) { DeleteCriticalSection(m); return 0; }

typedef struct { SRWLOCK lock; } pthread_rwlock_t;

static inline int pthread_rwlock_init(pthread_rwlock_t *rw, const void *attr) {
    (void)attr;
    InitializeSRWLock(&rw->lock);
    return 0;
}
static inline int pthread_rwlock_rdlock(pthread_rwlock_t *rw) { AcquireSRWLockShared(&rw->lock); return 0; }
static inline int pthread_rwlock_wrlock(pthread_rwlock_t *rw) { AcquireSRWLockExclusive(&rw->lock); return 0; }
static inline int pthread_rwlock_unlock(pthread_rwlock_t *rw) { ReleaseSRWLockShared(&rw->lock); ReleaseSRWLockExclusive(&rw->lock); return 0; }
static inline int pthread_rwlock_destroy(pthread_rwlock_t *rw) { (void)rw; return 0; }

#endif
'@

$content | Out-File -FilePath (Join-Path $compatDir "pthread.h") -Encoding ascii -NoNewline
Write-Host "  Created $compatDir\pthread.h"
