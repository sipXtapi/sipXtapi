//
// Copyright (C) 2007-2013 SIPez LLC. All rights reserved.
//
// Copyright (C) 2007-2010 SIPfoundry Inc. All rights reserved.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

#ifdef HAVE_SPEEX // [
#  define USE_SPEEX_AEC
#endif // HAVE_SPEEX ]

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpInputDeviceManager.h>
#include <mp/MpOutputDeviceManager.h>
#include <mp/MpMisc.h>
#include <mp/MpResourceFactory.h>
#include <mp/MpResourceTopology.h>
#include <mp/MprFromInputDeviceConstructor.h>
#include <mp/MprToOutputDeviceConstructor.h>
#include <mp/MprToneGenConstructor.h>
#include <mp/MprFromFileConstructor.h>
#include <mp/MprNullConstructor.h>
#include <mp/MprHookConstructor.h>
#include <mp/MprBridgeConstructor.h>
#include <mp/MprRtpOutputConnectionConstructor.h>
#include <mp/MprEncodeConstructor.h>
#include <mp/MprRtpInputConnectionConstructor.h>
#include <mp/MprDecodeConstructor.h>
#include <mp/MprRecorderConstructor.h>
#include <mp/MprSplitterConstructor.h>
#include <mp/MprNullAecConstructor.h>
#include <mp/MprVadConstructor.h>
#include <mp/MprVoiceActivityNotifierConstructor.h>
#include <mp/MprDelayConstructor.h>
#include <mp/MprSpeakerSelectorConstructor.h>
#include <mp/MpMediaTask.h>
#include <mp/MpMMTimer.h>
#include "CpTopologyGraphFactoryImpl.h"
#include "mi/CpMediaInterfaceFactory.h"
#include "CpTopologyGraphInterface.h"
#include <os/OsSysLog.h>
#ifdef USE_SPEEX_AEC // [
#  include <mp/MprToOutputDeviceWithAecConstructor.h>
#endif // USE_SPEEX_AEC ]
#ifdef HAVE_SPEEX // [
#  include <mp/MprSpeexPreProcessConstructor.h>
#endif // HAVE_SPEEX ]

// REMOVE THIS when device enumerator/monitor is implemented
#define USE_DEVICE_ADD_HACK

#ifdef USE_DEVICE_ADD_HACK // [

#ifdef USE_TEST_INPUT_DRIVER // USE_TEST_DRIVER [
#  include <mp/MpSineWaveGeneratorDeviceDriver.h>
#  define INPUT_DRIVER MpSineWaveGeneratorDeviceDriver
#  define INPUT_DRIVER_DEFAULT_NAME "SineGenerator"
#  define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager, name) (name), (manager), 32000, 3000, 0

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#  include <mp/MpidWinMM.h>
#  define INPUT_DRIVER MpidWinMM
#  define INPUT_DRIVER_DEFAULT_NAME MpidWinMM::getDefaultDeviceName()
#  define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager, name) (name), (manager)

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#  ifdef __APPLE__
#     include <mp/MpidCoreAudio.h>
#     define INPUT_DRIVER MpidCoreAudio
#     define INPUT_DRIVER_DEFAULT_NAME "[default]"
#     define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager, name) (name), (manager)
#  elif defined(ANDROID)
#     include <mp/MpidAndroid.h>
#     define INPUT_DRIVER MpidAndroid
#     define INPUT_DRIVER_DEFAULT_NAME "default"
#     define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager, name) (MpidAndroid::AUDIO_SOURCE_DEFAULT), (manager)
#     define MP_LATE_DEVICE_ENABLE
#  else
#     include <mp/MpidOss.h>
#     define INPUT_DRIVER MpidOss
#     define INPUT_DRIVER_DEFAULT_NAME "/dev/dsp"
#     define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager, name) (name), (manager)
#  endif

#else // __pingtel_on_possix__ ]
#  error Unknown platform!
#endif

#ifdef USE_TEST_OUTPUT_DRIVER // USE_TEST_DRIVER [
#  include <mp/MpodBufferRecorder.h>
#  define OUTPUT_DRIVER MpodBufferRecorder
#  define OUTPUT_DRIVER_DEFAULT_NAME "default"
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS(name) (name), 60*1000*1000

#  include <os/OsFS.h> // for OpenAndWrite() to write captured data.

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#  include <mp/MpodWinMM.h>
#  define OUTPUT_DRIVER MpodWinMM
#  define OUTPUT_DRIVER_DEFAULT_NAME MpodWinMM::getDefaultDeviceName()
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS(name) (name)

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#  ifdef __APPLE__
#     include <mp/MpodCoreAudio.h>
#     define OUTPUT_DRIVER MpodCoreAudio
#     define OUTPUT_DRIVER_DEFAULT_NAME "[default]"
#     define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS(name) (name)
#  elif defined(ANDROID)
#     include <mp/MpodAndroid.h>
#     define OUTPUT_DRIVER MpodAndroid
#     define OUTPUT_DRIVER_DEFAULT_NAME "default"
#     define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS(name) (MpodAndroid::VOICE_CALL)
#  else
#     include <mp/MpodOss.h>
#     define OUTPUT_DRIVER MpodOss
#     define OUTPUT_DRIVER_DEFAULT_NAME "/dev/dsp"
#     define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS(name) (name)
#  endif

#else // __pingtel_on_posix__ ]
#  error Unknown platform!
#endif

#endif // USE_DEVICE_ADD_HACK ]

// EXTERNAL FUNCTIONS
extern void showWaveError(char *syscall, int e, int N, int line) ;  // dmaTaskWnt.cpp

// EXTERNAL VARIABLES
// CONSTANTS
// GLOBAL FUNCTION
// FORWARD DECLARATIONS
// STATIC VARIABLE INITIALIZATIONS
extern CpMediaInterfaceFactory* spFactory;
extern int siInstanceCount;

#ifdef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb,
                                                            uint32_t frameSizeMs, 
                                                            uint32_t maxSamplesPerSec,
                                                            uint32_t defaultDeviceSamplesPerSec,
                                                            UtlBoolean enableLocalAudio,
                                                            const UtlString &inputDeviceName,
                                                            const UtlString &outputDeviceName)
{
   if(spFactory == NULL)
   {
      spFactory = new CpMediaInterfaceFactory();
      spFactory->setFactoryImplementation(new CpTopologyGraphFactoryImpl(pConfigDb,
                                                                         frameSizeMs, 
                                                                         maxSamplesPerSec,
                                                                         defaultDeviceSamplesPerSec,
                                                                         enableLocalAudio,
                                                                         inputDeviceName,
                                                                         outputDeviceName));
   }    
   siInstanceCount++;
   return spFactory;
}
#endif


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpTopologyGraphFactoryImpl::CpTopologyGraphFactoryImpl(OsConfigDb* pConfigDb,
                                                       uint32_t frameSizeMs, 
                                                       uint32_t maxSamplesPerSec,
                                                       uint32_t defaultSamplesPerSec,
                                                       UtlBoolean enableLocalAudio,
                                                       const UtlString &inputDeviceName,
                                                       const UtlString &outputDeviceName)
