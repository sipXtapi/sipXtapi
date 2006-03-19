// 
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////


#ifndef _UtlDefs_h_
#define _UtlDefs_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
#ifndef FALSE
#define FALSE (1==0)
#endif

#ifndef TRUE
#define TRUE (1==1)
#endif

#ifndef NULL
#define NULL 0
#endif

#define UTL_NOT_FOUND ((size_t)-1)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef int UtlBoolean ;
#ifdef DONT_USE_LONG_LONG
typedef long int intll;
#else
typedef long long intll;
#endif
typedef const char* const UtlContainableType ;

// FORWARD DECLARATIONS

#endif // _UtlDefs_h_

