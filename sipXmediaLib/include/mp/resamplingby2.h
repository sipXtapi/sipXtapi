//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#if !defined _RESAMPBY2FIX
#define _RESAMPBY2FIX

/* Definitions of data types */
typedef int                GIPS_Word32;
typedef short int          GIPS_Word16;
typedef unsigned int       GIPS_UWord32;
typedef unsigned short int GIPS_UWord16;


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
