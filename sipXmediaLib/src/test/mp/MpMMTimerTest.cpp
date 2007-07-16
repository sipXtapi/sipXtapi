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

#include <mp/MpMMTimer.h>
#include <os/OsDateTime.h>
#include <os/OsSysLog.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mp/MpMMTimerWnt.h>
#endif

/**
 * Unittest for MpMMTimer and its successors
 */
class MpMMTimerTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpMMTimerTest);
   CPPUNIT_TEST(testLinearTimer);
   CPPUNIT_TEST_SUITE_END();


public:
   void setUp()
   {
   }

   void tearDown()
   {
   }

   void testLinearTimer()
   {
      MpMMTimer* pMMTimer = NULL;
#ifdef WIN32
      MpMMTimerWnt mmTimerWnt(MpMMTimer::Linear);
      pMMTimer = &mmTimerWnt;
#else
      // Right now MMTimers are only implemented for win32.
      // as other platforms are implemented, change this.
      printf("MMTimer not implemented for this platform.  Test disabled.\n");
      return;
#endif

      // Test getting the resolution, and get it..
      unsigned resolution;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->getResolution(resolution));

      // Set the below two variables and preprocessor defines to tweak this test.
      // TLT_LOOP_COUNT defines the number of timer fire repetitions to do 
      // (set this to an even value),
      // and periodUSecs defines how long to wait in each one.
#     define TLT_LOOP_CNT 50
      unsigned periodUSecs = 10000;



      // Test the period range static method..
      unsigned unusedMin = 0;
      unsigned unusedMax = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           MpMMTimer::getPeriodRange(&unusedMin, &unusedMax));


#ifdef WIN32
      // Initialize performance counting measurement tools
      LARGE_INTEGER perfFreqPerSec;
      CPPUNIT_ASSERT(QueryPerformanceFrequency(&perfFreqPerSec) > 0);
      //printf("Performance frequency is %I64d ticks per sec\n", 
      //       perfFreqPerSec.QuadPart);
      // Convert it to per usec instead of per sec.
      double perfFreqPerUSec = double(perfFreqPerSec.QuadPart) / double(1000000.0);
      //printf("Performance frequency is %f ticks per usec\n", 
      //       perfFreqPerUSec);
      LARGE_INTEGER perfCount[TLT_LOOP_CNT];
#endif

      // Initialize the timer.
      printf("Minimum timer resolution is %d usecs\n", resolution);
      printf("Firing every %d usecs\n", periodUSecs);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->run(periodUSecs));

      // Perform the timing measurements.
      int i;
      for(i = 0; i < TLT_LOOP_CNT; i++)
      {
#ifdef WIN32
         CPPUNIT_ASSERT(QueryPerformanceCounter(&perfCount[i]) > 0);
#endif
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->waitForNextTick());
      }

#ifdef WIN32
      printf("Timing values in microseconds, CSV: \n");
      __int64 delta;
      __int64 valOutsideThreshs = 0;
      for(i = 0; i < TLT_LOOP_CNT; i = i+2)
      {
         delta = __int64(perfCount[i+1].QuadPart / perfFreqPerUSec) - 
                 __int64(perfCount[i].QuadPart / perfFreqPerUSec);

         // Print output in CSV format, for easy graphing.
         printf("%I64d", delta);
         if(i < TLT_LOOP_CNT-2)
         {
            printf(", ");
         }
         else
         {
            printf("\n");
         }

         // Check if we're outside some reasonable thresholds.
         if(i > 0 && // don't include the first value in our tests - it's always high.
            valOutsideThreshs == 0) // Only check if we haven't already gone outside threshold.
         {
            if(delta-periodUSecs > 0 ||
               delta-periodUSecs < 2500)
            {
               valOutsideThreshs = delta;
            }
         }
      }

      // Assert when outside an error range of -0us to +2500us,
      CPPUNIT_ASSERT_MESSAGE("Timer error falls outside thresholds of -0 to +2500us",
                             (valOutsideThreshs-periodUSecs > 0 && 
                              valOutsideThreshs-periodUSecs < 2500));
#endif

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->stop());
   }

protected:

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpMMTimerTest);
