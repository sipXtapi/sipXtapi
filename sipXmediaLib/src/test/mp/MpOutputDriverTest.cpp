//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#include <os/OsIntTypes.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpAudioBuf.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#include <os/OsTask.h>
#include <os/OsEvent.h>
#include <os/OsDateTime.h>
#include <utl/UtlString.h>
#include <os/OsFS.h>

#define TEST_SAMPLES_PER_FRAME_SIZE   80
#define BUFFER_NUM                    500
#define TEST_SAMPLES_PER_SECOND       8000
#define TEST_SAMPLE_DATA_SIZE         (TEST_SAMPLES_PER_SECOND*1)
#define TEST_SAMPLE_DATA_MAGNITUDE    32000
#define TEST_SAMPLE_DATA_PERIOD       (1000000/60) //in microseconds 60 Hz

#define CREATE_TEST_RUNS_NUMBER              3
#define ENABLE_DISABLE_TEST_RUNS_NUMBER      5
#define ENABLE_DISABLE_FAST_TEST_RUNS_NUMBER 10
#define DIRECT_WRITE_TEST_RUNS_NUMBER        3
#define TICKER_TEST_WRITE_RUNS_NUMBER        3
#define MEASURE_JITTER_TEST_RUNS_NUMBER      1
#define MEASURE_JITTER_TEST_LENGTH_SEC       5
// Define this to enable writing of colected jitter data to files.
//#define WRITE_JITTER_RESULTS_TO_FILE

#undef USE_TEST_DRIVER

#ifdef USE_TEST_DRIVER // USE_TEST_DRIVER [
#include <mp/MpodBufferRecorder.h>
#define OUTPUT_DRIVER MpodBufferRecorder
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "default", TEST_SAMPLE_DATA_SIZE*1000/TEST_SAMPLES_PER_SECOND

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#include <mp/MpodWinMM.h>
#define OUTPUT_DRIVER MpodWinMM
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS MpodWinMM::getDefaultDeviceName()

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#include <mp/MpodOss.h>
#define OUTPUT_DRIVER MpodOss
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "/dev/dsp"

#else // __pingtel_on_posix__ ]
#error Unknown platform!
#endif

//static MpAudioSample sampleData[TEST_SAMPLE_DATA_SIZE];
//static UtlBoolean sampleDataInitialized=FALSE;

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
class MpOutputDeviceDriverTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpOutputDeviceDriverTest);
   CPPUNIT_TEST(testCreate);
   CPPUNIT_TEST(testEnableDisable);
   CPPUNIT_TEST(testEnableDisableFast);
   // This test is disabled, because it audio quality during it may be very bad.
   // Thus enable it only to test brand new drivers, which don't support notifications yet.
//   CPPUNIT_TEST(testDirectWrite);
   CPPUNIT_TEST(testTickerNotification);
   CPPUNIT_TEST(measureJitter);
   CPPUNIT_TEST_SUITE_END();


