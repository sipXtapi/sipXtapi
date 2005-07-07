#ifndef SIPX_GLIB_STUBS_H
#define SIPX_GLIB_STUBS_H
/*
#define G_LOCK(MUTEX)  utlSipxLock(MUTEX)

#define G_UNLOCK(MUTEX) utlSipxUnlock(MUTEX)

#define G_LOCK_DEFINE_STATIC(MUTEX) static UltSipxMutex MUTEX = \
   createUtlSipxMutext();

typedef void* UltSipxMutex;
void utlSipxLock(UltSipxMutex mutex);
void utlSipxUnlock(UltSipxMutex mutex);
UltSipxMutex createUtlSipxMutext();
*/


void g_get_current_time		        (GTimeVal	*result);


#endif
