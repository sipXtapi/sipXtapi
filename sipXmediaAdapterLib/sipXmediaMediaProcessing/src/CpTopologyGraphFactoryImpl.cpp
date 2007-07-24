//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2007 SIPez LLC. 
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
#include <mp/MprRtpOutputAudioConnectionConstructor.h>
#include <mp/MprRtpInputAudioConnectionConstructor.h>
#include <mp/MprBufferRecorderConstructor.h>
#include <mp/MprSplitterConstructor.h>
#include <mp/MprNullAecConstructor.h>
#include <include/CpTopologyGraphFactoryImpl.h>
#include <mi/CpMediaInterfaceFactory.h>
#include <include/CpTopologyGraphInterface.h>
#include <os/OsSysLog.h>

// REMOVE THIS when device enumerator/monitor would be implemented
#define USE_DEVICE_ADD_HACK

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
#  include <mp/MpidOSS.h>
#  define INPUT_DRIVER MpidOSS
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
#  include <mp/MpodOSS.h>
#  define OUTPUT_DRIVER MpodOSS
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

extern "C" CpMediaInterfaceFactory* cpTopologyGraphFactoryFactory(OsConfigDb* pConfigDb)
{
    if(spFactory == NULL)
    {
        spFactory = new CpMediaInterfaceFactory();
        spFactory->setFactoryImplementation(new CpTopologyGraphFactoryImpl(pConfigDb));
    }    
    siInstanceCount++;
    return spFactory;
}

#ifdef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb)
{
    return(cpTopologyGraphFactoryFactory(pConfigDb));
}
#endif


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpTopologyGraphFactoryImpl::CpTopologyGraphFactoryImpl(OsConfigDb* pConfigDb)
: sipXmediaFactoryImpl(pConfigDb)
, mpInitialResourceTopology(NULL)
, mpResourceFactory(NULL)
, mpConnectionResourceTopology(NULL)
, mpInputDeviceManager(NULL)
, mpOutputDeviceManager(NULL)
{    
    assert(MpMisc.RawAudioPool);
    mpInputDeviceManager = 
       new MpInputDeviceManager(80,   // samples per frame
                                8000, // samples per second
                                4,    // number of buffered frames
                                *MpMisc.RawAudioPool);

    mpOutputDeviceManager =
       new MpOutputDeviceManager(80,   // samples per frame
                                 8000, // samples per second
                                 0);   // mixer buffer length (ms)




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

    // Set flowgraph ticker
    tempRes = mpOutputDeviceManager->setFlowgraphTickerSource(sinkDeviceId);
    assert(tempRes == OS_SUCCESS);
#endif // USE_DEVICE_ADD_HACK ]

    mpInitialResourceTopology = buildDefaultInitialResourceTopology();
    mpResourceFactory = buildDefaultResourceFactory();
    int firstInvalidResourceIndex;
    OsStatus result = 
        mpInitialResourceTopology->validateResourceTypes(*mpResourceFactory, 
                                                         firstInvalidResourceIndex);
    assert(result == OS_SUCCESS);
    assert(firstInvalidResourceIndex == -1);

    mpConnectionResourceTopology = buildDefaultIncrementalResourceTopology();
}


// Destructor
CpTopologyGraphFactoryImpl::~CpTopologyGraphFactoryImpl()
{
#ifdef USE_DEVICE_ADD_HACK // [
   OsStatus result;

   // Clear flowgraph ticker
   result = mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE);
   assert(result == OS_SUCCESS);

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
#endif // USE_DEVICE_ADD_HACK ]

   // Free factory and topologies.
   delete mpResourceFactory;
   delete mpInitialResourceTopology;
   delete mpConnectionResourceTopology;

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
                                                 UtlBoolean enableIce)
{
    return(new CpTopologyGraphInterface(this, 
                                       publicAddress, 
                                       localAddress, 
                                       numCodecs, 
                                       sdpCodecArray, 
                                       locale, 
                                       expeditedIpTos, 
                                       stunServer,
                                       stunPort, 
                                       stunKeepAliveSecs, 
                                       turnServer, 
                                       turnPort, 
                                       turnUserName, 
                                       turnPassword, 
                                       turnKeepAliveSecs, 
                                       enableIce));
}

MpResourceFactory* CpTopologyGraphFactoryImpl::buildDefaultResourceFactory()
{
    MpResourceFactory* resourceFactory = 
        new MpResourceFactory();

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

    // Output RTP connection
    resourceFactory->addConstructor(*(new MprRtpOutputAudioConnectionConstructor()));

    // Input RTP connection
    resourceFactory->addConstructor(*(new MprRtpInputAudioConnectionConstructor()));

    // Buffer Recorder
    resourceFactory->addConstructor(*(new MprBufferRecorderConstructor()));

    // Splitter
    resourceFactory->addConstructor(*(new MprSplitterConstructor()));

    // NULL AEC
    resourceFactory->addConstructor(*(new MprNullAecConstructor()));

    return(resourceFactory);
}