public:

   void setUp()
   {
      // Create pool for data buffers
      mpPool = new MpBufPool(TEST_SAMPLES_PER_FRAME_SIZE * sizeof(MpAudioSample)
                             + MpArrayBuf::getHeaderSize(), BUFFER_NUM);
      CPPUNIT_ASSERT(mpPool != NULL);

      // Create pool for buffer headers
      mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), BUFFER_NUM);
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
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!driver.isEnabled());

      for (int i=0; i<ENABLE_DISABLE_TEST_RUNS_NUMBER; i++)
      {
         driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND, 0);
         CPPUNIT_ASSERT(driver.isEnabled());

         OsTask::delay(50);

         driver.disableDevice();
         CPPUNIT_ASSERT(!driver.isEnabled());
      }
   }

   void testEnableDisableFast()
   {
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!driver.isEnabled());

      for (int i=0; i<ENABLE_DISABLE_FAST_TEST_RUNS_NUMBER; i++)
      {
         driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND, 0);
         CPPUNIT_ASSERT(driver.isEnabled());

         driver.disableDevice();
         CPPUNIT_ASSERT(!driver.isEnabled());
      }
   }

   void testDirectWrite()
   {
      MpAudioSample sampleData[TEST_SAMPLE_DATA_SIZE];
      calculateSampleData(440, sampleData, TEST_SAMPLE_DATA_SIZE, 80, 8000);

      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!driver.isEnabled());

      for (int i=0; i<DIRECT_WRITE_TEST_RUNS_NUMBER; i++)
      {
         MpFrameTime frameTime=0;

         driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND, 0);
         CPPUNIT_ASSERT(driver.isEnabled());

         // Write some data to device.
         for (int frame=0; frame<TEST_SAMPLE_DATA_SIZE/TEST_SAMPLES_PER_FRAME_SIZE; frame++)
         {
            OsTask::delay(1000*TEST_SAMPLES_PER_FRAME_SIZE/TEST_SAMPLES_PER_SECOND);
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 driver.pushFrame(TEST_SAMPLES_PER_FRAME_SIZE,
                                                  sampleData + TEST_SAMPLES_PER_FRAME_SIZE*frame,
                                                  frameTime));

            frameTime += TEST_SAMPLES_PER_FRAME_SIZE*1000/TEST_SAMPLES_PER_SECOND;
         }

         driver.disableDevice();
         CPPUNIT_ASSERT(!driver.isEnabled());
      }
   }

   void testTickerNotification()
   {
      OsEvent notificationEvent;
      int sampleRates[]={8000, 16000, 32000, 48000};
      int numRates = sizeof(sampleRates)/sizeof(int);
      int frequencies[] = {1000, 2000, 4000, 8000, 16000, 20000, 24000, 28000};
      int numFreqs = sizeof(frequencies)/sizeof(int);

      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!driver.isEnabled());

      int rateIndex;
      for(rateIndex = 0; rateIndex < numRates; rateIndex++)
      {
         int sampleDataSz = sampleRates[rateIndex];
         MpAudioSample* sampleData = new MpAudioSample[sampleDataSz];

         for (int i=0; i<numFreqs; i++)
         {
            printf("Frequency: %d (Hz) Sample rate: %d/sec.\n", 
               frequencies[i], sampleRates[rateIndex]);
            calculateSampleData(frequencies[i], sampleData, sampleDataSz, 
                                sampleRates[rateIndex]/100, sampleRates[rateIndex]);
            MpFrameTime frameTime=0;

            driver.enableDevice(sampleRates[rateIndex]/100, sampleRates[rateIndex], 0);
            CPPUNIT_ASSERT(driver.isEnabled());

            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.setTickerNotification(&notificationEvent));

            // Write some data to device.
            for (int frame=0; frame<100; frame++)
            {
               CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notificationEvent.wait(OsTime(1000)));
               notificationEvent.reset();
               CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                    driver.pushFrame(sampleRates[rateIndex]/100,
                                                     sampleData + sampleRates[rateIndex]/100*frame,
                                                     frameTime));

               frameTime += sampleRates[rateIndex]/100*1000/sampleRates[rateIndex];
            }

            CPPUNIT_ASSERT(driver.setTickerNotification(NULL) == OS_SUCCESS);

            driver.disableDevice();
            CPPUNIT_ASSERT(!driver.isEnabled());
         }
         delete[] sampleData;
      }
   }

   void measureJitter()
   {
      OsEvent notificationEvent;
      int sampleRates[]={8000, 16000, 32000, 44100, 48000, 96000};
      int numRates = sizeof(sampleRates)/sizeof(int);
      // The tone we will test with is an 'A' 440Hz sine tone.
      int testFrequency = 440;

      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!driver.isEnabled());

      int rateIndex;
      for(rateIndex = 0; rateIndex < numRates; rateIndex++)
      {
         int sampleDataSz = MEASURE_JITTER_TEST_LENGTH_SEC*sampleRates[rateIndex];
         int samplesPerFrame = sampleRates[rateIndex]/100; // 10ms(=1/100sec) frames
         int numFrames = sampleDataSz/samplesPerFrame;
         MpAudioSample* sampleData = new MpAudioSample[sampleDataSz];
         MpFrameTime frameTime=0;

         // Calculate the data for playing one tone
         calculateSampleData(testFrequency, sampleData, sampleDataSz, 
            samplesPerFrame, sampleRates[rateIndex]);

         // Create buffers for capturing jitter data
         OsTime* jitterTimes[MEASURE_JITTER_TEST_RUNS_NUMBER];
         int i;
         for(i = 0; i< MEASURE_JITTER_TEST_RUNS_NUMBER; i++)
            jitterTimes[i] = new OsTime[numFrames];

         // Collect a number of runs of test data defined at the top of the test file.
         for (int curRunIdx=0; curRunIdx<MEASURE_JITTER_TEST_RUNS_NUMBER; curRunIdx++)
         {
            MpFrameTime frameTime=0;

            // Enable the device for each run so that we get an accurate setup/teardown
            // for enable/disable cycle captured in each test run.
            driver.enableDevice(samplesPerFrame, sampleRates[rateIndex], 0);
            CPPUNIT_ASSERT(driver.isEnabled());

            // Register a callback to receive ticker notifications from the device driver.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.setTickerNotification(&notificationEvent));

            // Write all the data we calculated to the device.
            for (int frame=0; frame<numFrames; frame++)
            {
               CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notificationEvent.wait(OsTime(1000)));
               OsDateTime::getCurTime(jitterTimes[curRunIdx][frame]);
               notificationEvent.reset();

               // Push the frame to the device!
               CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                  driver.pushFrame(samplesPerFrame,
                                   sampleData + samplesPerFrame*frame,
                                   frameTime));

               frameTime += samplesPerFrame*1000/sampleRates[rateIndex];
            }

            // Disable the ticker notifications
            CPPUNIT_ASSERT(driver.setTickerNotification(NULL) == OS_SUCCESS);

            // Reset any notifications that may have been triggered.
            notificationEvent.reset();

            // Disable the device to complete the run -- so that we get an accurate
            // setup/teardown for enable/disable cycle captured.
            driver.disableDevice();
            CPPUNIT_ASSERT(!driver.isEnabled());

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
                     jitterTimes[curRunIdx][frame].cvtToMsecs()-jitterTimes[curRunIdx][0].cvtToMsecs());
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
         for(i = 0; i< MEASURE_JITTER_TEST_RUNS_NUMBER; i++)
            delete[] jitterTimes[i];

         delete[] sampleData;
      }
   }

protected:
   MpBufPool *mpPool;         ///< Pool for data buffers
   MpBufPool *mpHeadersPool;  ///< Pool for buffers headers

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpOutputDeviceDriverTest);
