//  
// Copyright (C) 2007-2026 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#include <os/OsIntTypes.h>
#include <sipxunittests.h>
#include <mp/MpBufPool.h>
#include <mp/MpArrayBuf.h>
#include <mp/MpAudioBuf.h>
#include <mp/MpAudioDeviceWatcher.h>
#include <mp/MpAudioDeviceChangeObserver.h>
#include <mp/MpInputDeviceManager.h>
#ifdef WIN32
#   include <mp/MpidWinMM.h>
#   include <mmdeviceapi.h>
#elif defined ANDROID
#   include <mp/MpidAndroid.h>
#   include <mp/MpAndroidAudioTrack.h>
#elif defined __linux__
#   include <mp/MpidOss.h>
#elif defined __APPLE__
#   include <mp/MpidCoreAudio.h>
#else
#   include <mp/MpSineWaveGeneratorDeviceDriver.h>
#endif
#include <os/OsTask.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>

#define MIDDT_SAMPLES_PER_FRAME 80
#define MIDDT_NBUFS 20

/**
 * Records what a watcher told it, in order, so a test can assert on the
 * whole sequence rather than a count.
 *
 * Assertions about absence matter as much as assertions about presence
 * here: the bug this exists to pin is a device being reported that
 * should have been silent.  A recorder that only counted could not tell
 * "nothing happened" from "something happened that I did not check for".
 */
class MpTestDeviceChangeObserver : public MpAudioDeviceChangeObserver
{
public:
 
   MpTestDeviceChangeObserver()
   : mOwnDeviceLostCount(0)
   , mNoDefaultCount(0)
   {
   };
 
   virtual ~MpTestDeviceChangeObserver()
   {
      mEvents.destroyAll();
   };
 
   void onDevicePresent(const UtlString& deviceId,
                        const UtlString& deviceName)
   {
      record("present", deviceId, deviceName);
   };
 
   void onDeviceNotPresent(const UtlString& deviceId,
                           const UtlString& deviceName)
   {
      record("notPresent", deviceId, deviceName);
   };
 
   void onOwnDeviceLost()
   {
      mOwnDeviceLostCount++;
   };
 
   void onDefaultDeviceChanged(const UtlString& deviceId,
                               const UtlString& deviceName)
   {
      record("default", deviceId, deviceName);
   };
 
   void onNoDefaultDevice()
   {
      mNoDefaultCount++;
      record("noDefault", "", "");
   };
 
      /// Number of events recorded so far.
   int getCount() const
   {
      return (int) mEvents.entries();
   };
 
      /// One event, as "kind|id|name".  Empty if the index is past the end.
   UtlString getEvent(int index) const
   {
      UtlString empty;
      if (index < 0 || index >= (int) mEvents.entries())
      {
         return empty;
      }
      return *((UtlString*) mEvents.at(index));
   };
 
   void clear()
   {
      mEvents.destroyAll();
      mOwnDeviceLostCount = 0;
      mNoDefaultCount = 0;
   };
 
   int mOwnDeviceLostCount;
   int mNoDefaultCount;
 
private:
 
   void record(const char* kind,
               const UtlString& deviceId,
               const UtlString& deviceName)
   {
      UtlString* event = new UtlString(kind);
      event->append("|");
      event->append(deviceId);
      event->append("|");
      event->append(deviceName);
      mEvents.append(event);
   };
 
   UtlSList mEvents;
};
 
 
/**
 * A watcher whose seed is whatever the test says it is.
 *
 * The cases worth testing are devices arriving and departing on cue,
 * and a machine that was already in a particular state when we started.
 * No test machine can be relied on to provide either.  Seeding by hand
 * and driving onDeviceChanged directly exercises exactly the logic that
 * decides what an application gets told, with no COM, no drivers and no
 * hardware, so these run on the build machine and on Linux alike.
 */
class MpTestAudioDeviceWatcher : public MpAudioDeviceWatcher
{
public:
 
   MpTestAudioDeviceWatcher(MpDeviceFlow flow,
                            MpAudioDeviceChangeObserver* observer)
   : MpAudioDeviceWatcher(flow, observer)
   , mRegisterCount(0)
   , mUnregisterCount(0)
   {
   };
 
      /// Add a device to the seed, to be applied when start() is called.
   void addToSeed(const char* deviceId,
                  const char* displayName,
                  const char* apiName)
   {
      UtlString* seed = new UtlString(deviceId);
      seed->append("|");
      seed->append(displayName);
      seed->append("|");
      seed->append(apiName);
      mSeed.append(seed);
   };
 
   void setSeedDefaultDevice(const char* deviceId, const char* displayName)
   {
      mSeedDefaultId = deviceId;
      mSeedDefaultName = displayName;
   };
 
   int mRegisterCount;
   int mUnregisterCount;
 
protected:
 
   OsStatus seedAvailableDevices()
   {
      UtlSListIterator iterator(mSeed);
      UtlString* seed = NULL;
      while ((seed = (UtlString*) iterator()))
      {
         UtlString deviceId;
         UtlString displayName;
         UtlString apiName;
         split(*seed, deviceId, displayName, apiName);
         addSeedDevice(deviceId, displayName, apiName);
      }
 
      if (!mSeedDefaultId.isNull())
      {
         setSeedDefault(mSeedDefaultId, mSeedDefaultName);
      }
 
      return OS_SUCCESS;
   };
 
   OsStatus registerForChanges()
   {
      mRegisterCount++;
      return OS_SUCCESS;
   };
 
   OsStatus unregisterForChanges()
   {
      mUnregisterCount++;
      return OS_SUCCESS;
   };
 
private:
 
   static void split(const UtlString& packed,
                     UtlString& first,
                     UtlString& second,
                     UtlString& third)
   {
      // A local copy because UtlString::operator() is not const, and
      // plain int rather than ssize_t, which is not defined here.
      UtlString work(packed);
      int firstBar = (int) work.index("|");
      int secondBar = (int) work.index("|", firstBar + 1);

      first = work(0, firstBar);
      second = work(firstBar + 1, secondBar - firstBar - 1);
      third = work(secondBar + 1, (int) work.length() - secondBar - 1);
   };
 
   UtlSList  mSeed;
   UtlString mSeedDefaultId;
   UtlString mSeedDefaultName;
};


class MpInputDeviceDriverTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(MpInputDeviceDriverTest);

   CPPUNIT_TEST(testSetup);
   CPPUNIT_TEST(testNonexistentInputDevice);
   CPPUNIT_TEST(testReEnableInputDevice);
   CPPUNIT_TEST(testInputDriverWithEmptyName);
   CPPUNIT_TEST(testDoubleEnableInputDevice);
   CPPUNIT_TEST(testDoubleDisableInputDevice);
   CPPUNIT_TEST(testIsDeviceHardwareDetached);
   CPPUNIT_TEST(testCaptureEndpointsMatchWinMM);
   CPPUNIT_TEST(testRenderEndpointsMatchWinMM);
   CPPUNIT_TEST(testGetEndpointDataFlow);

   CPPUNIT_TEST(testEscapedDriverRefusesReenable);
   CPPUNIT_TEST(testFireEscapeOnStuckWorker);
   CPPUNIT_TEST(testGenerationAdvancesAcrossSessions);

   CPPUNIT_TEST(testWatcherSeedIsSilent);
   CPPUNIT_TEST(testWatcherReportsArrival);
   CPPUNIT_TEST(testWatcherIgnoresRepeatArrival);
   CPPUNIT_TEST(testWatcherReportsDeparture);
   CPPUNIT_TEST(testWatcherIgnoresDepartureOfUnknownDevice);
   CPPUNIT_TEST(testWatcherReportsCachedNameOnDeparture);
   CPPUNIT_TEST(testWatcherReportsTwoDeparturesInOrder);
   CPPUNIT_TEST(testWatcherDeduplicatesDefaultChange);
   CPPUNIT_TEST(testWatcherReportsNoDefaultSeparately);
   CPPUNIT_TEST(testWatcherFindsDeviceByEitherName);

   CPPUNIT_TEST_SUITE_END();

private:
   MpBufPool* mpBufPool;
   MpBufPool* mpHeadersPool;

   int mNumBufferedFrames;
   unsigned int mSamplesPerSecond;
   unsigned int mFramePeriodMSecs;

public:
   void setUp()
   {
#ifdef ANDROID
      OsStatus stat = MpAndroidAudioBindingInterface::spGetAndroidAudioBinding()->getStatus();
#endif

      mpBufPool = 
         new MpBufPool(MIDDT_SAMPLES_PER_FRAME * sizeof(MpAudioSample)
                       + MpArrayBuf::getHeaderSize(), 
                       MIDDT_NBUFS, "MpInputDeviceDriverTest");
      CPPUNIT_ASSERT(mpBufPool != NULL);

      // Create pool for buffer headers
      mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), MIDDT_NBUFS, "MpInputDeviceDriverTestHeaders");
      CPPUNIT_ASSERT(mpHeadersPool != NULL);

      // Set mpHeadersPool as default pool for audio and data pools.
      MpAudioBuf::smpDefaultPool = mpHeadersPool;
      MpDataBuf::smpDefaultPool = mpHeadersPool;

      mNumBufferedFrames = 5;
      mSamplesPerSecond = 8000;
      mFramePeriodMSecs = MIDDT_SAMPLES_PER_FRAME * 1000 / mSamplesPerSecond;
   }



   void testSetup()
   {
      MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME, 
                                    mSamplesPerSecond,
                                    mNumBufferedFrames, 
                                    *mpBufPool);

      // Buffer for recorded data.
      MpAudioSample* pRecordBuffer = new MpAudioSample[mNumBufferedFrames* MIDDT_SAMPLES_PER_FRAME];
      int pRecordBufferPointer = 0;


      MpInputDeviceDriver* pInDevDriver = 
#ifdef WIN32
         new MpidWinMM(MpidWinMM::getDefaultDeviceName(), inDevMgr);
#elif defined ANDROID
         new MpidAndroid(MpidAndroid::AUDIO_SOURCE_DEFAULT, inDevMgr);
#elif defined __linux__
         new MpidOss("/dev/dsp", inDevMgr);
#elif defined __APPLE__
         new MpidCoreAudio("[default]", inDevMgr);
#else
         new MpSineWaveGeneratorDeviceDriver("SineWaveDriver", inDevMgr,
                                             3000, 3000, 0);
#endif
      if (pInDevDriver != NULL)
      {
         // Verify that our device is indeed valid and, if not using the test
         // driver, is indeed pointing at an actual device in the OS.
         CPPUNIT_ASSERT(pInDevDriver->isDeviceValid());

         // Since we've only just created this device, it shouldn't be enabled.
         CPPUNIT_ASSERT(!pInDevDriver->isEnabled());
         // And shouldn't have a valid device handle/ID.
         CPPUNIT_ASSERT(pInDevDriver->getDeviceId() < 0);

         // Try to enable the device when it isn't added to a manager..
         // SHOULDN'T DO THIS - Only the manager should be able to do this..
         // perhaps enabling should be protected, and manager be friended?
         //CPPUNIT_ASSERT(iDrv->enableDevice(10,10,10) != OS_SUCCESS);

         // Add the device to an input manager.
         MpInputDeviceHandle iDrvHnd = inDevMgr.addDevice(*pInDevDriver);

         // Verify it has a valid handle/ID.
         CPPUNIT_ASSERT(iDrvHnd > 0);

         // Try to disable it -- this should fail, since it isn't enabled yet.
         // Also note that one should be disabling/enabling via the manager..
         // I'm just verifying that disabling the device itself when it isn't
         // set up doesn't kill things.
         CPPUNIT_ASSERT(pInDevDriver->disableDevice() != OS_SUCCESS);

         // Now enable it via the manager -- this should succeed.
         CPPUNIT_ASSERT(inDevMgr.enableDevice(iDrvHnd) == OS_SUCCESS);

         int nMSPerBuffer = mNumBufferedFrames * mFramePeriodMSecs;
         unsigned nMSecsToRecord = 5000;
         double* derivs = new double[(mNumBufferedFrames-1)*(nMSecsToRecord/nMSPerBuffer)];
         // Round nMSecsToRecord to nMSPerBuffer boundary.
         nMSecsToRecord = (nMSecsToRecord/nMSPerBuffer) * nMSPerBuffer;

         UtlString derivPlotStr;
         derivPlotStr.capacity((nMSecsToRecord/mFramePeriodMSecs) << 2);
         UtlString derivWAvgStr;

         unsigned i;
         for(i=0;i<(mNumBufferedFrames-1)*(nMSecsToRecord/nMSPerBuffer);i++)
            derivs[i] = -1;

         unsigned derivBufPos;
         unsigned derivBufSz = 0;
         unsigned nDerivsPerBuf = mNumBufferedFrames-1;
         for(i = 0, derivBufPos = 0;
             i < nMSecsToRecord; 
             i = i+nMSPerBuffer, derivBufPos += nDerivsPerBuf)
         {
            // Reset nDerivsPerBuf, as getting time derivs could have changed it.
            nDerivsPerBuf = mNumBufferedFrames-1;
            
            // Sleep till when the input buffer should be full
            OsTask::delay(nMSPerBuffer);

            // Grab time derivative statistics..
            double* curDerivFramePtr = (double*)(derivs + derivBufPos);
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 inDevMgr.getTimeDerivatives(iDrvHnd, 
                                                             nDerivsPerBuf, 
                                                             curDerivFramePtr));
            derivBufSz += nDerivsPerBuf;
         }

         // Ok, now disable it via the manager -- this time it should succeed.
         CPPUNIT_ASSERT(inDevMgr.disableDevice(iDrvHnd) == OS_SUCCESS);

         // Define weighted average accumulator and period.
         double derivWeightedAverage = 0;
         int derivWAvgPeriod = 5;

         // Now that we have all the derivatives, 
         // make a string out of em..
         for(i = 0; i < derivBufSz; i++)
         {
            // Prepare the derivative line to print.
#           define NUMSTRSZ 32
            char tmpBuf[NUMSTRSZ];

            // Add derivative to our big-long string that can be used for plotting.
            snprintf(tmpBuf, NUMSTRSZ, "%.2f", derivs[i]);
            derivPlotStr.append(tmpBuf);
            if(i < derivBufSz-1) // While there's still one more, put a comma
               derivPlotStr.append(", ");

            if ((i != 0) && (i % derivWAvgPeriod) == 0)
            {
               // Now that we have derivWAvgPeriod samples,
               // calculate and assign the actual weighted average.
               derivWeightedAverage = derivWeightedAverage / derivWAvgPeriod;

               // Now append this to our weighted average string.
               snprintf(tmpBuf, NUMSTRSZ, "%.2f", derivWeightedAverage);
               derivWAvgStr.append(tmpBuf);
               derivWAvgStr.append(", ");

               // reset the weighted average collector.
               derivWeightedAverage = 0;
            }

            derivWeightedAverage += derivs[i];

            CPPUNIT_ASSERT(derivs[i] <= 4);
         }

         // Remove the device from the manager explicitly, 
         // Otherwise the manager will assert fail if there are devices
         // still present when the manager is destroyed
         inDevMgr.removeDevice(iDrvHnd);

         // Now print out our derivative results.
         printf(" derivatives: %s\n", derivPlotStr.data());
         printf("weighted avg: %s\n", derivWAvgStr.data());
      }  // if pInDevDriver != NULL
   }

   void testNonexistentInputDevice()
   {
#  ifdef WIN32
      // Construct an input driver with a name that cannot match any
      // enumerated device. Verifies the driver lands in the
      // "device-not-found" state cleanly: isDeviceValid returns
      // false, isEnabled returns false, getDeviceId returns < 0,
      // and enableDevice returns OS_INVALID_STATE.

      MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME,
                                    mSamplesPerSecond,
                                    mNumBufferedFrames,
                                    *mpBufPool);

      // A name that contains characters no real device would have.
      MpidWinMM driver("__no_such_device_name_test__", inDevMgr);

      CPPUNIT_ASSERT_MESSAGE(
         "Driver constructed with a non-existent name must report "
         "isDeviceValid() == false.",
         !driver.isDeviceValid());

      CPPUNIT_ASSERT_MESSAGE(
         "A freshly-constructed driver must not report enabled.",
         !driver.isEnabled());

      CPPUNIT_ASSERT_MESSAGE(
         "A driver not added to a manager must report deviceId < 0.",
         driver.getDeviceId() < 0);

      // enableDevice on an invalid driver must return OS_INVALID_STATE
      // and must not transition to enabled.
      OsStatus enableStatus = driver.enableDevice(MIDDT_SAMPLES_PER_FRAME,
                                                  mSamplesPerSecond,
                                                  0);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "enableDevice on a non-existent device must return "
         "OS_INVALID_STATE.",
         OS_INVALID_STATE, enableStatus);
      CPPUNIT_ASSERT_MESSAGE(
         "Failed enableDevice must not leave the driver enabled.",
         !driver.isEnabled());
