//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#if !defined _RESAMPBY2FIX
#define _RESAMPBY2FIX

#include <os/OsIntTypes.h>

/* Please, DO NOT USE GIPS_* types in new code! */
/* All entries of this types in existing code will be removed some day. */
typedef int32_t            GIPS_Word32;
typedef int16_t            GIPS_Word16;
typedef uint32_t           GIPS_UWord32;
typedef uint16_t           GIPS_UWord16;


#ifdef __cplusplus
extern "C" {
#endif

/* decimator */
void GIPS_downsampling2(
   const GIPS_Word16* in,               /* input array */
   int                len,              /* length of input array */
   GIPS_Word16*       out,              /* output array (of length len/2) */
   GIPS_Word32*       filtState         /* filter state array; length = 8 */
);

void GIPS_upsampling2(
   const GIPS_Word16* in,               /* input array */
   int                len,              /* length of input array */
   GIPS_Word16*       out,              /* output array (of length len*2) */
   GIPS_Word32*       filtState         /* filter state array; length = 8 */
);

#ifdef __cplusplus
}
#endif

#endif
