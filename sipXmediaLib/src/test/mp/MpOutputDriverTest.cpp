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

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpOutputDeviceManager.h>
#include <mp/MpAudioBuf.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#include <os/OsTask.h>
#include <os/OsEvent.h>

#define TEST_SAMPLES_PER_FRAME_SIZE   80
#define BUFFER_NUM                    500
#define TEST_SAMPLES_PER_SECOND       8000
#define TEST_SAMPLE_DATA_SIZE         (TEST_SAMPLES_PER_SECOND*1)
#define TEST_SAMPLE_DATA_MAGNITUDE    32000
#define TEST_SAMPLE_DATA_PERIOD       11  // in milliseconds

#define CREATE_TEST_RUNS_NUMBER              3
#define ENABLE_DISABLE_TEST_RUNS_NUMBER      5
#define ENABLE_DISABLE_FAST_TEST_RUNS_NUMBER 10
#define DIRECT_WRITE_TEST_RUNS_NUMBER        3
#define TICKER_TEST_WRITE_RUNS_NUMBER        3

#define USE_TEST_DRIVER

#ifdef USE_TEST_DRIVER // USE_TEST_DRIVER [
#include <mp/MpodBufferRecorder.h>
#define OUTPUT_DRIVER MpodBufferRecorder
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "default", TEST_SAMPLE_DATA_SIZE*1000/TEST_SAMPLES_PER_SECOND

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#include <mp/MpodWinMM.h>
#define OUTPUT_DRIVER MpodWinMM
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS MpodWinMM::getDefaultDeviceName()

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#include <mp/MpodOSS.h>
#define OUTPUT_DRIVER MpodOSS
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "/dev/dsp"

#else // __pingtel_on_posix__ ]
#error Unknown platform!
#endif

static MpAudioSample sampleData[TEST_SAMPLE_DATA_SIZE];
static UtlBoolean sampleDataInitialized=FALSE;

static void calculateSampleData()
{
   if (sampleDataInitialized)
      return;

   for (int i=0; i<TEST_SAMPLE_DATA_SIZE; i++)
   {
      sampleData[i] = 
         MpSineWaveGeneratorDeviceDriver::calculateSample(0,
                                                          TEST_SAMPLE_DATA_MAGNITUDE,
                                                          TEST_SAMPLE_DATA_PERIOD,
                                                          i,
                                                          TEST_SAMPLES_PER_FRAME_SIZE,
                                                          TEST_SAMPLES_PER_SECOND);
   }

   sampleDataInitialized = TRUE;
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
   CPPUNIT_TEST(testDirectWrite);
   CPPUNIT_TEST(testTickerNotification);
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
      calculateSampleData();

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

      calculateSampleData();

      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!driver.isEnabled());

      for (int i=0; i<TICKER_TEST_WRITE_RUNS_NUMBER; i++)
      {
         MpFrameTime frameTime=0;

         driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND, 0);
         CPPUNIT_ASSERT(driver.isEnabled());

         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.setTickerNotification(&notificationEvent));

         // Write some data to device.
         for (int frame=0; frame<TEST_SAMPLE_DATA_SIZE/TEST_SAMPLES_PER_FRAME_SIZE; frame++)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notificationEvent.wait(OsTime(1000)));
            notificationEvent.reset();
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 driver.pushFrame(TEST_SAMPLES_PER_FRAME_SIZE,
                                                  sampleData + TEST_SAMPLES_PER_FRAME_SIZE*frame,
                                                  frameTime));

            frameTime += TEST_SAMPLES_PER_FRAME_SIZE*1000/TEST_SAMPLES_PER_SECOND;
         }

         CPPUNIT_ASSERT(driver.setTickerNotification(NULL) == OS_SUCCESS);

         driver.disableDevice();
         CPPUNIT_ASSERT(!driver.isEnabled());
      }
   }

protected:
   MpBufPool *mpPool;         ///< Pool for data buffers
   MpBufPool *mpHeadersPool;  ///< Pool for buffers headers

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpOutputDeviceDriverTest);
