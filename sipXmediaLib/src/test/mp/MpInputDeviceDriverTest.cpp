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

#define MIDDT_SAMPLES_PER_FRAME 80
#define MIDDT_NBUFS 20

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
   CPPUNIT_TEST(testGetEndpointDataFlow);
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

