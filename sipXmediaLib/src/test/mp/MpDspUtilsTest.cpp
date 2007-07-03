//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpDspUtils.h>

/**
 * Unittest for MpDspUtils
 */
class MpDspUtilsTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpDspUtilsTest);
   CPPUNIT_TEST(testNegate_int16);
   CPPUNIT_TEST(testNegate_int32);
   CPPUNIT_TEST(testShiftLeft_int32);
   CPPUNIT_TEST(testAdd_int16);
   CPPUNIT_TEST(testAdd_I_int16);
   CPPUNIT_TEST(testAdd_int32);
   CPPUNIT_TEST(testAdd_I_int32);
#ifndef MP_FIXED_POINT // [
   CPPUNIT_TEST(testAdd_float);
   CPPUNIT_TEST(testAdd_I_float);
#endif // !MP_FIXED_POINT ]
#ifdef MP_FIXED_POINT // [
   CPPUNIT_TEST(testAdd_I_int16_int32_vector);
   CPPUNIT_TEST(testAdd_IGain_int16_int32_vector);
   CPPUNIT_TEST(testAdd_IAtt_int16_int32_vector);
   CPPUNIT_TEST(testAddMul_I_int16_int32_vector);
#else  // MP_FIXED_POINT ][
   CPPUNIT_TEST(testAdd_I_int16_float_vector);
   CPPUNIT_TEST(testAddMul_I_int16_float_vector);
#endif // MP_FIXED_POINT ]
#ifdef MP_FIXED_POINT // [
   CPPUNIT_TEST(testConvert_int32_int16);
   CPPUNIT_TEST(testConvert_Gain_int32_int16);
   CPPUNIT_TEST(testConvert_Att_int32_int16);
#else  // MP_FIXED_POINT ][
   CPPUNIT_TEST(testConvert_float_int16);
#endif // MP_FIXED_POINT ]
   CPPUNIT_TEST_SUITE_END();

public:

   void testNegate_int16()
   {
      // Try to negate minimum 16-bit integer.
      int16_t a = INT16_MIN;
      int16_t b = -a;

#if defined(__i386__) || defined(_M_IX86)
      // x86 CPUs behave strange in this case. Ensure that this is true for
      // this CPU too, though.
      CPPUNIT_ASSERT_EQUAL((int16_t)INT16_MIN, b);
#else
      // Unknown platform... testing for desired behaviour.
      CPPUNIT_ASSERT_EQUAL(INT16_MAX, b);
#endif
   }

   void testNegate_int32()
   {
      // Try to negate minimum 16-bit integer.
      int32_t a = INT32_MIN;
      int32_t b = -a;

#if defined(__i386__) || defined(_M_IX86)
      // x86 CPUs behave strange in this case. Ensure that this is true for
      // this CPU too, though.
      CPPUNIT_ASSERT_EQUAL(INT32_MIN, b);
#else
      // Unknown platform... testing for desired behaviour.
      CPPUNIT_ASSERT_EQUAL(INT32_MAX, b);
#endif
   }

   void testShiftLeft_int32()
   {
      int32_t a[] = {-2147483647-1, -2147483647, 2147483647};
      CPPUNIT_ASSERT_EQUAL(INT32_C(0), a[0]<<1);
      CPPUNIT_ASSERT_EQUAL(INT32_C(2), a[1]<<1);
      CPPUNIT_ASSERT_EQUAL(INT32_C(-2), a[2]<<1);
   }

   void testAdd_int16()
   {
      for (int i=0; i<ADD_INT16_TEST_LENGTH; i++)
      {
         for (int j=0; j<ADD_INT16_TEST_LENGTH; j++)
         {
            CPPUNIT_ASSERT_EQUAL(add_int16_res[i][j],
                                 MpDspUtils::add(add_int16_src[i], add_int16_src[j]));
         }
      }
   }

   void testAdd_I_int16()
   {
      for (int i=0; i<ADD_INT16_TEST_LENGTH; i++)
      {
         for (int j=0; j<ADD_INT16_TEST_LENGTH; j++)
         {
            int16_t a = add_int16_src[i];
            MpDspUtils::add_I(a, add_int16_src[j]);
            CPPUNIT_ASSERT_EQUAL(add_int16_res[i][j], a);
         }
      }
   }

   void testAdd_int32()
   {
      for (int i=0; i<ADD_INT32_TEST_LENGTH; i++)
      {
//         printf("{");
         for (int j=0; j<ADD_INT32_TEST_LENGTH; j++)
         {
            CPPUNIT_ASSERT_EQUAL(add_int32_res[i][j],
                                 MpDspUtils::add(add_int32_src[i], add_int32_src[j]));
//            printf("%12d,", MpDspUtils::add(add_int32_src[i], add_int32_src[j]));
         }
//         printf("},\n");
      }
   }

   void testAdd_I_int32()
   {
      for (int i=0; i<ADD_INT32_TEST_LENGTH; i++)
      {
         for (int j=0; j<ADD_INT32_TEST_LENGTH; j++)
         {
            int32_t a = add_int32_src[i];
            MpDspUtils::add_I(a, add_int32_src[j]);
            CPPUNIT_ASSERT_EQUAL(add_int32_res[i][j], a);
         }
      }
   }

