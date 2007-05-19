//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

/* The default LinuxThreads implementation does not have support for timing
* out while waiting for a synchronization object. Since I've already ported
* the rest of the OS dependent files to that interface, we can just drop in a
* mostly-compatible replacement written in C (like pthreads itself) that uses
* the pthread_cond_timedwait function and a mutex to build all the other
* synchronization objecs with timeout capabilities. */

/* This is the mutex implementation. */

#include <pthread.h>
#include <errno.h>
#include "os/linux/pt_mutex.h"

int pt_mutex_init(pt_mutex_t *mutex)
{
        mutex->count=0;
        return pthread_mutex_init(&mutex->mutex,NULL) | pthread_cond_init(&mutex->cond,NULL);
}

int pt_mutex_lock(pt_mutex_t *mutex)
{
        pthread_mutex_lock(&mutex->mutex);
        if(!mutex->count)
        {
                mutex->count=1;
                mutex->thread=pthread_self();
                pthread_mutex_unlock(&mutex->mutex);
                return 0;
        }
        else if(mutex->thread==pthread_self())
        {
                mutex->count++;
                pthread_mutex_unlock(&mutex->mutex);
                return 0;
        }
        while(mutex->count)
                pthread_cond_wait(&mutex->cond,&mutex->mutex);
        mutex->count=1;
        mutex->thread=pthread_self();
        pthread_mutex_unlock(&mutex->mutex);
        return 0;
}

int pt_mutex_timedlock(pt_mutex_t *mutex,const struct timespec *timeout)
{
        pthread_mutex_lock(&mutex->mutex);
        if(!mutex->count)
        {
                mutex->count=1;
                mutex->thread=pthread_self();
                pthread_mutex_unlock(&mutex->mutex);
                return 0;
        }
        else if(mutex->thread==pthread_self())
        {
                mutex->count++;
                pthread_mutex_unlock(&mutex->mutex);
                return 0;
        }
        pthread_cond_timedwait(&mutex->cond,&mutex->mutex,timeout);
        if(!mutex->count)
        {
                mutex->count=1;
                mutex->thread=pthread_self();
                pthread_mutex_unlock(&mutex->mutex);
                return 0;
        }
        errno=EAGAIN;
        pthread_mutex_unlock(&mutex->mutex);
        return -1;
}

int pt_mutex_trylock(pt_mutex_t *mutex)
{
        pthread_mutex_lock(&mutex->mutex);
        if(!mutex->count)
        {
                mutex->count=1;
                mutex->thread=pthread_self();
                pthread_mutex_unlock(&mutex->mutex);
                return 0;
        }
        else if(mutex->thread==pthread_self())
        {
                mutex->count++;
                pthread_mutex_unlock(&mutex->mutex);
                return 0;
        }
        errno=EAGAIN;
        pthread_mutex_unlock(&mutex->mutex);
        return -1;
}

int pt_mutex_unlock(pt_mutex_t *mutex)
{
        pthread_mutex_lock(&mutex->mutex);
/*      if(mutex->thread!=pthread_self())
        {
                errno=EPERM;
                return -1;
        }*/
        if(mutex->count)
        {
                mutex->count--;
                if(!mutex->count)
                        pthread_cond_signal(&mutex->cond);
                pthread_mutex_unlock(&mutex->mutex);
                return 0;
        }
        pthread_mutex_unlock(&mutex->mutex);
        return 0;
}

int pt_mutex_destroy(pt_mutex_t *mutex)
{
        if(mutex->count)
        {
                errno=EBUSY;
                return -1;
        }
        return pthread_mutex_destroy(&mutex->mutex) | pthread_cond_destroy(&mutex->cond);
}
