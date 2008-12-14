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

#ifndef _MpDspUtilsSumVect_h_
#define _MpDspUtilsSumVect_h_

/**
*  @file
*
*  DO NOT INCLUDE THIS FILE DIRECTLY! This files is designed to be included
*  to <mp/MpDspUtils.h> and should not be used outside of it.
*/

/* ============================ INLINE METHODS ============================ */

#ifdef MP_FIXED_POINT // [

OsStatus MpDspUtils::add_I(const int16_t *pSrc1, int32_t *pSrc2Dst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      add_I(pSrc2Dst[i], (int32_t)pSrc1[i]);
   }
   return OS_SUCCESS;
}

OsStatus MpDspUtils::add_IGain(const int16_t *pSrc1, int32_t *pSrc2Dst, int dataLength, unsigned src1ScaleFactor)
{
   for (int i=0; i<dataLength; i++)
   {
      add_I(pSrc2Dst[i], ((int32_t)pSrc1[i])<<src1ScaleFactor);
   }

   return OS_SUCCESS;
}

OsStatus MpDspUtils::add_IAtt(const int16_t *pSrc1, int32_t *pSrc2Dst, int dataLength, unsigned src1ScaleFactor)
{
   for (int i=0; i<dataLength; i++)
   {
      add_I(pSrc2Dst[i], (int32_t)pSrc1[i]>>src1ScaleFactor);
   }

   return OS_SUCCESS;
}

OsStatus MpDspUtils::add(const int32_t *pSrc1, const int32_t *pSrc2, int32_t *pDst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = add(pSrc1[i], pSrc2[i]);
   }
   return OS_SUCCESS;
}

OsStatus MpDspUtils::addMul_I(const int16_t *pSrc1, int16_t val, int32_t *pSrc2Dst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      addMul_I(pSrc2Dst[i], pSrc1[i], val);
   }

   return OS_SUCCESS;
}

OsStatus MpDspUtils::addMulLinear_I(const int16_t *pSrc1, int16_t valStart, int16_t valEnd,
                                    int32_t *pSrc2Dst, int dataLength)
{
   // TODO:: This works fine only when (dataLength << (valStart - valEnd)).
   //        In other case we need smarter step value calculation, e.g.
   //        http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
   //        Or at least we need to distinguish the case when
   //        (valStart - valEnd) < dataLength.
   int16_t step = (valEnd - valStart) / dataLength;
   int16_t val = valStart;

   for (int i=0; i<dataLength; i++, val += step)
   {
      addMul_I(pSrc2Dst[i], pSrc1[i], val);
   }
   return OS_SUCCESS;
}

OsStatus MpDspUtils::mul(const int16_t *pSrc, const int16_t val, int32_t *pDst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = pSrc[i]*val;
   }

   return OS_SUCCESS;
}

OsStatus MpDspUtils::mulLinear(const int16_t *pSrc, int16_t valStart, int16_t valEnd,
                               int32_t *pDst, int dataLength)
{
   // TODO:: This works fine only when (dataLength << (valStart - valEnd)).
   //        In other case we need smarter step value calculation, e.g.
   //        http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
   //        Or at least we need to distinguish the case when
   //        (valStart - valEnd) < dataLength.
   int16_t step = (valEnd - valStart) / dataLength;
   int16_t val = valStart;

   for (int i=0; i<dataLength; i++, val += step)
   {
      pDst[i] = pSrc[i] * val;
   }
   return OS_SUCCESS;
}

#else  // MP_FIXED_POINT ][

OsStatus MpDspUtils::add_I(const int16_t *pSrc1, float *pSrc2Dst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      add_I(pSrc2Dst[i], pSrc1[i]);
   }

   return OS_SUCCESS;
}

OsStatus MpDspUtils::add(const float *pSrc1, const float *pSrc2, float *pDst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = add(pSrc1[i], pSrc2[i]);
   }

   return OS_SUCCESS;
}

OsStatus MpDspUtils::addMul_I(const int16_t *pSrc1, float val, float *pSrc2Dst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      add_I(pSrc2Dst[i], pSrc1[i]*val);
   }

   return OS_SUCCESS;
}

OsStatus MpDspUtils::addMulLinear_I(const int16_t *pSrc1, float valStart, float valEnd,
                                    float *pSrc2Dst, int dataLength)
{
   float step = (valEnd - valStart) / dataLength;
   float val = valStart;

   for (int i=0; i<dataLength; i++, val += step)
   {
      addMul_I(pSrc2Dst[i], pSrc1[i], val);
   }
   return OS_SUCCESS;
}

OsStatus MpDspUtils::mul(const int16_t *pSrc, const float val, float *pDst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = pSrc[i]*val;
   }

   return OS_SUCCESS;
}

OsStatus MpDspUtils::mulLinear(const int16_t *pSrc, float valStart, float valEnd,
                               float *pDst, int dataLength)
{
   // TODO:: This works fine only when (dataLength << (valStart - valEnd)).
   //        In other case we need smarter step value calculation, e.g.
   //        http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
   //        Or at least we need to distinguish the case when
   //        (valStart - valEnd) < dataLength.
   float step = (valEnd - valStart) / dataLength;
   float val = valStart;

   for (int i=0; i<dataLength; i++, val += step)
   {
      pDst[i] = pSrc[i] * val;
   }
   return OS_SUCCESS;
}

#endif // MP_FIXED_POINT ]

int MpDspUtils::maxAbs(const int16_t *pSrc, int dataLength)
{
   int16_t startValue = pSrc[0];
   if (startValue < 0)
      startValue = -startValue;

   for (int i = 1; i < dataLength; i++)
   {
      startValue = maximum(startValue, 
         (int16_t)( (pSrc[i] > 0) ? pSrc[i] : -(pSrc[i]) ));
   }
   return startValue;
}

int16_t MpDspUtils::maximum(const int16_t *pSrc, int dataLength)
{
   int16_t val = pSrc[0];
   for (int i = 0; i < dataLength; i++)
      if (pSrc[i] > val) 
         val = pSrc[i];

   return val;
}

int32_t MpDspUtils::maximum(const int32_t *pSrc, int dataLength)
{
   int32_t val = pSrc[0];
   for (int i = 0; i < dataLength; i++)
      if (pSrc[i] > val) 
         val = pSrc[i];

   return val;
}

int16_t MpDspUtils::minimum(const int16_t *pSrc, int dataLength)
{
   int16_t val = pSrc[0];
   for (int16_t i = 0; i < dataLength; i++)
      if (pSrc[i] < val) 
         val = pSrc[i];

   return val;
}

int32_t MpDspUtils::minimum(const int32_t *pSrc, int dataLength)
{
   int32_t val = pSrc[0];
   for (int32_t i = 0; i < dataLength; i++)
      if (pSrc[i] < val) 
         val = pSrc[i];

   return val;
}

#endif  // _MpDspUtilsSumVect_h_
