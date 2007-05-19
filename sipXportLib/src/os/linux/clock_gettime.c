//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <time.h>
#include <sys/time.h>

/* This function comes from librt.so. We would prefer not to need that library
 * but the RTCP code uses clock_gettime(). Rather than rewrite that code, we
 * just implement clock_gettime() using gettimeofday(). */

#ifdef __MACH__
/* we don't even have the typedef on OS X */
typedef int clockid_t;
#endif

int clock_gettime(clockid_t clock_id, struct timespec * tp)
{
        struct timeval tv;
        if(gettimeofday(&tv, NULL))
                return -1;
        tp->tv_sec = tv.tv_sec;
        tp->tv_nsec = tv.tv_usec * 1000;
        return 0;
}
