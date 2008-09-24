//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpDspUtilsSerials_h_
#define _MpDspUtilsSerials_h_

/**
*  @file
*
*  DO NOT INCLUDE THIS FILE DIRECTLY! This files is designed to be included
*  to <mp/MpDspUtils.h> and should not be used outside of it.
*/

// DEFINES
#define UINT32_HALF  UINT32_C(2147483648) ///< Half of unsigned 32-bit integer range (2^31)
#define UINT16_HALF  UINT16_C(32768)      ///< Half of unsigned 16-bit integer range (2^15)

// MACROS
#define MP_COMPARE_SERIALS(val1, val2, range_half) \
   (((val1) == (val2)) ? 0 : \
                     ((val1) >= (range_half)) ? ((((val1)-(range_half) < (val2)) && ((val2) < (val1))) ? 1 : -1) : \
                                            ((((val1)+(range_half) < (val2)) || ((val2) < (val1))) ? 1 : -1))

/* ============================ INLINE METHODS ============================ */

int MpDspUtils::compareSerials(uint32_t val1, uint32_t val2)
{
   return MP_COMPARE_SERIALS(val1, val2, UINT32_HALF);
}

int MpDspUtils::compareSerials(uint16_t val1, uint16_t val2)
{
   return MP_COMPARE_SERIALS(val1, val2, UINT16_HALF);
}

#endif  // _MpDspUtilsSerials_h_
