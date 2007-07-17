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
   CPPUNIT_TEST(testGetResolution);
   CPPUNIT_TEST(testPeriodRange);
   CPPUNIT_TEST(testLinearTimer);
   CPPUNIT_TEST_SUITE_END();


public:
   void setUp()
   {
   }

   void tearDown()
   {
   }

#ifdef WIN32
   double initializeWin32PerfMeasurementTools()
   {
      // Initialize performance counting measurement tools
      LARGE_INTEGER perfFreqPerSec;
      CPPUNIT_ASSERT(QueryPerformanceFrequency(&perfFreqPerSec) > 0);
      //printf("Performance frequency is %I64d ticks per sec\n", 
      //       perfFreqPerSec.QuadPart);
      // Convert it to per usec instead of per sec.
      double perfFreqPerUSec = double(perfFreqPerSec.QuadPart) / double(1000000.0);
      //printf("Performance frequency is %f ticks per usec\n", 
      //       perfFreqPerUSec);
      return perfFreqPerUSec;
   }
#endif

   void checkDeltasAgainstThresholds(long deltas[], unsigned nDeltas, 
                                     unsigned targetDelta, // <-- was periodUSecs
                                     long lowerThresh, long upperThresh,
                                     long lowerMeanThresh, long upperMeanThresh)
   {
      CPPUNIT_ASSERT_MESSAGE("Timer didn't fire or deltas were not collected!",
                             nDeltas > 0);

      printf("Timing values in microseconds, CSV: \n");
      long valOutsideThreshs = targetDelta; // initialize to exactly what we want.
      long meanAvg = 0;
      unsigned i;
      for(i = 0; i < nDeltas; i++)
      {
         // Tack on the current delta to our mean avg
         meanAvg += deltas[i];

         // Print output in CSV format, for easy graphing.
         printf("%ld", deltas[i]);
         printf((i < nDeltas-1) ? ", " : "\n");

         // Check if we're outside some reasonable thresholds.
         if(i > 0 && // don't include the first value in our tests - it's always high.
            valOutsideThreshs == targetDelta) // Only check if we haven't already gone outside threshold.
         {
            if(deltas[i]-(long)targetDelta < lowerThresh ||
               deltas[i]-(long)targetDelta > upperThresh)
            {
               valOutsideThreshs = deltas[i];
            }
         }
      }

      // Finalize determining mean.
      meanAvg /= nDeltas;
      printf("Mean: %ld\n", meanAvg);

      // Assert when single value outside error range specified above.
      char errStrBuf[256];
      snprintf(errStrBuf, 256, 
               "Single timer value %ld falls outside threshold of %ld to %ldus",
               valOutsideThreshs, lowerThresh, upperThresh);

      CPPUNIT_ASSERT_MESSAGE(errStrBuf,
                             (valOutsideThreshs-(long)targetDelta >= lowerThresh && 
                              valOutsideThreshs-(long)targetDelta <= upperThresh));

      // Assert when mean is outside error range specified above.
      snprintf(errStrBuf, 256, 
         "Mean timer value %ld falls outside threshold of %ld to %ldus",
         meanAvg, lowerMeanThresh, upperMeanThresh);

      CPPUNIT_ASSERT_MESSAGE(errStrBuf,
                             (meanAvg-(long)targetDelta >= lowerMeanThresh && 
                              meanAvg-(long)targetDelta <= upperMeanThresh));
   }

   void testGetResolution()
   {
      // Test getting the resolution, and get it..
      unsigned resolution;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, MpMMTimer::getResolution(resolution));
      printf("Minimum timer resolution is %d usecs\n", resolution);
   }

   void testPeriodRange()
   {
      // Test the period range static method..
      unsigned unusedMin = 0;
      unsigned unusedMax = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         MpMMTimer::getPeriodRange(&unusedMin, &unusedMax));
   }

   void testLinearTimer()
   {
      // Set the below variables and preprocessor defines to tweak this test.
      // TLT_LOOP_COUNT defines the number of timer fire repetitions to do 
      // (set this to an even value),
      // and periodUSecs defines how long to wait in each one.
#     define TLT_LOOP_CNT 200
      unsigned periodUSecs = 10000;
      long lowerThresh = -3000;    // Assert when outside an error range
      long lowerMeanThresh = -50;  // specified below.
      long upperThresh = 3000;     // One for single values
      long upperMeanThresh = 50;   // One for mean values


      MpMMTimer* pMMTimer = NULL;
#ifdef WIN32
      MpMMTimerWnt mmTimerWnt(MpMMTimer::Linear);
      pMMTimer = &mmTimerWnt;

      double perfFreqPerUSec = initializeWin32PerfMeasurementTools();
      LARGE_INTEGER perfCount[TLT_LOOP_CNT];
#else
      // Right now MMTimers are only implemented for win32.
      // as other platforms are implemented, change this.
      printf("MMTimer not implemented for this platform.  Test disabled.\n");
      return;
#endif

      // Initialize the timer.
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

      // Determine delta times from individual time measurements..
      long deltas[TLT_LOOP_CNT/2];
      for(i = 0; i < TLT_LOOP_CNT; i = i+2)
      {
         deltas[(i+1)/2] = 
#ifdef WIN32
            (long)(__int64(perfCount[i+1].QuadPart / perfFreqPerUSec) - 
                   __int64(perfCount[i].QuadPart / perfFreqPerUSec));
#else
            0;
#endif
      }


      checkDeltasAgainstThresholds(deltas, TLT_LOOP_CNT/2, 
                                   periodUSecs, 
                                   lowerThresh, upperThresh,
                                   lowerMeanThresh, upperMeanThresh);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->stop());
   }

protected:

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpMMTimerTest);
