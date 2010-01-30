//  
// Copyright (C) 2007-2010 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <sipxunittests.h>

#include <mp/MpOutputDeviceManager.h>
#include <mp/MpAudioBuf.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#include <os/OsTask.h>
#include <os/OsEvent.h>
#include <os/OsCallback.h>

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
#include <mp/MpodWinMM.h>
#define OUTPUT_DRIVER MpodWinMM
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS MpodWinMM::getDefaultDeviceName()

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#include <mp/MpodOss.h>
#define OUTPUT_DRIVER MpodOss
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "/dev/dsp"

#else // __pingtel_on_possix__ ]
#error Unknown platform!
#endif

static MpAudioSample sampleData[TEST_SAMPLE_DATA_SIZE];
static UtlBoolean sampleDataInitialized=FALSE;
static MpFrameTime frameTime;
static int frame;

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
class MpOutputFrameworkTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(MpOutputFrameworkTest);
   CPPUNIT_TEST(testCreate);
   CPPUNIT_TEST(testAddRemoveToManager);
   CPPUNIT_TEST(testEnableDisable);
   CPPUNIT_TEST(testEnableDisableFast);
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

      // Test with mixer mode
      MpOutputDeviceManager deviceManager(TEST_SAMPLES_PER_FRAME_SIZE,
                                          TEST_SAMPLES_PER_SECOND,
                                          TEST_MIXER_BUFFER_LENGTH);

      deviceId = deviceManager.addDevice(&device);
      CPPUNIT_ASSERT(deviceId > 0);
      CPPUNIT_ASSERT(deviceManager.removeDevice(deviceId) == &device);
   }

   void testEnableDisable()
   {
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CallbackUserData userData;
      userData.mDriver = &driver;
      userData.mEvent = NULL;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);
      driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND,
                          0, notificationCallback);
      OsTask::delay(50);
      driver.disableDevice();
   }

   void testEnableDisableFast()
   {
      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CallbackUserData userData;
      userData.mDriver = &driver;
      userData.mEvent = NULL;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);
      driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND,
                          0, notificationCallback);
      driver.disableDevice();
   }

   void testTickerNotification()
   {
      CallbackUserData userData;
      OsEvent notificationEvent;
     
      RTL_START(1000000);

      calculateSampleData();

      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);

      userData.mDriver = &driver;
      userData.mEvent = &notificationEvent;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);

      frameTime = 0;
      driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND,
                          0, notificationCallback);

      // Write some data to device.
      for (frame=0; frame<TEST_SAMPLE_DATA_SIZE/TEST_SAMPLES_PER_FRAME_SIZE; frame++)
      {
         notificationEvent.wait(OsTime(500));
         notificationEvent.reset();
         RTL_BLOCK("test ticker loop");
      }

      driver.disableDevice();

      RTL_WRITE("testTickerNotification.rtl");
      RTL_STOP
   }

protected:

   struct CallbackUserData
   {
      OsEvent *mEvent;
      MpOutputDeviceDriver *mDriver;
   };

   static void driverCallback(const intptr_t userData, const intptr_t eventData)
   {
      CallbackUserData *pData = (CallbackUserData*)userData;

      // Push data to the driver first.
      pData->mDriver->pushFrame(TEST_SAMPLES_PER_FRAME_SIZE, sampleData + TEST_SAMPLES_PER_FRAME_SIZE*frame, -1);
      frameTime += TEST_SAMPLES_PER_FRAME_SIZE*1000/TEST_SAMPLES_PER_SECOND;

      // Signal the callback.
      if (pData->mEvent != NULL)
      {
         pData->mEvent->signal(eventData);
      }
   }

   MpBufPool *mpPool;         ///< Pool for data buffers
   MpBufPool *mpHeadersPool;  ///< Pool for buffers headers
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpOutputFrameworkTest);