#  else
      SIPX_TEST_SKIP("MpidWinMM is Windows-only");
#  endif
   }

   void testReEnableInputDevice()
   {
#  ifdef WIN32
      // Construct, add to manager, enable, disable, enable, disable.
      // Direct coverage for the disable-cleanup fix in Change 8
      // mirror: a second enable must succeed even after the first
      // enable+disable cycle has fully exercised the driver's
      // buffer allocation, prepare-header, add-buffer, and reset
      // paths.

      MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME,
                                    mSamplesPerSecond,
                                    mNumBufferedFrames,
                                    *mpBufPool);

      MpidWinMM* pDriver = new MpidWinMM(MpidWinMM::getDefaultDeviceName(),
                                         inDevMgr);

      if (!pDriver->isDeviceValid())
      {
         delete pDriver;
         SIPX_TEST_SKIP("no valid input audio device available");
      }

      MpInputDeviceHandle iDrvHnd = inDevMgr.addDevice(*pDriver);
      CPPUNIT_ASSERT(iDrvHnd > 0);

      // First enable/disable cycle.
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "First enableDevice must succeed.",
         OS_SUCCESS, inDevMgr.enableDevice(iDrvHnd));
      CPPUNIT_ASSERT(pDriver->isEnabled());

      // Brief delay so the wave callback has a chance to run; not
      // required for correctness of the re-enable, but makes the
      // test exercise the full enable-with-callbacks path rather
      // than a near-immediate disable.
      OsTask::delay(50);

      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "First disableDevice must succeed.",
         OS_SUCCESS, inDevMgr.disableDevice(iDrvHnd));
      CPPUNIT_ASSERT(!pDriver->isEnabled());

      // Second enable/disable cycle. The key assertion: this must
      // succeed. Before Change 8 mirror, the driver could leave
      // residual state that caused the second enable to fail or
      // crash.
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "Second enableDevice must succeed. Failure here means the "
         "input-side disable cleanup is leaving residual state that "
         "prevents re-enable.",
         OS_SUCCESS, inDevMgr.enableDevice(iDrvHnd));
      CPPUNIT_ASSERT(pDriver->isEnabled());

      OsTask::delay(50);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inDevMgr.disableDevice(iDrvHnd));
      CPPUNIT_ASSERT(!pDriver->isEnabled());

      inDevMgr.removeDevice(iDrvHnd);
      delete pDriver;
#  else
      SIPX_TEST_SKIP("MpidWinMM is Windows-only");
#  endif
   }

   void testInputDriverWithEmptyName()
   {
#  ifdef WIN32
      // Construct an input driver with the empty string as a device
      // name. The empty-string-means-default convention is consumed
      // at the sipXmediaAdapterLib layer, so the driver should never
      // see "" in practice. This test pins down what happens if it
      // does: empty string matches no device, isDeviceValid is false,
      // enableDevice returns OS_INVALID_STATE.

      MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME,
                                    mSamplesPerSecond,
                                    mNumBufferedFrames,
                                    *mpBufPool);

      MpidWinMM driver("", inDevMgr);

      CPPUNIT_ASSERT_MESSAGE(
         "Driver constructed with empty-string name must report "
         "isDeviceValid() == false (empty string matches no device).",
         !driver.isDeviceValid());

      OsStatus enableStatus = driver.enableDevice(MIDDT_SAMPLES_PER_FRAME,
                                                  mSamplesPerSecond,
                                                  0);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "enableDevice on empty-name driver must return "
         "OS_INVALID_STATE.",
         OS_INVALID_STATE, enableStatus);
#  else
      SIPX_TEST_SKIP("MpidWinMM is Windows-only");
