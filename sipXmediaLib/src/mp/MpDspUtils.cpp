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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpDspUtils.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* ========================= Arithmetic Functions ========================= */

#ifdef MP_FIXED_POINT // [

OsStatus MpDspUtils::add_IS(const int16_t *pSrc1, int32_t *pSrc2Dst, int dataLength, int src1ScaleFactor)
{
   if (src1ScaleFactor > 0)
   {
      const unsigned scale = src1ScaleFactor;
      for (int i=0; i<dataLength; i++)
      {
         pSrc2Dst[i] += ((int32_t)pSrc1[i])<<scale;
      }
   }
   else
   {
      const unsigned scale = -src1ScaleFactor;
      for (int i=0; i<dataLength; i++)
      {
         pSrc2Dst[i] += pSrc1[i]>>scale;
      }
   }

   return OS_SUCCESS;
}

OsStatus MpDspUtils::addMul_I(const int16_t *pSrc1, int16_t val, int32_t *pSrc2Dst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pSrc2Dst[i] += pSrc1[i]*val;
   }

   return OS_SUCCESS;
}

#else  // MP_FIXED_POINT ][

OsStatus MpDspUtils::add_I(const int16_t *pSrc1, float *pSrc2Dst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pSrc2Dst[i] += pSrc1[i];
   }

   return OS_SUCCESS;
}

OsStatus MpDspUtils::addMul_I(const int16_t *pSrc1, float val, float *pSrc2Dst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pSrc2Dst[i] += pSrc1[i]*val;
   }

   return OS_SUCCESS;
}

#endif // MP_FIXED_POINT ]

/* ========================= Conversion Functions ========================= */

#ifdef MP_FIXED_POINT // [

OsStatus MpDspUtils::convert_S(const int32_t *pSrc, int16_t *pDst, int dataLength, int srcScaleFactor)
{
   if (srcScaleFactor > 0)
   {
      const unsigned scale = srcScaleFactor;
      for (int i=0; i<dataLength; i++)
      {
         pDst[i] = pSrc[i]<<scale;
      }
   }
   else
   {
      const unsigned scale = -srcScaleFactor;
      for (int i=0; i<dataLength; i++)
      {
         pDst[i] = pSrc[i]>>scale;
      }
   }

   return OS_SUCCESS;
}

#else  // MP_FIXED_POINT ][

OsStatus MpDspUtils::convert(const float *pSrc, int16_t *pDst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = (int16_t)pSrc[i];
   }

   return OS_SUCCESS;
}

#endif // MP_FIXED_POINT ]

/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */

