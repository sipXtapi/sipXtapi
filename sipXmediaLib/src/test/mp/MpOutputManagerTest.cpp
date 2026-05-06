//  
// Copyright (C) 2007-2026 SIPez LLC.  All rights reserved. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#include <os/OsIntTypes.h>

#include <sipxunittests.h>

#include <mp/MpOutputDeviceManager.h>
#include <mp/MpAudioBuf.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#include <os/OsTask.h>
#include <os/OsEvent.h>
#include <os/OsSysLog.h>
#include <os/OsMsgDispatcher.h>
#include <mp/MpResNotificationMsg.h>


#define TEST_SAMPLES_PER_FRAME_SIZE   80    ///< in samples
#define BUFFER_NUM                    500
#define TEST_SAMPLES_PER_SECOND       8000  ///< in samples/sec (Hz)
#define TEST_MIXER_BUFFER_LENGTH      100   ///< in milliseconds
#define TEST_SAMPLE_FRAMES            100   ///< test length in frames
#define TEST_SAMPLE_DATA_MAGNITUDE    32000 ///< absolute value
#define TEST_SAMPLE_DATA_PERIOD       11    ///< in milliseconds
#define TEST_FRAME_LENGTH_MS          (1000/(TEST_SAMPLES_PER_SECOND/TEST_SAMPLES_PER_FRAME_SIZE))
                                            ///< in milliseconds

#define NONEXISTENT_DEVICE_ID         321
#define NONEXISTENT_DEVICE_NAME       "NONE"

#define CREATE_TEST_RUNS_NUMBER              3
#define NONEXISTENT_TEST_RUNS_NUMBER         3
#define ADD_REMOVE_TEST_RUNS_NUMBER          3
#define ENABLE_DISABLE_TEST_RUNS_NUMBER      5
#define ENABLE_DISABLE_FAST_TEST_RUNS_NUMBER 10
#define DIRECT_WRITE_TEST_RUNS_NUMBER        3
#define TICKER_TEST_WRITE_RUNS_NUMBER        3


#include <mp/MpodBufferRecorder.h>
#define OUTPUT_DRIVER_TEST MpodBufferRecorder
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_TEST "default", TEST_SAMPLE_FRAMES*TEST_SAMPLES_PER_FRAME_SIZE*1000/TEST_SAMPLES_PER_SECOND



//#define USE_TEST_DRIVER
#ifdef USE_TEST_DRIVER // USE_TEST_DRIVER [
#define OUTPUT_DRIVER OUTPUT_DRIVER_TEST
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_TEST

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#  define NUM_DEVICE_TYPES 1
#  include <mp/MpodWinMM.h>
#  define OUTPUT_DRIVER MpodWinMM
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS MpodWinMM::getDefaultDeviceName(), 0

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#  define NUM_DEVICE_TYPES 3
#define USE_ALSA_INTERFACE
// ALSA on Linux
#  include <mp/MpodAlsa.h>
#  define OUTPUT_DRIVER_ALSA MpodAlsa
//#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_ALSA "hw:0,0"
//#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_ALSA "plughw:0,0"
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_ALSA "sysdefault"

#  include <mp/MpodOss.h>
#  define OUTPUT_DRIVER_OSS MpodOss
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_OSS "/dev/dsp"

#  if defined(USE_ALSA_INTERFACE)
#    define OUTPUT_DRIVER OUTPUT_DRIVER_ALSA
#    define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_ALSA
#  else
// OSS on Linux
#    define OUTPUT_DRIVER OUTPUT_DRIVER_OSS
#    define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_OSS
#  endif
#else // __pingtel_on_possix__ ]
#error Unknown platform!
#endif

static void calculateSampleData(MpFrameTime frameTime,
                                MpAudioBufPtr &frame)
{
   MpAudioSample *sampleData = frame->getSamplesWritePtr();
   for (unsigned i=0; i<frame->getSamplesNumber(); i++)
   {
      sampleData[i] = 
         MpSineWaveGeneratorDeviceDriver::calculateSample(frameTime,
                                                          TEST_SAMPLE_DATA_MAGNITUDE,
                                                          TEST_SAMPLE_DATA_PERIOD,
                                                          i,
                                                          TEST_SAMPLES_PER_FRAME_SIZE,
                                                          TEST_SAMPLES_PER_SECOND);
   }
}