: sipXmediaFactoryImpl(pConfigDb, frameSizeMs, maxSamplesPerSec,
                       defaultSamplesPerSec, enableLocalAudio,
                       inputDeviceName, outputDeviceName)
, mpInitialResourceTopology(NULL)
, mpResourceFactory(NULL)
, mpConnectionResourceTopology(NULL)
, mpMcastConnectionResourceTopology(NULL)
, mIsLocalAudioEnabled(enableLocalAudio)
, mpInputDeviceManager(NULL)
, mpOutputDeviceManager(NULL)
, mpMediaTaskTicker(NULL)
, mDefaultToOutputDevice(MP_INVALID_OUTPUT_DEVICE_HANDLE)
, mDefaultToInputDevice(MP_INVALID_INPUT_DEVICE_HANDLE)
, mNumMcastStreams(3)
{
    assert(MpMisc.RawAudioPool);
#ifdef ANDROID
    int numBufferedFrames = 16;
#else
    int numBufferedFrames = 3;
#endif
    uint32_t mgrSamplesPerFrame = (mFrameSizeMs*mDefaultSamplesPerSec)/1000;
    mpInputDeviceManager = 
        new MpInputDeviceManager(mgrSamplesPerFrame,    // samples per frame
                                 mDefaultSamplesPerSec, // samples per second
                                 numBufferedFrames,    // number of buffered frames
                                 *MpMisc.RawAudioPool);

    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphFactoryImpl::CpTopologyGraphFactoryImpl MpInputDeviceManager constructed");
    mpOutputDeviceManager =
        new MpOutputDeviceManager(mgrSamplesPerFrame,    // samples per frame
                                  mDefaultSamplesPerSec, // samples per second
#ifdef ANDROID // [
                                  (100 > mFrameSizeMs*3) ? 100 : mFrameSizeMs*5 // mixer buffer length (ms)
#else // ANDROID ][
                                  // On Windows (especially on Vista)
                                  // audio is consumed by 50ms chunks, causing
                                  // bursts of ticker notifications. So we
                                  // should set mixer size big enough to
                                  // iron out this jitter. In case of a good
                                  // OS/sound card combination this does not
                                  // hurt, because by default only minimal part
                                  // of mixer buffer is used which increases
                                  // in case of bursts.
                                  (60 > mFrameSizeMs*3) ? 60 : mFrameSizeMs*3 // mixer buffer length (ms)
#endif // !ANDROID ]
                                  );

    // Get media task ticker notification
    OsNotification *pTickerNotf = MpMediaTask::getMediaTask()->getTickerNotification();

    if (mIsLocalAudioEnabled)
    {
       OsSysLog::add(FAC_CP, PRI_INFO, "CpTopologyGraphFactoryImpl: enabling local audio");
#ifdef USE_DEVICE_ADD_HACK // [
       // Create source (input) device and add it to manager.
       UtlString tmpInputDeviceName = inputDeviceName.isNull()?INPUT_DRIVER_DEFAULT_NAME:
                                                               inputDeviceName;
       INPUT_DRIVER *sourceDevice =
          new INPUT_DRIVER(INPUT_DRIVER_CONSTRUCTOR_PARAMS(*mpInputDeviceManager,
                                                           tmpInputDeviceName));

       MpInputDeviceHandle sourceDeviceId = mpInputDeviceManager->addDevice(*sourceDevice);
       OsSysLog::add(FAC_CP, (sourceDeviceId > 0) ? PRI_INFO : PRI_ERR, "CpTopologyGraphFactoryImpl::CpTopologyGraphFactoryImpl mpInputDeviceManager->addDevice returned: %d", sourceDeviceId);
       assert(sourceDeviceId > 0);

       // Create sink (output) device and add it to manager.
       UtlString tmpOutputDeviceName = outputDeviceName.isNull()?OUTPUT_DRIVER_DEFAULT_NAME:
                                                                 outputDeviceName;
       OUTPUT_DRIVER *sinkDevice =
          new OUTPUT_DRIVER(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS(tmpOutputDeviceName));
       MpOutputDeviceHandle sinkDeviceId = mpOutputDeviceManager->addDevice(sinkDevice);
       OsSysLog::add(FAC_CP, (sinkDeviceId > 0) ? PRI_INFO : PRI_ERR, "CpTopologyGraphFactoryImpl::CpTopologyGraphFactoryImpl mpOutputDeviceManager->addDevice returned: %d", sourceDeviceId);
       assert(sinkDeviceId > 0);

       OsStatus tempRes;

       // Enable devices
#ifndef MP_LATE_DEVICE_ENABLE // [
       tempRes = mpInputDeviceManager->enableDevice(sourceDeviceId);
       mDefaultToInputDevice = sourceDeviceId;
       if (tempRes != OS_SUCCESS)
       {
          OsSysLog::add(FAC_CP, PRI_ERR, "CpTopologyGraphFactoryImpl(): "
                        "Can't enable input audio device \"%s\"",
                        sourceDevice->getDeviceName().data());
          mpInputDeviceManager->removeDevice(sourceDeviceId);
          delete sourceDevice;
       }
#else
       OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphFactoryImpl::CpTopologyGraphFactoryImpl delaying enabling of input device");
#endif // MP_LATE_DEVICE_ENABLE ]

       tempRes = mpOutputDeviceManager->enableDevice(sinkDeviceId);
       mDefaultToOutputDevice = sinkDeviceId;
       if (tempRes == OS_SUCCESS)
       {
          // Use our output device as a source of media task ticks.
          tempRes = mpOutputDeviceManager->setFlowgraphTickerSource(sinkDeviceId,
                                                                    pTickerNotf);
          if(tempRes != OS_SUCCESS)
          {
             OsSysLog::add(FAC_CP, PRI_ERR, "CpTopologyGraphFactoryImpl::CpTopologyGraphFactoryImpl mpOutputDeviceManager->setFlowgraphTickerSource returned: %d", tempRes);
          }
          assert(tempRes == OS_SUCCESS);
       }
       else
       {
          OsSysLog::add(FAC_CP, PRI_ERR, "CpTopologyGraphFactoryImpl(): "
                        "Can't enable output audio device \"%s\"",
                        sinkDevice->getDeviceName().data());
          mpOutputDeviceManager->removeDevice(sinkDeviceId);
          delete sinkDevice;

          // Use multimedia timer as a source of media task ticks.
          mpMediaTaskTicker = MpMMTimer::create(MpMMTimer::Notification);
          mpMediaTaskTicker->setNotification(pTickerNotf);
          mpMediaTaskTicker->run(mFrameSizeMs*1000);
       }
#else // USE_DEVICE_ADD_HACK ][
       assert(!"Can't use local audio without USE_DEVICE_ADD_HACK defined!");
#endif // USE_DEVICE_ADD_HACK ]
    }
    else
    {
       // Use multimedia timer as a source of media task ticks.
       OsSysLog::add(FAC_CP, PRI_INFO, "CpTopologyGraphFactoryImpl: disabling local audio");
       mpMediaTaskTicker = MpMMTimer::create(MpMMTimer::Notification);
       mpMediaTaskTicker->setNotification(pTickerNotf);
       mpMediaTaskTicker->run(mFrameSizeMs*1000);
    }

    mpInitialResourceTopology = buildDefaultInitialResourceTopology();
    if (mIsLocalAudioEnabled)
    {
       // Add one local connection to initial topology by default.
       addLocalConnectionTopology(mpInitialResourceTopology);
    }
    mpResourceFactory = buildDefaultResourceFactory();

    int firstInvalidResourceIndex;
    OsStatus result = 
        mpInitialResourceTopology->validateResourceTypes(*mpResourceFactory, 
                                                         firstInvalidResourceIndex);
    if(result != OS_SUCCESS)
    {
       OsSysLog::add(FAC_CP, PRI_ERR, "CpTopologyGraphFactoryImpl: validateResourceTypes returned error: %d", result);
    }
    assert(result == OS_SUCCESS);
    assert(firstInvalidResourceIndex == -1);

    mpConnectionResourceTopology = buildUnicastConnectionResourceTopology();
    mpMcastConnectionResourceTopology = buildMulticastConnectionResourceTopology();
}


