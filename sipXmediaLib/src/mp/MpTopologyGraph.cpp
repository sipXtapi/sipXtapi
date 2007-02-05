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


//  REMOVE THESE WHEN MpResourceFactory and MpResourceTopology are implemented
#include <mp/MprFromFile.h>
#include <mp/MprToneGen.h>
#include <mp/MprFromMic.h>
#include <mp/MprBridge.h>
#include <mp/MprToSpkr.h>
#include <mp/MprNull.h>
#include <mp/MpMisc.h>

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
    MprFromFile* fromFile = 
        new MprFromFile("FromFile1", 
                        samplesPerFrame,
                        samplesPerSec);
    assert(addResource(*fromFile) == OS_SUCCESS);
    printf("fromFile: %p\n", fromFile);

    MprFromMic* fromMic =
        new MprFromMic("FromMic1",
                       samplesPerFrame,
                       samplesPerSec,
                       MpMisc.pMicQ);
    assert(addResource(*fromMic) == OS_SUCCESS);
    printf("fromMic: %p\n", fromMic);

    MprBridge* bridge =
        new MprBridge("Bridge1",
                      samplesPerFrame,
                      samplesPerSec);
    assert(addResource(*bridge) == OS_SUCCESS);
    printf("bridge: %p\n", bridge);

    // Link fromFile to bridge
    assert(addLink(*fromFile, 0, *bridge, 0) == OS_SUCCESS);

    // Link mic to bridge
    assert(addLink(*fromMic, 0, *bridge, 1) == OS_SUCCESS);

    // Create a tone generator
    MprToneGen* toneGen =
        new MprToneGen("ToneGen1",
                       samplesPerFrame,
                       samplesPerSec,
                       NULL /* locale */);
    assert(addResource(*toneGen) == OS_SUCCESS);
    printf("toneGen: %p\n", toneGen);

    // TODO: add a mixer for locally generated audio (e.g. tones, fromFile, etc)
    assert(addLink(*toneGen, 0, *bridge, 2) == OS_SUCCESS);

    // TODO: need a recorder to be added to bridge output port 0

    MprToSpkr* toSpeaker =
        new MprToSpkr("ToSpeaker1",
                      samplesPerFrame,
                      samplesPerSec,
                      MpMisc.pSpkQ, 
                      MpMisc.pEchoQ);
    assert(addResource(*toSpeaker) == OS_SUCCESS);
    printf("toSpeaker: %p\n", toSpeaker);

    // Link bridge to speaker
    assert(addLink(*bridge, 1, *toSpeaker, 0) == OS_SUCCESS);

    MprNull* nullResource = 
        new MprNull("Null1",
                    5, // max input or outputs
                    samplesPerFrame,
                    samplesPerSec);
    assert(addResource(*nullResource) == OS_SUCCESS);

    // Fill up the unpaired bridge outputs as it currently barfs if
    // it does not have the same number of inputs and outputs.
    assert(addLink(*bridge, 0, *nullResource, 0) == OS_SUCCESS);
    assert(addLink(*bridge, 2, *nullResource, 2) == OS_SUCCESS);

    // Enable the flowgraph and all its resources
    assert(enable() == OS_SUCCESS);

    // Remove when topology is used to construct and add resources
    assert(toneGen->isEnabled());
    assert(bridge->isEnabled());
    assert(toSpeaker->isEnabled());

    // ask the media processing task to manage the new flowgraph
    MpMediaTask* mediaTask = MpMediaTask::getMediaTask(0);
    assert(mediaTask->manageFlowGraph(*this) == OS_SUCCESS);

    // start the flowgraph
    assert(mediaTask->startFlowGraph(*this) == OS_SUCCESS);
    assert(isStarted());
}

// Destructor
MpTopologyGraph::~MpTopologyGraph()
{
    // unmanage the flowgraph
    MpMediaTask* mediaTask = MpMediaTask::getMediaTask(0);
    assert(mediaTask->unmanageFlowGraph(*this) == OS_SUCCESS);

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

