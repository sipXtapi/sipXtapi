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
#include <os/OsNotification.h>
#include <os/OsTask.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mp/MpMMTimerWnt.h>
#endif

// Forward Decl's
class MpMMTimerTest;

class TimerNotification : public OsNotification
{
public:
   TimerNotification(MpMMTimerTest* pMMTimerTest);
   virtual ~TimerNotification() {}
   OsStatus signal(const intptr_t eventData);

private:
   MpMMTimerTest* mpMMTimerTest;
};



/**
 * Unittest for MpMMTimer and its successors
 */
class MpMMTimerTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpMMTimerTest);
   CPPUNIT_TEST(testGetResolution);
   CPPUNIT_TEST(testPeriodRange);
   CPPUNIT_TEST(testLinearTimer);
   CPPUNIT_TEST(testNotificationTimer);
   CPPUNIT_TEST_SUITE_END();


public:
   void setUp()
   {
#ifdef WIN32
      mpPerfCounts = NULL;
#endif
      mPerfCountsSz = 0;
      mCurNPerfCounts = 0;
   }

   void tearDown()
   {
#ifdef WIN32
      if(mpPerfCounts != NULL)
      {
         delete[] mpPerfCounts;
      }
      mpPerfCounts = NULL;
#endif
      mPerfCountsSz = 0;
      mCurNPerfCounts = 0;
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

   void checkDeltasAgainstThresholds(long deltas[], long absDeltas[], unsigned nDeltas, 
                                     unsigned targetDelta, // <-- was periodUSecs
                                     long lowerThresh, long upperThresh)
   {
      CPPUNIT_ASSERT_MESSAGE("Timer didn't fire or deltas were not collected!",
                             nDeltas > 0);

      printf("Timing values in microseconds, CSV: \n");
      long valOutsideThreshs = targetDelta; // initialize to exactly what we want.
      unsigned i;
      for(i = 0; i < nDeltas; i++)
      {
         // Print output in CSV format, for easy graphing.
         printf("%ld\t%ld", deltas[i], absDeltas[i]);
         printf((i < nDeltas-1) ? "\n" : "\n\n");  // done this way for easy change to comma CSV

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

      // Assert when single value outside error range specified above.
      char errStrBuf[256];
      snprintf(errStrBuf, 256, 
               "Single timer value %ld falls outside threshold of %ld to %ld us",
               valOutsideThreshs, lowerThresh, upperThresh);

      CPPUNIT_ASSERT_MESSAGE(errStrBuf,
                             (valOutsideThreshs-(long)targetDelta >= lowerThresh && 
                              valOutsideThreshs-(long)targetDelta <= upperThresh));
   }

   void checkMeanAgainstThresholds(int64_t start, int64_t stop, unsigned nDeltas,
                                   unsigned targetDelta, // <-- was periodUSecs
                                   long lowerMeanThresh, long upperMeanThresh)
   {
      CPPUNIT_ASSERT_MESSAGE("Timer didn't fire or deltas were not collected!",
                             nDeltas > 0);

      double meanAvg = 0;

      meanAvg = (stop-start)/(double)nDeltas;
      printf("Mean: %.2f us\n", meanAvg);

      // Assert when mean is outside error range specified above.
      char errStrBuf[256];
      snprintf(errStrBuf, 256, 
         "Mean timer value %.2f falls outside threshold of %ld to %ld us",
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
#     define TLT_LOOP_CNT 2000
      unsigned periodUSecs = 10000;
      long lowerThresh = -(long)periodUSecs+1;    // Assert when outside an error range
      long lowerMeanThresh = -50;  // specified below.
      long upperThresh = periodUSecs*2-1;     // One for single values
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

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->stop());

      // Determine delta times from individual time measurements..
      long deltas[TLT_LOOP_CNT-1];
      long absDeltas[TLT_LOOP_CNT-1];
      for(i = 0; i < TLT_LOOP_CNT-1; i++)
      {
#ifdef WIN32
         deltas[i] = 
            (long)(__int64(perfCount[i+1].QuadPart / perfFreqPerUSec) - 
                   __int64(perfCount[i].QuadPart / perfFreqPerUSec));
         absDeltas[i] = 
            (long)((int64_t)((perfCount[i+1].QuadPart-perfCount[0].QuadPart) / perfFreqPerUSec) - 
                   (int64_t)(periodUSecs)*i+1);
#else
         deltas[i] = 0; 
         absDeltas[i] = 0;
#endif
      }


      checkDeltasAgainstThresholds(deltas, absDeltas, TLT_LOOP_CNT-1, 
                                   periodUSecs, 
                                   lowerThresh, upperThresh);
#ifdef WIN32
      checkMeanAgainstThresholds((int64_t)(perfCount[0].QuadPart / perfFreqPerUSec),
                                 (int64_t)(perfCount[TLT_LOOP_CNT-1].QuadPart / perfFreqPerUSec),
                                 TLT_LOOP_CNT-1, 
                                 periodUSecs, 
                                 lowerMeanThresh, upperMeanThresh);
#else
      printf("Mean performance timing not implemented on this platform\n");
#endif
   }

   void notificationTimerRecordTick()
   {
#ifdef WIN32
      int x;
      x = mPerfCountsSz;
      CPPUNIT_ASSERT(mpPerfCounts != NULL);
      CPPUNIT_ASSERT(mPerfCountsSz > 0);

      // Collect measurements while we have space left.
      if(mCurNPerfCounts < mPerfCountsSz)
      {
         CPPUNIT_ASSERT(QueryPerformanceCounter(&mpPerfCounts[mCurNPerfCounts++]) > 0);
      }
#else
      // Nothing is currently done on other platforms, as none are implemented
      // on other platforms
#endif
   }

   void testNotificationTimer()
   {
      // Set the below variables and preprocessor defines to tweak this test.
      // mPerfCountsSz defines the number of timer fire repetitions to do 
      // (set this to an even value),
      // and periodUSecs defines how long to wait in each one.
      mPerfCountsSz = 2000;
      unsigned periodUSecs = 10000;
      long lowerThresh = -(long)periodUSecs+1;    // Assert when outside an error range
      long lowerMeanThresh = -50;  // specified below.
      long upperThresh = periodUSecs*2-1;     // One for single values
      long upperMeanThresh = 50;   // One for mean values

      TimerNotification timerNotification(this);
      MpMMTimer* pMMTimer = NULL;
#ifdef WIN32
      MpMMTimerWnt mmTimerWnt(MpMMTimer::Notification);
      pMMTimer = &mmTimerWnt;

      double perfFreqPerUSec = initializeWin32PerfMeasurementTools();
      mpPerfCounts = new LARGE_INTEGER[mPerfCountsSz];
#else
      // Right now MMTimers are only implemented for win32.
      // as other platforms are implemented, change this.
      printf("MMTimer not implemented for this platform.  Test disabled.\n");
      return;
#endif

      // Set the notification..
      pMMTimer->setNotification(&timerNotification);

      // Initialize the timer.
      printf("Firing every %d usecs\n", periodUSecs);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->run(periodUSecs));

      // Wait for the callbacks to perform the timing measurements.
      OsTask::delay(periodUSecs*mPerfCountsSz/1000 + 50);
      
      // The callbacks should be done by now, so if they aren't,
      // then bitch.
      //We should have a current count of the actual size of the perf buffer.
      CPPUNIT_ASSERT_EQUAL(mPerfCountsSz, mCurNPerfCounts);

      // Clear the OsNotification, as it doesn't need to be continuing to notify.
      pMMTimer->setNotification(NULL);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->stop());

      // Determine delta times from individual time measurements..
      long* pDeltas = new long[mPerfCountsSz-1];
      long* pAbsDeltas = new long[mPerfCountsSz-1];
      unsigned i;
      for(i = 0; i < mPerfCountsSz-1; i = i++)
      {
#ifdef WIN32
         pDeltas[i] = 
            (long)(__int64(mpPerfCounts[i+1].QuadPart / perfFreqPerUSec) - 
                   __int64(mpPerfCounts[i].QuadPart / perfFreqPerUSec));
         pAbsDeltas[i] = 
            (long)((int64_t)((mpPerfCounts[i+1].QuadPart-mpPerfCounts[0].QuadPart) / perfFreqPerUSec) - 
            (int64_t)(periodUSecs)*i+1);
#else
         pDeltas[i] = 0;
         pAbsDeltas[i] = 0;
#endif
      }

      checkDeltasAgainstThresholds(pDeltas, pAbsDeltas, mPerfCountsSz-1,
                                   periodUSecs,
                                   lowerThresh, upperThresh);
#ifdef WIN32
      checkMeanAgainstThresholds((int64_t)(mpPerfCounts[0].QuadPart / perfFreqPerUSec),
                                 (int64_t)(mpPerfCounts[mPerfCountsSz-1].QuadPart / perfFreqPerUSec),
                                 mPerfCountsSz-1, 
                                 periodUSecs,
                                 lowerMeanThresh, upperMeanThresh);
#else
      printf("Mean performance timing not implemented on this platform\n");
#endif

      // Cleanup!
      delete[] pDeltas;
      pDeltas = NULL;
      delete[] pAbsDeltas;
      pAbsDeltas = NULL;
#ifdef WIN32
      delete[] mpPerfCounts;
      mpPerfCounts = NULL;
#endif
      mCurNPerfCounts = 0;
      mPerfCountsSz = 0;
   }

protected:

#ifdef WIN32
   LARGE_INTEGER* mpPerfCounts;
#endif
   unsigned mPerfCountsSz;
   unsigned mCurNPerfCounts;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpMMTimerTest);





// Implementation of TimerNotification methods.
TimerNotification::TimerNotification(MpMMTimerTest* pMMTimerTest) 
   : mpMMTimerTest(pMMTimerTest)
{
   CPPUNIT_ASSERT(pMMTimerTest != NULL);
}

OsStatus TimerNotification::signal(const intptr_t eventData) 
{
   mpMMTimerTest->notificationTimerRecordTick();
   return OS_SUCCESS;
}