MpResourceTopology* CpTopologyGraphFactoryImpl::buildDefaultInitialResourceTopology()
{
    MpResourceTopology* resourceTopology = new MpResourceTopology();

    // For now we just hardcode construct a few resources to get the framework
    // working
    OsStatus result;
    result = resourceTopology->addResource(DEFAULT_FROM_FILE_RESOURCE_TYPE, 
                                           DEFAULT_FROM_FILE_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_FROM_INPUT_DEVICE_RESOURCE_TYPE, 
                                           DEFAULT_FROM_INPUT_DEVICE_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_BRIDGE_RESOURCE_TYPE, 
                                           DEFAULT_BRIDGE_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_TONE_GEN_RESOURCE_TYPE, 
                                           DEFAULT_TONE_GEN_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_TYPE, 
                                           DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_BUFFER_RECORDER_RESOURCE_TYPE,
                                           DEFAULT_BUFFER_RECORDER_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_NULL_RESOURCE_TYPE, 
                                           DEFAULT_NULL_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_SPLITTER_RESOURCE_TYPE, 
                                           DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_NULL_AEC_RESOURCE_TYPE, 
                                           DEFAULT_AEC_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    // Link fromFile to bridge
    result = resourceTopology->addConnection(DEFAULT_FROM_FILE_RESOURCE_NAME, 0, DEFAULT_BRIDGE_RESOURCE_NAME, 1);
    assert(result == OS_SUCCESS);

    // Link mic to AEC
    result = resourceTopology->addConnection(DEFAULT_FROM_INPUT_DEVICE_RESOURCE_NAME, 0, DEFAULT_AEC_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);

    // Link AEC to bridge
    result = resourceTopology->addConnection(DEFAULT_AEC_RESOURCE_NAME, 0, DEFAULT_BRIDGE_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);

    // TODO: add a mixer for locally generated audio (e.g. tones, fromFile, etc)
    result = resourceTopology->addConnection(DEFAULT_TONE_GEN_RESOURCE_NAME, 0, DEFAULT_BRIDGE_RESOURCE_NAME, 2);
    assert(result == OS_SUCCESS);

    // Link bridge to splitter, the splitter leaves a tap for AEC to see the output to speaker
    result = resourceTopology->addConnection(DEFAULT_BRIDGE_RESOURCE_NAME, 0, DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);

    // Link splitter to speaker
    result = resourceTopology->addConnection(DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, 0, DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);
    
    // Link splitter to output buffer resource (part of AEC)
    UtlString outBufferResourceName(DEFAULT_AEC_RESOURCE_NAME);
    outBufferResourceName.append(AEC_OUTPUT_BUFFER_RESOURCE_NAME_SUFFIX);
    result = resourceTopology->addConnection(DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME, 1, outBufferResourceName, 0);
    assert(result == OS_SUCCESS);

    // Link bridge to buffer recorder
    // This buffer recorder is intended to record the microphone.
    // Currently, this records all inputs from the bridge.  Once the bridge
    // can configure what inputs go to what outputs, this will behave as 
    // intended
    result = resourceTopology->addConnection(
                DEFAULT_BRIDGE_RESOURCE_NAME, 1, 
                DEFAULT_BUFFER_RECORDER_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);

    // Fill up the unpaired bridge outputs as it currently barfs if
    // it does not have the same number of inputs and outputs.
    result = resourceTopology->addConnection(DEFAULT_BRIDGE_RESOURCE_NAME, 2, DEFAULT_NULL_RESOURCE_NAME, 2);
    assert(result == OS_SUCCESS);

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

MpResourceTopology* CpTopologyGraphFactoryImpl::buildDefaultIncrementalResourceTopology()
{
    MpResourceTopology* resourceTopology = new MpResourceTopology();

    OsStatus result;
    result = resourceTopology->addResource(DEFAULT_RTP_INPUT_RESOURCE_TYPE, 
                                           DEFAULT_RTP_INPUT_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    result = resourceTopology->addResource(DEFAULT_RTP_OUTPUT_RESOURCE_TYPE, 
                                           DEFAULT_RTP_OUTPUT_RESOURCE_NAME);
    assert(result == OS_SUCCESS);

    int logicalPortNum = resourceTopology->getNextLogicalPortNumber();
    // Link RTP input to bridge
    result = resourceTopology->addConnection(DEFAULT_RTP_INPUT_RESOURCE_NAME, 0, 
        DEFAULT_BRIDGE_RESOURCE_NAME, logicalPortNum);
    assert(result == OS_SUCCESS);

    // Link RTP output to bridge
    result = resourceTopology->addConnection(DEFAULT_BRIDGE_RESOURCE_NAME, logicalPortNum, 
        DEFAULT_RTP_OUTPUT_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);

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

MpInputDeviceManager* CpTopologyGraphFactoryImpl::getInputDeviceManager() const
{
    return(mpInputDeviceManager);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


