
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// File:    ssns.h
// Author:  Brian Allen Vanderburg II
// Purpose: A simple signals and slots implemnetation
// ---------------------------------------------------------------------

// Include needed files
#define SSNS_BUILDING
#include "ssns.h"


// Detect threading support
// If the user set the value, assume it is correct
#if !defined(SSNS_NO_THREADS) && !defined(SSNS_WIN32_THREADS) && !defined(SSNS_POSIX_THREADS)
#if defined(WIN32)
#define SSNS_WIN32_THREADS
#elif defined(__GNUG__)
#define SSNS_POSIX_THREADS
#else
#define SSNS_NO_THREADS
#endif
#endif

// Included needed headers
#if defined(SSNS_WIN32_THREADS)
#include <windows.h>
#elif defined(SSNS_POSIX_THREADS)
#include <pthread.h>
#endif

// Private implementation details
namespace
{

// A mutex object
class mutex
{
public:
    mutex()
    {
#if defined(SSNS_WIN32_THREADS)
        InitializeCriticalSection(&m_internal);
#elif defined(SSNS_POSIX_THREADS)
        pthread_mutex_init(&m_internal, NULL);
#endif
    }

    ~mutex()
    {
#if defined(SSNS_WIN32_THREADS)
        DeleteCriticalSection(&m_internal);
#elif defined(SSNS_POSIX_THREADS)
        pthread_mutex_destroy(&m_internal);
#endif
    }

    void lock()
    {
#if defined(SSNS_WIN32_THREADS)
        EnterCriticalSection(&m_internal);
#elif defined(SSNS_POSIX_THREADS)
        pthread_mutex_lock(&m_internal);
#endif
    }

    void unlock()
    {
#if defined(SSNS_WIN32_THREADS)
        LeaveCriticalSection(&m_internal);
#elif defined(SSNS_POSIX_THREADS)
        pthread_mutex_unlock(&m_internal);
#endif
    }

private:
#if defined(SSNS_WIN32_THREADS)
    CRITICAL_SECTION m_internal;
#elif defined(SSNS_POSIX_THREADS)
    pthread_mutex_t m_internal;
#endif

    // Prevent copy contstruction/assignment
    mutex(const mutex& copy);// { }
    mutex& operator=(const mutex& copy);// { }
};

// The global mutex object
static mutex gs_mutex;

} // namespace (private implementation details)

// The mutex locker
ssns::mutex_locker::mutex_locker()
{
    gs_mutex.lock();
}

ssns::mutex_locker::~mutex_locker()
{
    gs_mutex.unlock();
}




