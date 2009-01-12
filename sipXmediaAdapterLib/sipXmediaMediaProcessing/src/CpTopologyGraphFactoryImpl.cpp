//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

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
#include <mp/MprBridgeConstructor.h>
#include <mp/MprRtpOutputConnectionConstructor.h>
#include <mp/MprEncodeConstructor.h>
#include <mp/MprRtpInputConnectionConstructor.h>
#include <mp/MprDecodeConstructor.h>
#include <mp/MprBufferRecorderConstructor.h>
#include <mp/MprSplitterConstructor.h>
#include <mp/MprNullAecConstructor.h>
#include <mp/MprVadConstructor.h>
#include <mp/MprVoiceActivityNotifierConstructor.h>
#include <mp/MprDelayConstructor.h>
#include <mp/MprSpeakerSelectorConstructor.h>
#include <mp/MpMediaTask.h>
#include "CpTopologyGraphFactoryImpl.h"
#include "mi/CpMediaInterfaceFactory.h"
#include "CpTopologyGraphInterface.h"
#include <os/OsSysLog.h>

#define DISABLE_LOCAL_AUDIO

#ifdef DISABLE_LOCAL_AUDIO // [
#  include <mp/MpMMTimerWnt.h>
#else // DISABLE_LOCAL_AUDIO ][
   // REMOVE THIS when device enumerator/monitor is implemented
#  define USE_DEVICE_ADD_HACK
#endif // DISABLE_LOCAL_AUDIO ]

#ifdef USE_DEVICE_ADD_HACK // [

//#define USE_TEST_INPUT_DRIVER
//#define USE_TEST_OUTPUT_DRIVER

#ifdef USE_TEST_INPUT_DRIVER // USE_TEST_DRIVER [
#  include <mp/MpSineWaveGeneratorDeviceDriver.h>
#  define INPUT_DRIVER MpSineWaveGeneratorDeviceDriver
#  define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager) "SineGenerator", (manager), 32000, 3000, 0

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#  include <mp/MpidWinMM.h>
#  define INPUT_DRIVER MpidWinMM
#  define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager) MpidWinMM::getDefaultDeviceName(), (manager)

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#  include <mp/MpidOss.h>
#  define INPUT_DRIVER MpidOss
#  define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager) "/dev/dsp", (manager)

#else // __pingtel_on_possix__ ]
#  error Unknown platform!
#endif

#ifdef USE_TEST_OUTPUT_DRIVER // USE_TEST_DRIVER [
#  include <mp/MpodBufferRecorder.h>
#  define OUTPUT_DRIVER MpodBufferRecorder
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "default", 60*1000*1000

#  include <os/OsFS.h> // for OpenAndWrite() to write captured data.

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#include <mp/MpodWinMM.h>
#define OUTPUT_DRIVER MpodWinMM
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS MpodWinMM::getDefaultDeviceName()

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#  include <mp/MpodOss.h>
#  define OUTPUT_DRIVER MpodOss
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "/dev/dsp"

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

extern "C" CpMediaInterfaceFactory* cpTopologyGraphFactoryFactory(OsConfigDb* pConfigDb, 
                                                                  uint32_t frameSizeMs, 
                                                                  uint32_t maxSamplesPerSec,
                                                                  uint32_t defaultDeviceSamplesPerSec)
{
    if(spFactory == NULL)
    {
        spFactory = new CpMediaInterfaceFactory();
        spFactory->setFactoryImplementation(new CpTopologyGraphFactoryImpl(pConfigDb,
                                                                           frameSizeMs, 
                                                                           maxSamplesPerSec,
                                                                           defaultDeviceSamplesPerSec));
    }    
    siInstanceCount++;
    return spFactory;
}

#ifdef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb,
                                                            uint32_t frameSizeMs, 
                                                            uint32_t maxSamplesPerSec,
                                                            uint32_t defaultDeviceSamplesPerSec)
{
    return(cpTopologyGraphFactoryFactory(pConfigDb, frameSizeMs, 
                                         maxSamplesPerSec, 
                                         defaultDeviceSamplesPerSec));
}
#endif


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpTopologyGraphFactoryImpl::CpTopologyGraphFactoryImpl(OsConfigDb* pConfigDb,
                                                       uint32_t frameSizeMs, 
                                                       uint32_t maxSamplesPerSec,
                                                       uint32_t defaultSamplesPerSec)