// Destructor
CpTopologyGraphFactoryImpl::~CpTopologyGraphFactoryImpl()
{
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpTopologyGraphFactoryImpl::~CpTopologyGraphFactoryImpl");

   if (mIsLocalAudioEnabled)
   {
#ifdef USE_DEVICE_ADD_HACK // [
      OsStatus result;

      // Clear flowgraph ticker
      result = mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                               NULL);
      assert(result == OS_SUCCESS);

      // Wait for last frame processing cycle call to complete.
      OsTask::delay(30);

      // Disable devices
      result = mpInputDeviceManager->disableAllDevicesExcept();
//      assert(result == OS_SUCCESS);
      result = mpOutputDeviceManager->disableAllDevicesExcept();
//      assert(result == OS_SUCCESS);

      // Free input device driver
      int inDevicesRemoved = mpInputDeviceManager->removeAllDevices();

      // Free output device driver
      int outDevicesRemoved = mpOutputDeviceManager->removeAllDevices();

      if(outDevicesRemoved)
      {
      }
      else
      {
         // Stop our ticker timer
         mpMediaTaskTicker->stop();
         delete mpMediaTaskTicker;
         mpMediaTaskTicker = NULL;
      }
#else // USE_DEVICE_ADD_HACK ][
      assert(!"Can't use local audio without USE_DEVICE_ADD_HACK defined!");
#endif // USE_DEVICE_ADD_HACK ]
   }
   else
   {
      // Stop our ticker timer
      mpMediaTaskTicker->stop();
      delete mpMediaTaskTicker;
      mpMediaTaskTicker = NULL;
   }

   // Free factory and topologies.
   delete mpResourceFactory;
   mpResourceFactory = NULL;
   delete mpInitialResourceTopology;
   mpInitialResourceTopology = NULL;
   delete mpConnectionResourceTopology;
   mpConnectionResourceTopology = NULL;
   delete mpMcastConnectionResourceTopology;
   mpMcastConnectionResourceTopology = NULL;

   // Free input and output device managers.
   delete mpInputDeviceManager;
   mpInputDeviceManager = NULL;
   delete mpOutputDeviceManager;
   mpOutputDeviceManager = NULL;
}

/* ============================ MANIPULATORS ============================== */

CpMediaInterface* 
CpTopologyGraphFactoryImpl::createMediaInterface(const char* publicAddress,
                                                 const char* localAddress,
                                                 int numCodecs,
                                                 SdpCodec* sdpCodecArray[],
                                                 const char* locale,
                                                 int expeditedIpTos,
                                                 const char* stunServer,
                                                 int stunPort,
                                                 int stunKeepAliveSecs,
                                                 const char* turnServer,
                                                 int turnPort,
                                                 const char* turnUserName,
                                                 const char* turnPassword,
                                                 int turnKeepAliveSecs,
                                                 UtlBoolean enableIce, 
                                                 uint32_t samplesPerSec,
                                                 OsMsgDispatcher* pDispatcher)
{
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpTopologyGraphFactoryImpl::createMediaInterface");
#ifdef MP_LATE_DEVICE_ENABLE // [
    if (mpInputDeviceManager->enableDevice(1) != OS_SUCCESS)
    {
       return NULL;
    }
#endif // MP_LATE_DEVICE_ENABLE ]

    // if the sample rate passed in is zero, use the default.
    samplesPerSec = (samplesPerSec == 0) ? mDefaultSamplesPerSec : samplesPerSec;
    CpTopologyGraphInterface *pIf =
          new CpTopologyGraphInterface(this,
                                       (mFrameSizeMs*samplesPerSec)/1000, samplesPerSec, 
                                       publicAddress, localAddress, 
                                       numCodecs, sdpCodecArray, 
                                       locale, 
                                       expeditedIpTos, 
                                       stunServer, stunPort, stunKeepAliveSecs, 
                                       turnServer, turnPort, turnUserName, 
                                       turnPassword, turnKeepAliveSecs, 
                                       enableIce,
                                       pDispatcher,
                                       mpInputDeviceManager,
                                       1,
                                       TRUE,
                                       mpOutputDeviceManager,
                                       1);

    if(mConfiguredIpAddress.length() > 0)
    {
        pIf->setConfiguredIpAddress(mConfiguredIpAddress);
    }

    pIf->setValue(miInterfaceId);
    miInterfaceId++;

    // Set the default input device in the MprToInputDevice resource
    MprFromInputDevice::setDeviceId(DEFAULT_FROM_INPUT_DEVICE_RESOURCE_NAME, *(pIf->getMsgQ()), mDefaultToInputDevice);

    // Set the default output device in the MprToOutputDevice resource
    MprToOutputDevice::setDeviceId(DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME, *(pIf->getMsgQ()), mDefaultToOutputDevice);

    return(pIf);
}

