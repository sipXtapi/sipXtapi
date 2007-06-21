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

#ifndef _MpDspUtils_h_
#define _MpDspUtils_h_

/// Switch between fixed and floating point math.
#define MP_FIXED_POINT
//#undef  MP_FIXED_POINT

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsIntTypes.h>
#ifdef MP_FIXED_POINT // [
#  include <math.h>
#endif // MP_FIXED_POINT ]

// DEFINES
// MACROS
#ifdef MP_FIXED_POINT // [
#  define MPF_FLOAT(num, intLength, fracLength) (floor((num)*(float)(1<<(fracLength))))
#else  // MP_FIXED_POINT ][
#  define MPF_FLOAT(num, intLength, fracLength) (num)
#endif // MP_FIXED_POINT ]

#define MPF_STEP(intLength, fracLength)    MPF_FLOAT(1.0/(1<<(fracLength)), (intLength), (fracLength))
#define MPF_MAX(intLength, fracLength)     MPF_FLOAT((1<<(intLength))-1.0/(1<<(fracLength)), (intLength), (fracLength))


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Class for generic DSP functions.
*
*  This is utils class, containing only static functions. It provide all DSP
*  processing primitives we need, and meant to be good optimized. In fact,
*  we want this to be only place, which will contain hardware specific
*  speed optimizations. If you could not find what you need here - feel free
*  to add it. But do not forget to add generic C-version of each function,
*  so it will be available if there is no optimized version.
*
*  @warning Please, keep all methods of this class static, and stateless!
*/
class MpDspUtils
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* ===================== Logical and Shift Functions ====================== */
///@name Logical and Shift Functions
//@{


//@}

/* ========================= Arithmetic Functions ========================= */
///@name Arithmetic Functions
//@{

#ifdef MP_FIXED_POINT // [

     /// Scale source vector and add it to accumulator.
   static OsStatus add_IS(const int16_t *pSrc1, int32_t *pSrc2Dst, int dataLength, int src1ScaleFactor);

     /// Multiply source vector by given \p val and add it to accumulator.
   static OsStatus addMul_I(const int16_t *pSrc1, int16_t val, int32_t *pSrc2Dst, int dataLength);

#else  // MP_FIXED_POINT ][

     /// Add source vector to accumulator.
   static OsStatus add_I(const int16_t *pSrc1, float *pSrc2Dst, int dataLength);

     /// Stub for fixed-point mode compatibility. It DOES NOT scale values.
   static inline OsStatus add_IS(const int16_t *pSrc1, float *pSrc2Dst, int dataLength, int src1ScaleFactor)
   {
      return add_I(pSrc1, pSrc2Dst, dataLength);
   }

     /// Multiply source vector by given \p val and add it to accumulator.
   static OsStatus addMul_I(const int16_t *pSrc1, float val, float *pSrc2Dst, int dataLength);

#endif // MP_FIXED_POINT ]

//@}

/* ========================= Conversion Functions ========================= */
///@name Conversion Functions
//@{

#ifdef MP_FIXED_POINT // [

     /// Scale source vector and convert to destination.
   static OsStatus convert_S(const int32_t *pSrc, int16_t *pDst, int dataLength, int srcScaleFactor);

#else  // MP_FIXED_POINT ][

     /// Convert type of source vector.
   static OsStatus convert(const float *pSrc, int16_t *pDst, int dataLength);

     /// Stub for fixed-point mode compatibility. It DOES NOT scale values.
   static inline OsStatus convert_S(const float *pSrc, int16_t *pDst, int dataLength, int srcScaleFactor)
   {
      return convert(pSrc, pDst, dataLength);
   }

#endif // MP_FIXED_POINT ]

//@}

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDspUtils_h_
