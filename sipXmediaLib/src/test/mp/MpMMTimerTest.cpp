//  
// Copyright (C) 2007-2010 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <os/OsIntTypes.h>

#include <sipxunittests.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpMMTimer.h>
#include <os/OsDateTime.h>
#include <os/OsSysLog.h>
#include <os/OsNotification.h>
#include <os/OsTask.h>

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
class MpMMTimerTest : public SIPX_UNIT_BASE_CLASS
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
      mpPerfTimes = NULL;
      mPerfTimesSz = 0;
      mCurNPerfTimes = 0;
   }

   void tearDown()
   {
      if(mpPerfTimes != NULL)
      {
         delete[] mpPerfTimes;
      }
      mpPerfTimes = NULL;
      mPerfTimesSz = 0;
      mCurNPerfTimes = 0;
   }

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

   void checkMeanAgainstThresholds(const OsTime& start, const OsTime& stop, 
                                   unsigned nDeltas,
                                   unsigned targetDelta, // <-- was periodUSecs
                                   long lowerMeanThresh, long upperMeanThresh)
   {
      CPPUNIT_ASSERT_MESSAGE("Timer didn't fire or deltas were not collected!",
                             nDeltas > 0);

      double meanAvg = 0;

      int64_t totStartUSecs = start.seconds()*1000*1000 + start.usecs();
      int64_t totStopUSecs = stop.seconds()*1000*1000 + stop.usecs();

      meanAvg = (totStopUSecs-totStartUSecs)/(double)nDeltas;
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
      MpMMTimer* pMMTimer = MpMMTimer::create(MpMMTimer::Notification);
      if (pMMTimer == NULL)
      {
         printf("MpMMTimer is not yet implemented on this platform, excluding test.\n");
         return;
      }

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->getResolution(resolution));
      printf("Minimum timer resolution is %d usecs\n", resolution);
      delete pMMTimer;
   }

   void testPeriodRange()
   {
      // Test the period range static method..
      unsigned unusedMin = 0;
      unsigned unusedMax = 0;
      MpMMTimer* pMMTimer = MpMMTimer::create(MpMMTimer::Notification);
      if (pMMTimer == NULL)
      {
         printf("MpMMTimer is not yet implemented on this platform, excluding test.\n");
         return;
      }

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         pMMTimer->getPeriodRange(&unusedMin, &unusedMax));
      delete pMMTimer;
   }

   void testLinearTimer()
   {
#ifdef ANDROID
      CPPUNIT_ASSERT_MESSAGE("ANDROID_HANG", 0);
      return;
#endif

      // Set the below variables and preprocessor defines to tweak this test.
      // TLT_LOOP_COUNT defines the number of timer fire repetitions to do 
      // (set this to an even value),
      // and periodUSecs defines how long to wait in each one.
#     define TLT_LOOP_CNT 2000
      unsigned periodUSecs = 10000;
      long lowerThresh = -(long)periodUSecs; // Assert when outside an error range
      long lowerMeanThresh = -50;            // specified below.
      long upperThresh = periodUSecs*2;      // One for single values
      long upperMeanThresh = 50;             // One for mean values

      MpMMTimer* pMMTimer = MpMMTimer::create(MpMMTimer::Linear);
      if (pMMTimer == NULL)
      {
         printf("MpMMTimer is not yet implemented on this platform, excluding test.\n");
         return;
      }

      // Allocate time objects to hold measurements
      OsTime perfTimes[TLT_LOOP_CNT];

      // Initialize the timer.
      printf("Firing every %d usecs\n", periodUSecs);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->run(periodUSecs));

      // Perform the timing measurements.
      int i;
      for(i = 0; i < TLT_LOOP_CNT; i++)
      {
         OsDateTime::getCurTime(perfTimes[i]);
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->waitForNextTick());
      }

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->stop());

      // Determine delta times from individual time measurements..
      int64_t curUsec_i = 0;
      int64_t curUsec_iPlus1 = 0;
      int64_t curUsec_0 = 0;
      long deltas[TLT_LOOP_CNT-1];
      long absDeltas[TLT_LOOP_CNT-1];
      for(i = 0; i < TLT_LOOP_CNT-1; i++)
      {
         curUsec_i = perfTimes[i].seconds()*1000*1000 + perfTimes[i].usecs();
         curUsec_iPlus1 = perfTimes[i+1].seconds()*1000*1000 + perfTimes[i+1].usecs();
         curUsec_0 = perfTimes[0].seconds()*1000*1000 + perfTimes[0].usecs();

         deltas[i] = (long)(curUsec_iPlus1 - curUsec_i);
         absDeltas[i] = (long)((curUsec_iPlus1 - curUsec_0) - periodUSecs*i);
      }


      delete pMMTimer;

      checkDeltasAgainstThresholds(deltas, absDeltas, TLT_LOOP_CNT-1, 
                                   periodUSecs, 
                                   lowerThresh, upperThresh);
      checkMeanAgainstThresholds(perfTimes[0], perfTimes[TLT_LOOP_CNT-1],
                                 TLT_LOOP_CNT-1, 
                                 periodUSecs, 
                                 lowerMeanThresh, upperMeanThresh);

   }

   void notificationTimerRecordTick()
   {
      int x;
      x = mPerfTimesSz;
      CPPUNIT_ASSERT(mpPerfTimes != NULL);
      CPPUNIT_ASSERT(mPerfTimesSz > 0);

      // Collect measurements while we have space left.
      if(mCurNPerfTimes < mPerfTimesSz)
      {
         OsDateTime::getCurTime(mpPerfTimes[mCurNPerfTimes]);
         mCurNPerfTimes++;
      }
   }

   void testNotificationTimer()
   {
      // Set the below variables and preprocessor defines to tweak this test.
      // mPerfCountsSz defines the number of timer fire repetitions to do 
      // (set this to an even value),
      // and periodUSecs defines how long to wait in each one.
      mPerfTimesSz = 2000;
      unsigned periodUSecs = 10000;
      long lowerThresh = -(long)periodUSecs+1; // Assert when outside an error range
      long lowerMeanThresh = -50;              // specified below.
      long upperThresh = periodUSecs*2-1;      // One for single values
      long upperMeanThresh = 50;               // One for mean values

      TimerNotification timerNotification(this);
      MpMMTimer* pMMTimer = MpMMTimer::create(MpMMTimer::Notification);
      if (pMMTimer == NULL)
      {
         printf("MpMMTimer is not yet implemented on this platform, excluding test.\n");
         return;
      }

      // Set the notification..
      pMMTimer->setNotification(&timerNotification);

      // Allocate time objects to hold measurements
      mpPerfTimes = new OsTime[mPerfTimesSz];

      // Initialize the timer.
      printf("Firing every %d usecs\n", periodUSecs);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->run(periodUSecs));

      // Wait for the callbacks to perform the timing measurements.
      OsTask::delay(periodUSecs*mPerfTimesSz/1000 + 50);
      
      // The callbacks should be done by now, so if they aren't,
      // then bitch.
      //We should have a current count of the actual size of the perf buffer.
      CPPUNIT_ASSERT_EQUAL(mPerfTimesSz, mCurNPerfTimes);

      // Clear the OsNotification, as it doesn't need to be continuing to notify.
      pMMTimer->setNotification(NULL);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->stop());

      // Determine delta times from individual time measurements..
      int64_t curUsec_i = 0;
      int64_t curUsec_iPlus1 = 0;
      int64_t curUsec_0 = 0;
      long* pDeltas = new long[mPerfTimesSz-1];
      long* pAbsDeltas = new long[mPerfTimesSz-1];
      unsigned i;
      for(i = 0; i < mPerfTimesSz-1; i++)
      {
         curUsec_i = mpPerfTimes[i].seconds()*1000*1000 + mpPerfTimes[i].usecs();
         curUsec_iPlus1 = mpPerfTimes[i+1].seconds()*1000*1000 + mpPerfTimes[i+1].usecs();
         curUsec_0 = mpPerfTimes[0].seconds()*1000*1000 + mpPerfTimes[0].usecs();

         pDeltas[i] = (long)(curUsec_iPlus1 - curUsec_i);
         pAbsDeltas[i] = (long)((curUsec_iPlus1 - curUsec_0) - periodUSecs*i);
      }

      delete pMMTimer;

      checkDeltasAgainstThresholds(pDeltas, pAbsDeltas, mPerfTimesSz-1,
                                   periodUSecs,
                                   lowerThresh, upperThresh);
      checkMeanAgainstThresholds(mpPerfTimes[0], mpPerfTimes[mPerfTimesSz-1],
                                 mPerfTimesSz-1, 
                                 periodUSecs,
                                 lowerMeanThresh, upperMeanThresh);

      // Cleanup!
      delete[] pDeltas;
      pDeltas = NULL;
      delete[] pAbsDeltas;
      pAbsDeltas = NULL;
      delete[] mpPerfTimes;
      mpPerfTimes = NULL;
      mCurNPerfTimes = 0;
      mPerfTimesSz = 0;
   }

protected:

   OsTime* mpPerfTimes;
   unsigned mPerfTimesSz;
   unsigned mCurNPerfTimes;
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