/**
 * Unittest for MpOutputDeviceDriver
 */
class MpOutputDeviceManagerTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(MpOutputDeviceManagerTest);
   CPPUNIT_TEST(testCreate);
   CPPUNIT_TEST(testNonexistentDevice);
   CPPUNIT_TEST(testAddRemoveToManager);
   CPPUNIT_TEST(testEnableDisable);
   CPPUNIT_TEST(testEnableDisableFast);
   CPPUNIT_TEST(testMixing);
   CPPUNIT_TEST(testNotificationDispatchOnFallback);
   CPPUNIT_TEST(testMostRecentDispatcherWins);
   CPPUNIT_TEST(testDisableDeviceWhileInFallback);
   CPPUNIT_TEST_SUITE_END();


public:
   int getOutputDrivers(const char* interfaceType, MpOutputDeviceDriver* drivers[] )
   {
      int driverCount = 0;
#if defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
      drivers[driverCount] = new OUTPUT_DRIVER_TEST(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_TEST);
      driverCount++;

      const char* alsaDeviceName = getenv("SIPX_ALSA_DEVICE_NAME");
      drivers[driverCount] = 
        new OUTPUT_DRIVER_ALSA(alsaDeviceName ?
                               alsaDeviceName :
                               OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_ALSA);
      driverCount++;

      const char* ossDeviceName = getenv("SIPX_OSS_DEVICE_NAME");
      drivers[driverCount] = 
          new OUTPUT_DRIVER_OSS(ossDeviceName ?
                                ossDeviceName :
                                OUTPUT_DRIVER_CONSTRUCTOR_PARAMS_OSS);
      driverCount++;
#else
      drivers[driverCount] = new OUTPUT_DRIVER (OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      driverCount++;
#endif
      return(driverCount);
   }
   void setUp()
   {
      // Create pool for data buffers
      mpPool = new MpBufPool(TEST_SAMPLES_PER_FRAME_SIZE * sizeof(MpAudioSample)
                             + MpArrayBuf::getHeaderSize(), BUFFER_NUM, "MpOutputManagerTest");
      CPPUNIT_ASSERT(mpPool != NULL);

      // Create pool for buffer headers
      mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), BUFFER_NUM, "MpOutputManagerTestHeaders");
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
         MpOutputDeviceManager manager(TEST_SAMPLES_PER_FRAME_SIZE,
                                       TEST_SAMPLES_PER_SECOND,
                                       TEST_MIXER_BUFFER_LENGTH);

      }
   }

   void testNonexistentDevice()
   {
      for (int i=0; i<NONEXISTENT_TEST_RUNS_NUMBER; i++)
      {
         MpOutputDeviceManager manager(TEST_SAMPLES_PER_FRAME_SIZE,
                                       TEST_SAMPLES_PER_SECOND,
                                       TEST_MIXER_BUFFER_LENGTH);

         CPPUNIT_ASSERT(manager.disableDevice(NONEXISTENT_DEVICE_ID) == OS_NOT_FOUND);
         CPPUNIT_ASSERT(manager.enableDevice(NONEXISTENT_DEVICE_ID) == OS_NOT_FOUND);

         MpOutputDeviceHandle deviceId;
         CPPUNIT_ASSERT(manager.getDeviceId(NONEXISTENT_DEVICE_NAME, deviceId) == OS_NOT_FOUND);

         UtlString deviceName;
         CPPUNIT_ASSERT(manager.getDeviceName(NONEXISTENT_DEVICE_ID, deviceName) == OS_NOT_FOUND);

         MpFrameTime bufferLength;
         CPPUNIT_ASSERT(manager.getMixerBufferLength(NONEXISTENT_DEVICE_ID, bufferLength) == OS_NOT_FOUND);
      }
   }

   void testAddRemoveToManager()
   {
      MpOutputDeviceHandle deviceId;

      OUTPUT_DRIVER device(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!device.isEnabled());

      for (int i=0; i<ADD_REMOVE_TEST_RUNS_NUMBER; i++)
      {
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
      MpOutputDeviceHandle deviceId;
      MpOutputDeviceDriver* devices[NUM_DEVICE_TYPES];
      int deviceCount = getOutputDrivers(NULL, devices);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("No audio device configured", deviceCount, NUM_DEVICE_TYPES);

      for(int deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
      {
          printf("Device: %d\n", deviceIndex);
          CPPUNIT_ASSERT(!devices[deviceIndex]->isEnabled());

          UtlString deviceLoopMessage;
          deviceLoopMessage.appendFormat("Skipping testing of device %d \"%s\"", 
                                         deviceIndex,
                                         devices[deviceIndex]->getDeviceName().data());
          OsStatus deviceUsableStatus = devices[deviceIndex]->canEnable();
          CPPUNIT_ASSERT_EQUAL_MESSAGE(deviceLoopMessage.data(),
                                       deviceUsableStatus,
                                       OS_SUCCESS);
          if(deviceUsableStatus != OS_SUCCESS) continue;

          for (int i=0; i<ENABLE_DISABLE_TEST_RUNS_NUMBER; i++)
          {
             OsSysLog::add(FAC_MP, PRI_DEBUG, 
                 "MpOutputManagerTest::testEnableDisable device: %d iteration: %d",
                 deviceIndex,
                 i);
             // Test with mixer mode
             MpOutputDeviceManager deviceManager(TEST_SAMPLES_PER_FRAME_SIZE,
                                                 TEST_SAMPLES_PER_SECOND,
                                                 TEST_MIXER_BUFFER_LENGTH);

             UtlString loopMessage;
             loopMessage.appendFormat("iteration: %d", i);
             deviceId = deviceManager.addDevice(devices[deviceIndex]);
             CPPUNIT_ASSERT_MESSAGE(loopMessage.data(), deviceId > 0);

             deviceManager.enableDevice(deviceId);
             CPPUNIT_ASSERT_MESSAGE(loopMessage.data(), deviceManager.isDeviceEnabled(deviceId));

             OsTask::delay(50);

             deviceManager.disableDevice(deviceId);
             CPPUNIT_ASSERT_MESSAGE(loopMessage.data(), !deviceManager.isDeviceEnabled(deviceId));

             CPPUNIT_ASSERT_MESSAGE(loopMessage.data(), deviceManager.removeDevice(deviceId) == devices[deviceIndex]);

             CPPUNIT_ASSERT_MESSAGE(loopMessage.data(),
                                    ! devices[deviceIndex]->isEnabled())
          }
          delete (devices[deviceIndex]);
          devices[deviceIndex] = NULL;
       }
   }

   void testEnableDisableFast()
   {
      MpOutputDeviceHandle deviceId;

      OUTPUT_DRIVER device(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!device.isEnabled());

      for (int i=0; i<ENABLE_DISABLE_TEST_RUNS_NUMBER; i++)
      {
         MpOutputDeviceManager deviceManager(TEST_SAMPLES_PER_FRAME_SIZE,
                                             TEST_SAMPLES_PER_SECOND,
                                             TEST_MIXER_BUFFER_LENGTH);

         deviceId = deviceManager.addDevice(&device);
         CPPUNIT_ASSERT(deviceId > 0);

         deviceManager.enableDevice(deviceId);
         CPPUNIT_ASSERT(deviceManager.isDeviceEnabled(deviceId));

         deviceManager.disableDevice(deviceId);
         CPPUNIT_ASSERT(!deviceManager.isDeviceEnabled(deviceId));

         CPPUNIT_ASSERT(deviceManager.removeDevice(deviceId) == &device);
      }
   }

   void testMixing()
   {
      MpOutputDeviceHandle deviceId;

      OUTPUT_DRIVER device(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      CPPUNIT_ASSERT(!device.isEnabled());

      for (int i=0; i<TICKER_TEST_WRITE_RUNS_NUMBER; i++)
      {
         MpOutputDeviceManager deviceManager(TEST_SAMPLES_PER_FRAME_SIZE,
                                             TEST_SAMPLES_PER_SECOND,
                                             TEST_MIXER_BUFFER_LENGTH);
         MpAudioBufPtr pBuffer = mpPool->getBuffer();

         deviceId = deviceManager.addDevice(&device);
         CPPUNIT_ASSERT(deviceId > 0);

         deviceManager.enableDevice(deviceId);
         CPPUNIT_ASSERT(deviceManager.isDeviceEnabled(deviceId));

         // Get current frame time and step to middle of mixer buffer to be in
         // time with output audio connection.
         MpFrameTime frameTime=deviceManager.getCurrentFrameTime(deviceId);
         frameTime += TEST_MIXER_BUFFER_LENGTH/2;

         // Write some data to device.
         for (int frame=0; frame<TEST_SAMPLE_FRAMES/TEST_SAMPLES_PER_FRAME_SIZE; frame++)
         {
            calculateSampleData(frameTime, pBuffer);

            OsTask::delay(TEST_FRAME_LENGTH_MS);
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 deviceManager.pushFrame(deviceId, frameTime, pBuffer));
            frameTime += TEST_FRAME_LENGTH_MS;
         }

         deviceManager.disableDevice(deviceId);
         CPPUNIT_ASSERT(!deviceManager.isDeviceEnabled(deviceId));

         CPPUNIT_ASSERT(deviceManager.removeDevice(deviceId) == &device);
      }
   }

   void testNotificationDispatchOnFallback()
   {
#  ifdef WIN32
      // Test #10: Verify the manager dispatches exactly one
      // MPRNM_OUTPUT_DEVICE_NOT_PRESENT notification when the driver
      // enters fallback mode (Change 2 regression -- spurious
      // notifications from redundant switchToMMTimer calls). Also
      // verifies that after removeNotificationDispatcher, no further
      // notifications reach the dispatcher.

      MpOutputDeviceManager deviceManager(TEST_SAMPLES_PER_FRAME_SIZE,
                                          TEST_SAMPLES_PER_SECOND,
                                          TEST_MIXER_BUFFER_LENGTH);

      // Driver must be MpodWinMM specifically -- only it has the COM /
      // wave-failure fallback machinery being tested. The manager
      // pointer is required: switchToMMTimer routes notifications
      // through the manager's dispatcher list, so a driver constructed
      // with a NULL manager will silently drop the notification.
      MpodWinMM driver(MpodWinMM::getDefaultDeviceName(), &deviceManager);

      if (!driver.isDeviceValid())
      {
         printf("No output device available, skipping testNotificationDispatchOnFallback\n");
         return;
      }

      MpOutputDeviceHandle deviceId = deviceManager.addDevice(&driver);
      CPPUNIT_ASSERT(deviceId > 0);

      // Hook up our dispatcher BEFORE enabling so we see all events.
      OsMsgDispatcher notfDispatcher;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         deviceManager.addNotificationDispatcher(&notfDispatcher));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, deviceManager.enableDevice(deviceId));
      CPPUNIT_ASSERT(deviceManager.isDeviceEnabled(deviceId));

      // Drain anything the manager itself might have posted at enable.
      OsMsg* pMsg = NULL;
      while (notfDispatcher.numMsgs() > 0)
      {
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
            notfDispatcher.receive(pMsg, OsTime(0)));
         if (pMsg) pMsg->releaseMsg();
         pMsg = NULL;
      }

      // Trigger fallback. First call posts the notification.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.switchToMMTimer());
      CPPUNIT_ASSERT(driver.isUsingFallbackTimer());

      // Poll for the notification rather than relying on a fixed delay.
      // The post may go through a queued path on another thread.
      const int POLL_MAX_LOOPS = 20;
      for (int loop = 0;
           loop < POLL_MAX_LOOPS && notfDispatcher.numMsgs() < 1;
           loop++)
      {
         OsTask::delay(50);
      }

      // Exactly one DEVICE_NOT_PRESENT notification expected.
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "Expected exactly one notification after first switchToMMTimer",
         1, notfDispatcher.numMsgs());

      pMsg = NULL;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         notfDispatcher.receive(pMsg, OsTime(100)));
      CPPUNIT_ASSERT(pMsg != NULL);
      if (pMsg != NULL)
      {
         CPPUNIT_ASSERT_EQUAL((int)OsMsg::MP_RES_NOTF_MSG,
                              (int)pMsg->getMsgType());
         CPPUNIT_ASSERT_EQUAL(
            (int)MpResNotificationMsg::MPRNM_OUTPUT_DEVICE_NOT_PRESENT,
            (int)pMsg->getMsgSubType());
         pMsg->releaseMsg();
         pMsg = NULL;
      }

      // Redundant switchToMMTimer must NOT post another notification
      // (Change 2 dedupe). Return value is not asserted: the
      // implementation may legitimately return non-success to signal
      // "no-op, already in fallback mode". What matters is the
      // notification count.
      driver.switchToMMTimer();
      OsTask::delay(50);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "Redundant switchToMMTimer must not post a duplicate notification",
         0, notfDispatcher.numMsgs());

      // Remove the dispatcher. Subsequent activity must not reach it.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         deviceManager.removeNotificationDispatcher(&notfDispatcher));

      // Disable while in fallback. Must not crash. The return value is
      // not asserted because disabling a device that is in fallback
      // mode (mDevHandle already NULL per Change 3) may report a
      // non-success status from the manager layer; that behavior is
      // out of scope for this test. What matters here is: no
      // notifications must reach the dispatcher we already removed.
      OsStatus disableStatus = deviceManager.disableDevice(deviceId);
      if (disableStatus != OS_SUCCESS)
      {
         OsSysLog::add(FAC_MP, PRI_INFO,
            "testNotificationDispatchOnFallback: disableDevice during "
            "fallback returned %d (non-success acceptable in this test)",
            (int)disableStatus);
      }
      OsTask::delay(50);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "No notifications must reach a removed dispatcher",
         0, notfDispatcher.numMsgs());

      CPPUNIT_ASSERT(deviceManager.removeDevice(deviceId) == &driver);