: sipXmediaFactoryImpl(pConfigDb, frameSizeMs, maxSamplesPerSec, defaultSamplesPerSec)
, mpInitialResourceTopology(NULL)
, mpResourceFactory(NULL)
, mpConnectionResourceTopology(NULL)
, mpMcastConnectionResourceTopology(NULL)
, mpInputDeviceManager(NULL)
, mpOutputDeviceManager(NULL)
, mpMediaTaskTicker(NULL)
, mNumMcastStreams(3)
{
    assert(MpMisc.RawAudioPool);
    uint32_t mgrSamplesPerFrame = (mFrameSizeMs*mDefaultSamplesPerSec)/1000;
    mpInputDeviceManager = 
        new MpInputDeviceManager(mgrSamplesPerFrame,    // samples per frame
                                 mDefaultSamplesPerSec, // samples per second
                                 2,    // number of buffered frames
                                 *MpMisc.RawAudioPool);

    mpOutputDeviceManager =
        new MpOutputDeviceManager(mgrSamplesPerFrame,    // samples per frame
                                  mDefaultSamplesPerSec, // samples per second
#ifdef WIN32 // [
                                  // On Windows (especially on Vista)
                                  // audio is consumed by 50ms chunks, causing
                                  // bursts of ticker notifications. So we
                                  // should set mixer size big enough to
                                  // iron out this jitter.
                                  mFrameSizeMs*7         // mixer buffer length (ms)
#else // WIN32 ][
                                  mFrameSizeMs*2         // mixer buffer length (ms)
#endif // !WIN32 ]
                                  );

    // Get media task ticker notification
    OsNotification *pTickerNotf = MpMediaTask::getMediaTask(0)->getTickerNotification();

#ifdef USE_DEVICE_ADD_HACK // [
    // Create source (input) device and add it to manager.
    INPUT_DRIVER *sourceDevice = new INPUT_DRIVER(INPUT_DRIVER_CONSTRUCTOR_PARAMS(*mpInputDeviceManager));
    MpInputDeviceHandle  sourceDeviceId = mpInputDeviceManager->addDevice(*sourceDevice);
    assert(sourceDeviceId > 0);

    // Create sink (output) device and add it to manager.
    OUTPUT_DRIVER *sinkDevice = new OUTPUT_DRIVER(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
    MpOutputDeviceHandle  sinkDeviceId = mpOutputDeviceManager->addDevice(sinkDevice);
    assert(sinkDeviceId > 0);

    OsStatus tempRes;

    // Enable devices
    tempRes = mpInputDeviceManager->enableDevice(sourceDeviceId);
    assert(tempRes == OS_SUCCESS);
    tempRes = mpOutputDeviceManager->enableDevice(sinkDeviceId);
    assert(tempRes == OS_SUCCESS);

    // Use our output device as a source of media task ticks.
    tempRes = mpOutputDeviceManager->setFlowgraphTickerSource(sinkDeviceId,
                                                              pTickerNotf);
    assert(tempRes == OS_SUCCESS);
#else // USE_DEVICE_ADD_HACK ][
    // Use multimedia timer as a source of media task ticks.
    mpMediaTaskTicker = new MpMMTimerWnt(MpMMTimer::Notification);
    mpMediaTaskTicker->setNotification(pTickerNotf);
    mpMediaTaskTicker->run(mFrameSizeMs*1000);
#endif // USE_DEVICE_ADD_HACK ]

    mpInitialResourceTopology = buildDefaultInitialResourceTopology();
#ifndef DISABLE_LOCAL_AUDIO // [
    // Add one local connection to initial topology by default.
    addLocalConnectionTopology(mpInitialResourceTopology);
#endif // DISABLE_LOCAL_AUDIO ][
    mpResourceFactory = buildDefaultResourceFactory();
    int firstInvalidResourceIndex;
    OsStatus result = 
        mpInitialResourceTopology->validateResourceTypes(*mpResourceFactory, 
                                                         firstInvalidResourceIndex);
    assert(result == OS_SUCCESS);
    assert(firstInvalidResourceIndex == -1);

    mpConnectionResourceTopology = buildUnicastConnectionResourceTopology();
    mpMcastConnectionResourceTopology = buildMulticastConnectionResourceTopology();
}


// Destructor
CpTopologyGraphFactoryImpl::~CpTopologyGraphFactoryImpl()
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
   result = mpInputDeviceManager->disableDevice(1);
   assert(result == OS_SUCCESS);
   result = mpOutputDeviceManager->disableDevice(1);
   assert(result == OS_SUCCESS);

   // Free input device driver
   MpInputDeviceDriver *pInDriver = mpInputDeviceManager->removeDevice(1);
   assert(pInDriver != NULL);
   delete pInDriver;

   // Free output device driver
   MpOutputDeviceDriver *pOutDriver = mpOutputDeviceManager->removeDevice(1);
   assert(pOutDriver != NULL);
   delete pOutDriver;

#else // USE_DEVICE_ADD_HACK ][

   // Stop our ticker timer
   mpMediaTaskTicker->stop();
   delete mpMediaTaskTicker;

#endif // USE_DEVICE_ADD_HACK ]

   // Free factory and topologies.
   delete mpResourceFactory;
   delete mpInitialResourceTopology;
   delete mpConnectionResourceTopology;
   delete mpMcastConnectionResourceTopology;

   // Free input and output device managers.
   delete mpInputDeviceManager;
   delete mpOutputDeviceManager;
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
    // if the sample rate passed in is zero, use the default.
    samplesPerSec = (samplesPerSec == 0) ? mDefaultSamplesPerSec : samplesPerSec;
    CpMediaInterface *pIf =
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
                                       pDispatcher);
    pIf->setValue(miInterfaceId);
    miInterfaceId++;
    return pIf;
}

