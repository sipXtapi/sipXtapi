//  
// Copyright (C) 2007-2026 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#include <os/OsIntTypes.h>
#ifdef ANDROID // [
// This fix is needed because in other case we gcc complains about struct iovec
// being redefined. This way we simply deny <linux/uio.h> from being included.
#  define __LINUX_UIO_H
#endif // ANDROID ]

#include <sipxunittests.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpAudioBuf.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#include <os/OsTask.h>
#include <os/OsEvent.h>
#include <os/OsCallback.h>
#include <os/OsDateTime.h>
#include <utl/UtlString.h>
#include <os/OsFS.h>

#define TEST_MSEC_PER_FRAME           10  // in milliseconds
#define TEST_FRAME_RATE               (1000/TEST_MSEC_PER_FRAME) // in Hz
// This defines are for the first set of tests ONLY (Create, Enable/Disable).
#define BUFFER_NUM                    500
#define TEST_SAMPLES_PER_SECOND       8000
#define TEST_SAMPLES_PER_FRAME_SIZE   (TEST_SAMPLES_PER_SECOND/TEST_FRAME_RATE)
#define TEST_SAMPLE_DATA_SIZE         (TEST_SAMPLES_PER_SECOND*1)
#define TEST_SAMPLE_DATA_MAGNITUDE    32000
#define TEST_SAMPLE_DATA_PERIOD       (1000000/60) //in microseconds 60 Hz

#define CREATE_TEST_RUNS_NUMBER              3
#define ENABLE_DISABLE_TEST_RUNS_NUMBER      5
#define ENABLE_DISABLE_FAST_TEST_RUNS_NUMBER 10
#define TICKER_TEST_LENGTH_SEC               1
#define MEASURE_JITTER_TEST_RUNS_NUMBER      1
#define MEASURE_JITTER_TEST_LENGTH_SEC       5
// Define this to enable writing of collected jitter data to files.
//#define WRITE_JITTER_RESULTS_TO_FILE

#undef USE_TEST_DRIVER

#ifdef USE_TEST_DRIVER // USE_TEST_DRIVER [
#include <mp/MpodBufferRecorder.h>
#define OUTPUT_DRIVER MpodBufferRecorder
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "default", TEST_SAMPLE_DATA_SIZE*1000/TEST_SAMPLES_PER_SECOND

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#include <mp/MpodWinMM.h>
#define OUTPUT_DRIVER MpodWinMM
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS MpodWinMM::getDefaultDeviceName(), 0

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#  if __APPLE__
#     include <mp/MpodCoreAudio.h>
#     define OUTPUT_DRIVER MpodCoreAudio
#     define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "[default]"
#  elif defined(ANDROID)
#     include <mp/MpodAndroid.h>
#     define OUTPUT_DRIVER MpodAndroid
#     define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS MpAndroidAudioBindingInterface::ENFORCED_AUDIBLE
#  else
#     include <mp/MpodOss.h>
#     define OUTPUT_DRIVER MpodOss
#     define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "/dev/dsp"
#  endif
#else // __pingtel_on_posix__ ]
#error Unknown platform!
#endif

// Test sample rates set
static int sampleRates[]={8000, 16000, 32000, 48000};
static int rateIndex;
// Sample data array
static MpAudioSample* sampleData;
static int sampleDataSz;
// Current time for callback
static int frameInCallback;
static MpFrameTime frameTime;

static void calculateSampleData(int frequency, 
                                MpAudioSample sampleData[], 
                                int sampleDataSz,
                                int samplesPerFrame,
                                int samplesPerSecond)
{
   for (int i=0; i<sampleDataSz; i++)
   {
      sampleData[i] = 
         MpSineWaveGeneratorDeviceDriver::calculateSample(0,
                                                          TEST_SAMPLE_DATA_MAGNITUDE,
                                                          1000000 / frequency,
                                                          i,
                                                          samplesPerFrame,
                                                          samplesPerSecond);
   }
}

/**
 * Unittest for MpOutputDeviceDriver
 */
class MpOutputDeviceDriverTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(MpOutputDeviceDriverTest);
   CPPUNIT_TEST(testCreate);
   CPPUNIT_TEST(testEnableDisable);
   CPPUNIT_TEST(testEnableDisableFast);
   CPPUNIT_TEST(testTickerNotification);
   CPPUNIT_TEST(testEnableAfterSwitchToMMTimer);
   CPPUNIT_TEST(testMultipleEnableDisableCyclesAfterFallback);
   CPPUNIT_TEST(testSwitchToMMTimerWhenDisabled);
   CPPUNIT_TEST(testPushFrameDuringFallback);
   CPPUNIT_TEST(measureJitter);
   CPPUNIT_TEST_SUITE_END();


public:

   void setUp()
   {
      // Create pool for data buffers
      mpPool = new MpBufPool(TEST_SAMPLES_PER_FRAME_SIZE * sizeof(MpAudioSample)
                             + MpArrayBuf::getHeaderSize(), BUFFER_NUM, "MpOutputDriverTest");
      CPPUNIT_ASSERT(mpPool != NULL);

      // Create pool for buffer headers
      mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), BUFFER_NUM, "MpOutputDriverTestHeaders");
      CPPUNIT_ASSERT(mpHeadersPool != NULL);

      // Set mpHeadersPool as default pool for audio and data pools.
      MpAudioBuf::smpDefaultPool = mpHeadersPool;
      MpDataBuf::smpDefaultPool = mpHeadersPool;
   }

   void tearDown()
   {
      if (mpPool != NULL)
      {
         delete mpPool;
      }
      if (mpHeadersPool != NULL)
      {
         delete mpHeadersPool;
      }
   }

   void testCreate()
   {
      for (int i=0; i<CREATE_TEST_RUNS_NUMBER; i++)
      {
         OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);

         CPPUNIT_ASSERT(!driver.isEnabled());
      }
   }

   void testEnableDisable()
   {
      CallbackUserData userData;
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      printf("testEnableDisable created driver: %p\n",
              &driver);
      CPPUNIT_ASSERT(!driver.isEnabled());

      userData.mDriver = &driver;
      userData.mTickTime = NULL;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);
      sampleDataSz = 0;

      for (int i=0; i<ENABLE_DISABLE_TEST_RUNS_NUMBER; i++)
      {
         driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND,
                             0, notificationCallback);
         CPPUNIT_ASSERT(driver.isEnabled());

         OsTask::delay(50);

         driver.disableDevice();
         CPPUNIT_ASSERT(!driver.isEnabled());
      }
   }

   void testEnableDisableFast()
   {
      CallbackUserData userData;
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!driver.isEnabled());

      userData.mDriver = &driver;
      userData.mTickTime = NULL;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);
      sampleDataSz = 0;

      for (int i=0; i<ENABLE_DISABLE_FAST_TEST_RUNS_NUMBER; i++)
      {
         driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND,
                             0, notificationCallback);
         CPPUNIT_ASSERT(driver.isEnabled());

         driver.disableDevice();
         CPPUNIT_ASSERT(!driver.isEnabled());
      }
   }

   void testTickerNotification()
   {
      CallbackUserData userData;
      int numRates = sizeof(sampleRates)/sizeof(int);
      int frequencies[] = {1000, 2000, 4000, 8000, 16000, 20000, 24000, 28000};
      int numFreqs = sizeof(frequencies)/sizeof(int);

      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!driver.isEnabled());

      userData.mDriver = &driver;
      userData.mTickTime = NULL;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);

      for(rateIndex = 0; rateIndex < numRates; rateIndex++)
      {
         sampleDataSz = sampleRates[rateIndex]*TICKER_TEST_LENGTH_SEC;
         sampleData = new MpAudioSample[sampleDataSz];

         for (int i=0; i<numFreqs; i++)
         {
            printf("Frequency: %d (Hz) Sample rate: %d/sec.\n", 
               frequencies[i], sampleRates[rateIndex]);
            calculateSampleData(frequencies[i], sampleData, sampleDataSz, 
                                sampleRates[rateIndex]/TEST_FRAME_RATE, sampleRates[rateIndex]);

            frameInCallback=0;
            driver.enableDevice(sampleRates[rateIndex]/TEST_FRAME_RATE, sampleRates[rateIndex],
                                0, notificationCallback);
            CPPUNIT_ASSERT(driver.isEnabled());

            // Wait defined amount of time.
            OsTask::delay(TICKER_TEST_LENGTH_SEC*1000);

            driver.disableDevice();
            printf("Frames played: %d\n", frameInCallback);
            CPPUNIT_ASSERT(!driver.isEnabled());
            CPPUNIT_ASSERT(frameInCallback > TICKER_TEST_LENGTH_SEC*TEST_FRAME_RATE);
         }
         delete[] sampleData;
      }
   }

   void testEnableAfterSwitchToMMTimer()
   {
   #ifdef WIN32
       CallbackUserData userData;
       OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
   
       if (!driver.isDeviceValid())
       {
           printf("No output device available, skipping testEnableAfterSwitchToMMTimer\n");
           return;
       }
   
       CPPUNIT_ASSERT(!driver.isEnabled());
       CPPUNIT_ASSERT(!driver.isUsingFallbackTimer());
   
       userData.mDriver = &driver;
       userData.mTickTime = NULL;
       OsCallback notificationCallback((intptr_t)&userData, &driverCallback);
       sampleDataSz = 0;
   
       // Enable the device
       CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
           driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND,
                               0, notificationCallback));
       CPPUNIT_ASSERT(driver.isEnabled());
       CPPUNIT_ASSERT(!driver.isUsingFallbackTimer());
   
       // Let it run briefly
       OsTask::delay(50);
   
       // Simulate device failure by switching to fallback timer
       CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.switchToMMTimer());
       CPPUNIT_ASSERT(driver.isUsingFallbackTimer());
   
       // Let the fallback timer run briefly
       OsTask::delay(50);
   
       // Disable
       driver.disableDevice();
       CPPUNIT_ASSERT(!driver.isEnabled());
       // Timer still exists after disable (not cleaned up there)
       CPPUNIT_ASSERT(driver.isUsingFallbackTimer());
   
       // Re-enable -- this is the key test.
       // Before the fix, mpTickerTimer would survive and internalPushFrame
       // would silently discard all audio.
       CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
           driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND,
                               0, notificationCallback));
       CPPUNIT_ASSERT(driver.isEnabled());
       CPPUNIT_ASSERT(!driver.isUsingFallbackTimer());
   
       // Let it run to confirm audio is flowing
       OsTask::delay(100);
   
       // Clean up
       driver.disableDevice();
       CPPUNIT_ASSERT(!driver.isEnabled());
   #else
       // This test is Windows-specific
       printf("Skipping testEnableAfterSwitchToMMTimer on non-Windows platform\n");
   #endif
   }