#ifndef MP_FIXED_POINT // [

   void testAdd_float()
   {
      for (int i=0; i<ADD_FLOAT_TEST_LENGTH; i++)
      {
//         printf("{");
         for (int j=0; j<ADD_FLOAT_TEST_LENGTH; j++)
         {
            CPPUNIT_ASSERT_EQUAL(add_float_res[i][j],
                                 MpDspUtils::add(add_float_src[i], add_float_src[j]));
//            printf("%15.1ff,", MpDspUtils::add(add_float_src[i], add_float_src[j]));
         }
//         printf("},\n");
      }
   }

   void testAdd_I_float()
   {
      for (int i=0; i<ADD_FLOAT_TEST_LENGTH; i++)
      {
         for (int j=0; j<ADD_FLOAT_TEST_LENGTH; j++)
         {
            float a = add_float_src[i];
            MpDspUtils::add_I(a, add_float_src[j]);
            CPPUNIT_ASSERT_EQUAL(add_float_res[i][j], a);
         }
      }
   }

#endif // !MP_FIXED_POINT ]

#ifdef MP_FIXED_POINT // [

   void testAdd_I_int16_int32_vector()
   {
      for (int i=0; i<ADD_INT16_INT32_TEST_LENGTH; i++)
      {
         int32_t pSrcDst[ADD_INT16_INT32_TEST_LENGTH];
         memcpy(pSrcDst, &add_int16_int32_vec_src2[i], ADD_INT16_INT32_TEST_LENGTH*sizeof(int32_t));
         MpDspUtils::add_I(add_int16_int32_vec_src1, pSrcDst, ADD_INT16_INT32_TEST_LENGTH);
//         printf("{");
         for (int j=0; j<ADD_INT16_INT32_TEST_LENGTH; j++)
         {
            CPPUNIT_ASSERT_EQUAL(add_int16_int32_vec_res[i][j], pSrcDst[j]);
//            printf("%12d, ", pSrcDst[j]);
         }
//         printf("},\n");
      }
   }

   void testAdd_IGain_int16_int32_vector()
   {
      for (int i=0; i<ADD_INT16_INT32_TEST_LENGTH; i++)
      {
         int32_t pSrcDst[ADD_INT16_INT32_TEST_LENGTH];
         memcpy(pSrcDst, &add_int16_int32_vec_src2[i], ADD_INT16_INT32_TEST_LENGTH*sizeof(int32_t));
         MpDspUtils::add_IGain(add_int16_int32_vec_src1, pSrcDst, ADD_INT16_INT32_TEST_LENGTH, 16);
//         printf("{");
         for (int j=0; j<ADD_INT16_INT32_TEST_LENGTH; j++)
         {
            CPPUNIT_ASSERT_EQUAL(add_Gain_int16_int32_vec_res[i][j], pSrcDst[j]);
//            printf("%11d, ", pSrcDst[j]);
         }
//         printf("},\n");
      }
   }

   void testAdd_IAtt_int16_int32_vector()
   {
      for (int i=0; i<ADD_INT16_INT32_TEST_LENGTH; i++)
      {
         int32_t pSrcDst[ADD_INT16_INT32_TEST_LENGTH];
         memcpy(pSrcDst, &add_int16_int32_vec_src2[i], ADD_INT16_INT32_TEST_LENGTH*sizeof(int32_t));
         MpDspUtils::add_IAtt(add_int16_int32_vec_src1, pSrcDst, ADD_INT16_INT32_TEST_LENGTH, 1);
//         printf("{");
         for (int j=0; j<ADD_INT16_INT32_TEST_LENGTH; j++)
         {
            CPPUNIT_ASSERT_EQUAL(add_Att_int16_int32_vec_res[i][j], pSrcDst[j]);
//            printf("%11d, ", pSrcDst[j]);
         }
//         printf("},\n");
      }
   }

   void testAddMul_I_int16_int32_vector()
   {
      for (int i=0; i<ADD_INT16_INT32_TEST_LENGTH; i++)
      {
         int32_t pSrcDst[ADD_INT16_INT32_TEST_LENGTH];
         memcpy(pSrcDst, &add_int16_int32_vec_src2[i], ADD_INT16_INT32_TEST_LENGTH*sizeof(int32_t));
         MpDspUtils::addMul_I(add_int16_int32_vec_src1, INT16_C(65535), pSrcDst, ADD_INT16_INT32_TEST_LENGTH);
//         printf("{");
         for (int j=0; j<ADD_INT16_INT32_TEST_LENGTH; j++)
         {
//            printf("%11d, ", pSrcDst[j]);
            CPPUNIT_ASSERT_EQUAL(addMul_int16_int32_vec_res[i][j], pSrcDst[j]);
         }
//         printf("},\n");
      }
   }

#else  // MP_FIXED_POINT ][

   void testAdd_I_int16_float_vector()
   {
      for (int i=0; i<ADD_INT16_INT32_TEST_LENGTH; i++)
      {
         float pSrcDst[ADD_INT16_INT32_TEST_LENGTH];
         memcpy(pSrcDst, &add_int16_float_vec_src2[i], ADD_INT16_INT32_TEST_LENGTH*sizeof(int32_t));
         MpDspUtils::add_I(add_int16_int32_vec_src1, pSrcDst, ADD_INT16_INT32_TEST_LENGTH);
//         printf("{");
         for (int j=0; j<ADD_INT16_INT32_TEST_LENGTH; j++)
         {
            CPPUNIT_ASSERT_EQUAL(add_int16_float_vec_res[i][j], pSrcDst[j]);
//            printf("%13.1ff, ", pSrcDst[j]);
         }
//         printf("},\n");
      }
   }

   void testAddMul_I_int16_float_vector()
   {
      for (int i=0; i<ADD_INT16_INT32_TEST_LENGTH; i++)
      {
         float pSrcDst[ADD_INT16_INT32_TEST_LENGTH];
         memcpy(pSrcDst, &add_int16_float_vec_src2[i], ADD_INT16_INT32_TEST_LENGTH*sizeof(int32_t));
         MpDspUtils::addMul_I(add_int16_int32_vec_src1, 65535, pSrcDst, ADD_INT16_INT32_TEST_LENGTH);
//         printf("{");
         for (int j=0; j<ADD_INT16_INT32_TEST_LENGTH; j++)
         {
//            printf("%13.1ff, ", pSrcDst[j]);
            CPPUNIT_ASSERT_EQUAL(addMul_int16_float_vec_res[i][j], pSrcDst[j]);
         }
//         printf("},\n");
      }
   }