MpResourceFactory* CpTopologyGraphFactoryImpl::buildDefaultResourceFactory()
{
    MpResourceFactory* resourceFactory = new MpResourceFactory();

    // Input device
    resourceFactory->addConstructor(*(new MprFromInputDeviceConstructor(mpInputDeviceManager)));

    // Output device
    resourceFactory->addConstructor(*(new MprToOutputDeviceConstructor(mpOutputDeviceManager)));

    // ToneGen
    resourceFactory->addConstructor(*(new MprToneGenConstructor()));

    // FromFile
    resourceFactory->addConstructor(*(new MprFromFileConstructor()));

    // Null resource
    resourceFactory->addConstructor(*(new MprNullConstructor()));

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

    // Buffer Recorder
    resourceFactory->addConstructor(*(new MprBufferRecorderConstructor()));

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
   {DEFAULT_BUFFER_RECORDER_RESOURCE_TYPE, DEFAULT_BUFFER_RECORDER_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
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
    // This buffer recorder is intended to record the microphone.
    // Currently, this records all inputs from the bridge.  Once the bridge
    // can configure what inputs go to what outputs, this will behave as 
    // intended
   {DEFAULT_BRIDGE_RESOURCE_NAME, 0, DEFAULT_BUFFER_RECORDER_RESOURCE_NAME, 0},
    // Bridge(1) -> Null(1)
    // Fill up the unpaired bridge outputs as it currently barfs if
    // it does not have the same number of inputs and outputs.
   {DEFAULT_BRIDGE_RESOURCE_NAME, 1, DEFAULT_NULL_RESOURCE_NAME, 1},
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
   {DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_TYPE, DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
   {DEFAULT_SPLITTER_RESOURCE_TYPE, DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1},
   {DEFAULT_NULL_AEC_RESOURCE_TYPE, DEFAULT_AEC_RESOURCE_NAME, MP_INVALID_CONNECTION_ID, -1}
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
#ifdef INSERT_DELAY_RESOURCE // [
    //     -> Delay
   {NULL, 0, DEFAULT_DELAY_RESOURCE_NAME MIC_NAME_SUFFIX, 0},
#endif // INSERT_DELAY_RESOURCE ]
    //     -> AEC
   {NULL, 0, DEFAULT_AEC_RESOURCE_NAME, 0},
    //     -> Connection Port(-1)
   {NULL, 0, VIRTUAL_NAME_CONNECTION_PORTS, -1},

    // Connection Port(-1) -> Splitter
    // The splitter leaves a tap for AEC to see the output to speaker
   {VIRTUAL_NAME_CONNECTION_PORTS, -1, DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, 0},
    // Splitter(0) -> VAD
   {DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, 0, DEFAULT_VAD_RESOURCE_NAME SPEAKER_NAME_SUFFIX, 0},
    //             -> Voice Activity Notifier
   {NULL, 0, DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME SPEAKER_NAME_SUFFIX, 0},
    //             -> Speaker
   {NULL, 0, DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME, 0},
    // Splitter(1) -> Output Buffer (part of AEC)
   {DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, 1, DEFAULT_AEC_RESOURCE_NAME AEC_OUTPUT_BUFFER_RESOURCE_NAME_SUFFIX, 0}
};
static const int localConnectionConnectionsNum =
   sizeof(localConnectionConnections)/sizeof(MpResourceTopology::ConnectionDef);

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
    for (int i=0; i<DEFAULT_SPEAKER_SELECTOR_MAX_IN_OUTPUTS; i++)
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

/* ============================ ACCESSORS ================================= */

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
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


