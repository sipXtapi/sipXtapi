//
// Copyright (C) 2006-2013 SIPez LLC. All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsTaskId_h_
#define _OsTaskId_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// ENUMS

// TYPEDEFS
#if defined(WIN32)
   typedef int OsTaskId_t; // Not sure this is proper for this platform at least on 64bit ...
#elif defined(_VXWORKS)
   typedef intptr_t OsTaskId_t; // Not sure this is proper for this platform...
#elif defined(__pingtel_on_posix__)
   typedef pthread_t OsTaskId_t;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsTaskId_h_