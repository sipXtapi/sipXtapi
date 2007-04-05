//  
// Copyright (C) 2006-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpTopologyGraph.h>
#include <mp/MpMediaTask.h>
#include <mp/MpResourceFactory.h>


//  REMOVE THESE WHEN MpResourceFactory and MpResourceTopology are implemented
//#include <mp/MprToneGen.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpTopologyGraph::MpTopologyGraph(int samplesPerFrame, 
                                 int samplesPerSec,
                                 MpResourceTopology& initialResourceTopology,
                                 MpResourceFactory& resourceFactory) :
MpFlowGraphBase(samplesPerFrame, samplesPerSec)
{
    printf("constructing MpTopologyGraph\n");
    mpResourceFactory = &resourceFactory;

    // TODO: This needs to be removed and resources listed in the topology
    // need to be constructed using the resource factory.

    // For now we just hardcode construct a few resources to get the framework
    // working
    MpResource* fromFile = 
        resourceFactory.newResource(DEFAULT_FROM_FILE_RESOURCE_TYPE, "FromFile1");
    assert(fromFile);
    OsStatus result = addResource(*fromFile);
    assert(result == OS_SUCCESS);
    printf("fromFile: %p\n", fromFile);

    MpResource* fromMic =
        resourceFactory.newResource(DEFAULT_FROM_INPUT_DEVICE_RESOURCE_TYPE, "FromMic1");
    assert(fromMic);
    result = addResource(*fromMic);
    assert(result == OS_SUCCESS);
    printf("fromMic: %p\n", fromMic);

    MpResource* bridge =
        resourceFactory.newResource(DEFAULT_BRIDGE_RESOURCE_TYPE, "Bridge1");
    assert(bridge);
    result = addResource(*bridge);
    assert(result == OS_SUCCESS);
    printf("bridge: %p\n", bridge);

    // Link fromFile to bridge
    result = addLink(*fromFile, 0, *bridge, 0);
    assert(result == OS_SUCCESS);

    // Link mic to bridge
    result = addLink(*fromMic, 0, *bridge, 1);
    assert(result == OS_SUCCESS);

    // Create a tone generator
    MpResource* toneGen =
        resourceFactory.newResource(DEFAULT_TONE_GEN_RESOURCE_TYPE, "ToneGen1");
    assert(toneGen);
    result = addResource(*toneGen);
    assert(result == OS_SUCCESS);
    printf("toneGen: %p\n", toneGen);

    // TODO: add a mixer for locally generated audio (e.g. tones, fromFile, etc)
    result = addLink(*toneGen, 0, *bridge, 2);
    assert(result == OS_SUCCESS);

    // TODO: need a recorder to be added to bridge output port 0

    MpResource* toSpeaker =
        resourceFactory.newResource(DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_TYPE, "ToSpeaker1");
    assert(toSpeaker);
    result = addResource(*toSpeaker);
    assert(result == OS_SUCCESS);
    printf("toSpeaker: %p\n", toSpeaker);

    // Link bridge to speaker
    result = addLink(*bridge, 1, *toSpeaker, 0);
    assert(result == OS_SUCCESS);

    MpResource* nullResource = 
        resourceFactory.newResource(DEFAULT_NULL_RESOURCE_TYPE, "Null1");
    assert(nullResource);
    result = addResource(*nullResource);
    assert(result == OS_SUCCESS);

    // Fill up the unpaired bridge outputs as it currently barfs if
    // it does not have the same number of inputs and outputs.
    result = addLink(*bridge, 0, *nullResource, 0);
    assert(result == OS_SUCCESS);
    result = addLink(*bridge, 2, *nullResource, 2);
    assert(result == OS_SUCCESS);

    // Enable the flowgraph and all its resources
    result = enable();
    assert(result == OS_SUCCESS);

    // Remove when topology is used to construct and add resources
    assert(toneGen->isEnabled());
    assert(bridge->isEnabled());
    assert(toSpeaker->isEnabled());

    // DO NOT CHECK IN
    //((MprToneGen*)toneGen)->startTone('2');

    // ask the media processing task to manage the new flowgraph
    MpMediaTask* mediaTask = MpMediaTask::getMediaTask(0);
    result = mediaTask->manageFlowGraph(*this);
    assert(result == OS_SUCCESS);

    // start the flowgraph
    result = mediaTask->startFlowGraph(*this);
    assert(result == OS_SUCCESS);
    assert(isStarted());
}

// Destructor
MpTopologyGraph::~MpTopologyGraph()
{
    // unmanage the flowgraph
    MpMediaTask* mediaTask = MpMediaTask::getMediaTask(0);
    OsStatus result = mediaTask->unmanageFlowGraph(*this);
    assert(result == OS_SUCCESS);

    // wait until the flowgraph is unmanaged.
    while (mediaTask->isManagedFlowGraph(this))
    {
        printf("waiting to delete MpTopologyGraph: %p\n", this);
        OsTask::delay(20);   // wait 20 msecs before checking again
    }

}

/* ============================ MANIPULATORS ============================== */

OsStatus MpTopologyGraph::addResources(MpResourceTopology& incrementalTopology,
                                         MpResourceFactory* resourceFactory,
                                         int resourceInstanceId)
{
    if(resourceFactory == NULL)
    {
        resourceFactory = mpResourceFactory;
    }

    // Not implemented
    //assert(resourceFactory);
    return(OS_NOT_YET_IMPLEMENTED);
}

OsStatus MpTopologyGraph::postMessage(const MpFlowGraphMsg& message,
                                      const OsTime& waitTime)
{
    // TODO:  handle or dispatch specific resource messages
    return(MpFlowGraphBase::postMessage(message, waitTime));
}

UtlBoolean MpTopologyGraph::handleMessage(OsMsg& message)
{
    // TODO:  handle or dispatch specific resource messages
    return(MpFlowGraphBase::handleMessage(message));
}

OsStatus MpTopologyGraph::processNextFrame()
{
    //printf("making the doughnuts\n");
    return(MpFlowGraphBase::processNextFrame());
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