void testMultipleEnableDisableCyclesAfterFallback()
   {
   #ifdef WIN32
      // Test #4: Verify enable->fallback->disable->enable->disable cycles
      // do not get stuck in fallback mode. Direct regression for the
      // "no audio after reconnect" bug (Change 5: mpTickerTimer leak across
      // disable/enable). Each successful enable must clear the fallback
      // timer before opening the wave device.

      CallbackUserData userData;
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);

      if (!driver.isDeviceValid())
      {
         printf("No output device available, skipping testMultipleEnableDisableCyclesAfterFallback\n");
         return;
      }

      userData.mDriver = &driver;
      userData.mTickTime = NULL;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);
      sampleDataSz = 0;

      const int CYCLES = 5;
      for (int cycle = 0; cycle < CYCLES; cycle++)
      {
         UtlString cycleMsg;
         cycleMsg.appendFormat("cycle: %d", cycle);

         // Enable -- must clear any leftover fallback timer from the
         // previous cycle.
         CPPUNIT_ASSERT_EQUAL_MESSAGE(cycleMsg.data(), OS_SUCCESS,
            driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE,
                                TEST_SAMPLES_PER_SECOND,
                                0, notificationCallback));
         CPPUNIT_ASSERT_MESSAGE(cycleMsg.data(), driver.isEnabled());
         CPPUNIT_ASSERT_MESSAGE(cycleMsg.data(),
                                !driver.isUsingFallbackTimer());

         OsTask::delay(50);

         // Force the fallback path. This is what would happen on a real
         // USB unplug.
         CPPUNIT_ASSERT_EQUAL_MESSAGE(cycleMsg.data(), OS_SUCCESS,
                                      driver.switchToMMTimer());
         CPPUNIT_ASSERT_MESSAGE(cycleMsg.data(),
                                driver.isUsingFallbackTimer());

         OsTask::delay(50);

         driver.disableDevice();
         CPPUNIT_ASSERT_MESSAGE(cycleMsg.data(), !driver.isEnabled());
      }

      // After the loop, prove the next enable still works cleanly with
      // no leftover fallback state.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE,
                             TEST_SAMPLES_PER_SECOND,
                             0, notificationCallback));
      CPPUNIT_ASSERT(driver.isEnabled());
      CPPUNIT_ASSERT(!driver.isUsingFallbackTimer());

      driver.disableDevice();
      CPPUNIT_ASSERT(!driver.isEnabled());
   #else
      printf("Skipping testMultipleEnableDisableCyclesAfterFallback on non-Windows platform\n");
   #endif
   }

   void testSwitchToMMTimerWhenDisabled()
   {
   #ifdef WIN32
      // Test #5: Verify the isEnabled() guard at the top of
      // switchToMMTimer (Change 5). Calling switchToMMTimer on a disabled
      // device must not create a fallback timer and must not crash. This
      // simulates a COM callback firing during or after disableDevice.

      CallbackUserData userData;
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);

      if (!driver.isDeviceValid())
      {
         printf("No output device available, skipping testSwitchToMMTimerWhenDisabled\n");
         return;
      }

      // Case A: never enabled.
      CPPUNIT_ASSERT(!driver.isEnabled());
      CPPUNIT_ASSERT(!driver.isUsingFallbackTimer());
      // Should be a no-op. Return value is OS_SUCCESS by current contract.
      driver.switchToMMTimer();
      CPPUNIT_ASSERT(!driver.isEnabled());
      CPPUNIT_ASSERT_MESSAGE("switchToMMTimer must not create a timer when device was never enabled",
                             !driver.isUsingFallbackTimer());

      // Case B: enabled then cleanly disabled.
      userData.mDriver = &driver;
      userData.mTickTime = NULL;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);
      sampleDataSz = 0;

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE,
                             TEST_SAMPLES_PER_SECOND,
                             0, notificationCallback));
      CPPUNIT_ASSERT(driver.isEnabled());
      OsTask::delay(50);
      driver.disableDevice();
      CPPUNIT_ASSERT(!driver.isEnabled());
      CPPUNIT_ASSERT(!driver.isUsingFallbackTimer());

      // Now the late COM-callback case.
      driver.switchToMMTimer();
      CPPUNIT_ASSERT_MESSAGE("switchToMMTimer must not create a timer after disableDevice",
                             !driver.isUsingFallbackTimer());
      CPPUNIT_ASSERT(!driver.isEnabled());

      // Case C: re-enable must still work after the spurious switchToMMTimer.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE,
                             TEST_SAMPLES_PER_SECOND,
                             0, notificationCallback));
      CPPUNIT_ASSERT(driver.isEnabled());
      CPPUNIT_ASSERT(!driver.isUsingFallbackTimer());
      driver.disableDevice();
      CPPUNIT_ASSERT(!driver.isEnabled());
   #else
      printf("Skipping testSwitchToMMTimerWhenDisabled on non-Windows platform\n");
   #endif
   }

   void testPushFrameDuringFallback()
   {
   #ifdef WIN32
      // Test #6: pushFrame must succeed and must not crash while the
      // device is in fallback mode (mDevHandle is NULL). Direct regression
      // for Change 7 (pushFrame check on mDevHandle instead of
      // mpTickerTimer) and Change 3 (mDevHandle reliably NULL after
      // failure). Verifies internalPushFrame's silent-discard path also
      // returns success so the flowgraph keeps ticking.

      CallbackUserData userData;
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);

      if (!driver.isDeviceValid())
      {
         printf("No output device available, skipping testPushFrameDuringFallback\n");
         return;
      }

      userData.mDriver = &driver;
      userData.mTickTime = NULL;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);

      // Build a small buffer of audio data the test will push.
      const int TEST_PUSH_FRAMES = 50;
      const int samplesPerFrame = TEST_SAMPLES_PER_FRAME_SIZE;
      sampleDataSz = TEST_PUSH_FRAMES * samplesPerFrame;
      sampleData = new MpAudioSample[sampleDataSz];
      calculateSampleData(440, sampleData, sampleDataSz, samplesPerFrame,
                          TEST_SAMPLES_PER_SECOND);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         driver.enableDevice(samplesPerFrame, TEST_SAMPLES_PER_SECOND,
                             0, notificationCallback));
      CPPUNIT_ASSERT(driver.isEnabled());
      CPPUNIT_ASSERT(!driver.isUsingFallbackTimer());

      // Force fallback mode. After this, mDevHandle should be NULL and
      // pushFrame must follow the silent-discard path.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.switchToMMTimer());
      CPPUNIT_ASSERT(driver.isUsingFallbackTimer());

      // Push frames directly from this thread (not via the callback), to
      // exercise the pushFrame path independently of the fallback timer
      // tick rate.
      MpFrameTime t = 0;
      for (int i = 0; i < TEST_PUSH_FRAMES; i++)
      {
         UtlString frameMsg;
         frameMsg.appendFormat("pushFrame iteration: %d", i);
         CPPUNIT_ASSERT_EQUAL_MESSAGE(frameMsg.data(), OS_SUCCESS,
            driver.pushFrame(samplesPerFrame,
                             sampleData + samplesPerFrame * i,
                             t));
         t += samplesPerFrame;
      }

      // Also push a zero-length frame -- another path through pushFrame.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.pushFrame(0, NULL, t));

      // Disable while in fallback mode -- must not crash even though
      // mDevHandle is NULL.
      driver.disableDevice();
      CPPUNIT_ASSERT(!driver.isEnabled());

      delete[] sampleData;
      sampleData = NULL;
      sampleDataSz = 0;
   #else
      printf("Skipping testPushFrameDuringFallback on non-Windows platform\n");
   #endif
   }

   void measureJitter()
   {
      CallbackUserData userData;
      OsEvent notificationEvent;
      int numRates = sizeof(sampleRates)/sizeof(int);
      // The tone we will test with is an 'A' 440Hz sine tone.
      int testFrequency = 440;

      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!driver.isEnabled());

      userData.mDriver = &driver;
      userData.mTickTime = NULL;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);

      for(rateIndex = 0; rateIndex < numRates; rateIndex++)
      {
         sampleDataSz = MEASURE_JITTER_TEST_LENGTH_SEC*sampleRates[rateIndex];
         const int samplesPerFrame = sampleRates[rateIndex]/TEST_FRAME_RATE;
         const int numFrames = sampleDataSz/samplesPerFrame;
         sampleData = new MpAudioSample[sampleDataSz];

         // Calculate the data for playing one tone
         calculateSampleData(testFrequency, sampleData, sampleDataSz, 
                             samplesPerFrame, sampleRates[rateIndex]);

         // Create buffers for capturing jitter data
         OsTime* jitterTimes[MEASURE_JITTER_TEST_RUNS_NUMBER];
         for(int i = 0; i< MEASURE_JITTER_TEST_RUNS_NUMBER; i++)
            jitterTimes[i] = new OsTime[numFrames];

         // Collect a number of runs of test data defined at the top of the test file.
         for (int curRunIdx=0; curRunIdx<MEASURE_JITTER_TEST_RUNS_NUMBER; curRunIdx++)
         {
            // Enable the device for each run so that we get an accurate setup/teardown
            // for enable/disable cycle captured in each test run.
            frameInCallback=0;
            userData.mTickTime = jitterTimes[curRunIdx];
            userData.mTickTimeLength = numFrames;
            driver.enableDevice(samplesPerFrame, sampleRates[rateIndex], 0, notificationCallback);
            CPPUNIT_ASSERT(driver.isEnabled());

            // Write all the data we calculated to the device.
            OsTask::delay(MEASURE_JITTER_TEST_LENGTH_SEC*1000);

            // Disable the device to complete the run -- so that we get an accurate
            // setup/teardown for enable/disable cycle captured.
            driver.disableDevice();
            CPPUNIT_ASSERT(!driver.isEnabled());
            CPPUNIT_ASSERT(frameInCallback > TICKER_TEST_LENGTH_SEC*TEST_FRAME_RATE);

            // Now we write out the jitter data we collected to a csv file
            // First we create a UtlString that contains the formatting of data to write
            // to file.
            UtlString jitterDataStr = "";
#define TMPBUFLEN 50
            char tmpBuf[TMPBUFLEN];
            int curTmpBufLen = 0;
            for(int run = 0; run<MEASURE_JITTER_TEST_RUNS_NUMBER; run++)
            {
               for(int frame=0; frame<numFrames; frame++)
               {
                  curTmpBufLen = snprintf(tmpBuf, TMPBUFLEN, "%d",
                     (int)(jitterTimes[curRunIdx][frame].cvtToMsecs()-jitterTimes[curRunIdx][0].cvtToMsecs()));
                  jitterDataStr += tmpBuf;
                  jitterDataStr += ",";
               }
               jitterDataStr += "\n";
            }
            printf(jitterDataStr.data());
#ifdef WRITE_JITTER_RESULTS_TO_FILE // [
            curTmpBufLen = snprintf(tmpBuf, TMPBUFLEN,
               "./audiodevice_out_jitter_%dHz.csv", sampleRates[rateIndex]);
            CPPUNIT_ASSERT_EQUAL(jitterDataStr.length(),
               (size_t)OsFile::openAndWrite(tmpBuf, jitterDataStr));
#endif // WRITE_JITTER_RESULTS_TO_FILE ]
         }

         // Clean up - delete each of the second arrays of the jitterTimes multidimensional array.
         for (int i = 0; i< MEASURE_JITTER_TEST_RUNS_NUMBER; i++)
            delete[] jitterTimes[i];

         delete[] sampleData;
      }
   }

protected:

   struct CallbackUserData
   {
      MpOutputDeviceDriver *mDriver;
      OsTime* mTickTime;
      int mTickTimeLength;
   };

   static void driverCallback(const intptr_t userData, const intptr_t eventData)
   {
      CallbackUserData *pData = (CallbackUserData*)userData;
      const int samplesPerFrame = sampleRates[rateIndex]/TEST_FRAME_RATE;

      // Measure tick time
      if (pData->mTickTime != NULL && frameInCallback < pData->mTickTimeLength)
      {
         OsDateTime::getCurTime(pData->mTickTime[frameInCallback]);
      }

      // Push data to the driver
      if (samplesPerFrame*frameInCallback < sampleDataSz)
      {
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              pData->mDriver->pushFrame(samplesPerFrame,
                                                        sampleData + samplesPerFrame*frameInCallback,
                                                        frameTime));
      }
      else
      {
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              pData->mDriver->pushFrame(0, NULL, frameTime));
      }
      frameInCallback++;
      frameTime += samplesPerFrame;
   }

   MpBufPool *mpPool;         ///< Pool for data buffers
   MpBufPool *mpHeadersPool;  ///< Pool for buffers headers
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpOutputDeviceDriverTest);
