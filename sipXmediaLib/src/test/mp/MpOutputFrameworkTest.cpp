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

#include <mp/MpOutputDeviceManager.h>
#include <mp/MpAudioBuf.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#include <os/OsTask.h>
#include <os/OsEvent.h>

#ifdef RTL_ENABLED // [
#  include "rtl_macro.h"
#else  // RTL_ENABLED ][
#  define RTL_WRITE(x)
#  define RTL_BLOCK(x)
#  define RTL_START(x)
#  define RTL_STOP
#endif // RTL_ENABLED ]

#define TEST_SAMPLES_PER_FRAME_SIZE   80
#define BUFFER_NUM                    50
#define TEST_SAMPLES_PER_SECOND       8000
#define TEST_MIXER_BUFFER_LENGTH      10
#define TEST_SAMPLE_DATA_LENGTH_SEC   2  // test length in seconds
#define TEST_SAMPLE_DATA_SIZE         (TEST_SAMPLES_PER_SECOND*TEST_SAMPLE_DATA_LENGTH_SEC)
#define TEST_SAMPLE_DATA_MAGNITUDE    32000
#define TEST_SAMPLE_DATA_PERIOD       7  // in milliseconds

//#define USE_TEST_DRIVER

#ifdef USE_TEST_DRIVER // USE_TEST_DRIVER [
#include <mp/MpodBufferRecorder.h>
#define OUTPUT_DRIVER MpodBufferRecorder
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "default", TEST_SAMPLE_DATA_SIZE*1000/TEST_SAMPLES_PER_SECOND

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#error No output driver for Windows exist!

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#include <mp/MpodOSS.h>
#define OUTPUT_DRIVER MpodOSS
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "/dev/dsp"

#else // __pingtel_on_possix__ ]
#error Unknown platform!
#endif



MpAudioSample sampleData[TEST_SAMPLE_DATA_SIZE];
UtlBoolean sampleDataInitialized=FALSE;

void calculateSampleData()
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
 * Unittest for MpOutputDeviceManager
 */
class MpOutputFrameworkTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpOutputFrameworkTest);
   CPPUNIT_TEST(testCreate);
   CPPUNIT_TEST(testAddRemoveToManager);
   CPPUNIT_TEST(testEnableDisable);
   CPPUNIT_TEST(testEnableDisableFast);
   // This is disabled, as it gives very bad sound quality
   // and may be used only for very first driver testing.
   //CPPUNIT_TEST(testDirectWrite);
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
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
   }

   void testAddRemoveToManager()
   {
      OUTPUT_DRIVER device(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      MpOutputDeviceHandle deviceId;

      {
         // Test with direct write mode
         MpOutputDeviceManager deviceManager(TEST_SAMPLES_PER_FRAME_SIZE,
            TEST_SAMPLES_PER_SECOND,
            0);

         deviceId = deviceManager.addDevice(&device);
         CPPUNIT_ASSERT(deviceId > 0);
         CPPUNIT_ASSERT(deviceManager.removeDevice(deviceId) == &device);
      }

      {
         // Test with mixer mode
         MpOutputDeviceManager deviceManager(TEST_SAMPLES_PER_FRAME_SIZE,
                                             TEST_SAMPLES_PER_SECOND,
                                             TEST_MIXER_BUFFER_LENGTH);

         deviceId = deviceManager.addDevice(&device);
         CPPUNIT_ASSERT(deviceId > 0);
         CPPUNIT_ASSERT(deviceManager.removeDevice(deviceId) == &device);
      }
   }

   void testEnableDisable()
   {
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND, 0);
      OsTask::delay(50);
      driver.disableDevice();
   }

   void testEnableDisableFast()
   {
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND, 0);
      driver.disableDevice();
   }

   void testDirectWrite()
   {
      calculateSampleData();

      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND, 0);

      // Write some data to device.
      for (int frame=0; frame<TEST_SAMPLE_DATA_SIZE/TEST_SAMPLES_PER_FRAME_SIZE; frame++)
      {
         OsTask::delay(1000*TEST_SAMPLES_PER_FRAME_SIZE/TEST_SAMPLES_PER_SECOND);
         driver.pushFrame(TEST_SAMPLES_PER_FRAME_SIZE, sampleData + TEST_SAMPLES_PER_FRAME_SIZE*frame, -1);
      }

      driver.disableDevice();
   }

   void testTickerNotification()
   {
      OsEvent notificationEvent;
     
      RTL_START(1000000);

      calculateSampleData();

      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND, 0);
      driver.setTickerNotification(&notificationEvent);

      // Write some data to device.
      for (int frame=0; frame<TEST_SAMPLE_DATA_SIZE/TEST_SAMPLES_PER_FRAME_SIZE; frame++)
      {
         notificationEvent.wait(OsTime(500));
         notificationEvent.reset();
         RTL_BLOCK("test ticker loop");
         driver.pushFrame(TEST_SAMPLES_PER_FRAME_SIZE, sampleData + TEST_SAMPLES_PER_FRAME_SIZE*frame, -1);
      }

      driver.disableDevice();

      RTL_WRITE("testTickerNotification.rtl");
      RTL_STOP
   }

protected:
   MpBufPool *mpPool;         ///< Pool for data buffers
   MpBufPool *mpHeadersPool;  ///< Pool for buffers headers

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpOutputFrameworkTest);
