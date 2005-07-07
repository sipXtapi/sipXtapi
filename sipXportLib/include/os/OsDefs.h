// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _OsDefs_h_
#define _OsDefs_h_

// SYSTEM INCLUDES
#ifdef _VXWORKS
#include "os/Vxw/OsVxwDefs.h"
#endif // _VXWORKS
#ifdef __pingtel_on_posix__
#include "os/linux/OsLinuxDefs.h"
#endif // __pingtel_on_posix__

// APPLICATION INCLUDES
// MACROS
// EXTERNAL FUNCTIONS
// DEFINES

#ifdef __cplusplus
extern "C" {
#endif

void enableConsoleOutput(int bEnable) ;
void osPrintf(const char* format , ...)
#ifdef __GNUC__
            // with the -Wformat switch, this enables format string checking
            __attribute__ ((format (printf, 1, 2)))
#endif
         ;
         

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#ifdef __cplusplus
}
#endif

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "time.h"

#endif  // _OsDefs_h_

