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

#ifndef _MpDspUtilsConvertVect_h_
#define _MpDspUtilsConvertVect_h_

/**
*  @file
*
*  DO NOT INCLUDE THIS FILE DIRECTLY! This files is designed to be included
*  to <mp/MpDspUtils.h> and should not be used outside of it.
*/

/* ============================ INLINE METHODS ============================ */

#ifdef MP_FIXED_POINT // [

OsStatus MpDspUtils::convert(const int32_t *pSrc, int16_t *pDst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = MPF_EXTRACRT16(MPF_SATURATE16(pSrc[i]));
   }
   return OS_SUCCESS;
}

OsStatus MpDspUtils::convert_Gain(const int32_t *pSrc, int16_t *pDst, int dataLength, unsigned srcScaleFactor)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = MPF_EXTRACRT16(shl16(pSrc[i], srcScaleFactor));
   }
   return OS_SUCCESS;
}

OsStatus MpDspUtils::convert_Att(const int32_t *pSrc, int16_t *pDst, int dataLength, unsigned srcScaleFactor)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = MPF_EXTRACRT16(MPF_SATURATE16(pSrc[i]>>srcScaleFactor));
   }
   return OS_SUCCESS;
}

OsStatus MpDspUtils::convert(const int16_t *pSrc, int32_t *pDst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = pSrc[i];
   }
   return OS_SUCCESS;
}

OsStatus MpDspUtils::convert_Gain(const int16_t *pSrc, int32_t *pDst, int dataLength, unsigned srcScaleFactor)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = shl32((int32_t)pSrc[i], srcScaleFactor);
   }
   return OS_SUCCESS;
}

OsStatus MpDspUtils::convert_Att(const int16_t *pSrc, int32_t *pDst, int dataLength, unsigned srcScaleFactor)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = pSrc[i]>>srcScaleFactor;
   }
   return OS_SUCCESS;
}

#else  // MP_FIXED_POINT ][

OsStatus MpDspUtils::convert(const float *pSrc, int16_t *pDst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = MPF_EXTRACRT16(MPF_SATURATE16(pSrc[i]));
   }

   return OS_SUCCESS;
}

#endif // MP_FIXED_POINT ]

#endif  // _MpDspUtilsConvertVect_h_
