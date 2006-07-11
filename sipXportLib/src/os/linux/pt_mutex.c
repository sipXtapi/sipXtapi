//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

/* The default LinuxThreads implementation does not have support for timing
* out while waiting for a synchronization object. Since I've already ported
* the rest of the OS dependent files to that interface, we can just drop in a
* mostly-compatible replacement written in C (like pthreads itself) that uses
* the pthread_cond_timedwait function and a mutex to build all the other
* synchronization objecs with timeout capabilities. */

/* This is the mutex implementation. */

#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include "os/linux/pt_mutex.h"

int pt_mutex_init(pt_mutex_t *mutex)
{
        mutex->count=0;
        assert(0 == (pthread_mutex_init(&mutex->mutex,NULL) | pthread_cond_init(&mutex->cond,NULL)));
        return 0;
}

int pt_mutex_lock(pt_mutex_t *mutex)
{
        pthread_mutex_lock(&mutex->mutex);
        if( mutex->count && mutex->thread==pthread_self())
        {
                mutex->count++;
        }
        else
        {
           while(mutex->count)
           {
              pthread_cond_wait(&mutex->cond,&mutex->mutex);
           }
           mutex->count=1;
           mutex->thread=pthread_self();
        }
        pthread_mutex_unlock(&mutex->mutex);
        
        return 0;
}

int pt_mutex_timedlock(pt_mutex_t *mutex,const struct timespec *timeout)
{
        int retval;
        pthread_mutex_lock(&mutex->mutex);
        if(mutex->count && mutex->thread==pthread_self()) // allow recursive locks
        {
                mutex->count++;
                retval = 0;
        }
        else
        {
           retval = 0;
           while(ETIMEDOUT != retval && mutex->count)
           {
              retval = pthread_cond_timedwait(&mutex->cond,&mutex->mutex,timeout);
           }
           switch ( retval )
           {
           case 0: // we got the mutex
              mutex->count=1;
              mutex->thread=pthread_self();
              break;

           case ETIMEDOUT:
              errno=EAGAIN;
              retval = -1;
              break;

           default: // all error cases
              errno = retval;
              retval = -1;
              break;
           }
        }
           
        pthread_mutex_unlock(&mutex->mutex);
        return retval;
}

int pt_mutex_trylock(pt_mutex_t *mutex)
{
        int retval;
        pthread_mutex_lock(&mutex->mutex);
        if(!mutex->count)
        {
                mutex->count=1;
                mutex->thread=pthread_self();
                retval = 0;
        }
        else if(mutex->thread==pthread_self())
        {
                mutex->count++;
                retval = 0;
        }
        else
        {
           errno=EAGAIN;
           retval = -1;
        }

        pthread_mutex_unlock(&mutex->mutex);        
        return retval;
}

int pt_mutex_unlock(pt_mutex_t *mutex)
{
        pthread_mutex_lock(&mutex->mutex);

        if(mutex->count)
        {
                mutex->count--;
                if(!mutex->count)
                {
                   pthread_cond_broadcast(&mutex->cond);
                }
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
        assert(0 == (pthread_mutex_destroy(&mutex->mutex) | pthread_cond_destroy(&mutex->cond)));
        return 0;
}
