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
      add_I(pSrc2Dst[i], pSrc1[i]*val);
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

OsStatus MpDspUtils::mul(const int16_t *pSrc, const float val, float *pDst, int dataLength)
{
   for (int i=0; i<dataLength; i++)
   {
      pDst[i] = pSrc[i]*val;
   }

   return OS_SUCCESS;
}

#endif // MP_FIXED_POINT ]

/* ========================= Conversion Functions ========================= */

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

/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */

