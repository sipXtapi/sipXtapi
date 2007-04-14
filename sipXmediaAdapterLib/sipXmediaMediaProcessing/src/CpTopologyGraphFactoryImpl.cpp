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
#include <mp/MpMisc.h>
#include <mp/MpResourceFactory.h>
#include <mp/MpResourceTopology.h>
#include <mp/MprFromInputDeviceConstructor.h>
#include <mp/MprToOutputDeviceConstructor.h>
#include <mp/MprToneGenConstructor.h>
#include <mp/MprFromFileConstructor.h>
#include <mp/MprNullConstructor.h>
#include <mp/MprBridgeConstructor.h>
#include <include/CpTopologyGraphFactoryImpl.h>
#include <mi/CpMediaInterfaceFactory.h>
#include <include/CpTopologyGraphInterface.h>

// EXTERNAL FUNCTIONS
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
CpTopologyGraphFactoryImpl::CpTopologyGraphFactoryImpl(OsConfigDb* pConfigDb) :
sipXmediaFactoryImpl(pConfigDb)
{    
    mpInitialResourceTopology = buildDefaultInitialResourceTopology();
    mpResourceFactory = buildDefaultResourceFactory();
    int firstInvalidResourceIndex;
    OsStatus result = 
        mpInitialResourceTopology->validateResourceTypes(*mpResourceFactory, 
                                                         firstInvalidResourceIndex);
    assert(result == OS_SUCCESS);
    assert(firstInvalidResourceIndex == -1);

    mpConnectionResourceTopology = NULL;

    assert(MpMisc.RawAudioPool);
    mpInputDeviceManager = 
        new MpInputDeviceManager(80, // samples per frame
                                 8000, // samples per second
                                 4, // number of buffered frames saved
                                 *MpMisc.RawAudioPool);
}


// Destructor
CpTopologyGraphFactoryImpl::~CpTopologyGraphFactoryImpl()
{
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
    resourceFactory->addConstructor(*(new MprFromInputDeviceConstructor()));

    // Output device
    resourceFactory->addConstructor(*(new MprToOutputDeviceConstructor()));

    // Tonegen
    resourceFactory->addConstructor(*(new MprToneGenConstructor()));

    // FromFile
    resourceFactory->addConstructor(*(new MprFromFileConstructor()));

    // Null resource
    resourceFactory->addConstructor(*(new MprNullConstructor()));

    // Bridge
    resourceFactory->addConstructor(*(new MprBridgeConstructor()));

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

    result = resourceTopology->addResource(DEFAULT_NULL_RESOURCE_TYPE, 
                                           DEFAULT_NULL_RESOURCE_NAME);
    assert(result == OS_SUCCESS);



    // Link fromFile to bridge
    result = resourceTopology->addConnection(DEFAULT_FROM_FILE_RESOURCE_NAME, 0, DEFAULT_BRIDGE_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);

    // Link mic to bridge
    result = resourceTopology->addConnection(DEFAULT_FROM_INPUT_DEVICE_RESOURCE_NAME, 0, DEFAULT_BRIDGE_RESOURCE_NAME, 1);
    assert(result == OS_SUCCESS);

    // TODO: add a mixer for locally generated audio (e.g. tones, fromFile, etc)
    result = resourceTopology->addConnection(DEFAULT_TONE_GEN_RESOURCE_NAME, 0, DEFAULT_BRIDGE_RESOURCE_NAME, 2);
    assert(result == OS_SUCCESS);

    // Link bridge to speaker
    result = resourceTopology->addConnection(DEFAULT_BRIDGE_RESOURCE_NAME, 1, DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);

    // Fill up the unpaired bridge outputs as it currently barfs if
    // it does not have the same number of inputs and outputs.
    result = resourceTopology->addConnection(DEFAULT_BRIDGE_RESOURCE_NAME, 0, DEFAULT_NULL_RESOURCE_NAME, 0);
    assert(result == OS_SUCCESS);

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


