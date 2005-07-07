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

#ifndef DSP_TYPE   /* [ */
#define DSP_TYPE

#if (defined(_VXWORKS) || defined(__linux__) || defined(sun)) /* [ */
typedef long long int Word64S;
typedef unsigned long long int Word64;

#elif defined(WIN32) /* ] [ */
typedef __int64 Word64S;
typedef unsigned __int64 Word64;

#else /* ] [ */
#error "What are Word64, Word64S on this system???"
#endif /* others ] */

#define Word32S         int
#define Word32          unsigned int

#endif  /* DSP_TYPE ] */