OsStatus CpTopologyGraphFactoryImpl::setSpeakerDevice(const UtlString& device) 
{
    // See if the device already is setup
    MpOutputDeviceHandle deviceId = MP_INVALID_OUTPUT_DEVICE_HANDLE;
    OsStatus status = mpOutputDeviceManager->getDeviceId(device, deviceId);

    OsSysLog::add(FAC_CP, PRI_DEBUG,
        "CpTopologyGraphFactoryImpl::setSpeakerDevice(%s) found id: %d getDeviceId returned: %d", 
        device.data(), deviceId, status);


    // This interface assumes only one device enabled at a time.  Need addition arguments or another
    // interface to allow multiple devices enabled at once.  The MpOutputDeviceManager supports multiple
    // simultainious output devices.

    // Disable all existing output devices, except the given device
    mpOutputDeviceManager->disableAllDevicesExcept(deviceId > MP_INVALID_OUTPUT_DEVICE_HANDLE ? 1 : 0, &deviceId);

    // If the device does not already exist, add it
    if(deviceId <= MP_INVALID_OUTPUT_DEVICE_HANDLE)
    {
        OUTPUT_DRIVER* newDevice =
          new OUTPUT_DRIVER(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS(device));
        deviceId = mpOutputDeviceManager->addDevice(newDevice);
        OsSysLog::add(FAC_CP, PRI_DEBUG,
            "CpTopologyGraphFactoryImpl::setSpeakerDevice addDevice(%s) id: %d",
            device.data(), deviceId);

        if(deviceId > MP_INVALID_OUTPUT_DEVICE_HANDLE)
        {
            status = OS_SUCCESS;
        }

    }

    // Enable the device if it was not already
    if(deviceId > MP_INVALID_OUTPUT_DEVICE_HANDLE &&
       !mpOutputDeviceManager->isDeviceEnabled(deviceId))
    {
        status = mpOutputDeviceManager->enableDevice(deviceId);
        OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpTopologyGraphFactoryImpl::setSpeakerDevice enableDevice id: %d (%s) returned: %d",
            deviceId, device.data(), status);

        // The device is the clock for the media task frameprocessing.
        OsNotification* processFrameNotifier = MpMediaTask::getMediaTask()->getTickerNotification();
        status = mpOutputDeviceManager->setFlowgraphTickerSource(deviceId,
                                                                 processFrameNotifier);
        
    }
    else
    {
        // TODO: ???
        // If the device is already enabled, should we set the process frame notifier anyway??
    }

    // Set the default device ID to use for the local output path in the flowgraph
    mDefaultToOutputDevice = deviceId;

    return(status);
}

OsStatus CpTopologyGraphFactoryImpl::setMicrophoneDevice(const UtlString& device) 
{
    // See if the device already is setup
    MpInputDeviceHandle deviceId = MP_INVALID_INPUT_DEVICE_HANDLE;
    OsStatus status = mpInputDeviceManager->getDeviceId(device, deviceId);

    OsSysLog::add(FAC_CP, PRI_DEBUG,
        "CpTopologyGraphFactoryImpl::setMicrophoneDevice(%s) found id: %d getDeviceId returned: %d", 
        device.data(), deviceId, status);


    // This interface assumes only one device enabled at a time.  Need addition arguments or another
    // interface to allow multiple devices enabled at once.  The MpInputDeviceManager supports multiple
    // simultainious output devices.

    // Disable all existing input devices, except the given device
    mpInputDeviceManager->disableAllDevicesExcept(deviceId > MP_INVALID_INPUT_DEVICE_HANDLE ? 1 : 0, &deviceId);

    // If the device does not already exist, add it
    if(deviceId <= MP_INVALID_INPUT_DEVICE_HANDLE)
    {
        INPUT_DRIVER* newDevice =
          new INPUT_DRIVER(INPUT_DRIVER_CONSTRUCTOR_PARAMS(*mpInputDeviceManager,
                                                           device));
        deviceId = mpInputDeviceManager->addDevice(*newDevice);
        OsSysLog::add(FAC_CP, PRI_DEBUG,
            "CpTopologyGraphFactoryImpl::setMicrophoneDevice addDevice(%s) id: %d",
            device.data(), deviceId);

        if(deviceId > MP_INVALID_INPUT_DEVICE_HANDLE)
        {
            status = OS_SUCCESS;
        }
    }

    // Enable the device if it was not already
    if(deviceId > MP_INVALID_INPUT_DEVICE_HANDLE &&
       !mpInputDeviceManager->isDeviceEnabled(deviceId))
    {
        status = mpInputDeviceManager->enableDevice(deviceId);
        OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "CpTopologyGraphFactoryImpl::setMicrophoneDevice enableDevice id: %d (%s) returned: %d",
            deviceId, device.data(), status);
    }

    // Set the default device ID to use for the local output path in the flowgraph
    mDefaultToInputDevice = deviceId;

    return(status);
}

OsStatus CpTopologyGraphFactoryImpl::setAudioAECMode(const MEDIA_AEC_MODE mode)
{
#ifdef USE_SPEEX_AEC // [
   switch (mode) {
   case MEDIA_AEC_CANCEL_AUTO:
      MprSpeexEchoCancel::setGlobalEnableState(MprSpeexEchoCancel::LOCAL_MODE);
      break;

   case MEDIA_AEC_CANCEL:
   case MEDIA_AEC_SUPPRESS:
      MprSpeexEchoCancel::setGlobalEnableState(MprSpeexEchoCancel::GLOBAL_ENABLE);
      break;

   case MEDIA_AEC_DISABLED:
      MprSpeexEchoCancel::setGlobalEnableState(MprSpeexEchoCancel::GLOBAL_DISABLE);
      break;
   }

   return OS_SUCCESS;

#else // USE_SPEEX_AEC ][
   return OS_NOT_SUPPORTED;
#endif // !USE_SPEEX_AEC ]
}


OsStatus CpTopologyGraphFactoryImpl::getAudioAECMode(MEDIA_AEC_MODE& mode) const
{
   OsStatus status = OS_SUCCESS;
#ifdef USE_SPEEX_AEC // [

   MprSpeexEchoCancel::GlobalEnableState state = MprSpeexEchoCancel::getGlobalEnableState();
   switch (state) 
   {
       case MprSpeexEchoCancel::LOCAL_MODE:
          mode = MEDIA_AEC_CANCEL_AUTO;
       break;
       case MprSpeexEchoCancel::GLOBAL_ENABLE:
          mode = MEDIA_AEC_CANCEL;
       break;
       case MprSpeexEchoCancel::GLOBAL_DISABLE:
          mode = MEDIA_AEC_DISABLED;
       break;
       default:
          OsSysLog::add(FAC_CP, PRI_ERR, 
                        "CpTopologyGraphFactoryImpl::setAudioAECMode MprSpeexEchoCancel::getGlobalEnableState returned unknown state: %d", 
                        state);
          status = OS_INVALID_STATE;
       break;
   }

#else // USE_SPEEX_AEC ][
   status = OS_NOT_SUPPORTED;
#endif // !USE_SPEEX_AEC ]
   return(status);
}