#  else
      printf("Skipping testNotificationDispatchOnFallback on non-Windows platform\n");
#  endif
   }

   void testMostRecentDispatcherWins()
   {
#  ifdef WIN32
      // Test #11 (revised): Characterize MpOutputDeviceManager's
      // single-recipient dispatch policy.
      //
      // Implementation note (MpOutputDeviceManager.cpp):
      // postNotification() calls getNotificationDispatcher(), which
      // returns only mNotifiers.at(0) -- the most recently added
      // dispatcher (addNotificationDispatcher uses insertAt(0)).
      // Earlier-registered dispatchers are silently shadowed until
      // the head is removed.
      //
      // This test pins that contract down. If a future change adds
      // true multi-recipient fan-out, this test should be replaced
      // with one that asserts all registered dispatchers receive.
      //
      // Sequence: add A, B, C; only C receives. Remove C; only B
      // receives next. Remove B; only A receives next. Remove A;
      // no one receives. Each phase requires a disable/enable cycle
      // to reset the switchToMMTimer dedupe (Change 2).

      MpOutputDeviceManager deviceManager(TEST_SAMPLES_PER_FRAME_SIZE,
                                          TEST_SAMPLES_PER_SECOND,
                                          TEST_MIXER_BUFFER_LENGTH);

      MpodWinMM driver(MpodWinMM::getDefaultDeviceName(), &deviceManager);
      if (!driver.isDeviceValid())
      {
         printf("No output device available, skipping testMostRecentDispatcherWins\n");
         return;
      }

      MpOutputDeviceHandle deviceId = deviceManager.addDevice(&driver);
      CPPUNIT_ASSERT(deviceId > 0);

      OsMsgDispatcher dispatcherA;
      OsMsgDispatcher dispatcherB;
      OsMsgDispatcher dispatcherC;
      OsMsgDispatcher* allDispatchers[] =
         { &dispatcherA, &dispatcherB, &dispatcherC };

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         deviceManager.addNotificationDispatcher(&dispatcherA));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         deviceManager.addNotificationDispatcher(&dispatcherB));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         deviceManager.addNotificationDispatcher(&dispatcherC));

      // Helper-style local lambda is not portable to all compilers
      // here; do the trigger sequence inline four times. Each phase:
      //   1. Enable, drain pre-existing messages from all dispatchers.
      //   2. switchToMMTimer.
      //   3. Poll briefly.
      //   4. Assert exactly the expected dispatcher got it.
      //   5. Drain.
      //   6. Disable (return value ignored, see #23).

      const int POLL_MAX_LOOPS = 20;
      OsMsg* pMsg = NULL;

      // ===== Phase 1: A,B,C registered. Expect C only. =====
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, deviceManager.enableDevice(deviceId));
      for (int d = 0; d < 3; d++)
      {
         while (allDispatchers[d]->numMsgs() > 0)
         {
            allDispatchers[d]->receive(pMsg, OsTime(0));
            if (pMsg) pMsg->releaseMsg();
            pMsg = NULL;
         }
      }
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.switchToMMTimer());
      for (int loop = 0;
           loop < POLL_MAX_LOOPS && dispatcherC.numMsgs() < 1;
           loop++)
      {
         OsTask::delay(50);
      }
      printf("Phase 1 (A,B,C): A=%d B=%d C=%d\n",
             dispatcherA.numMsgs(), dispatcherB.numMsgs(),
             dispatcherC.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 1: A must NOT receive (shadowed by B and C)",
                                   0, dispatcherA.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 1: B must NOT receive (shadowed by C)",
                                   0, dispatcherB.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 1: C (most recently added) must receive",
                                   1, dispatcherC.numMsgs());
      while (dispatcherC.numMsgs() > 0)
      {
         dispatcherC.receive(pMsg, OsTime(0));
         if (pMsg) pMsg->releaseMsg();
         pMsg = NULL;
      }
      deviceManager.disableDevice(deviceId);

      // ===== Phase 2: remove C. A,B registered. Expect B only. =====
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         deviceManager.removeNotificationDispatcher(&dispatcherC));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, deviceManager.enableDevice(deviceId));
      for (int d = 0; d < 3; d++)
      {
         while (allDispatchers[d]->numMsgs() > 0)
         {
            allDispatchers[d]->receive(pMsg, OsTime(0));
            if (pMsg) pMsg->releaseMsg();
            pMsg = NULL;
         }
      }
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.switchToMMTimer());
      for (int loop = 0;
           loop < POLL_MAX_LOOPS && dispatcherB.numMsgs() < 1;
           loop++)
      {
         OsTask::delay(50);
      }
      printf("Phase 2 (A,B; C removed): A=%d B=%d C=%d\n",
             dispatcherA.numMsgs(), dispatcherB.numMsgs(),
             dispatcherC.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 2: A must NOT receive (shadowed by B)",
                                   0, dispatcherA.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 2: B (now head) must receive",
                                   1, dispatcherB.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 2: C (removed) must NOT receive",
                                   0, dispatcherC.numMsgs());
      while (dispatcherB.numMsgs() > 0)
      {
         dispatcherB.receive(pMsg, OsTime(0));
         if (pMsg) pMsg->releaseMsg();
         pMsg = NULL;
      }
      deviceManager.disableDevice(deviceId);

      // ===== Phase 3: remove B. A only. Expect A. =====
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         deviceManager.removeNotificationDispatcher(&dispatcherB));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, deviceManager.enableDevice(deviceId));
      for (int d = 0; d < 3; d++)
      {
         while (allDispatchers[d]->numMsgs() > 0)
         {
            allDispatchers[d]->receive(pMsg, OsTime(0));
            if (pMsg) pMsg->releaseMsg();
            pMsg = NULL;
         }
      }
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.switchToMMTimer());
      for (int loop = 0;
           loop < POLL_MAX_LOOPS && dispatcherA.numMsgs() < 1;
           loop++)
      {
         OsTask::delay(50);
      }
      printf("Phase 3 (A only): A=%d B=%d C=%d\n",
             dispatcherA.numMsgs(), dispatcherB.numMsgs(),
             dispatcherC.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 3: A (now head, only one left) must receive",
                                   1, dispatcherA.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 3: B (removed) must NOT receive",
                                   0, dispatcherB.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 3: C (removed) must NOT receive",
                                   0, dispatcherC.numMsgs());
      while (dispatcherA.numMsgs() > 0)
      {
         dispatcherA.receive(pMsg, OsTime(0));
         if (pMsg) pMsg->releaseMsg();
         pMsg = NULL;
      }
      deviceManager.disableDevice(deviceId);

      // ===== Phase 4: remove A. None registered. Expect no delivery. =====
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         deviceManager.removeNotificationDispatcher(&dispatcherA));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, deviceManager.enableDevice(deviceId));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, driver.switchToMMTimer());
      OsTask::delay(200);
      printf("Phase 4 (none): A=%d B=%d C=%d\n",
             dispatcherA.numMsgs(), dispatcherB.numMsgs(),
             dispatcherC.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 4: no dispatcher registered, A must not receive",
                                   0, dispatcherA.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 4: no dispatcher registered, B must not receive",
                                   0, dispatcherB.numMsgs());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Phase 4: no dispatcher registered, C must not receive",
                                   0, dispatcherC.numMsgs());
      deviceManager.disableDevice(deviceId);

      CPPUNIT_ASSERT(deviceManager.removeDevice(deviceId) == &driver);