#  endif
   }

   void testDoubleEnableInputDevice()
   {
#  ifdef WIN32
      // Verify the input driver's "if (isEnabled()) return OS_FAILED"
      // guard. Calling enableDevice on an already-enabled driver must
      // return non-success and must not disturb the enabled state.

      MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME,
                                    mSamplesPerSecond,
                                    mNumBufferedFrames,
                                    *mpBufPool);

      MpidWinMM* pDriver = new MpidWinMM(MpidWinMM::getDefaultDeviceName(),
                                         inDevMgr);

      if (!pDriver->isDeviceValid())
      {
         delete pDriver;
         SIPX_TEST_SKIP("no valid input audio device available");
      }

      MpInputDeviceHandle iDrvHnd = inDevMgr.addDevice(*pDriver);
      CPPUNIT_ASSERT(iDrvHnd > 0);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inDevMgr.enableDevice(iDrvHnd));
      CPPUNIT_ASSERT(pDriver->isEnabled());

      // Second enableDevice on the already-enabled driver. Must not
      // crash, must not return OS_SUCCESS, must leave the driver
      // enabled.
      OsStatus secondStatus = pDriver->enableDevice(MIDDT_SAMPLES_PER_FRAME,
                                                    mSamplesPerSecond,
                                                    0);
      CPPUNIT_ASSERT_MESSAGE(
         "Double enableDevice must not return OS_SUCCESS.",
         secondStatus != OS_SUCCESS);
      CPPUNIT_ASSERT_MESSAGE(
         "Double enableDevice must leave the driver enabled.",
         pDriver->isEnabled());

      // Clean up.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inDevMgr.disableDevice(iDrvHnd));
      inDevMgr.removeDevice(iDrvHnd);
      delete pDriver;
#  else
      SIPX_TEST_SKIP("MpidWinMM is Windows-only");
#  endif
   }

   void testDoubleDisableInputDevice()
   {
#  ifdef WIN32
      // Verify disableDevice's behavior when called on an
      // already-disabled driver. Must not crash and must not
      // transition state in unexpected ways.

      MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME,
                                    mSamplesPerSecond,
                                    mNumBufferedFrames,
                                    *mpBufPool);

      MpidWinMM* pDriver = new MpidWinMM(MpidWinMM::getDefaultDeviceName(),
                                         inDevMgr);

      if (!pDriver->isDeviceValid())
      {
         delete pDriver;
         SIPX_TEST_SKIP("no valid input audio device available");
      }

      MpInputDeviceHandle iDrvHnd = inDevMgr.addDevice(*pDriver);
      CPPUNIT_ASSERT(iDrvHnd > 0);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inDevMgr.enableDevice(iDrvHnd));
      CPPUNIT_ASSERT(pDriver->isEnabled());

      OsTask::delay(50);

      // First disable.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inDevMgr.disableDevice(iDrvHnd));
      CPPUNIT_ASSERT(!pDriver->isEnabled());

      // Second disable on the already-disabled driver. Must not
      // return OS_SUCCESS and must not crash.
      OsStatus secondStatus = pDriver->disableDevice();
      CPPUNIT_ASSERT_MESSAGE(
         "Double disableDevice must not return OS_SUCCESS.",
         secondStatus != OS_SUCCESS);
      CPPUNIT_ASSERT_MESSAGE(
         "Double disableDevice must leave the driver disabled.",
         !pDriver->isEnabled());

      inDevMgr.removeDevice(iDrvHnd);
      delete pDriver;
#  else
      SIPX_TEST_SKIP("MpidWinMM is Windows-only");
#  endif
   }

void testIsDeviceHardwareDetached()
   {
#  ifdef WIN32
      MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME,
                                    mSamplesPerSecond,
                                    mNumBufferedFrames,
                                    *mpBufPool);

      MpidWinMM* pDriver = new MpidWinMM(MpidWinMM::getDefaultDeviceName(),
                                         inDevMgr);

      if (!pDriver->isDeviceValid())
      {
         delete pDriver;
         SIPX_TEST_SKIP("no valid input audio device available");
      }

      MpInputDeviceHandle iDrvHnd = inDevMgr.addDevice(*pDriver);
      CPPUNIT_ASSERT(iDrvHnd > 0);

      // Before enable: not detached.
      CPPUNIT_ASSERT(!pDriver->isDeviceHardwareDetached());
      CPPUNIT_ASSERT(!inDevMgr.isDeviceInFallbackMode(iDrvHnd));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inDevMgr.enableDevice(iDrvHnd));
      CPPUNIT_ASSERT(pDriver->isEnabled());

      // Enabled with hardware open: not detached.
      CPPUNIT_ASSERT(!pDriver->isDeviceHardwareDetached());
      CPPUNIT_ASSERT(!inDevMgr.isDeviceInFallbackMode(iDrvHnd));

      // Simulate hardware detach (USB unplug).
      pDriver->setIsOpenForTesting(FALSE);
      CPPUNIT_ASSERT_MESSAGE(
         "isDeviceHardwareDetached must return TRUE when enabled but mIsOpen is FALSE",
         pDriver->isDeviceHardwareDetached());
      CPPUNIT_ASSERT_MESSAGE(
         "isDeviceInFallbackMode must return TRUE when driver is hardware-detached",
         inDevMgr.isDeviceInFallbackMode(iDrvHnd));

      // Disable clears the detached state (mIsEnabled becomes FALSE).
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inDevMgr.disableDevice(iDrvHnd));
      CPPUNIT_ASSERT(!pDriver->isEnabled());
      CPPUNIT_ASSERT(!pDriver->isDeviceHardwareDetached());
      CPPUNIT_ASSERT(!inDevMgr.isDeviceInFallbackMode(iDrvHnd));

      // Re-enable must succeed after the simulated detach + disable cycle.
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "Re-enable after simulated detach must succeed.",
         OS_SUCCESS, inDevMgr.enableDevice(iDrvHnd));
      CPPUNIT_ASSERT(pDriver->isEnabled());

      inDevMgr.disableDevice(iDrvHnd);
      inDevMgr.removeDevice(iDrvHnd);
      delete pDriver;
#  else
      SIPX_TEST_SKIP("MpidWinMM is Windows-only");
#  endif
   }

#ifdef WIN32
   void testEscapedDriverRefusesReenable()
   {
#  ifdef WIN32
      // A driver whose disable took the fire-escape path is permanently
      // damaged and must refuse to run again; recovery in the field is
      // a new driver instance. Cheapest contract test: force the flag
      // via friendship, assert enable is refused and the manager
      // retires rather than deletes.

      MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME,
                                    mSamplesPerSecond,
                                    mNumBufferedFrames,
                                    *mpBufPool);

      MpidWinMM* pDriver = new MpidWinMM(MpidWinMM::getDefaultDeviceName(),
                                         inDevMgr);
      if (!pDriver->isDeviceValid())
      {
         delete pDriver;
         SIPX_TEST_SKIP("no valid input audio device available");
      }

      MpInputDeviceHandle iDrvHnd = inDevMgr.addDevice(*pDriver);
      CPPUNIT_ASSERT(iDrvHnd > 0);

      pDriver->mLastDisableEscaped = TRUE;

      CPPUNIT_ASSERT_MESSAGE(
         "enableDevice must be refused for an escaped driver.",
         inDevMgr.enableDevice(iDrvHnd) != OS_SUCCESS);
      CPPUNIT_ASSERT(!pDriver->isEnabled());

      // removeAllDevices must retire the escaped driver, not delete it.
      // The object staying readable afterwards is the assertion.
      inDevMgr.removeAllDevices();
      CPPUNIT_ASSERT_MESSAGE(
         "Escaped driver must survive removeAllDevices (retired, not "
         "deleted).",
         pDriver->lastDisableEscaped() == TRUE);

      // pDriver intentionally NOT deleted: it is on the process-lifetime
      // retire list. One object leaked per run of this test, by design.