OsStatus CpTopologyGraphFactoryImpl::enableAGC(UtlBoolean bEnable) 
{
#ifdef HAVE_SPEEX // [
   MprSpeexPreprocess::setGlobalAgcEnableState(bEnable?MprSpeexPreprocess::GLOBAL_ENABLE
                                                      :MprSpeexPreprocess::GLOBAL_DISABLE);
   return OS_SUCCESS;
#else // HAVE_SPEEX ][
   return OS_NOT_SUPPORTED; 
#endif // !HAVE_SPEEX ]
}

OsStatus CpTopologyGraphFactoryImpl::setAudioNoiseReductionMode(const MEDIA_NOISE_REDUCTION_MODE mode) 
{
#ifdef HAVE_SPEEX // [
   switch (mode)
   {
   case MEDIA_NOISE_REDUCTION_LOW:
   case MEDIA_NOISE_REDUCTION_MEDIUM:
   case MEDIA_NOISE_REDUCTION_HIGH:
      MprSpeexPreprocess::setGlobalNoiseReductionEnableState(MprSpeexPreprocess::GLOBAL_ENABLE);
      break;

   case MEDIA_NOISE_REDUCTION_DISABLED:
      MprSpeexPreprocess::setGlobalNoiseReductionEnableState(MprSpeexPreprocess::GLOBAL_DISABLE);
      break;
   }
   return OS_SUCCESS;
#else // HAVE_SPEEX ][
   return OS_NOT_SUPPORTED; 
#endif // !HAVE_SPEEX ]
}

MpResourceFactory* CpTopologyGraphFactoryImpl::buildDefaultResourceFactory()
{
    MpResourceFactory* resourceFactory = new MpResourceFactory();

    // Input device
    resourceFactory->addConstructor(*(new MprFromInputDeviceConstructor(mpInputDeviceManager)));

    // Output device
    resourceFactory->addConstructor(*(new MprToOutputDeviceConstructor(mpOutputDeviceManager)));

#ifdef USE_SPEEX_AEC // [
    // Output device with AEC
    resourceFactory->addConstructor(*(new MprToOutputDeviceWithAecConstructor(mpOutputDeviceManager)));
#endif // USE_SPEEX_AEC ]

#ifdef HAVE_SPEEX // [
    // Speex preprocessor
    resourceFactory->addConstructor(*(new MprSpeexPreProcessConstructor(FALSE, FALSE)));
#endif // HAVE_SPEEX ]

    // ToneGen
    resourceFactory->addConstructor(*(new MprToneGenConstructor()));

    // FromFile
    resourceFactory->addConstructor(*(new MprFromFileConstructor()));

    // Null resource
    resourceFactory->addConstructor(*(new MprNullConstructor()));

    // Hook resource
    resourceFactory->addConstructor(*(new MprHookConstructor()));

    // Bridge
    resourceFactory->addConstructor(*(new MprBridgeConstructor()));

    // Speaker Selector
    resourceFactory->addConstructor(*(new MprSpeakerSelectorConstructor()));

    // Output RTP connection
    resourceFactory->addConstructor(*(new MprRtpOutputConnectionConstructor()));

    // Encoder
    resourceFactory->addConstructor(*(new MprEncodeConstructor()));

    // Input RTP connection
    resourceFactory->addConstructor(*(new MprRtpInputConnectionConstructor(FALSE)));

    // Input RTP connection (multicast version)
    resourceFactory->addConstructor(*(new MprRtpInputConnectionConstructor(TRUE, mNumMcastStreams)));

    // Decoder
    resourceFactory->addConstructor(*(new MprDecodeConstructor()));

    // Recorder
    resourceFactory->addConstructor(*(new MprRecorderConstructor()));

    // Splitter
    resourceFactory->addConstructor(*(new MprSplitterConstructor()));

    // NULL AEC
    resourceFactory->addConstructor(*(new MprNullAecConstructor()));

    // VAD
    resourceFactory->addConstructor(*(new MprVadConstructor()));

    // Voice Activity Notifier
    resourceFactory->addConstructor(*(new MprVoiceActivityNotifierConstructor()));

    // Delay resource
    resourceFactory->addConstructor(*(new MprDelayConstructor()));

    return(resourceFactory);
}