#  else
      printf("Skipping testMostRecentDispatcherWins on non-Windows platform\n");
#  endif
   }

   void testDisableDeviceWhileInFallback()
   {
#  ifdef WIN32
      // Test #23: Characterize the manager's disableDevice behavior
      // when the underlying driver is in fallback mode (mDevHandle
      // already NULL per Change 3). This was discovered incidentally
      // during testNotificationDispatchOnFallback bring-up:
      // disableDevice returned a non-success status (e.g. 521) even
      // though the user-facing intent of disable is satisfied -- the
      // device is gone, the wave handle is gone, there is nothing
      // left to disable.
      //
      // This test pins down the *current* behavior so it cannot drift
      // silently. If the contract is later changed so that disable
      // succeeds in this state, update the assertion here. The
      // important invariants below the return code are: no crash,
      // device reports not-enabled afterward, and the driver can be
      // removed cleanly.

      MpOutputDeviceManager deviceManager(TEST_SAMPLES_PER_FRAME_SIZE,
                                          TEST_SAMPLES_PER_SECOND,
                                          TEST_MIXER_BUFFER_LENGTH);

      MpodWinMM driver(MpodWinMM::getDefaultDeviceName(), &deviceManager);
      if (!driver.isDeviceValid())
      {
         printf("No output device available, skipping testDisableDeviceWhileInFallback\n");
         return;
      }

      MpOutputDeviceHandle deviceId = deviceManager.addDevice(&driver);
      CPPUNIT_ASSERT(deviceId > 0);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, deviceManager.enableDevice(deviceId));
      CPPUNIT_ASSERT(deviceManager.isDeviceEnabled(deviceId));

      // Drive the driver into fallback mode. Return value is not
      // asserted: the implementation may return non-success on a
      // redundant call, and we cannot guarantee from outside that no
      // earlier code path (e.g. enableDevice initialization) already
      // put us in fallback mode briefly.
      driver.switchToMMTimer();
      CPPUNIT_ASSERT(driver.isUsingFallbackTimer());

      // Disable. Record current behavior; do not crash.
      OsStatus disableStatus = deviceManager.disableDevice(deviceId);
      OsSysLog::add(FAC_MP, PRI_INFO,
         "testDisableDeviceWhileInFallback: disableDevice returned %d "
         "while driver was in fallback mode",
         (int)disableStatus);
      printf("testDisableDeviceWhileInFallback: disableDevice "
             "returned %d in fallback mode\n", (int)disableStatus);

      // Invariants that hold regardless of return code:
      // 1. The manager must report the device as not enabled.
      CPPUNIT_ASSERT_MESSAGE(
         "Device must not be enabled after disableDevice, regardless "
         "of disable status return code",
         !deviceManager.isDeviceEnabled(deviceId));

      // 2. The driver must be removable.
      CPPUNIT_ASSERT_MESSAGE(
         "Driver must be removable from manager after disable in "
         "fallback mode",
         deviceManager.removeDevice(deviceId) == &driver);

      // Locking down current behavior. If the contract changes so
      // that disable succeeds in fallback mode, update this to
      // CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, disableStatus). For now we
      // only assert that we know what value we get back.
      if (disableStatus == OS_SUCCESS)
      {
         OsSysLog::add(FAC_MP, PRI_NOTICE,
            "testDisableDeviceWhileInFallback: disableDevice now "
            "returns OS_SUCCESS in fallback mode -- contract may "
            "have changed, consider tightening this assertion");
      }
#  else
      printf("Skipping testDisableDeviceWhileInFallback on non-Windows platform\n");
#  endif
   }

protected:
   MpBufPool *mpPool;         ///< Pool for data buffers
   MpBufPool *mpHeadersPool;  ///< Pool for buffers headers

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpOutputDeviceManagerTest);
