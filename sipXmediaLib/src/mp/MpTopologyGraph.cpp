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
#include <utl/UtlInt.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlHashBagIterator.h>
#include <mp/MpTopologyGraph.h>
#include <mp/MpMediaTask.h>
#include <mp/MpResourceFactory.h>
#include <mp/MpResourceTopology.h>


//  REMOVE THESE WHEN MpResourceFactory and MpResourceTopology are implemented
#include <mp/MprToneGen.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAX_CONSTRUCTED_RESOURCES 10
//#define TEST_PRINT

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpTopologyGraph::MpTopologyGraph(int samplesPerFrame, 
                                 int samplesPerSec,
                                 MpResourceTopology& initialResourceTopology,
                                 MpResourceFactory& resourceFactory)
: MpFlowGraphBase(samplesPerFrame, samplesPerSec)
, mpResourceFactory(&resourceFactory)
{
    OsStatus result;

    // construct the new resources defined in the topology and add to the flowgraph
    UtlHashBag newResourcesAdded;
    addTopologyResources(initialResourceTopology, 
                         resourceFactory,
                         newResourcesAdded);

    // Add the links for the resources in the topology
    linkTopologyResources(initialResourceTopology, newResourcesAdded);

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
    if(result != OS_SUCCESS)
    {
        printf("MpTopologyGraph mediaTask->startFlowGraph failed: %d\n", result);
    }
    assert(result == OS_SUCCESS);
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

    assert(resourceFactory);

    // Add new resources
    UtlHashBag newResourcesAdded;
    addTopologyResources(incrementalTopology, 
                         *resourceFactory, 
                         newResourcesAdded,
                         TRUE, 
                         resourceInstanceId);

    // Add the links for the resources in the topology
    linkTopologyResources(incrementalTopology, 
                          newResourcesAdded,
                          TRUE, 
                          resourceInstanceId);

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

// Notification that this flow graph has just been granted the focus.
// Enable our microphone and speaker resources
OsStatus MpTopologyGraph::gainFocus(void)
{
   UtlBoolean    boolRes;
   OsStatus      result;

#ifndef DISABLE_LOCAL_AUDIO // ]
   // enable the FromInputDevice and ToOutputDevice -- we have focus
   {
      MpResource* pResource = NULL;
      result = lookupResource("FromMic1",
                              pResource);
      assert(result == OS_SUCCESS);
      assert(pResource);
      boolRes = pResource->enable();
      assert(boolRes);
   }
   {
      MpResource* pResource = NULL;
      result = lookupResource("ToSpeaker1",
                              pResource);
      assert(result == OS_SUCCESS);
      assert(pResource);
      boolRes = pResource->enable();
      assert(boolRes);
   }
#endif // DISABLE_LOCAL_AUDIO

   return OS_SUCCESS;
}

// Notification that this flow graph has just lost the focus.
// Disable our microphone and speaker resources
OsStatus MpTopologyGraph::loseFocus(void)
{
   UtlBoolean    boolRes;
   OsStatus      result;

#ifndef DISABLE_LOCAL_AUDIO // [
   // disable the FromInputDevice and ToOutputDevice -- we no longer have the focus.
   {
      MpResource* pResource = NULL;
      result = lookupResource("FromMic1",
                              pResource);
      assert(result == OS_SUCCESS);
      assert(pResource);
      boolRes = pResource->disable();
      assert(boolRes);
   }
   {
      MpResource* pResource = NULL;
      result = lookupResource("ToSpeaker1",
                              pResource);
      assert(result == OS_SUCCESS);
      assert(pResource);
      boolRes = pResource->disable();
      assert(boolRes);
   }
#endif //DISABLE_LOCAL_AUDIO ]

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

int MpTopologyGraph::addTopologyResources(MpResourceTopology& resourceTopology,
                                          MpResourceFactory& resourceFactory,
                                          UtlHashBag& newResources,
                                          UtlBoolean replaceNumInName,
                                          int resourceNum)
{
    // Add the resources
    int resourceIndex = 0;
    MpResource* resourcePtr = NULL;
    MpResource* resourceArray[MAX_CONSTRUCTED_RESOURCES];
    UtlString resourceType;
    UtlString resourceName;
    OsStatus result;
    while(resourceTopology.getResource(resourceIndex, resourceType, resourceName) == OS_SUCCESS)
    {
        if(replaceNumInName)
        {
            resourceTopology.replaceNumInName(resourceName, resourceNum);
        }

        int numConstructorResources;
        OsStatus status;
        status = resourceFactory.newResource(resourceType, resourceName, MAX_CONSTRUCTED_RESOURCES,
           numConstructorResources, resourceArray);        
        if(status == OS_SUCCESS)
        {
           assert(numConstructorResources > 0);
           int arrayIndex;
           // We now can potentially get more than one resource back from the
           // constructor
           for(arrayIndex = 0; arrayIndex < numConstructorResources; arrayIndex++)
           {
              resourcePtr = resourceArray[arrayIndex];
              assert(resourcePtr);
              if(resourcePtr)
              {
#ifdef TEST_PRINT
                  printf("constructed and adding resource name: %s type: %s\n",
                      resourcePtr->getName().data(),
                      resourceType.data());
#endif
                  newResources.insert(resourcePtr);
                  result = addResource(*resourcePtr, FALSE);
                  assert(result == OS_SUCCESS);
               }
           }
        }
        else
        {
           OsSysLog::add(FAC_MP, PRI_ERR, 
              "Failed to create resource type: %s name: %s status: %d",
              resourceType.data(), resourceName.data(), status);
        }
        resourceIndex++;
    }
    return(resourceIndex);
}

int MpTopologyGraph::linkTopologyResources(MpResourceTopology& resourceTopology,
                                           UtlHashBag& newResources,
                                           UtlBoolean replaceNumInName,
                                           int resourceNum)
{
    // Link the resources
    int connectionIndex = 0;
    UtlString outputResourceName;
    UtlString inputResourceName;
    int outputResourcePortIndex;
    int inputResourcePortIndex;
    MpResource* outputResource = NULL;
    MpResource* inputResource = NULL;
    OsStatus result;
    UtlHashMap newConnectionIds;

#ifdef TEST_PRINT
    printf("%d new resources in the list\n", newResources.entries());
    UtlHashBagIterator iterator(newResources);
    MpResource* containerResource = NULL;
    while(containerResource = (MpResource*) iterator())
    {
        printf("found list resource: \"%s\" value: \"%s\"\n", containerResource->getName().data(), containerResource->data());
    }
#endif

    while(resourceTopology.getConnection(connectionIndex, outputResourceName, outputResourcePortIndex, inputResourceName, inputResourcePortIndex) == OS_SUCCESS)
    {
        if(replaceNumInName)
        {
            resourceTopology.replaceNumInName(outputResourceName, resourceNum);
            resourceTopology.replaceNumInName(inputResourceName, resourceNum);
        }

        // Look in the container of new resources first as this is more 
        // efficient and new resources are not added immediately to a running
        // flowgraph
        outputResource = (MpResource*) newResources.find(&outputResourceName);
        if(outputResource == NULL)
        {
            result = lookupResource(outputResourceName,
                                    outputResource);
            if(result != OS_SUCCESS)
            {
                printf("MpTopologyGraph::linkTopologyResources could not find output resource: \"%s\"\n",
                    outputResourceName.data());
            }
            assert(result == OS_SUCCESS);
        }
        inputResource = (MpResource*) newResources.find(&inputResourceName);
        if(inputResource == NULL)
        {
            result = lookupResource(inputResourceName,
                                    inputResource);
            assert(result == OS_SUCCESS);
        }
        assert(outputResource);
        assert(inputResource);

        if(outputResource && inputResource)
        {
            if(outputResourcePortIndex == MpResourceTopology::MP_TOPOLOGY_NEXT_AVAILABLE_PORT)
            {
                outputResourcePortIndex = outputResource->reserveFirstUnconnectedOutput();
            }
            else if(outputResourcePortIndex < MpResourceTopology::MP_TOPOLOGY_NEXT_AVAILABLE_PORT)
            {
                // First see if a real port is already in the dictionary
                UtlInt searchKey(outputResourcePortIndex);
                UtlInt* foundValue = NULL;
                if((foundValue = (UtlInt*) newConnectionIds.findValue(&searchKey)))
                {
                    // Use the mapped index
                    outputResourcePortIndex = foundValue->getValue();
                }
                else
                {
                    // Find an available port and add it to the map
                    int realPortNum = outputResource->reserveFirstUnconnectedOutput();
                    UtlInt* portKey = new UtlInt(outputResourcePortIndex);
                    UtlInt* portValue = new UtlInt(realPortNum);
                    newConnectionIds.insertKeyAndValue(portKey, portValue);
                    outputResourcePortIndex = realPortNum;
                }
            }

            if(inputResourcePortIndex == MpResourceTopology::MP_TOPOLOGY_NEXT_AVAILABLE_PORT)
            {
                inputResourcePortIndex = outputResource->reserveFirstUnconnectedOutput();
            }
            else if(inputResourcePortIndex < MpResourceTopology::MP_TOPOLOGY_NEXT_AVAILABLE_PORT)
            {
                // First see if a real port is already in the dictionary
                UtlInt searchKey(inputResourcePortIndex);
                UtlInt* foundValue = NULL;
                if((foundValue = (UtlInt*) newConnectionIds.findValue(&searchKey)))
                {
                    // Use the mapped index
                    inputResourcePortIndex = foundValue->getValue();
                }
                else
                {
                    // Find an available port and add it to the map
                    int realPortNum = inputResource->reserveFirstUnconnectedInput();
                    UtlInt* portKey = new UtlInt(inputResourcePortIndex);
                    UtlInt* portValue = new UtlInt(realPortNum);
                    newConnectionIds.insertKeyAndValue(portKey, portValue);
                    inputResourcePortIndex = realPortNum;
                }
            }


            result = addLink(*outputResource, outputResourcePortIndex, *inputResource, inputResourcePortIndex);
            assert(result == OS_SUCCESS);
        }
        connectionIndex++;
    }

    newConnectionIds.destroyAll();
    return(connectionIndex);
}
    
/* ============================ FUNCTIONS ================================= */

