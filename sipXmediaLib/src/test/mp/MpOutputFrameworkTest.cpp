//  
// Copyright (C) 2007-2013 SIPez LLC.  All rights reserved.
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

#ifdef ANDROID // [
#  define TEST_SAMPLES_PER_FRAME_SIZE   160
#  define TEST_SAMPLES_PER_SECOND       16000
#else // ANDROID ][
#  define TEST_SAMPLES_PER_FRAME_SIZE   80
#  define TEST_SAMPLES_PER_SECOND       8000
#endif // !ANDROID ]
#define BUFFER_NUM                    50
#define TEST_FRAME_RATE               (TEST_SAMPLES_PER_SECOND/TEST_SAMPLES_PER_FRAME_SIZE)
#define TEST_MIXER_BUFFER_LENGTH      10
#define TEST_SAMPLE_DATA_LENGTH_SEC   2  // test length in seconds
#define TEST_SAMPLE_DATA_SIZE         (TEST_SAMPLES_PER_SECOND*TEST_SAMPLE_DATA_LENGTH_SEC)
#define TEST_SAMPLE_DATA_MAGNITUDE    32000
#define TEST_SAMPLE_DATA_PERIOD       7  // in milliseconds

//#define USE_TEST_DRIVER

#ifdef USE_TEST_DRIVER // USE_TEST_DRIVER [
#  include <mp/MpodBufferRecorder.h>
#  define OUTPUT_DRIVER MpodBufferRecorder
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "default", TEST_SAMPLE_DATA_SIZE*1000/TEST_SAMPLES_PER_SECOND

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#  include <mp/MpodWinMM.h>
#  define OUTPUT_DRIVER MpodWinMM
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS MpodWinMM::getDefaultDeviceName()

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

#else // __pingtel_on_possix__ ]
#error Unknown platform!
#endif

// Sample data array
static UtlBoolean sampleDataInitialized=FALSE;
static MpAudioSample sampleData[TEST_SAMPLE_DATA_SIZE];
static int sampleDataSz;
// Current time for callback
static int frameInCallback;
static MpFrameTime frameTime;

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
                             + MpArrayBuf::getHeaderSize(), BUFFER_NUM, "MpOutputFrameworkTest");
      CPPUNIT_ASSERT(mpPool != NULL);

      // Create pool for buffer headers
      mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), BUFFER_NUM, "MpOutputFrameworkTestHeaders");
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
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);
      sampleDataSz = 0;
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
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);
      sampleDataSz = 0;
      driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND,
                          0, notificationCallback);
      driver.disableDevice();
   }

   void testTickerNotification()
   {
      CallbackUserData userData;
     
      RTL_START(1000000);

      calculateSampleData();

      OUTPUT_DRIVER driver(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);

      userData.mDriver = &driver;
      OsCallback notificationCallback((intptr_t)&userData, &driverCallback);

      frameTime = 0;
      sampleDataSz = TEST_SAMPLE_DATA_SIZE;
      driver.enableDevice(TEST_SAMPLES_PER_FRAME_SIZE, TEST_SAMPLES_PER_SECOND,
                          0, notificationCallback);

      // Wait defined amount of time.
      OsTask::delay(TEST_SAMPLE_DATA_LENGTH_SEC*1000);

      driver.disableDevice();
      CPPUNIT_ASSERT(!driver.isEnabled());
      CPPUNIT_ASSERT(frameInCallback > TEST_SAMPLE_DATA_LENGTH_SEC*TEST_FRAME_RATE);

      RTL_WRITE("testTickerNotification.rtl");
      RTL_STOP
   }

protected:

   struct CallbackUserData
   {
      MpOutputDeviceDriver *mDriver;
   };

   static void driverCallback(const intptr_t userData, const intptr_t eventData)
   {
      CallbackUserData *pData = (CallbackUserData*)userData;
      const int samplesPerFrame = TEST_SAMPLES_PER_FRAME_SIZE;

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

CPPUNIT_TEST_SUITE_REGISTRATION(MpOutputFrameworkTest);
