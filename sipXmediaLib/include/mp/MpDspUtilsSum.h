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

#ifndef _MpDspUtilsSum_h_
#define _MpDspUtilsSum_h_

/**
*  @file
*
*  DO NOT INCLUDE THIS FILE DIRECTLY! This files is designed to be included
*  to <mp/MpDspUtils.h> and should not be used outside of it.
*/

/* ============================ INLINE METHODS ============================ */

int16_t MpDspUtils::add(int16_t a, int16_t b)
{
   return MPF_EXTRACRT16(MPF_SATURATE16(a+b));
}

void MpDspUtils::add_I(int16_t &a, int16_t b)
{
   a = add(a, b);
}

void MpDspUtils::addMul_I(int32_t &a, int16_t b, int16_t c)
{
   a = add(a, b*c);
}

int32_t MpDspUtils::add(int32_t a, int32_t b)
{
#ifdef ARMv5E_ASM
   int32_t tmp;
   __asm__ __volatile__ ("qadd  %0,%1,%2;\n"
            : "=&r"(tmp)
            : "r"(a),"r"(b));
   return tmp;
#else
   int32_t c;
   c = a + b;

   if (((a ^ b) & MPF_SIGN_BIT32) == 0)
   {
      // a and b have same sign
      if ((c ^ a) & MPF_SIGN_BIT32)
      {
         // c have different signs then a and b, that is addition wrapped.
         c = (a < 0) ? (INT32_MIN+1) : INT32_MAX;
      }
   }
   if (c == INT32_MIN)
   {
      // Saturate to (INT32_MIN+1). We do not work with INT32_MIN,
      // because -INT32_MIN=INT32_MIN and thus break negation.
      c = INT32_MIN+1;
   }
   return c;
#endif
}

void MpDspUtils::add_I(int32_t &a, int32_t b)
{
   a = add(a, b);
}

#ifndef MP_FIXED_POINT // [

float MpDspUtils::add(float a, float b)
{
   return a+b;
}

void MpDspUtils::add_I(float &a, float b)
{
   a = add(a, b);
}

void MpDspUtils::addMul_I(float &a, float b, float c)
{
   a = add(a, b*c);
}

#endif // MP_FIXED_POINT ]

int16_t MpDspUtils::abs(int16_t a)
{
   return (a < 0) ? 
      ((a == INT16_MIN) ? INT16_MAX : -a) : a;
}

int16_t MpDspUtils::minimum(int16_t a, int16_t b)
{
   return (a < b) ? a : b;
}

int16_t MpDspUtils::maximum(int16_t a, int16_t b)
{
   return (a > b) ? a : b;
}

int32_t MpDspUtils::abs(int32_t a)
{
   return (a < 0) ? 
      ((a == INT32_MIN) ? INT32_MAX : -a) : a;
}

int32_t MpDspUtils::minimum(int32_t a, int32_t b)
{
   return (a < b) ? a : b;
}

int32_t MpDspUtils::maximum(int32_t a, int32_t b)
{
   return (a > b) ? a : b;
}

#endif  // _MpDspUtilsSum_h_
