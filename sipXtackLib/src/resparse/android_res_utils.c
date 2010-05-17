//
// Copyright (C) 2006-2010 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2010 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Author: Dan Petrie (dpetrie AT SIPez DOT com)
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <resparse/rr.h>
#include <resolv_private.h>
#include <resolv_static.h>

#if defined(ANDROID)
// Could not put this in SipSrvLookup for Android due to header file errors/conflicts in C++ code
void android_res_setDnsSrvTimeouts(int initialTimeoutInSecs, int retries)
{
    struct __res_state* staticResPtr = __res_get_state();
    assert(staticResPtr);

    if (initialTimeoutInSecs > 0)
    {
        staticResPtr->retrans = initialTimeoutInSecs;
    }

    if(retries > 0)
    {
        staticResPtr->retry = retries;
    }
}

void android_res_getDnsSrvTimeouts(int* initialTimeoutInSecs, int* retries)
{
    struct __res_state* staticResPtr = __res_get_state();
    assert(staticResPtr);

    *initialTimeoutInSecs = staticResPtr->retrans;
    *retries = staticResPtr->retry;
}

#endif

