//  
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpTopologyGraph.h>


//  REMOVE THESE WHEN MpResourceFactory and MpResourceTopology are implemented
#include <mp/MprFromFile.h>
#include <mp/MprFromMic.h>
#include <mp/MprBridge.h>
#include <mp/MprToSpkr.h>
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
    addResource(*fromFile);

    MprFromMic* fromMic =
        new MprFromMic("FromMic1",
                       samplesPerFrame,
                       samplesPerSec,
                       MpMisc.pMicQ);
    addResource(*fromMic);

    MprBridge* bridge =
        new MprBridge("Bridge1",
                      samplesPerFrame,
                      samplesPerSec);
    addResource(*bridge);

    // Link fromFile to bridge
    addLink(*fromFile, 0,
            *bridge, 0);

    // Link mic to bridge
    addLink(*fromMic, 0,
            *bridge, 1);

    // TODO: need a recorder to be added to bridge output port 0

    MprToSpkr* toSpeaker =
        new MprToSpkr("ToSpeaker1",
                      samplesPerFrame,
                      samplesPerSec,
                      MpMisc.pSpkQ, 
                      MpMisc.pEchoQ);
    addResource(*toSpeaker);

    // Link bridge to speaker
    addLink(*toSpeaker, 0,
            *bridge, 1);
}

// Destructor
MpTopologyGraph::~MpTopologyGraph()
{
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

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