#  else
      SIPX_TEST_SKIP("MpidWinMM is Windows-only");
#  endif
   }

   void testFireEscapeOnStuckWorker()
   {
#  ifdef WIN32
      // The customer failure: a worker that never returns from a wave
      // call. disableDevice must return within its bound, take the
      // fire-escape path, and the driver must then be retire-only.
      //
      // The stuck-in-wave-call condition is asserted via the flag with
      // the worker parked; see the comment at the suspend for why it
      // must never be frozen inside a wave call.
      //
      // Same-device recovery after an escape is deliberately NOT
      // asserted here: on a healthy device the leaked session still
      // holds the device open, a conflict that cannot occur in the
      // field (the device departed). Field recovery -- fresh driver on
      // the returned device -- is validated on the Bluetooth bench.

      MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME,
                                    mSamplesPerSecond,
                                    mNumBufferedFrames,
                                    *mpBufPool);

      MpidWinMM* pDriver = new MpidWinMM(MpidWinMM::getDefaultDeviceName(),
                                         inDevMgr);
      if (!pDriver->isDeviceValid())
      {
         delete pDriver;
         SIPX_TEST_SKIP("no valid input audio device available");
      }
      if (pDriver->mWorkerThread == NULL)
      {
         delete pDriver;
         SIPX_TEST_SKIP("worker thread was not created");
      }

      MpInputDeviceHandle iDrvHnd = inDevMgr.addDevice(*pDriver);
      CPPUNIT_ASSERT(iDrvHnd > 0);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inDevMgr.enableDevice(iDrvHnd));

      // Attempt the physical catch: suspend while the worker is inside
      // waveInAddBuffer.
      // Establish the stuck-in-wave-call condition deterministically.
      // The worker must be frozen ONLY while parked in its event wait:
      // frozen inside waveInAddBuffer it holds winmmbase's lock (mid-RPC
      // to audiosrv) and every later WinMM call in the process blocks --
      // that is what SuspendThread does, not what a stuck driver does
      // (the customer dump shows the stuck thread waiting for that lock,
      // not holding it). So quiesce, suspend parked, then assert the
      // condition through the flag disableDevice actually reads.
      InterlockedExchange(&pDriver->mWorkerStop, 1);
      OsTask::delay(30);
      SuspendThread(pDriver->mWorkerThread);
      InterlockedExchange(&pDriver->mWorkerStop, 0);
      InterlockedExchange(&pDriver->mWorkerInWaveCall, 1);

      DWORD t0 = GetTickCount();
      OsStatus disableStat = inDevMgr.disableDevice(iDrvHnd);
      DWORD elapsed = GetTickCount() - t0;

      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "disableDevice must return success on the escape path.",
         OS_SUCCESS, disableStat);
      CPPUNIT_ASSERT_MESSAGE(
         "disableDevice must return within its bound, not block. "
         "(Bound: join timeout plus manager in-use retries plus "
         "scheduling slack.)",
         elapsed < (DWORD)(MPID_WINMM_JOIN_TIMEOUT_MS * 5));
      CPPUNIT_ASSERT_MESSAGE(
         "The escape must be recorded.",
         pDriver->lastDisableEscaped() == TRUE);
      CPPUNIT_ASSERT(!pDriver->isEnabled());

      // Escaped driver must refuse re-enable.
      CPPUNIT_ASSERT_MESSAGE(
         "Re-enable of an escaped driver must be refused.",
         inDevMgr.enableDevice(iDrvHnd) != OS_SUCCESS);

      // Let the worker run again. A real caught worker completes its
      // wave call against the still-open handle, sees mWorkerStop, and
      // parks; a forced one just resumes its event wait.
      InterlockedExchange(&pDriver->mWorkerInWaveCall, 0);
      ResumeThread(pDriver->mWorkerThread);
      OsTask::delay(50);

      // Retirement instead of deletion, with the worker live again:
      // the object must survive removeAllDevices untouched.
      inDevMgr.removeAllDevices();
      CPPUNIT_ASSERT_MESSAGE(
         "Escaped driver must survive removeAllDevices (retired, not "
         "deleted).",
         pDriver->lastDisableEscaped() == TRUE);

      // pDriver and its worker thread intentionally leaked: retired,
      // process-lifetime, by design.
#  else
      SIPX_TEST_SKIP("MpidWinMM is Windows-only");
#  endif
   }

   void testGenerationAdvancesAcrossSessions()
   {
#  ifdef WIN32
      // Each enable is a new session: the generation must advance so
      // queued work from an old session can never recycle a buffer
      // into a new handle, and cycling must stay clean with the
      // constructor-lifetime worker parked between sessions.

      MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME,
                                    mSamplesPerSecond,
                                    mNumBufferedFrames,
                                    *mpBufPool);

      MpidWinMM* pDriver = new MpidWinMM(MpidWinMM::getDefaultDeviceName(),
                                         inDevMgr);
      if (!pDriver->isDeviceValid())
      {
         delete pDriver;
         SIPX_TEST_SKIP("no valid input audio device available");
      }

      MpInputDeviceHandle iDrvHnd = inDevMgr.addDevice(*pDriver);
      CPPUNIT_ASSERT(iDrvHnd > 0);

      LONG genBefore = pDriver->mGeneration;
      for (int cycle = 0; cycle < 3; cycle++)
      {
         CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "enableDevice must succeed on every cycle.",
            OS_SUCCESS, inDevMgr.enableDevice(iDrvHnd));
         CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Generation must advance by exactly one per enable.",
            genBefore + cycle + 1, (LONG)pDriver->mGeneration);

         // Let several capture bursts land: the WinMM path delivers in
         // batches tens of ms apart, so a short window sees nothing.
         OsTask::delay(500);

         // Pull frames back out of the manager -- the same path
         // MprFromInputDevice uses -- and prove this session delivered
         // real audio, not just correctly timed buffers of zeros.
         {
            MpFrameTime frameTime = inDevMgr.getCurrentFrameTime(iDrvHnd)
                                    - (MIDDT_SAMPLES_PER_FRAME * 1000
                                       / mSamplesPerSecond) * 4;
            unsigned framesGot = 0;
            unsigned framesNonSilent = 0;
            for (int f = 0; f < 4; f++)
            {
               MpBufPtr buffer;
               unsigned before = 0, after = 0;
               if (inDevMgr.getFrame(iDrvHnd, frameTime, buffer, before, after)
                   == OS_SUCCESS && buffer.isValid())
               {
                  framesGot++;
                  MpAudioBufPtr audio = buffer;
                  const MpAudioSample* s = audio->getSamplesPtr();
                  unsigned n = audio->getSamplesNumber();
                  for (unsigned i = 0; i < n; i++)
                  {
                     if (s[i] != 0)
                     {
                        framesNonSilent++;
                        break;
                     }
                  }
               }
               frameTime += MIDDT_SAMPLES_PER_FRAME * 1000 / mSamplesPerSecond;
            }
            CPPUNIT_ASSERT_MESSAGE(
               "Frames must reach the manager in every session.",
               framesGot > 0);
            CPPUNIT_ASSERT_MESSAGE(
               "Captured frames must contain audio, not only zeros. A mic "
               "delivering pure silence is a real finding, not a flake.",
               framesNonSilent > 0);
         }

         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inDevMgr.disableDevice(iDrvHnd));
         CPPUNIT_ASSERT(!pDriver->isEnabled());
         CPPUNIT_ASSERT_MESSAGE(
            "Clean disable must not set the escape flag.",
            pDriver->lastDisableEscaped() == FALSE);
      }

      inDevMgr.removeDevice(iDrvHnd);
      delete pDriver;
