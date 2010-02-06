//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsRWMutex_h_
#define _OsRWMutex_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "utl/UtlDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Mutual exclusion semaphore handling multiple readers and writers
// Two kinds of concurrent tasks, called "readers" and "writers", share a
// single resource. The readers can use the resource simultaneously, but each
// writer must have exclusive access to it. When a writer is ready to use the
// resource, it should be enabled to do so as soon as possible.

/// Depending on the native OS that we are running on, we include the class
/// declaration for the appropriate lower level implementation and use a
/// "typedef" statement to associate the OS-independent class name (OsRWMutex)
/// with the OS-dependent realization of that type (e.g., OsRWMutexWnt).
#if defined(_WIN32)
#  include "os/shared/OsRWMutexShared.h"
   typedef class OsRWMutexShared OsRWMutex;
#elif defined(_VXWORKS) || defined(ANDROID)
#  include "os/shared/OsRWMutexShared.h"
   typedef class OsRWMutexShared OsRWMutex;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsRWMutexLinux.h"
   typedef class OsRWMutexLinux OsRWMutex;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsRWMutex_h_