#endif // MP_FIXED_POINT ]

#ifdef MP_FIXED_POINT // [

   void testConvert_int32_int16()
   {
      int16_t pDst[CONVERT_TO_INT16_LENGTH];
      MpDspUtils::convert(convert_int32_int16_src, pDst, CONVERT_TO_INT16_LENGTH);
//      printf("{");
      for (int i=0; i<CONVERT_TO_INT16_LENGTH; i++)
      {
//         printf("%12d, ", convert_to_int16_dst[i]);
         CPPUNIT_ASSERT_EQUAL(convert_to_int16_dst[i], pDst[i]);
      }
//      printf("};\n");
   }

   void testConvert_Gain_int32_int16()
   {
      int16_t pDst[CONVERT_TO_INT16_LENGTH];
      MpDspUtils::convert_Gain(convert_int32_int16_src, pDst, CONVERT_TO_INT16_LENGTH, 1);
//      printf("{");
      for (int i=0; i<CONVERT_TO_INT16_LENGTH; i++)
      {
//         printf("%12d, ", pDst[i]);
         CPPUNIT_ASSERT_EQUAL(convert_Gain_int32_int16_dst[i], pDst[i]);
      }
//      printf("};\n");
   }

   void testConvert_Att_int32_int16()
   {
      int16_t pDst[CONVERT_TO_INT16_LENGTH];
      MpDspUtils::convert_Att(convert_int32_int16_src, pDst, CONVERT_TO_INT16_LENGTH, 1);
//      printf("{");
      for (int i=0; i<CONVERT_TO_INT16_LENGTH; i++)
      {
//         printf("%12d, ", pDst[i]);
         CPPUNIT_ASSERT_EQUAL(convert_Att_int32_int16_dst[i], pDst[i]);
      }
//      printf("};\n");
   }

#else  // MP_FIXED_POINT ][

   void testConvert_float_int16()
   {
      int16_t pDst[CONVERT_TO_INT16_LENGTH];
      MpDspUtils::convert(convert_float_int16_src, pDst, CONVERT_TO_INT16_LENGTH);
//      printf("{");
      for (int i=0; i<CONVERT_TO_INT16_LENGTH; i++)
      {
//         printf("%12d, ", pDst[i]);
         CPPUNIT_ASSERT_EQUAL(convert_to_int16_dst[i], pDst[i]);
      }
//      printf("};\n");
   }

#endif // MP_FIXED_POINT ]

protected:

   // Data set for 16-bit integer addition test.
   enum {ADD_INT16_TEST_LENGTH=12};
   static const int16_t add_int16_src[ADD_INT16_TEST_LENGTH];
   static const int16_t add_int16_res[ADD_INT16_TEST_LENGTH][ADD_INT16_TEST_LENGTH];

   // Data set for 32-bit integer addition test.
   enum {ADD_INT32_TEST_LENGTH=12};
   static const int32_t add_int32_src[ADD_INT32_TEST_LENGTH];
   static const int32_t add_int32_res[ADD_INT32_TEST_LENGTH][ADD_INT32_TEST_LENGTH];

   // Data set for 32-bit integer addition test.
   enum {ADD_INT16_INT32_TEST_LENGTH=12};
   static const int16_t add_int16_int32_vec_src1[ADD_INT16_INT32_TEST_LENGTH];
#ifdef MP_FIXED_POINT // [
   static const int32_t add_int16_int32_vec_src2[2*ADD_INT16_INT32_TEST_LENGTH-1];
   static const int32_t add_int16_int32_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH];
   static const int32_t add_Gain_int16_int32_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH];
   static const int32_t add_Att_int16_int32_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH];
   static const int32_t addMul_int16_int32_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH];
#else  // MP_FIXED_POINT ][
   static const float add_int16_float_vec_src2[2*ADD_INT16_INT32_TEST_LENGTH-1];
   static const float add_int16_float_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH];
   static const float addMul_int16_float_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH];
#endif // MP_FIXED_POINT ]

#ifndef MP_FIXED_POINT // [
   // Data set for floating point addition test.
   enum {ADD_FLOAT_TEST_LENGTH=12};
   static const float add_float_src[ADD_FLOAT_TEST_LENGTH];
   static const float add_float_res[ADD_FLOAT_TEST_LENGTH][ADD_FLOAT_TEST_LENGTH];
#endif // !MP_FIXED_POINT ]

   enum {CONVERT_TO_INT16_LENGTH=20};
   static const int16_t convert_to_int16_dst[CONVERT_TO_INT16_LENGTH];
#ifdef MP_FIXED_POINT // [
   static const int32_t convert_int32_int16_src[CONVERT_TO_INT16_LENGTH];
   static const int16_t convert_Gain_int32_int16_dst[CONVERT_TO_INT16_LENGTH];
   static const int16_t convert_Att_int32_int16_dst[CONVERT_TO_INT16_LENGTH];
#else  // MP_FIXED_POINT ][
   static const float convert_float_int16_src[CONVERT_TO_INT16_LENGTH];