#  else
      SIPX_TEST_SKIP("MpidWinMM is Windows-only");
#  endif
   }

   // Collect friendly names of all MMDevice endpoints in the ACTIVE
   // state for the given flow. Caller owns the UtlString entries.
   void collectActiveEndpointNames(IMMDeviceEnumerator* enumerator,
                                   EDataFlow flow,
                                   DWORD stateMask,
                                   UtlSList& names)
   {
      IMMDeviceCollection* collection = NULL;
      HRESULT hr = enumerator->EnumAudioEndpoints(flow,
                                                  stateMask,
                                                  &collection);
      CPPUNIT_ASSERT_MESSAGE("EnumAudioEndpoints failed",
                             hr == S_OK && collection != NULL);

      UINT count = 0;
      hr = collection->GetCount(&count);
      CPPUNIT_ASSERT_MESSAGE("IMMDeviceCollection::GetCount failed",
                             hr == S_OK);

      for (UINT i = 0; i < count; i++)
      {
         IMMDevice* device = NULL;
         if (collection->Item(i, &device) == S_OK && device != NULL)
         {
            LPWSTR endpointId = NULL;
            if (device->GetId(&endpointId) == S_OK && endpointId != NULL)
            {
               UtlString name;
               MpidWinMM::getWinNameForDevice(enumerator, endpointId, name);
               names.append(new UtlString(name));
               CoTaskMemFree(endpointId);
            }
            device->Release();
         }
      }

      collection->Release();
   }

   // The device change watcher decides whether a state change is news
   // by tracking which endpoints were ACTIVE. That is only a valid
   // proxy for the application visible device list if MMDevice ACTIVE
   // covers everything WinMM enumerates, with corresponding names.
   void compareEnumerations(UtlBoolean isCapture)
   {
      const char* label = isCapture ? "capture" : "render";

      UINT winmmCount = isCapture ? waveInGetNumDevs() : waveOutGetNumDevs();
      if (winmmCount == 0)
      {
         SIPX_TEST_SKIP("no WinMM audio devices present");
      }

      // Deliberately not released: matches current driver behavior,
      // where this enumerator is retained for the life of the object.
      // See the TODO in ~MpodWinMM.
      IMMDeviceEnumerator* enumerator = MpidWinMM::getWinDeviceEnumerator();
      CPPUNIT_ASSERT_MESSAGE("could not obtain IMMDeviceEnumerator",
                             enumerator != NULL);

      UtlSList activeNames;
collectActiveEndpointNames(enumerator,
                                 isCapture ? eCapture : eRender,
                                 DEVICE_STATE_ACTIVE,
                                 activeNames);

      // Endpoints in every state, to see whether this machine has any
      // non-active endpoints at all. Without them the ACTIVE-only
      // comparison above proves nothing.
      UtlSList allNames;
      collectActiveEndpointNames(enumerator,
                                 isCapture ? eCapture : eRender,
                                 DEVICE_STATEMASK_ALL,
                                 allNames);
      printf("ENDPOINTCHECK %s endpoints all states: %d active: %d\n",
             label, (int) allNames.entries(), (int) activeNames.entries());
      UtlSListIterator allIterator(allNames);
      UtlString* anyName = NULL;
      while ((anyName = (UtlString*) allIterator()))
      {
         printf("ENDPOINTCHECK   %s any-state: \"%s\"\n",
                label, anyName->data());
      }
      fflush(stdout);
      allNames.destroyAll();

      printf("ENDPOINTCHECK %s WinMM devices: %u MMDevice active endpoints: %d\n",
             label, winmmCount, (int) activeNames.entries());
      fflush(stdout);
      OsSysLog::add(FAC_AUDIO, PRI_INFO,
         "MpInputDeviceDriverTest: %s WinMM devices: %u "
         "MMDevice active endpoints: %d",
         label, winmmCount, (int) activeNames.entries());

      int unmatched = 0;

      for (UINT i = 0; i < winmmCount; i++)
      {
         UtlString winmmName;
         if (isCapture)
         {
            WAVEINCAPSA caps;
            if (waveInGetDevCapsA(i, &caps, sizeof(caps)) != MMSYSERR_NOERROR)
            {
               continue;
            }
            winmmName = caps.szPname;
         }
         else
         {
            WAVEOUTCAPSA caps;
            if (waveOutGetDevCapsA(i, &caps, sizeof(caps)) != MMSYSERR_NOERROR)
            {
               continue;
            }
            winmmName = caps.szPname;
         }

         UtlBoolean found = FALSE;
         UtlSListIterator iterator(activeNames);
         UtlString* endpointName = NULL;
         while (!found && (endpointName = (UtlString*) iterator()))
         {
            if (MpidWinMM::nameIsSame(winmmName, *endpointName))
            {
               found = TRUE;
            }
         }

         printf("ENDPOINTCHECK %s WinMM[%u] \"%s\" %s\n",
             label, i, winmmName.data(),
             found ? "matched" : "NO ACTIVE MMDevice MATCH");
         fflush(stdout);
         OsSysLog::add(FAC_AUDIO, found ? PRI_INFO : PRI_ERR,
            "MpInputDeviceDriverTest: %s WinMM[%u] \"%s\" %s",
            label, i, winmmName.data(),
            found ? "matched an active endpoint"
                  : "HAS NO ACTIVE MMDevice MATCH");

         if (!found)
         {
            unmatched++;
         }
      }

      int activeCount = (int) activeNames.entries();
      activeNames.destroyAll();

      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "MMDevice ACTIVE endpoint count differs from WinMM device count; "
         "the active endpoint set may not track the application visible "
         "device list",
         (int) winmmCount, activeCount);

      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "one or more WinMM devices had no matching active MMDevice "
         "endpoint; the active endpoint set is not a valid proxy for "
         "the application visible device list",
         0, unmatched);
   }
