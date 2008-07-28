// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
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
*  re-implementation was created to work out the fact that LinuxThreads
*  implementation of pthread did not have timed versions of semaphore wait
*  (sem_timedwait()). At modern time NPTL is widely adopted, thus
*  solving this problem efficiently. So, if you're not forced to use
*  pthread implementation without sem_timedwait() it is better
*  to define SIPX_USE_NATIVE_PTHREADS.
*  See original comment to sipX implementation below #else.
*/

#ifndef _PT_CSEM_H
#define _PT_CSEM_H

#define SIPX_USE_NATIVE_PTHREADS
#ifdef __MACH__
#undef SIPX_USE_NATIVE_PTHREADS
#endif

#ifdef SIPX_USE_NATIVE_PTHREADS // [

#include <semaphore.h>

#define pt_sem_t sem_t

#define pt_sem_init(sem, max, count) sem_init((sem), 0, (count))
#define pt_sem_wait(sem)     sem_wait((sem))
#define pt_sem_timedwait(sem, timeout) sem_timedwait((sem), (timeout))
#define pt_sem_trywait(sem)  sem_trywait((sem))
#define pt_sem_post(sem)     sem_post((sem))
#define pt_sem_getvalue(sem) sem_getvalue((sem))
#define pt_sem_destroy(sem)  sem_destroy((sem))


#else // SIPX_USE_NATIVE_PTHREADS ][

/* The default LinuxThreads implementation does not have support for timing
* out while waiting for a synchronization object. Since I've already ported
* the rest of the OS dependent files to that interface, we can just drop in a
* mostly-compatible replacement written in C (like pthreads itself) that uses
* the pthread_cond_timedwait function and a mutex to build all the other
* synchronization objecs with timeout capabilities. */

#include <pthread.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct pt_sem {
        unsigned int count;
        unsigned int max;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
} pt_sem_t;

int pt_sem_init(pt_sem_t *sem, unsigned int max, unsigned int count);

int pt_sem_wait(pt_sem_t *sem);

int pt_sem_timedwait(pt_sem_t *sem,const struct timespec *timeout);

int pt_sem_trywait(pt_sem_t *sem);

int pt_sem_post(pt_sem_t *sem);

int pt_sem_getvalue(pt_sem_t *sem);

int pt_sem_destroy(pt_sem_t *sem);

#ifdef  __cplusplus
}
#endif

#endif // SIPX_USE_NATIVE_PTHREADS ]

#endif /* _PT_CSEM_H */