#endif // MP_FIXED_POINT ]

};

const int16_t MpDspUtilsTest::add_int16_src[ADD_INT16_TEST_LENGTH]
   =  {-32768, -32767, -32766, -16384, -16383,  -8192,      0,   8192,  16383,  16384, 32766, 32767};
const int16_t MpDspUtilsTest::add_int16_res[ADD_INT16_TEST_LENGTH][ADD_INT16_TEST_LENGTH] 
   = {{-32767, -32767, -32767, -32767, -32767, -32767, -32767, -24576, -16385, -16384,    -2,    -1},
      {-32767, -32767, -32767, -32767, -32767, -32767, -32767, -24575, -16384, -16383,    -1,     0},
      {-32767, -32767, -32767, -32767, -32767, -32767, -32766, -24574, -16383, -16382,     0,     1},
      {-32767, -32767, -32767, -32767, -32767, -24576, -16384,  -8192,     -1,      0, 16382, 16383},
      {-32767, -32767, -32767, -32767, -32766, -24575, -16383,  -8191,      0,      1, 16383, 16384},
      {-32767, -32767, -32767, -24576, -24575, -16384,  -8192,      0,   8191,   8192, 24574, 24575},
      {-32767, -32767, -32766, -16384, -16383, -8192,       0,   8192,  16383,  16384, 32766, 32767},
      {-24576, -24575, -24574,  -8192,  -8191,     0,    8192,  16384,  24575,  24576, 32767, 32767},
      {-16385, -16384, -16383,     -1,      0,  8191,   16383,  24575,  32766,  32767, 32767, 32767},
      {-16384, -16383, -16382,      0,      1,  8192,   16384,  24576,  32767,  32767, 32767, 32767},
      {    -2,     -1,      0,  16382,  16383, 24574,   32766,  32767,  32767,  32767, 32767, 32767},
      {    -1,      0,      1,  16383,  16384, 24575,   32767,  32767,  32767,  32767, 32767, 32767}};

const int32_t MpDspUtilsTest::add_int32_src[ADD_INT32_TEST_LENGTH]
   =  {-2147483647-1, -2147483647, -2147483646, -1073741824, -1073741823,  -536870912,           0,   536870912,  1073741823,  1073741824, 2147483646, 2147483647};
const int32_t MpDspUtilsTest::add_int32_res[ADD_INT32_TEST_LENGTH][ADD_INT32_TEST_LENGTH] 
   = {{ -2147483647,  -2147483647, -2147483647, -2147483647, -2147483647, -2147483647, -2147483647, -1610612736, -1073741825, -1073741824,         -2,         -1},
      { -2147483647,  -2147483647, -2147483647, -2147483647, -2147483647, -2147483647, -2147483647, -1610612735, -1073741824, -1073741823,         -1,          0},
      { -2147483647,  -2147483647, -2147483647, -2147483647, -2147483647, -2147483647, -2147483646, -1610612734, -1073741823, -1073741822,          0,          1},
      { -2147483647,  -2147483647, -2147483647, -2147483647, -2147483647, -1610612736, -1073741824,  -536870912,          -1,           0, 1073741822, 1073741823},
      { -2147483647,  -2147483647, -2147483647, -2147483647, -2147483646, -1610612735, -1073741823,  -536870911,           0,           1, 1073741823, 1073741824},
      { -2147483647,  -2147483647, -2147483647, -1610612736, -1610612735, -1073741824,  -536870912,           0,   536870911,   536870912, 1610612734, 1610612735},
      { -2147483647,  -2147483647, -2147483646, -1073741824, -1073741823,  -536870912,           0,   536870912,  1073741823,  1073741824, 2147483646, 2147483647},
      { -1610612736,  -1610612735, -1610612734,  -536870912,  -536870911,           0,   536870912,  1073741824,  1610612735,  1610612736, 2147483647, 2147483647},
      { -1073741825,  -1073741824, -1073741823,          -1,           0,   536870911,  1073741823,  1610612735,  2147483646,  2147483647, 2147483647, 2147483647},
      { -1073741824,  -1073741823, -1073741822,           0,           1,   536870912,  1073741824,  1610612736,  2147483647,  2147483647, 2147483647, 2147483647},
      {          -2,           -1,           0,  1073741822,  1073741823,  1610612734,  2147483646,  2147483647,  2147483647,  2147483647, 2147483647, 2147483647},
      {          -1,            0,           1,  1073741823,  1073741824,  1610612735,  2147483647,  2147483647,  2147483647,  2147483647, 2147483647, 2147483647}};

const int16_t MpDspUtilsTest::add_int16_int32_vec_src1[ADD_INT16_INT32_TEST_LENGTH]
   =  {       -32768,      -32767,      -32766,      -16384,      -16383,       -8192,           0,        8192,       16383,       16384,      32766,      32767};
#ifdef MP_FIXED_POINT // [
const int32_t MpDspUtilsTest::add_int16_int32_vec_src2[2*ADD_INT16_INT32_TEST_LENGTH-1]
   =  {-2147483647-1, -2147483647, -2147483646, -1073741824, -1073741823,  -536870912,           0,   536870912,  1073741823,  1073741824, 2147483646, 2147483647,
       -2147483647-1, -2147483647, -2147483646, -1073741824, -1073741823,  -536870912,           0,   536870912,  1073741823,  1073741824, 2147483646};