#endif

   void testCaptureEndpointsMatchWinMM()
   {
#  ifdef WIN32
      compareEnumerations(TRUE);
#  else
      SIPX_TEST_SKIP("MMDevice enumeration is Windows-only");
#  endif
   }

   void testRenderEndpointsMatchWinMM()
   {
#  ifdef WIN32
      compareEnumerations(FALSE);
#  else
      SIPX_TEST_SKIP("MMDevice enumeration is Windows-only");
#  endif
   }

   void testGetEndpointDataFlow()
   {
#ifdef WIN32
       IMMDeviceEnumerator* pEnum = MpidWinMM::getWinDeviceEnumerator();
       if (!pEnum)
       {
           SIPX_TEST_SKIP("could not get IMMDeviceEnumerator");
       }
   
       IMMDeviceCollection* pCollection = NULL;
       HRESULT hr = pEnum->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &pCollection);
       if (hr != S_OK || !pCollection)
       {
           pEnum->Release();
           SIPX_TEST_SKIP("no active audio endpoints available");
       }
   
       UINT count = 0;
       pCollection->GetCount(&count);
       if (count == 0)
       {
           pCollection->Release();
           pEnum->Release();
           SIPX_TEST_SKIP("no active audio endpoints available");
       }
   
       for (UINT i = 0; i < count; i++)
       {
           IMMDevice* pDevice = NULL;
           if (pCollection->Item(i, &pDevice) != S_OK || !pDevice)
               continue;
   
           LPWSTR pwszId = NULL;
           if (pDevice->GetId(&pwszId) != S_OK || !pwszId)
           {
               pDevice->Release();
               continue;
           }
   
           MpidWinMM::MpAudioEndpointFlow flow = MpidWinMM::MP_FLOW_UNKNOWN;
           bool result = MpidWinMM::getEndpointDataFlow(pEnum, pwszId, flow);
   
           UtlString deviceName;
           MpidWinMM::getWinNameForDevice(pEnum, pwszId, deviceName);
   
           UtlString msg;
           msg.appendFormat("getEndpointDataFlow failed for device: %s", deviceName.data());
           CPPUNIT_ASSERT_MESSAGE(msg.data(), result);
   
           msg = "";
           msg.appendFormat("unexpected MP_FLOW_UNKNOWN for device: %s", deviceName.data());
           CPPUNIT_ASSERT_MESSAGE(msg.data(),
               flow == MpidWinMM::MP_FLOW_RENDER || flow == MpidWinMM::MP_FLOW_CAPTURE);
   
           CoTaskMemFree(pwszId);
           pDevice->Release();
       }
   
       pCollection->Release();
       pEnum->Release();
#else
       SIPX_TEST_SKIP("MpidWinMM is Windows-only");
