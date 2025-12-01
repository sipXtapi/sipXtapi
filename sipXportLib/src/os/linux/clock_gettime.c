//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


/*
 * NOTE: This file historically defined clock_gettime to avoid linking with librt
 * on older Linux or macOS systems. On modern platforms:
 *
 * 1. Linux (glibc ≥ 2.17) provides clock_gettime natively, no -lrt required.
 * 2. MacOS (10.12+) provides clock_gettime natively.
 *
 * Defining your own clock_gettime on Linux is dangerous because it overrides
 * the system implementation. This breaks C++ standard library components
 * that rely on the correct system behavior, particularly std::condition_variable.
 *
 * std::condition_variable::wait_for and wait_until internally call clock_gettime
 * for timed waits. If clock_gettime is overridden, timed waits can fail,
 * return immediately, or never wake — causing serious bugs.
 *
 * Therefore, we only define clock_gettime if it is not already defined.
 */
 

#include <time.h>
#include <sys/time.h>

#ifndef clock_gettime
#define NEED_CLOCK_GETTIME
#endif

#ifdef NEED_CLOCK_GETTIME
int clock_gettime(clockid_t clk_id, struct timespec* tp)
{
    struct timeval now;
    gettimeofday(&now, NULL);

    tp->tv_sec  = now.tv_sec;
    tp->tv_nsec = now.tv_usec * 1000;

    return 0;
}
#endif
