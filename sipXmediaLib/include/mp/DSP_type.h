//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef DSP_TYPE   /* [ */
#define DSP_TYPE

#if (defined(_VXWORKS) || defined(__pingtel_on_posix__)) /* [ */
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
