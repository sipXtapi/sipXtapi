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

#ifndef _MpDspUtilsShift_h_
#define _MpDspUtilsShift_h_

/**
*  @file
*
*  DO NOT INCLUDE THIS FILE DIRECTLY! This files is designed to be included
*  to <mp/MpDspUtils.h> and should not be used outside of it.
*/

/* ============================ INLINE METHODS ============================ */

void MpDspUtils::shl16_I(int32_t &a, unsigned scale)
{
   a = shl16(a, scale);
}

int32_t MpDspUtils::shl16(int32_t a, unsigned scale)
{
   const int32_t thresold = INT16_MAX>>scale;
   return (a>=thresold) ? INT16_MAX : 
                          a<-thresold ? -INT16_MAX :
                                        a<<scale;
}

void MpDspUtils::shl32_I(int32_t &a, unsigned scale)
{
   a = shl32(a, scale);
}

int32_t MpDspUtils::shl32(int32_t a, unsigned scale)
{
   const int32_t thresold = INT32_MAX>>scale;
   return (a>=thresold) ? INT32_MAX : 
                          a<-thresold ? -INT32_MAX :
                                        a<<scale;
}

#endif  // _MpDspUtilsShift_h_