const int32_t MpDspUtilsTest::add_int16_int32_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH] 
   = {{-2147483647,   -2147483647, -2147483647, -1073758208, -1073758206,  -536879104,           0,   536879104,  1073758206,  1073758208,  2147483647,  2147483647},
      {-2147483647,   -2147483647, -1073774590, -1073758207,  -536887295,       -8192,   536870912,  1073750015,  1073758207,  2147483647,  2147483647, -2147450881},
      {-2147483647,   -1073774591, -1073774589,  -536887296,      -16383,   536862720,  1073741823,  1073750016,  2147483647,  2147483647, -2147450882, -2147450880},
      {-1073774592,   -1073774590,  -536903678,      -16384,   536854529,  1073733631,  1073741824,  2147483647,  2147483647, -2147467264, -2147450881, -2147450879},
      {-1073774591,    -536903679,      -32766,   536854528,  1073725440,  1073733632,  2147483646,  2147483647, -2147467265, -2147467263, -2147450880, -1073709057},
      { -536903680,        -32767,   536838146,  1073725439,  1073725441,  2147475454,  2147483647, -2147475456, -2147467264, -2147467262, -1073709058, -1073709056},
      {     -32768,     536838145,  1073709057,  1073725440,  2147467263,  2147475455, -2147483647, -2147475455, -2147467263, -1073725440, -1073709057,  -536838145},
      {  536838144,    1073709056,  1073709058,  2147467262,  2147467264, -2147483647, -2147483647, -2147475454, -1073725441, -1073725439,  -536838146,       32767},
      { 1073709055,    1073709057,  2147450880,  2147467263, -2147483647, -2147483647, -2147483646, -1073733632, -1073725440,  -536854528,       32766,   536903679},
      { 1073709056,    2147450879,  2147450881, -2147483647, -2147483647, -2147483647, -1073741824, -1073733631,  -536854529,       16384,   536903678,  1073774590},
      { 2147450878,    2147450880, -2147483647, -2147483647, -2147483647, -1073750016, -1073741823,  -536862720,       16383,   536887296,  1073774589,  1073774591},
      { 2147450879,   -2147483647, -2147483647, -2147483647, -1073758207, -1073750015,  -536870912,        8192,   536887295,  1073758207,  1073774590,  2147483647}};
const int32_t MpDspUtilsTest::add_Gain_int16_int32_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH] 
   = {{-2147483647,   -2147483647, -2147483647, -2147483647, -2147418111, -1073741824,           0,  1073741824,  2147418111,  2147483647,  2147483647,  2147483647},
      {-2147483647,   -2147483647, -2147483647, -2147483647, -1610547200,  -536870912,   536870912,  1610612735,  2147418112,  2147483647,  2147483647,      -65536},
      {-2147483647,   -2147483647, -2147483647, -1610612736, -1073676288,           0,  1073741823,  1610612736,  2147483647,  2147483647,     -131072,      -65535},
      {-2147483647,   -2147483647, -2147483647, -1073741824,  -536805376,   536870911,  1073741824,  2147483647,  2147483647, -1073741824,     -131071,      -65534},
      {-2147483647,   -2147483647, -2147352576,  -536870912,       65535,   536870912,  2147483646,  2147483647, -1073807360, -1073741823,     -131070,  1073676288},
      {-2147483647,   -2147418112, -1610481664,          -1,       65536,  1610612734,  2147483647, -1610612736, -1073807359, -1073741822,  1073610752,  1073676289},
      {-2147483647,   -1610547200, -1073610753,           0,  1073807358,  1610612735, -2147483647, -1610612735, -1073807358,           0,  1073610753,  1610547200},
      {-1610612736,   -1073676289, -1073610752,  1073741822,  1073807359, -2147483647, -2147483647, -1610612734,      -65536,           1,  1610481664,  2147418112},
      {-1073741825,   -1073676288,      131070,  1073741823, -2147483647, -2147483647, -2147483646,  -536870912,      -65535,   536870912,  2147352576,  2147483647},
      {-1073741824,         65534,      131071, -2147483647, -2147483647, -2147483647, -1073741824,  -536870911,   536805376,  1073741824,  2147483647,  2147483647},
      {         -2,         65535, -2147483647, -2147483647, -2147483647, -1610612736, -1073741823,           0,  1073676288,  1610612736,  2147483647,  2147483647},
      {         -1,   -2147483647, -2147483647, -2147483647, -2147418112, -1610612735,  -536870912,   536870912,  1610547200,  2147483647,  2147483647,  2147483647}};
const int32_t MpDspUtilsTest::add_Att_int16_int32_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH] 
   = {{-2147483647,   -2147483647, -2147483647, -1073750016, -1073750015,  -536875008,           0,   536875008,  1073750014,  1073750016,  2147483647,  2147483647},
      {-2147483647,   -2147483647, -1073758207, -1073750015,  -536879104,       -4096,   536870912,  1073745919,  1073750015,  2147483647,  2147483647, -2147467265},
      {-2147483647,   -1073758208, -1073758206,  -536879104,       -8192,   536866816,  1073741823,  1073745920,  2147483647,  2147483647, -2147467265, -2147467264},
      {-1073758208,   -1073758207,  -536887295,       -8192,   536862720,  1073737727,  1073741824,  2147483647,  2147483647, -2147475456, -2147467264, -2147467263},
      {-1073758207,    -536887296,      -16383,   536862720,  1073733631,  1073737728,  2147483646,  2147483647, -2147475457, -2147475455, -2147467263, -1073725441},
      { -536887296,        -16384,   536854529,  1073733631,  1073733632,  2147479550,  2147483647, -2147479552, -2147475456, -2147475454, -1073725441, -1073725440},
      {     -16384,     536854528,  1073725440,  1073733632,  2147475454,  2147479551, -2147483647, -2147479551, -2147475455, -1073733632, -1073725440,  -536854529},
      {  536854528,    1073725439,  1073725441,  2147475454,  2147475455, -2147483647, -2147483647, -2147479550, -1073733633, -1073733631,  -536854529,       16383},
      { 1073725439,    1073725440,  2147467263,  2147475455, -2147483647, -2147483647, -2147483646, -1073737728, -1073733632,  -536862720,       16383,   536887295},
      { 1073725440,    2147467262,  2147467264, -2147483647, -2147483647, -2147483647, -1073741824, -1073737727,  -536862721,        8192,   536887295,  1073758206},
      { 2147467262,    2147467263, -2147483647, -2147483647, -2147483647, -1073745920, -1073741823,  -536866816,        8191,   536879104,  1073758206,  1073758207},
      { 2147467263,   -2147483647, -2147483647, -2147483647, -1073750016, -1073745919,  -536870912,        4096,   536879103,  1073750015,  1073758207,  2147483647}};
