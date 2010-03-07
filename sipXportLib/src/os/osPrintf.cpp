//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <os/OsDefs.h>
#include "utl/UtlString.h"

#ifdef _WIN32
#include <assert.h>
#endif

// EXTERNAL FUNCTIONS

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void PrintIt(const char *s)
{
    printf("%s",s);
    OutputDebugString(s);
}

#elif defined(ANDROID)
#include <android/log.h>
void PrintIt(const char *s)
{
//   printf("%s",s);
   __android_log_print(ANDROID_LOG_VERBOSE, "sipXprintf", "%s",s);
}
#else
#define PrintIt(x) printf("%s", (char *) (x))
#endif

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
static volatile int bEnableConsoleOutput = FALSE ;  /**< Should osPrintf print to console? */
// FUNCTIONS


// Layer of indirection over printf

extern "C" void enableConsoleOutput(int bEnable) 
{
    bEnableConsoleOutput = bEnable ;
}

extern "C" void osPrintf(const char* format, ...)
{
    if (bEnableConsoleOutput)
    {
        va_list args;
        va_start(args, format);

        /* Guess we need no more than 128 bytes. */
        int n, size = 128;
        char *p;

        p = (char*) malloc(size) ;
     
        while (p != NULL)
        {
            va_list tmp;
            /* Argument list must be copied, because we call vsnprintf in a loop
            * and first call will invalidate list, so on second iteration it
            * will contain junk. (this is not a problem for i386, but appears
            * as such on e.g. x86_64) */
            va_copy(tmp, args);
            /* Try to print in the allocated space. */
            n = vsnprintf(p, size, format, tmp);
            va_end(tmp);

            /* If that worked, return the string. */
            if (n > -1 && n < size)
            {
                break;
            }
            /* Else try again with more space. */
            if (n > -1)    /* glibc 2.1 */
                size = n+1; /* precisely what is needed */
            else           /* glibc 2.0 */
                size *= 2;  /* twice the old size */

            if ((p = (char*) realloc (p, size)) == NULL)
            {
                break;
            }
        }

        if (p != NULL)
        {
            PrintIt(p) ;
            free(p) ;
        }

        va_end(args) ;
    }
}