#endif
   }


   void testWatcherSeedIsSilent()
   {
      // Establishing what was already true is not an observation that
      // anything changed.  If seeding fired events, every device on the
      // machine would look like it had just been plugged in.
      MpTestDeviceChangeObserver observer;
      MpTestAudioDeviceWatcher watcher(
         MpAudioDeviceWatcher::MP_DEVICE_FLOW_CAPTURE, &observer);
 
      watcher.addToSeed("{0.0.1.0}.{aaa}", "Microphone (Intel Array)",
                        "Microphone (Intel Array)");
      watcher.addToSeed("{0.0.1.0}.{bbb}", "Headset (EPOS)", "Headset (EPOS)");
 
      CPPUNIT_ASSERT(watcher.start() == OS_SUCCESS);
 
      CPPUNIT_ASSERT_EQUAL(0, observer.getCount());
      CPPUNIT_ASSERT_EQUAL(2, watcher.getNumDevices());
      CPPUNIT_ASSERT_EQUAL(1, watcher.mRegisterCount);
 
      CPPUNIT_ASSERT(watcher.stop() == OS_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(1, watcher.mUnregisterCount);
   };
 
 
   void testWatcherReportsArrival()
   {
      MpTestDeviceChangeObserver observer;
      MpTestAudioDeviceWatcher watcher(
         MpAudioDeviceWatcher::MP_DEVICE_FLOW_CAPTURE, &observer);
      CPPUNIT_ASSERT(watcher.start() == OS_SUCCESS);
 
      watcher.onDeviceChanged("{0.0.1.0}.{sanas}",
                              "Microphone (Sanas Audio)",
                              "Microphone (Sanas Audio",
                              TRUE);
 
      CPPUNIT_ASSERT_EQUAL(1, observer.getCount());
 
      // The API name is reported, not the display name: an application
      // hands this straight back to sipxAudioSetCallInputDevice.
      UtlString expected("present|{0.0.1.0}.{sanas}|Microphone (Sanas Audio");
      CPPUNIT_ASSERT_EQUAL(expected, observer.getEvent(0));
 
      CPPUNIT_ASSERT_EQUAL(1, watcher.getNumDevices());
   };
 
 
   void testWatcherIgnoresRepeatArrival()
   {
      // A platform may report a device as available more than once, for
      // reasons of its own.  Only the first is news.
      MpTestDeviceChangeObserver observer;
      MpTestAudioDeviceWatcher watcher(
         MpAudioDeviceWatcher::MP_DEVICE_FLOW_CAPTURE, &observer);
      CPPUNIT_ASSERT(watcher.start() == OS_SUCCESS);
 
      watcher.onDeviceChanged("{id}", "Mic", "Mic", TRUE);
      watcher.onDeviceChanged("{id}", "Mic", "Mic", TRUE);
      watcher.onDeviceChanged("{id}", "Mic", "Mic", TRUE);
 
      CPPUNIT_ASSERT_EQUAL(1, observer.getCount());
      CPPUNIT_ASSERT_EQUAL(1, watcher.getNumDevices());
   };
 
 
   void testWatcherReportsDeparture()
   {
      // The DELL monitor case from the 2026-08-20 customer log: a device
      // that was available goes unplugged.  A real departure, and the
      // code as it stood said nothing at all, because it handled only
      // one of the several states that mean gone.
      MpTestDeviceChangeObserver observer;
      MpTestAudioDeviceWatcher watcher(
         MpAudioDeviceWatcher::MP_DEVICE_FLOW_RENDER, &observer);
 
      watcher.addToSeed("{0.0.0.0}.{dell}", "DELL S2721HN", "DELL S2721HN");
      CPPUNIT_ASSERT(watcher.start() == OS_SUCCESS);
 
      watcher.onDeviceChanged("{0.0.0.0}.{dell}", "DELL S2721HN",
                              "DELL S2721HN", FALSE);
 
      CPPUNIT_ASSERT_EQUAL(1, observer.getCount());
      UtlString expected("notPresent|{0.0.0.0}.{dell}|DELL S2721HN");
      CPPUNIT_ASSERT_EQUAL(expected, observer.getEvent(0));
      CPPUNIT_ASSERT_EQUAL(0, watcher.getNumDevices());
   };
 
 
   void testWatcherIgnoresDepartureOfUnknownDevice()
   {
      // The DOSS headset case from the same log.  It was already
      // unplugged when we started, then moved to another state that
      // also means gone.  The application never saw it available, so
      // there is nothing to tell anyone about.  The code as it stood
      // reported this twice.
      MpTestDeviceChangeObserver observer;
      MpTestAudioDeviceWatcher watcher(
         MpAudioDeviceWatcher::MP_DEVICE_FLOW_CAPTURE, &observer);
 
      watcher.addToSeed("{0.0.1.0}.{intel}", "Microphone (Intel Array)",
                        "Microphone (Intel Array)");
      CPPUNIT_ASSERT(watcher.start() == OS_SUCCESS);
 
      watcher.onDeviceChanged("{0.0.1.0}.{doss}", "Headset (DOSS)",
                              "Headset (DOSS)", FALSE);
      watcher.onDeviceChanged("{0.0.1.0}.{doss}", "Headset (DOSS)",
                              "Headset (DOSS)", FALSE);
 
      CPPUNIT_ASSERT_EQUAL(0, observer.getCount());
      CPPUNIT_ASSERT_EQUAL(1, watcher.getNumDevices());
   };
 
 
   void testWatcherReportsCachedNameOnDeparture()
   {
      // A device that has gone away often cannot be asked for its name
      // any more, so the platform passes an empty one.  The name cached
      // when it arrived is reported instead; an event carrying an empty
      // name is no use to an application.
      MpTestDeviceChangeObserver observer;
      MpTestAudioDeviceWatcher watcher(
         MpAudioDeviceWatcher::MP_DEVICE_FLOW_CAPTURE, &observer);
      CPPUNIT_ASSERT(watcher.start() == OS_SUCCESS);
 
      watcher.onDeviceChanged("{id}", "Microphone (C-Media USB Audio Device   )",
                              "Microphone (C-Media USB Audio D", TRUE);
      observer.clear();
 
      watcher.onDeviceChanged("{id}", "", "", FALSE);
 
      CPPUNIT_ASSERT_EQUAL(1, observer.getCount());
      UtlString expected("notPresent|{id}|Microphone (C-Media USB Audio D");
      CPPUNIT_ASSERT_EQUAL(expected, observer.getEvent(0));
   };
 
 
   void testWatcherReportsTwoDeparturesInOrder()
   {
      // Sanas takes its capture and render endpoints away 752 ms apart.
      // Both are reported, in the order they happened.
      MpTestDeviceChangeObserver observer;
      MpTestAudioDeviceWatcher watcher(
         MpAudioDeviceWatcher::MP_DEVICE_FLOW_CAPTURE, &observer);
 
      watcher.addToSeed("{mic}", "Microphone (Sanas Audio)",
                        "Microphone (Sanas Audio");
      watcher.addToSeed("{spk}", "Speakers (Sanas Audio)",
                        "Speakers (Sanas Audio)");
      CPPUNIT_ASSERT(watcher.start() == OS_SUCCESS);
 
      watcher.onDeviceChanged("{mic}", "", "", FALSE);
      watcher.onDeviceChanged("{spk}", "", "", FALSE);
 
      CPPUNIT_ASSERT_EQUAL(2, observer.getCount());
      UtlString firstExpected("notPresent|{mic}|Microphone (Sanas Audio");
      UtlString secondExpected("notPresent|{spk}|Speakers (Sanas Audio)");
      CPPUNIT_ASSERT_EQUAL(firstExpected, observer.getEvent(0));
      CPPUNIT_ASSERT_EQUAL(secondExpected, observer.getEvent(1));
      CPPUNIT_ASSERT_EQUAL(0, watcher.getNumDevices());
   };
 
 
   void testWatcherDeduplicatesDefaultChange()
   {
      // One device change produces a default notification for each role
      // the platform keeps.  Only a change of device is news.
      MpTestDeviceChangeObserver observer;
      MpTestAudioDeviceWatcher watcher(
         MpAudioDeviceWatcher::MP_DEVICE_FLOW_CAPTURE, &observer);
      CPPUNIT_ASSERT(watcher.start() == OS_SUCCESS);
 
      watcher.onDefaultChanged("{sanas}", "Microphone (Sanas Audio)");
      watcher.onDefaultChanged("{sanas}", "Microphone (Sanas Audio)");
      watcher.onDefaultChanged("{sanas}", "Microphone (Sanas Audio)");
 
      CPPUNIT_ASSERT_EQUAL(1, observer.getCount());
      UtlString expected("default|{sanas}|Microphone (Sanas Audio)");
      CPPUNIT_ASSERT_EQUAL(expected, observer.getEvent(0));
 
      // A different device is news again.
      watcher.onDefaultChanged("{epos}", "Headset (EPOS)");
      CPPUNIT_ASSERT_EQUAL(2, observer.getCount());
   };
 
 
   void testWatcherReportsNoDefaultSeparately()
   {
      // No default device at all is a different thing from a default
      // whose name could not be resolved, and gets its own callback so
      // that an application never has to guess which it is looking at.
      MpTestDeviceChangeObserver observer;
      MpTestAudioDeviceWatcher watcher(
         MpAudioDeviceWatcher::MP_DEVICE_FLOW_CAPTURE, &observer);
 
      watcher.setSeedDefaultDevice("{sanas}", "Microphone (Sanas Audio)");
      CPPUNIT_ASSERT(watcher.start() == OS_SUCCESS);
 
      watcher.onDefaultChanged("", "");
 
      CPPUNIT_ASSERT_EQUAL(1, observer.mNoDefaultCount);
      CPPUNIT_ASSERT_EQUAL(1, observer.getCount());
 
      UtlString deviceId;
      UtlString displayName;
      CPPUNIT_ASSERT(!watcher.getDefaultDevice(deviceId, displayName));
   };
 
 
   void testWatcherFindsDeviceByEitherName()
   {
      // An application may have taken the display name from an event
      // and the API name from the device list, or the other way round,
      // and has no reason to know they are different strings.
      MpTestDeviceChangeObserver observer;
      MpTestAudioDeviceWatcher watcher(
         MpAudioDeviceWatcher::MP_DEVICE_FLOW_CAPTURE, &observer);
 
      watcher.addToSeed("{cmedia}",
                        "Microphone (C-Media USB Audio Device   )",
                        "Microphone (C-Media USB Audio D");
      CPPUNIT_ASSERT(watcher.start() == OS_SUCCESS);
 
      UtlString found;
      UtlString byDisplay("Microphone (C-Media USB Audio Device   )");
      UtlString byApi("Microphone (C-Media USB Audio D");
      UtlString expectedId("{cmedia}");
 
      CPPUNIT_ASSERT(watcher.findDeviceByName(byDisplay, found));
      CPPUNIT_ASSERT_EQUAL(expectedId, found);
 
      CPPUNIT_ASSERT(watcher.findDeviceByName(byApi, found));
      CPPUNIT_ASSERT_EQUAL(expectedId, found);
 
      UtlString absent("No Such Device");
      CPPUNIT_ASSERT(!watcher.findDeviceByName(absent, found));
      CPPUNIT_ASSERT(found.isNull());
 
      // Both names are retrievable by id.
      UtlString displayName;
      UtlString apiName;
      CPPUNIT_ASSERT(watcher.getDeviceInfo(expectedId, displayName, apiName));
      CPPUNIT_ASSERT_EQUAL(byDisplay, displayName);
      CPPUNIT_ASSERT_EQUAL(byApi, apiName);
   };


   void tearDown()
   {
      if (mpBufPool != NULL)
      {
         delete mpBufPool;
      }
      if (mpHeadersPool != NULL)
      {
         delete mpHeadersPool;
      }
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpInputDeviceDriverTest);