const int32_t MpDspUtilsTest::addMul_int16_int32_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH] 
   = {{-2147450880,   -2147450880, -2147450880, -1073725440, -1073725440,  -536862720,           0,   536862720,  1073725440,  1073725440,  2147450880,  2147450880},
      {-2147450879,   -2147450879, -1073709058, -1073725439,  -536854529,        8192,   536870912,  1073733631,  1073725441,  2147467262,  2147450881, -2147483647},
      {-2147450878,   -1073709057, -1073709057,  -536854528,       16383,   536879104,  1073741823,  1073733632,  2147467263,  2147467263, -2147483647, -2147483647},
      {-1073709056,   -1073709056,  -536838146,       16384,   536887295,  1073750015,  1073741824,  2147475454,  2147467264, -2147483647, -2147483647, -2147483647},
      {-1073709055,    -536838145,       32766,   536887296,  1073758206,  1073750016,  2147483646,  2147475455, -2147483647, -2147483647, -2147483647, -1073774591},
      { -536838144,         32767,   536903678,  1073758207,  1073758207,  2147483647,  2147483647, -2147483647, -2147483647, -2147483647, -1073774590, -1073774590},
      {      32768,     536903679,  1073774589,  1073758208,  2147483647,  2147483647, -2147483647, -2147483647, -2147483647, -1073758208, -1073774589,  -536903679},
      {  536903680,    1073774590,  1073774590,  2147483647,  2147483647, -2147475456, -2147483647, -2147483647, -1073758207, -1073758207,  -536903678,      -32767},
      { 1073774591,    1073774591,  2147483647,  2147483647, -2147467265, -2147475455, -2147483646, -1073750016, -1073758206,  -536887296,      -32766,   536838145},
      { 1073774592,    2147483647,  2147483647, -2147467264, -2147467264, -2147475454, -1073741824, -1073750015,  -536887295,      -16384,   536838146,  1073709056},
      { 2147483647,    2147483647, -2147450882, -2147467263, -2147467263, -1073733632, -1073741823,  -536879104,      -16383,   536854528,  1073709057,  1073709057},
      { 2147483647,   -2147450881, -2147450881, -2147467262, -1073725441, -1073733631,  -536870912,       -8192,   536854529,  1073725439,  1073709058,  2147450879}};
#else  // MP_FIXED_POINT ][
const float MpDspUtilsTest::add_int16_float_vec_src2[2*ADD_INT16_INT32_TEST_LENGTH-1]
   =  {-2147483648.0f,   -2147483647.0f, -2147483646.0f, -1073741824.0f, -1073741823.0f,  -536870912.0f,           0.0f,   536870912.0f,  1073741823.0f,  1073741824.0f, 2147483646.0f, 2147483647.0f,
       -2147483648.0f,   -2147483647.0f, -2147483646.0f, -1073741824.0f, -1073741823.0f,  -536870912.0f,           0.0f,   536870912.0f,  1073741823.0f,  1073741824.0f, 2147483646.0f};
const float MpDspUtilsTest::add_int16_float_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH]
   = {{-2147516416.0f, -2147516416.0f, -2147516416.0f, -1073758208.0f, -1073758208.0f,  -536879104.0f,           0.0f,   536879104.0f,  1073758208.0f,  1073758208.0f,  2147516416.0f,  2147516416.0f},
      {-2147516416.0f, -2147516416.0f, -1073774592.0f, -1073758208.0f,  -536887296.0f,       -8192.0f,   536870912.0f,  1073750016.0f,  1073758208.0f,  2147500032.0f,  2147516416.0f, -2147450880.0f},
      {-2147516416.0f, -1073774592.0f, -1073774592.0f,  -536887296.0f,      -16383.0f,   536862720.0f,  1073741824.0f,  1073750016.0f,  2147500032.0f,  2147500032.0f, -2147450880.0f, -2147450880.0f},
      {-1073774592.0f, -1073774592.0f,  -536903680.0f,      -16384.0f,   536854528.0f,  1073733632.0f,  1073741824.0f,  2147491840.0f,  2147500032.0f, -2147467264.0f, -2147450880.0f, -2147450880.0f},
      {-1073774592.0f,  -536903680.0f,      -32766.0f,   536854528.0f,  1073725440.0f,  1073733632.0f,  2147483648.0f,  2147491840.0f, -2147467264.0f, -2147467264.0f, -2147450880.0f, -1073709056.0f},
      { -536903680.0f,      -32767.0f,   536838144.0f,  1073725440.0f,  1073725440.0f,  2147475456.0f,  2147483648.0f, -2147475456.0f, -2147467264.0f, -2147467264.0f, -1073709056.0f, -1073709056.0f},
      {     -32768.0f,   536838144.0f,  1073709056.0f,  1073725440.0f,  2147467264.0f,  2147475456.0f, -2147483648.0f, -2147475456.0f, -2147467264.0f, -1073725440.0f, -1073709056.0f,  -536838144.0f},
      {  536838144.0f,  1073709056.0f,  1073709056.0f,  2147467264.0f,  2147467264.0f, -2147491840.0f, -2147483648.0f, -2147475456.0f, -1073725440.0f, -1073725440.0f,  -536838144.0f,       32767.0f},
      { 1073709056.0f,  1073709056.0f,  2147450880.0f,  2147467264.0f, -2147500032.0f, -2147491840.0f, -2147483648.0f, -1073733632.0f, -1073725440.0f,  -536854528.0f,       32766.0f,   536903680.0f},
      { 1073709056.0f,  2147450880.0f,  2147450880.0f, -2147500032.0f, -2147500032.0f, -2147491840.0f, -1073741824.0f, -1073733632.0f,  -536854528.0f,       16384.0f,   536903680.0f,  1073774592.0f},
      { 2147450880.0f,  2147450880.0f, -2147516416.0f, -2147500032.0f, -2147500032.0f, -1073750016.0f, -1073741824.0f,  -536862720.0f,       16383.0f,   536887296.0f,  1073774592.0f,  1073774592.0f},
      { 2147450880.0f, -2147516416.0f, -2147516416.0f, -2147500032.0f, -1073758208.0f, -1073750016.0f,  -536870912.0f,        8192.0f,   536887296.0f,  1073758208.0f,  1073774592.0f,  2147516416.0f}};
const float MpDspUtilsTest::addMul_int16_float_vec_res[ADD_INT16_INT32_TEST_LENGTH][ADD_INT16_INT32_TEST_LENGTH]
   = {{-4294934528.0f, -4294868992.0f, -4294803456.0f, -2147467264.0f, -2147401728.0f, -1073733632.0f,           0.0f,  1073733632.0f,  2147401728.0f,  2147467264.0f,  4294803456.0f,  4294868992.0f},
      {-4294934528.0f, -4294868992.0f, -3221061632.0f, -2147467264.0f, -1610530816.0f,  -536862720.0f,   536870912.0f,  1610604544.0f,  2147401728.0f,  3221209088.0f,  4294803456.0f,      -98304.0f},
      {-4294934528.0f, -3221127168.0f, -3221061632.0f, -1610596352.0f, -1073659904.0f,        8192.0f,  1073741824.0f,  1610604544.0f,  3221143552.0f,  3221209088.0f,     -163840.0f,      -98304.0f},
      {-3221192704.0f, -3221127168.0f, -2684190720.0f, -1073725440.0f,  -536788992.0f,   536879104.0f,  1073741824.0f,  2684346368.0f,  3221143552.0f, -1073758208.0f,     -163840.0f,      -98304.0f},
      {-3221192704.0f, -2684256256.0f, -2147319808.0f,  -536854528.0f,       81920.0f,   536879104.0f,  2147483648.0f,  2684346368.0f, -1073823744.0f, -1073758208.0f,     -163840.0f,  1073643520.0f},
      {-2684321792.0f, -2147385344.0f, -1610448896.0f,       16384.0f,       81920.0f,  1610620928.0f,  2147483648.0f, -1610620928.0f, -1073823744.0f, -1073758208.0f,  1073577984.0f,  1073643520.0f},
      {-2147450880.0f, -1610514432.0f, -1073577984.0f,       16384.0f,  1073823744.0f,  1610620928.0f, -2147483648.0f, -1610620928.0f, -1073823744.0f,      -16384.0f,  1073577984.0f,  1610514432.0f},
      {-1610579968.0f, -1073643520.0f, -1073577984.0f,  1073758208.0f,  1073823744.0f, -2684346368.0f, -2147483648.0f, -1610620928.0f,      -81920.0f,      -16384.0f,  1610448896.0f,  2147385344.0f},
      {-1073709056.0f, -1073643520.0f,      163840.0f,  1073758208.0f, -3221143552.0f, -2684346368.0f, -2147483648.0f,  -536879104.0f,      -81920.0f,   536854528.0f,  2147319808.0f,  2684256256.0f},
      {-1073709056.0f,       98304.0f,      163840.0f, -3221209088.0f, -3221143552.0f, -2684346368.0f, -1073741824.0f,  -536879104.0f,   536788992.0f,  1073725440.0f,  2684190720.0f,  3221127168.0f},
      {      32768.0f,       98304.0f, -4294803456.0f, -3221209088.0f, -3221143552.0f, -1610604544.0f, -1073741824.0f,       -8192.0f,  1073659904.0f,  1610596352.0f,  3221061632.0f,  3221127168.0f},
      {      32768.0f, -4294868992.0f, -4294803456.0f, -3221209088.0f, -2147401728.0f, -1610604544.0f,  -536870912.0f,   536862720.0f,  1610530816.0f,  2147467264.0f,  3221061632.0f,  4294868992.0f}};
#endif // MP_FIXED_POINT ]

#ifndef MP_FIXED_POINT // [
const float MpDspUtilsTest::add_float_src[ADD_FLOAT_TEST_LENGTH]
   =  {-21474836478.0f,  -2147483647.0f,  -2147483646.0f,  -1073741824.0f,  -1073741823.0f,   -536870912.0f,            0.0f,    536870912.0f,   1073741823.0f,   1073741824.0f,   2147483646.0f,   2147483647.0f};
