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
#include <mp/MpResourceTopology.h>


//  REMOVE THESE WHEN MpResourceFactory and MpResourceTopology are implemented
#include <mp/MprToneGen.h>

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

    // Add the resources
    int resourceIndex = 0;
    MpResource* resource = NULL;
    UtlString resourceType;
    UtlString resourceName;
    OsStatus result;
    while(initialResourceTopology.getResource(resourceIndex, resourceType, resourceName) == OS_SUCCESS)
    {
        resource = resourceFactory.newResource(resourceType, resourceName);
        assert(resource);
        if(resource)
        {
            result = addResource(*resource);
            assert(result == OS_SUCCESS);
        }
        resourceIndex++;
    }

    // Link the resources
    int connectionIndex = 0;
    UtlString outputResourceName;
    UtlString inputResourceName;
    int outputResourcePortIndex;
    int inputResourcePortIndex;
    MpResource* outputResource = NULL;
    MpResource* inputResource = NULL;
    while(initialResourceTopology.getConnection(connectionIndex, outputResourceName, outputResourcePortIndex, inputResourceName, inputResourcePortIndex) == OS_SUCCESS)
    {
        result = lookupResource(outputResourceName,
                                outputResource);
        assert(result == OS_SUCCESS);
        result = lookupResource(inputResourceName,
                                inputResource);
        assert(result == OS_SUCCESS);
        assert(outputResource);
        assert(inputResource);

        if(outputResource && inputResource)
        {
            result = addLink(*outputResource, outputResourcePortIndex, *inputResource, inputResourcePortIndex);
            assert(result == OS_SUCCESS);
        }
        connectionIndex++;
    }

    // Enable the flowgraph and all its resources
    result = enable();
    assert(result == OS_SUCCESS);

#if 0
    // DO NOT CHECK IN for test purposes until resource operation messages
    // are completed
    MprToneGen* toneGen = NULL;
    result = lookupResource("ToneGen1",
                            (MpResource*&)toneGen);
    assert(result == OS_SUCCESS);
    assert(toneGen);
    toneGen->startTone('2');
#endif

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

