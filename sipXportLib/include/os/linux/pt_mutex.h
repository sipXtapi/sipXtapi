//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

/* SIPX_USE_NATIVE_PTHREADS define enable use of standard pthread implementation
*  of synchonization primitives instead of sipX re-implementation of them. This
*  re-implementation was created to work around the fact that the LinuxThreads
*  implementation of pthreads did not have a timed version of semaphore wait
*  (sem_timedwait()). These days NPTL is widely adopted, thus solving this
*  problem more efficiently. So, if you're not forced to use a pthreads
*  implementation without sem_timedwait() (e.g. on OS X) it is better to
*  define SIPX_USE_NATIVE_PTHREADS.
*  See the original comment about the sipX implementation below #else.
*/

#ifndef _PT_MUTEX_H
#define _PT_MUTEX_H

#if !defined(__APPLE__) && !defined(ANDROID)
/* Also see pt_csem.h */
#define SIPX_USE_NATIVE_PTHREADS
//#undef SIPX_USE_NATIVE_PTHREADS
#endif

#include <pthread.h>

#ifdef SIPX_USE_NATIVE_PTHREADS // [

#define pt_mutex_t pthread_mutex_t
#if defined(__linux__) && !defined(PTHREAD_MUTEX_RECURSIVE)
#  define PTHREAD_MUTEX_RECURSIVE  PTHREAD_MUTEX_RECURSIVE_NP
#endif


//#define pt_mutex_init(mutex)    pthread_mutex_init((mutex), NULL)
static inline int pt_mutex_init(pthread_mutex_t *mutex)
{
   pthread_mutexattr_t a;
   pthread_mutexattr_init(&a);
   pthread_mutexattr_settype(&a,PTHREAD_MUTEX_RECURSIVE);
   return pthread_mutex_init(mutex, &a);
}

#define pt_mutex_lock(mutex)    pthread_mutex_lock((mutex))
#define pt_mutex_timedlock(mutex, timeout)  pthread_mutex_timedlock((mutex), (timeout))
#define pt_mutex_trylock(mutex) pthread_mutex_trylock((mutex))
#define pt_mutex_unlock(mutex)  pthread_mutex_unlock((mutex))
#define pt_mutex_destroy(mutex) pthread_mutex_destroy((mutex))


#else // SIPX_USE_NATIVE_PTHREADS ][

/* The default LinuxThreads implementation does not have support for timing
* out while waiting for a synchronization object. Since I've already ported
* the rest of the OS dependent files to that interface, we can just drop in a
* mostly-compatible replacement written in C (like pthreads itself) that uses
* the pthread_cond_timedwait function and a mutex to build all the other
* synchronization objecs with timeout capabilities. */

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct pt_mutex {
        unsigned int count;
        pthread_t thread;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
} pt_mutex_t;

void dumpPtMutex(pt_mutex_t* mutex);

int pt_mutex_init(pt_mutex_t *mutex);

int pt_mutex_lock(pt_mutex_t *mutex);

int pt_mutex_timedlock(pt_mutex_t *mutex,const struct timespec *timeout);

int pt_mutex_trylock(pt_mutex_t *mutex);

int pt_mutex_unlock(pt_mutex_t *mutex);

int pt_mutex_destroy(pt_mutex_t *mutex);

#ifdef  __cplusplus
}
#endif

#endif // SIPX_USE_NATIVE_PTHREADS ]

#endif /* _PT_MUTEX_H */