const float MpDspUtilsTest::add_float_res[ADD_FLOAT_TEST_LENGTH][ADD_FLOAT_TEST_LENGTH] 
   = {{-42949672960.0f, -23622320128.0f, -23622320128.0f, -22548578304.0f, -22548578304.0f, -22011707392.0f, -21474836480.0f, -20937965568.0f, -20401094656.0f, -20401094656.0f, -19327352832.0f, -19327352832.0f},
      {-23622320128.0f,  -4294967296.0f,  -4294967296.0f,  -3221225472.0f,  -3221225472.0f,  -2684354560.0f,  -2147483648.0f,  -1610612736.0f,  -1073741824.0f,  -1073741824.0f,            0.0f,            0.0f},
      {-23622320128.0f,  -4294967296.0f,  -4294967296.0f,  -3221225472.0f,  -3221225472.0f,  -2684354560.0f,  -2147483648.0f,  -1610612736.0f,  -1073741824.0f,  -1073741824.0f,            0.0f,            0.0f},
      {-22548578304.0f,  -3221225472.0f,  -3221225472.0f,  -2147483648.0f,  -2147483648.0f,  -1610612736.0f,  -1073741824.0f,   -536870912.0f,            0.0f,            0.0f,   1073741824.0f,   1073741824.0f},
      {-22548578304.0f,  -3221225472.0f,  -3221225472.0f,  -2147483648.0f,  -2147483648.0f,  -1610612736.0f,  -1073741824.0f,   -536870912.0f,            0.0f,            0.0f,   1073741824.0f,   1073741824.0f},
      {-22011707392.0f,  -2684354560.0f,  -2684354560.0f,  -1610612736.0f,  -1610612736.0f,  -1073741824.0f,   -536870912.0f,            0.0f,    536870912.0f,    536870912.0f,   1610612736.0f,   1610612736.0f},
      {-21474836480.0f,  -2147483648.0f,  -2147483648.0f,  -1073741824.0f,  -1073741824.0f,   -536870912.0f,            0.0f,    536870912.0f,   1073741824.0f,   1073741824.0f,   2147483648.0f,   2147483648.0f},
      {-20937965568.0f,  -1610612736.0f,  -1610612736.0f,   -536870912.0f,   -536870912.0f,            0.0f,    536870912.0f,   1073741824.0f,   1610612736.0f,   1610612736.0f,   2684354560.0f,   2684354560.0f},
      {-20401094656.0f,  -1073741824.0f,  -1073741824.0f,            0.0f,            0.0f,    536870912.0f,   1073741824.0f,   1610612736.0f,   2147483648.0f,   2147483648.0f,   3221225472.0f,   3221225472.0f},
      {-20401094656.0f,  -1073741824.0f,  -1073741824.0f,            0.0f,            0.0f,    536870912.0f,   1073741824.0f,   1610612736.0f,   2147483648.0f,   2147483648.0f,   3221225472.0f,   3221225472.0f},
      {-19327352832.0f,            0.0f,            0.0f,   1073741824.0f,   1073741824.0f,   1610612736.0f,   2147483648.0f,   2684354560.0f,   3221225472.0f,   3221225472.0f,   4294967296.0f,   4294967296.0f},
      {-19327352832.0f,            0.0f,            0.0f,   1073741824.0f,   1073741824.0f,   1610612736.0f,   2147483648.0f,   2684354560.0f,   3221225472.0f,   3221225472.0f,   4294967296.0f,   4294967296.0f}};
#endif // !MP_FIXED_POINT ]

const int16_t MpDspUtilsTest::convert_to_int16_dst[CONVERT_TO_INT16_LENGTH]
   = {       -32767,      -32767, -32767, -32767, -32767, -32767, -32767, -16384, -16383,  -8192, 0,  8192, 16383, 16384, 32766, 32767, 32767, 32767, 32767,      32767};
#ifdef MP_FIXED_POINT // [
const int32_t MpDspUtilsTest::convert_int32_int16_src[CONVERT_TO_INT16_LENGTH]
   = {-2147483647-1, -2147483647, -65536, -65535, -32769, -32768, -32767, -16384, -16383,  -8192, 0,  8192, 16383, 16384, 32766, 32767, 32768, 65535, 65536, 2147483647};
const int16_t MpDspUtilsTest::convert_Gain_int32_int16_dst[CONVERT_TO_INT16_LENGTH]
   = {       -32767,      -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32766, -16384, 0, 16384, 32767, 32767, 32767, 32767, 32767, 32767, 32767,      32767};
const int16_t MpDspUtilsTest::convert_Att_int32_int16_dst[CONVERT_TO_INT16_LENGTH]
   = {       -32767,      -32767, -32767, -32767, -16385, -16384, -16384,  -8192,  -8192,  -4096, 0,  4096,  8191,  8192, 16383, 16383, 16384, 32767, 32767,      32767};
#else  // MP_FIXED_POINT ][
const float MpDspUtilsTest::convert_float_int16_src[CONVERT_TO_INT16_LENGTH]
   = {-2147483648.0f, -2147483647.0f, -65536.0f, -65535.0f, -32769.0f, -32768.0f, -32767.0f, -16384.0f, -16383.0f,  -8192.0f, 0.0f,  8192.0f, 16383.0f, 16384.0f, 32766.0f, 32767.0f, 32768.0f, 65535.0f, 65536.0f, 2147483647.0f};
#endif // MP_FIXED_POINT ]


CPPUNIT_TEST_SUITE_REGISTRATION(MpDspUtilsTest);