/// Resource list for Initial Topology.
static MpResourceTopology::ResourceDef initialTopologyResources[] =
{
   {DEFAULT_BRIDGE_RESOURCE_TYPE, DEFAULT_BRIDGE_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
#ifdef INSERT_SPEAKER_SELECTOR // [
   {DEFAULT_SPEAKER_SELECTOR_RESOURCE_TYPE, DEFAULT_SPEAKER_SELECTOR_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
#endif // INSERT_SPEAKER_SELECTOR ]
   {DEFAULT_FROM_FILE_RESOURCE_TYPE, DEFAULT_FROM_FILE_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
   {DEFAULT_TONE_GEN_RESOURCE_TYPE, DEFAULT_TONE_GEN_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
   {DEFAULT_RECORDER_RESOURCE_TYPE, DEFAULT_RECORDER_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
   {DEFAULT_NULL_RESOURCE_TYPE, DEFAULT_NULL_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1}
};
static const int initialTopologyResourcesNum =
   sizeof(initialTopologyResources)/sizeof(MpResourceTopology::ResourceDef);
static const int initialTopologyInputResourcesNum = 2;

/// Connection list for Initial Topology.
static MpResourceTopology::ConnectionDef initialTopologyConnections[] =
{
    // FromFile -> Bridge(0)
   {DEFAULT_FROM_FILE_RESOURCE_NAME, 0, DEFAULT_BRIDGE_RESOURCE_NAME, 0},
    // ToneGen -> Bridge(1)
   {DEFAULT_TONE_GEN_RESOURCE_NAME, 0, DEFAULT_BRIDGE_RESOURCE_NAME, 1},

    // If INSERT_SPEAKER_SELECTOR is defined, a Speaker Selector will be
    // manually connected to the Bridge.

    // Bridge(0) -> Buffer Recorder
    // This buffer recorder records full call to a buffer.
   {DEFAULT_BRIDGE_RESOURCE_NAME, 0, DEFAULT_RECORDER_RESOURCE_NAME, 0},
    // Bridge(1) -> Null(0)
    // This file recorder records full call to a file.
   {DEFAULT_BRIDGE_RESOURCE_NAME, 1, DEFAULT_NULL_RESOURCE_NAME, 0},
};
static const int initialTopologyConnectionsNum =
   sizeof(initialTopologyConnections)/sizeof(MpResourceTopology::ConnectionDef);

/// Resource list for Local Connection Topology.
static MpResourceTopology::ResourceDef localConnectionResources[] =
{
   {DEFAULT_FROM_INPUT_DEVICE_RESOURCE_TYPE, DEFAULT_FROM_INPUT_DEVICE_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
   {DEFAULT_VAD_RESOURCE_TYPE, DEFAULT_VAD_RESOURCE_NAME MIC_NAME_SUFFIX, MP_INVALID_CONNECTION_ID, -1},
   {DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_TYPE, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX, MP_INVALID_CONNECTION_ID, -1},
#ifdef INSERT_DELAY_RESOURCE // [
   {DEFAULT_DELAY_RESOURCE_TYPE, DEFAULT_DELAY_RESOURCE_NAME MIC_NAME_SUFFIX, MP_INVALID_CONNECTION_ID, -1},
#endif // INSERT_DELAY_RESOURCE ]
   {DEFAULT_VAD_RESOURCE_TYPE, DEFAULT_VAD_RESOURCE_NAME SPEAKER_NAME_SUFFIX, MP_INVALID_CONNECTION_ID, -1},
   {DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_TYPE, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX, MP_INVALID_CONNECTION_ID, -1},
#ifdef USE_SPEEX_AEC // [
   {DEFAULT_TO_OUTPUT_DEVICE_WITH_AEC_RESOURCE_TYPE, DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
#else // USE_SPEEX_AEC ][
   {DEFAULT_SPLITTER_RESOURCE_TYPE, DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
   {DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_TYPE, DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
   {DEFAULT_NULL_AEC_RESOURCE_TYPE, DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME AEC_NAME_SUFFIX, MP_INVALID_CONNECTION_ID, -1},
#endif // !USE_SPEEX_AEC ]
#ifdef HAVE_SPEEX // [
   {DEFAULT_SPEEX_PREPROCESS_RESOURCE_TYPE, DEFAULT_SPEEX_PREPROCESS_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1}
#endif // HAVE_SPEEX ]
};
static const int localConnectionResourcesNum =
   sizeof(localConnectionResources)/sizeof(MpResourceTopology::ResourceDef);

/// Connection list for Local Connection Topology.
static MpResourceTopology::ConnectionDef localConnectionConnections[] =
{
    // Mic -> VAD
   {DEFAULT_FROM_INPUT_DEVICE_RESOURCE_NAME, 0, DEFAULT_VAD_RESOURCE_NAME MIC_NAME_SUFFIX, 0},
    //     -> Voice Activity Notifier
   {NULL, 0, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME MIC_NAME_SUFFIX, 0},
    //     -> AEC
   {NULL, 0, DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME AEC_NAME_SUFFIX, 0},
#ifdef HAVE_SPEEX // [
   //     -> Speex preprocessor
   {NULL, 0, DEFAULT_SPEEX_PREPROCESS_RESOURCE_NAME, 0},
#endif // HAVE_SPEEX ]
#ifdef INSERT_DELAY_RESOURCE // [
    //     -> Delay
   {NULL, 0, DEFAULT_DELAY_RESOURCE_NAME MIC_NAME_SUFFIX, 0},
#endif // INSERT_DELAY_RESOURCE ]
    //     -> Connection Port(-1)
   {NULL, 0, VIRTUAL_NAME_CONNECTION_PORTS, -1},

#ifdef USE_SPEEX_AEC // [
    // Connection Port(-1) -> VAD
   {VIRTUAL_NAME_CONNECTION_PORTS, -1, DEFAULT_VAD_RESOURCE_NAME SPEAKER_NAME_SUFFIX, 0},
#else // USE_SPEEX_AEC ][
    // Connection Port(-1) -> Splitter
    // The splitter leaves a tap for AEC to see the output to speaker
   {VIRTUAL_NAME_CONNECTION_PORTS, -1, DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, 0},
    // Splitter(1) -> Output Buffer (part of AEC)
   {DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, 1, DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME AEC_NAME_SUFFIX AEC_OUTPUT_BUFFER_RESOURCE_NAME_SUFFIX, 0},
    // Splitter(0) -> VAD
   {DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, 0, DEFAULT_VAD_RESOURCE_NAME SPEAKER_NAME_SUFFIX, 0},
#endif // !USE_SPEEX_AEC ]
    //             -> Voice Activity Notifier
   {NULL, 0, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX, 0},
    //             -> Speaker
   {NULL, 0, DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME, 0}
};
static const int localConnectionConnectionsNum =
   sizeof(localConnectionConnections)/sizeof(MpResourceTopology::ConnectionDef);

/// Name of last resource in the local input connection
static const char *sgLocalInputConnectionOutput =
#ifdef INSERT_DELAY_RESOURCE // [
                                        DEFAULT_DELAY_RESOURCE_NAME MIC_NAME_SUFFIX;
#elif defined(HAVE_SPEEX) // [
                                        DEFAULT_SPEEX_PREPROCESS_RESOURCE_NAME;
#else // INSERT_DELAY_RESOURCE ][
                                        DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME AEC_NAME_SUFFIX;
#endif
/// Name of first resource in the local output connection
static const char *sgLocalOutputConnectionInput =
#ifdef USE_SPEEX_AEC // [
                                        DEFAULT_VAD_RESOURCE_NAME SPEAKER_NAME_SUFFIX;
#else // USE_SPEEX_AEC ][
                                        DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME;
#endif



MpResourceTopology* CpTopologyGraphFactoryImpl::buildDefaultInitialResourceTopology()
{
    MpResourceTopology* resourceTopology = new MpResourceTopology();
    OsStatus result;

    // Add resources to initial topology
    result = resourceTopology->addResources(initialTopologyResources,
                                            initialTopologyResourcesNum);
    assert(result == OS_SUCCESS);

#ifndef INSERT_SPEAKER_SELECTOR // [
    // Map Bridge inputs to ConnectionPorts
    result = resourceTopology->addVirtualInput(DEFAULT_BRIDGE_RESOURCE_NAME, -1,
                                               VIRTUAL_NAME_CONNECTION_PORTS, -1);
    assert(result == OS_SUCCESS);
#else // !INSERT_SPEAKER_SELECTOR ][
    // Map Speaker Selector inputs to ConnectionPorts
    result = resourceTopology->addVirtualInput(DEFAULT_SPEAKER_SELECTOR_RESOURCE_NAME, -1,
                                               VIRTUAL_NAME_CONNECTION_PORTS, -1);
    assert(result == OS_SUCCESS);
#endif // INSERT_SPEAKER_SELECTOR ]

    // Map Bridge outputs to ConnectionPorts
    result = resourceTopology->addVirtualOutput(DEFAULT_BRIDGE_RESOURCE_NAME, -1,
                                                VIRTUAL_NAME_CONNECTION_PORTS, -1);
    assert(result == OS_SUCCESS);

    // Add connections to initial topology
    result = resourceTopology->addConnections(initialTopologyConnections,
                                              initialTopologyConnectionsNum);
    assert(result == OS_SUCCESS);

#ifdef INSERT_SPEAKER_SELECTOR // [
    // Add connections between Speaker Selector and Bridge
    for (int i=0; i<DEFAULT_SPEAKER_SELECTOR_MAX_OUTPUTS; i++)
    {
      result = resourceTopology->addConnection(DEFAULT_SPEAKER_SELECTOR_RESOURCE_NAME, i,
                                               DEFAULT_BRIDGE_RESOURCE_NAME, i+initialTopologyInputResourcesNum);
      assert(result == OS_SUCCESS);
    }
#endif // INSERT_SPEAKER_SELECTOR ]

    // Validate the topology to make sure all the resources are connected
    // and that there are no dangling resources
    UtlString firstUnconnectedResourceName;
    UtlString firstDanglingResourceName;
    result = resourceTopology->validateConnections(firstUnconnectedResourceName,
                    firstDanglingResourceName, 
                    FALSE); // disallow references to resources not in this topology
    assert(result == OS_SUCCESS);
    assert(firstUnconnectedResourceName.isNull());
    assert(firstDanglingResourceName.isNull());

    return(resourceTopology);
}

MpResourceTopology* CpTopologyGraphFactoryImpl::buildUnicastConnectionResourceTopology()
{
    MpResourceTopology* resourceTopology = new MpResourceTopology();

    OsStatus result;
    result = resourceTopology->addResource(DEFAULT_RTP_INPUT_RESOURCE_TYPE,
                                           DEFAULT_RTP_INPUT_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    // Add decoder resource
    UtlString decodeName = DEFAULT_DECODE_RESOURCE_NAME;
    decodeName.append("-0");
    result = resourceTopology->addResource(DEFAULT_DECODE_RESOURCE_TYPE,
                                           decodeName,
                                           MP_INVALID_CONNECTION_ID,
                                           0);
    assert(result == OS_SUCCESS);

#ifdef INSERT_DELAY_RESOURCE // [
    // Add delay resource
    UtlString delayName = DEFAULT_DELAY_RESOURCE_NAME CONNECTION_NAME_SUFFIX "-0";
    result = resourceTopology->addResource(DEFAULT_DELAY_RESOURCE_TYPE,
                                           delayName,
                                           MP_INVALID_CONNECTION_ID,
                                           0);
    assert(result == OS_SUCCESS);
#endif // INSERT_DELAY_RESOURCE ]

    // Add Voice Activity Notifier resource
    UtlString activityNotifName = DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME
                                  CONNECTION_NAME_SUFFIX "-0";
    result = resourceTopology->addResource(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_TYPE,
                                           activityNotifName,
                                           MP_INVALID_CONNECTION_ID,
                                           0);
    assert(result == OS_SUCCESS);

    // Abstract port number to be used when connecting to bridge.
#ifdef INSERT_SPEAKER_SELECTOR // [
    int logicalPortNum = -1;
#else // INSERT_SPEAKER_SELECTOR ][
    int logicalPortNum = resourceTopology->getNextLogicalPortNumber();
#endif // !INSERT_SPEAKER_SELECTOR ]

    // Link RTP input -> decoder
    result = resourceTopology->addConnection(DEFAULT_RTP_INPUT_RESOURCE_NAME, 0, 
                                             decodeName, 0);
    assert(result == OS_SUCCESS);
    UtlString &prevResourceName = decodeName;

    // -> Input connection Voice Activity Notifier
    result = resourceTopology->addConnection(prevResourceName, 0, 
                                             activityNotifName, 0);
    assert(result == OS_SUCCESS);
    prevResourceName = activityNotifName;

#ifdef INSERT_DELAY_RESOURCE // [
    // -> Delay
    result = resourceTopology->addConnection(prevResourceName, 0, 
                                             delayName, 0);
    assert(result == OS_SUCCESS);
    prevResourceName = delayName;
#endif // INSERT_DELAY_RESOURCE ]

    // Mark last resource as RTP Stream Output
    UtlString streamOutputName(VIRTUAL_NAME_RTP_STREAM_OUTPUT "-0");
    result = resourceTopology->addVirtualOutput(prevResourceName, 0,
                                                streamOutputName, 0);
    assert(result == OS_SUCCESS);

    // -> bridge
    result = resourceTopology->addConnection(prevResourceName, 0, 
                                             VIRTUAL_NAME_CONNECTION_PORTS, logicalPortNum);
    assert(result == OS_SUCCESS);

    addOutputConnectionTopology(resourceTopology, logicalPortNum);

    return(resourceTopology);
}

MpResourceTopology* CpTopologyGraphFactoryImpl::buildMulticastConnectionResourceTopology()
{
    MpResourceTopology* resourceTopology = new MpResourceTopology();
    OsStatus result;

    // Add multicast RTP input
    result = resourceTopology->addResource(DEFAULT_MCAST_RTP_INPUT_RESOURCE_TYPE,
                                           DEFAULT_RTP_INPUT_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    // Add stream resources and link them with RTP input and bridge
    UtlString decodeName = DEFAULT_DECODE_RESOURCE_NAME;
    UtlString activityNotifName = DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME;
    activityNotifName.append(CONNECTION_NAME_SUFFIX);
#ifdef INSERT_DELAY_RESOURCE // [
    UtlString delayName = DEFAULT_DELAY_RESOURCE_NAME;
    delayName.append(CONNECTION_NAME_SUFFIX);
#endif // INSERT_DELAY_RESOURCE ]
    for (int i=0; i<mNumMcastStreams; i++)
    {
       // Construct names of resources for this stream
       UtlString streamSuffix;
       streamSuffix.appendFormat(STREAM_NAME_SUFFIX, i);
       UtlString tmpDecodeName(decodeName);
       tmpDecodeName.append(streamSuffix);
       UtlString tmpActivityNotifName(activityNotifName);
       tmpActivityNotifName.append(streamSuffix);
#ifdef INSERT_DELAY_RESOURCE // [
       UtlString tmpDelayName(delayName);
       tmpDelayName.append(streamSuffix);
#endif // INSERT_DELAY_RESOURCE ]

       // Add decoder
       result = resourceTopology->addResource(DEFAULT_DECODE_RESOURCE_TYPE,
                                              tmpDecodeName,
                                              MP_INVALID_CONNECTION_ID,
                                              i);
       assert(result == OS_SUCCESS);

       // Add Voice Activity Notifier
       result = resourceTopology->addResource(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_TYPE,
                                              tmpActivityNotifName,
                                              MP_INVALID_CONNECTION_ID,
                                              i);
       assert(result == OS_SUCCESS);

#ifdef INSERT_DELAY_RESOURCE // [
       // Add delay resource
       result = resourceTopology->addResource(DEFAULT_DELAY_RESOURCE_TYPE,
                                              tmpDelayName,
                                              MP_INVALID_CONNECTION_ID,
                                              i);
       assert(result == OS_SUCCESS);
#endif // INSERT_DELAY_RESOURCE ]

       // Link RTP input -> decoder
       result = resourceTopology->addConnection(DEFAULT_RTP_INPUT_RESOURCE_NAME, i,
                                                tmpDecodeName, 0);
       assert(result == OS_SUCCESS);
       UtlString &lastResourceName = tmpDecodeName;

       // -> Voice Activity Notifier
       result = resourceTopology->addConnection(lastResourceName, 0,
                                                tmpActivityNotifName, 0);
       assert(result == OS_SUCCESS);
       lastResourceName = tmpActivityNotifName;

#ifdef INSERT_DELAY_RESOURCE // [
       // -> Delay
       result = resourceTopology->addConnection(lastResourceName, 0,
                                                tmpDelayName, 0);
       assert(result == OS_SUCCESS);
       lastResourceName = tmpDelayName;
#endif // INSERT_DELAY_RESOURCE ]

       // Mark last resource as RTP Stream Output
       UtlString streamOutputName(VIRTUAL_NAME_RTP_STREAM_OUTPUT);
       streamOutputName.append(streamSuffix);
       result = resourceTopology->addVirtualOutput(lastResourceName, 0,
                                                   streamOutputName, 0);
       assert(result == OS_SUCCESS);

       // -> bridge
       result = resourceTopology->addConnection(lastResourceName, 0,
                                                VIRTUAL_NAME_CONNECTION_PORTS,
                                                MpResourceTopology::MP_TOPOLOGY_NEXT_AVAILABLE_PORT);
       assert(result == OS_SUCCESS);
    }


    addOutputConnectionTopology(resourceTopology, -1);

    return(resourceTopology);
}

int CpTopologyGraphFactoryImpl::getMaxInputConnections()
{
#ifdef INSERT_SPEAKER_SELECTOR // [
   return DEFAULT_SPEAKER_SELECTOR_MAX_INPUTS;
#else  // INSERT_SPEAKER_SELECTOR ][
   return DEFAULT_BRIDGE_MAX_IN_OUTPUTS-initialTopologyInputResourcesNum;
#endif // INSERT_SPEAKER_SELECTOR ]
   
}

/* ============================ ACCESSORS ================================= */

OsStatus CpTopologyGraphFactoryImpl::getSpeakerDevice(UtlString& device) const
{
    OsStatus status = mpOutputDeviceManager->getDeviceName(mDefaultToOutputDevice, device);
    
    return(status);
}

OsStatus CpTopologyGraphFactoryImpl::getMicrophoneDevice(UtlString& device) const
{
    OsStatus status = mpInputDeviceManager->getDeviceName(mDefaultToInputDevice, device);
    
    return(status);
}

void CpTopologyGraphFactoryImpl::setInitialResourceTopology(MpResourceTopology& resourceTopology)
{
    mpInitialResourceTopology = &resourceTopology;
}

MpResourceTopology* CpTopologyGraphFactoryImpl::getInitialResourceTopology() const
{
    return(mpInitialResourceTopology);
}

void CpTopologyGraphFactoryImpl::setResourceFactory(MpResourceFactory& resourceFactory)
{
    mpResourceFactory = &resourceFactory;
}

MpResourceFactory* CpTopologyGraphFactoryImpl::getResourceFactory() const
{
    return(mpResourceFactory);
}

void CpTopologyGraphFactoryImpl::setConnectionResourceTopology(MpResourceTopology& connectionResourceTopology)
{
    mpConnectionResourceTopology = &connectionResourceTopology;
}

MpResourceTopology* CpTopologyGraphFactoryImpl::getConnectionResourceTopology() const
{
    return(mpConnectionResourceTopology);
}

void CpTopologyGraphFactoryImpl::setMcastConnectionResourceTopology(MpResourceTopology& connectionResourceTopology)
{
    mpMcastConnectionResourceTopology = &connectionResourceTopology;
}

MpResourceTopology* CpTopologyGraphFactoryImpl::getMcastConnectionResourceTopology() const
{
    return(mpMcastConnectionResourceTopology);
}

MpInputDeviceManager* CpTopologyGraphFactoryImpl::getInputDeviceManager() const
{
    return(mpInputDeviceManager);
}

OsStatus CpTopologyGraphFactoryImpl::getVideoCpuValue(int& cpuValue) const
{
    cpuValue = SdpCodec::SDP_CODEC_CPU_VERY_HIGH;
    return(OS_SUCCESS);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void CpTopologyGraphFactoryImpl::addOutputConnectionTopology(MpResourceTopology* resourceTopology,
                                                             int logicalPortNum)
{
    OsStatus result;

    UtlString outConnectionVaNotitName = DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME;
    outConnectionVaNotitName.append(CONNECTION_NAME_SUFFIX);
    result = resourceTopology->addResource(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_TYPE,
                                           outConnectionVaNotitName,
                                           MP_INVALID_CONNECTION_ID,
                                           0);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_ENCODE_RESOURCE_TYPE,
                                           DEFAULT_ENCODE_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_RTP_OUTPUT_RESOURCE_TYPE,
                                           DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    // Link bridge -> Output connection Voice Activity Notifier
    result = resourceTopology->addConnection(VIRTUAL_NAME_CONNECTION_PORTS, logicalPortNum, 
                                             outConnectionVaNotitName, 0);
    assert(result == OS_SUCCESS);

    // Output connection Voice Activity Notifier -> encoder
    result = resourceTopology->addConnection(outConnectionVaNotitName, 0, 
                                             DEFAULT_ENCODE_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);

    // Link encoder -> RTP output
    result = resourceTopology->addConnection(DEFAULT_ENCODE_RESOURCE_NAME, 0, 
                                             DEFAULT_RTP_OUTPUT_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);
}

void CpTopologyGraphFactoryImpl::addLocalConnectionTopology(MpResourceTopology* resourceTopology)
{
    OsStatus result;

    // Add resources
    result = resourceTopology->addResources(localConnectionResources,
                                            localConnectionResourcesNum);
    assert(result == OS_SUCCESS);

    // Add connections 
    result = resourceTopology->addConnections(localConnectionConnections,
                                              localConnectionConnectionsNum);
    assert(result == OS_SUCCESS);

    // Set Local Stream Output and Local Stream Input virtual names.
    result = resourceTopology->addVirtualOutput(sgLocalInputConnectionOutput, 0,
                                                VIRTUAL_NAME_LOCAL_STREAM_OUTPUT, 0);
    assert(result == OS_SUCCESS);
    result = resourceTopology->addVirtualInput(sgLocalOutputConnectionInput, 0,
                                               VIRTUAL_NAME_LOCAL_STREAM_INPUT, 0);
    assert(result == OS_SUCCESS);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


