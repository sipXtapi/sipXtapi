//  
// Copyright (C) 2006-2011 SIPez LLC. All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2006-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlDListIterator.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlHashBagIterator.h>
#include <utl/UtlContainablePair.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlInt.h>
#include <mp/MpResourceTopology.h>
#include <mp/MpResourceFactory.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// PRIVATE CLASS DEFINITIONS
class MpResourceDefinition : public UtlString
{
public:
   MpResourceDefinition(const UtlString& resourceType,
                        const UtlString& resourceName,
                        MpConnectionID connectionId,
                        int streamId)
   : UtlString(resourceName)
   , mResourceType(resourceType)
   , mConnectionId(connectionId)
   , mStreamId(streamId)
   {
   }

   virtual ~MpResourceDefinition(){};

   UtlString mResourceType;
   MpConnectionID mConnectionId;
   int mStreamId;

private:
   // Disable the following
   MpResourceDefinition();
   MpResourceDefinition(const MpResourceDefinition& ref);
   MpResourceDefinition& operator=(const MpResourceDefinition& ref);
};


class MpResourceConnectionDefinition : public UtlString
{
public:
   MpResourceConnectionDefinition(const UtlString& outputResourceName,
                                  int outputPortIndex,
                                  const UtlString& inputResourceName,
                                  int inputPortIndex)
   : UtlString(outputResourceName)
   , mInputResourceName(inputResourceName)
   , mOutputPortIndex(outputPortIndex)
   , mInputPortIndex(inputPortIndex)
   {
   };

   virtual ~MpResourceConnectionDefinition(){};

   UtlString mInputResourceName;
   int mOutputPortIndex;
   int mInputPortIndex;

private:
   // Disable the following
   MpResourceConnectionDefinition();
   MpResourceConnectionDefinition(const MpResourceConnectionDefinition& ref);
   MpResourceConnectionDefinition& operator=(const MpResourceConnectionDefinition& ref);
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpResourceTopology::MpResourceTopology() :
mPriorLogicalPort(MP_TOPOLOGY_NEXT_AVAILABLE_PORT)
{
}

// Destructor
MpResourceTopology::~MpResourceTopology()
{
   mResources.destroyAll();
   mConnections.destroyAll();
   mVirtualInputs.destroyAll();
   mVirtualOutputs.destroyAll();
}

/* ============================ MANIPULATORS ============================== */
OsStatus MpResourceTopology::addResource(const UtlString& resourceType,
                                         const UtlString& resourceName,
                                         MpConnectionID connectionId,
                                         int streamId)
{
   OsStatus result;

   if(mResources.find(&resourceName))
   {
      result = OS_NAME_IN_USE;
   }
   else
   {
      mResources.append(new MpResourceDefinition(resourceType, resourceName,
                                                 connectionId, streamId));
      result = OS_SUCCESS;
   }
   return result;
}

OsStatus MpResourceTopology::addResources(const ResourceDef *defines,
                                          int numDefines)
{
   OsStatus result;
   for (int i=0; i<numDefines; i++)
   {
      result = addResource(defines[i].resourceType, defines[i].resourceName,
                           defines[i].connectionId, defines[i].streamId);
      if (result != OS_SUCCESS)
      {
         return result;
      }
   }
   return OS_SUCCESS;
}

OsStatus MpResourceTopology::addConnection(const UtlString& outputResourceName,
                                           int outputPortIndex,
                                           const UtlString& inputResourceName,
                                           int inputPortIndex)
{
   mConnections.append(new MpResourceConnectionDefinition(outputResourceName,
                                                          outputPortIndex,
                                                          inputResourceName,
                                                          inputPortIndex));
   return OS_SUCCESS;
}

OsStatus MpResourceTopology::addConnections(const ConnectionDef *defines,
                                            int numDefines)
{
   OsStatus result;

   // At the beginning we don't know the previous resource.
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
   // Visual Studio 6.0 broken
   if (defines[0].outputResourceName == NULL)
#else
   if (defines[0].outputResourceName.isNull())
#endif
   {
      return OS_INVALID_ARGUMENT;
   }

   // Create connections
   for (int i=0; i<numDefines; i++)
   {
      // If NULL is passed as a source resource name, connect to last resource.
      const UtlString &outputResourceName = 
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
   // Visual Studio 6.0 broken
                                            defines[i].outputResourceName == NULL
#else
                                            defines[i].outputResourceName.isNull()
#endif
                                          ? defines[i-1].inputResourceName
                                          : defines[i].outputResourceName;
      // Add connection.
      result = addConnection(outputResourceName,
                             defines[i].outputPortIndex,
                             defines[i].inputResourceName,
                             defines[i].inputPortIndex);
      if (result != OS_SUCCESS)
      {
         return result;
      }
   }
   return OS_SUCCESS;
}

OsStatus MpResourceTopology::addVirtualInput(const UtlString& realResourceName,
                                             int realPortIndex,
                                             const UtlString& virtualResourceName,
                                             int virtualPortIndex)
{
   if (realPortIndex < 0 != virtualPortIndex < 0)
   {
      // Either both indices must be negative, or both must be positive
      return OS_INVALID_ARGUMENT;
   }

   // Allocate pair for the virtual port and check if it is already present in the map.
   UtlContainablePair *pVirtPort = new UtlContainablePair(new UtlString(virtualResourceName),
                                                          new UtlInt(virtualPortIndex));
   if (mVirtualInputs.contains(pVirtPort))
   {
      // This virtual port is already used.
      delete pVirtPort;
      return OS_INVALID_ARGUMENT;
   }

   // Check that the real port exists.
   UtlDListIterator resourceIterator(mResources);
   MpResourceDefinition *pResourceDef;
   UtlBoolean realResourceFound = FALSE;
   while((pResourceDef = (MpResourceDefinition*) resourceIterator()))
   {
      if (pResourceDef->compareTo(realResourceName))
      {
         realResourceFound = TRUE;
      }
   }
   if (!realResourceFound)
   {
      delete pVirtPort;
      return OS_NOT_FOUND;
   }

   // Allocate pair for the real port and try to add it to the map.
   UtlContainablePair *pRealPort = new UtlContainablePair(new UtlString(realResourceName),
                                                          new UtlInt(realPortIndex));
   if (mVirtualInputs.insertKeyAndValue(pVirtPort, pRealPort) == NULL)
   {
      // This virtual port is already assigned.
      delete pRealPort;
      delete pVirtPort;
      return OS_INVALID_ARGUMENT;
   } 

   return OS_SUCCESS;
}

OsStatus MpResourceTopology::addVirtualOutput(const UtlString& realResourceName,
                                              int realPortIndex,
                                              const UtlString& virtualResourceName,
                                              int virtualPortIndex)
{
   if (realPortIndex < 0 != virtualPortIndex < 0)
   {
      // Either both indices must be negative, or both must be positive
      return OS_INVALID_ARGUMENT;
   }

   // Allocate pair for the virtual port and check if it is already present in the map.
   UtlContainablePair *pVirtPort = new UtlContainablePair(new UtlString(virtualResourceName),
                                                          new UtlInt(virtualPortIndex));
   if (mVirtualOutputs.contains(pVirtPort))
   {
      // This virtual port is already used.
      delete pVirtPort;
      return OS_INVALID_ARGUMENT;
   }

   // Check that the real port exists.
   UtlDListIterator resourceIterator(mResources);
   MpResourceDefinition *pResourceDef;
   UtlBoolean realResourceFound = FALSE;
   while((pResourceDef = (MpResourceDefinition*) resourceIterator()))
   {
      if (pResourceDef->compareTo(realResourceName))
      {
         realResourceFound = TRUE;
      }
   }
   if (!realResourceFound)
   {
      delete pVirtPort;
      return OS_NOT_FOUND;
   }

   // Allocate pair for the real port and try to add it to the map.
   UtlContainablePair *pRealPort = new UtlContainablePair(new UtlString(realResourceName),
                                                          new UtlInt(realPortIndex));
   if (mVirtualOutputs.insertKeyAndValue(pVirtPort, pRealPort) == NULL)
   {
      // This virtual port is already assigned.
      delete pRealPort;
      delete pVirtPort;
      return OS_INVALID_ARGUMENT;
   } 

   return OS_SUCCESS;
}

OsStatus MpResourceTopology::validateConnections(UtlString& firstUnconnectedResourceName,
                                                 UtlString&  firstDanglingResourceName,
                                                 UtlBoolean allowExternalResources) const
{
   // First make sure every resource has a connection
   // This test must be true whether this is a full topology or an
   // incremental topology or we would have dangling resources
   UtlHashBag resourcesWithoutOutputConnections;
   UtlDListIterator resourceIterator(mResources);
   MpResourceDefinition* resourceDef = NULL;
   firstUnconnectedResourceName = "";
   firstDanglingResourceName = "";
   OsStatus result = OS_SUCCESS;
   while((resourceDef = (MpResourceDefinition*) resourceIterator()))
   {
      // Add resources with no output connection to the hash
      if(mConnections.find(resourceDef) == NULL)
      {
         resourcesWithoutOutputConnections.insert(resourceDef);
      }
   }

   // Of the resources with no output connection remove those with input
   // connections and we have the resources with no connections
   UtlDListIterator connectionIterator(mConnections);
   MpResourceConnectionDefinition* connectionDef = NULL;
   while((connectionDef = (MpResourceConnectionDefinition*) connectionIterator()))
   {
      resourcesWithoutOutputConnections.remove(&(connectionDef->mInputResourceName));
   }

   if(resourcesWithoutOutputConnections.entries())
   {
      result = OS_INVALID;
      resourceIterator.reset();
      while((resourceDef = (MpResourceDefinition*) resourceIterator()))
      {
         if(resourcesWithoutOutputConnections.find(resourceDef))
         {
            firstUnconnectedResourceName = *resourceDef;
            break;
         }
      }
   }

   // If everything is valid so far and we have a full topology
   if(!allowExternalResources && result == OS_SUCCESS)
   {
      // In the following code we will traverse the topology starting
      // at one resource following its connections and the resources
      // connected by those connections.  As we find connections to
      // resources we will eliminate the resources from the danglingResource
      // list.  At the end after we have pursued all the connections
      // and resources left in the dangling list are not connection.
      // There should be no dangling resources in a full topology
      // definition, hence an invalid topology if there are resources
      // left.

      // Start with a list of all resources, we will remove these
      // as we find connections
      UtlDList danglingResources;
      resourceIterator.reset();
      while((resourceDef = (MpResourceDefinition*) resourceIterator()))
      {
         danglingResources.append(resourceDef);
      }

      // Create a list to contain connections that we need to traverse
      UtlDList connectionsToTraverse;

      // Prime the connection list with the connections for the first
      // resource
      resourceDef = (MpResourceDefinition*) danglingResources.at(0);
      if(resourceDef)
      {
         findResourceConnections(*resourceDef, connectionsToTraverse);

         // Get the first connection
         while((connectionDef = (MpResourceConnectionDefinition*) connectionsToTraverse.get()))
         {
            UtlString resourceName;
            int resourcePort;

//            printf("%s(%d) -> %s(%d)\n", connectionDef->data(), connectionDef->mOutputPortIndex,
//               connectionDef->mInputResourceName.data(), connectionDef->mInputPortIndex);
            // If the output resource for the connection is still in the list
            // add its connections to the connection list to traverse and
            // remove the resource from the list.
            if(danglingResources.remove(connectionDef))
            {
               findResourceConnections(*connectionDef, connectionsToTraverse);
            }
            else
            {
               // Not found... Try virtual resource.
               OsStatus virtResult = 
                  getOutputVirtualResource(*connectionDef,
                                           connectionDef->mOutputPortIndex,
                                           resourceName,
                                           resourcePort);
               if(virtResult == OS_SUCCESS &&
                  danglingResources.remove(&resourceName))
               {
                  findResourceConnections(resourceName, connectionsToTraverse);
               }
            }


            //  Do the same for the input resource for this connection
            if(danglingResources.remove(&(connectionDef->mInputResourceName)))
            {
               findResourceConnections(connectionDef->mInputResourceName,
                                       connectionsToTraverse);
            }
            else
            {
               // Not found... Try virtual resource.
               OsStatus virtResult = 
                  getInputVirtualResource(connectionDef->mInputResourceName,
                                          connectionDef->mInputPortIndex,
                                          resourceName,
                                          resourcePort);
               if(virtResult == OS_SUCCESS &&
                  danglingResources.remove(&resourceName))
               {
                  findResourceConnections(resourceName, connectionsToTraverse);
               }
            }
         } // end while we have connections in the list
      }

      // If all is well we remove all the resources for the set of connections
      // that we collected and traversed.
      assert(connectionsToTraverse.entries() == 0);

      // If we still have resources left in the danglingResources list,
      // they were not connected to the rest
      if(danglingResources.entries())
      {
         result = OS_NOT_FOUND;
         firstDanglingResourceName = *((MpResourceDefinition*)danglingResources.get());
      }

   }

   return result;
}

OsStatus MpResourceTopology::validateResourceTypes(MpResourceFactory& resourceFactory,
                                                   int& firstInvalidResourceIndex) const
{
   UtlDListIterator iterator(mResources);
   MpResourceDefinition* resourceDef = NULL;
   int resourceIndex = 0;
   firstInvalidResourceIndex = -1;
   OsStatus result = OS_SUCCESS;
   while((resourceDef = (MpResourceDefinition*) iterator()))
   {
      if(!resourceFactory.constructorExists(resourceDef->mResourceType))
      {
         result = OS_NOT_FOUND;
         firstInvalidResourceIndex = resourceIndex;
         break;
      }
      resourceIndex++;
   }
   return result;
}

void MpResourceTopology::replaceNumInName(UtlString& resourceName,
                                          int resourceNum)
{
   int stringIndex = resourceName.index("%d");
   if(stringIndex >= 0)
   {
      char numBuf[20];
      sprintf(numBuf, "%d", resourceNum);
      resourceName.replace(stringIndex, 2, numBuf);
   }
}

/* ============================ ACCESSORS ================================= */

OsStatus MpResourceTopology::getResource(int resourceIndex,
                                         UtlString& resourceType,
                                         UtlString& resourceName,
                                         MpConnectionID& connectionId,
                                         int& streamId) const
{
   MpResourceDefinition* resourceDef =
      (MpResourceDefinition*) mResources.at(resourceIndex);
   OsStatus result;
   if(resourceDef)
   {
      result = OS_SUCCESS;
      resourceName = *resourceDef;
      resourceType = resourceDef->mResourceType;
      connectionId = resourceDef->mConnectionId;
      streamId = resourceDef->mStreamId;
   }
   else
   {
      result = OS_NOT_FOUND;
   }
   return result;
}


OsStatus MpResourceTopology::getResource(int resourceIndex,
                                         UtlString& resourceName) const
{
   MpResourceDefinition* resourceDef =
      (MpResourceDefinition*) mResources.at(resourceIndex);
   OsStatus result;
   if(resourceDef)
   {
      result = OS_SUCCESS;
      resourceName = *resourceDef;
   }
   else
   {
      result = OS_NOT_FOUND;
   }
   return result;
}

OsStatus MpResourceTopology::getInputVirtualResource(const UtlString &virtualName,
                                                     int virtualPort,
                                                     UtlString& resourceName,
                                                     int &resourcePort) const
{
   UtlInt virtualPortNum(virtualPort);
   UtlContainablePair virtPair((UtlContainable*)&virtualName, &virtualPortNum);
   UtlContainablePair *realPair;
   
   // Attempt to search for real port as is
   realPair = (UtlContainablePair*)mVirtualInputs.findValue(&virtPair);
   if (realPair == NULL && virtualPort >= 0)
   {
      // If not found - try to search for virtual port -1 (wildcard value).
      virtualPortNum.setValue(-1);
      realPair = (UtlContainablePair*)mVirtualInputs.findValue(&virtPair);
   }

   // Set to NULL to prevent deletion of stack variables
   virtPair.setFirst(NULL);
   virtPair.setSecond(NULL);

   if (!realPair)
   {
      // Nothing found
      return OS_NOT_FOUND;
   }

   // Return found values
   resourceName = *(UtlString*)realPair->getFirst();
   resourcePort = ((UtlInt*)realPair->getSecond())->getValue();
   if (resourcePort < 0 && virtualPort > 0)
   {
      // If concrete port is requested, but wildcard is found - return
      // concrete value.
      resourcePort = virtualPort;
   }

   return OS_SUCCESS;
}

OsStatus MpResourceTopology::getOutputVirtualResource(const UtlString &virtualName,
                                                      int virtualPort,
                                                      UtlString& resourceName,
                                                      int &resourcePort) const
{
   UtlInt virtualPortNum(virtualPort);
   UtlContainablePair virtPair((UtlContainable*)&virtualName, &virtualPortNum);
   UtlContainablePair *realPair;
   
   // Attempt to search for real port as is
   realPair = (UtlContainablePair*)mVirtualOutputs.findValue(&virtPair);
   if (realPair == NULL && virtualPort >= 0)
   {
      // If not found - try to search for virtual port -1 (wildcard value).
      virtualPortNum.setValue(-1);
      realPair = (UtlContainablePair*)mVirtualOutputs.findValue(&virtPair);
   }

   // Set to NULL to prevent deletion of stack variables
   virtPair.setFirst(NULL);
   virtPair.setSecond(NULL);

   if (!realPair)
   {
      // Nothing found
      return OS_NOT_FOUND;
   }

   // Return found values
   resourceName = *(UtlString*)realPair->getFirst();
   resourcePort = ((UtlInt*)realPair->getSecond())->getValue();
   if (resourcePort < 0 && virtualPort > 0)
   {
      // If concrete port is requested, but wildcard is found - return
      // concrete value.
      resourcePort = virtualPort;
   }

   return OS_SUCCESS;
}

OsStatus MpResourceTopology::getConnection(int connectionIndex,
                                           UtlString& outputResourceName,
                                           int& outputPortIndex,
                                           UtlString& inputResourceName,
                                           int& inputPortIndex)
{
    MpResourceConnectionDefinition* connectionDef =
        (MpResourceConnectionDefinition*) mConnections.at(connectionIndex);
    OsStatus result;

    if(connectionDef)
    {
        result = OS_SUCCESS;
        outputResourceName = *connectionDef;
        inputResourceName = connectionDef->mInputResourceName;
        outputPortIndex = connectionDef->mOutputPortIndex;
        inputPortIndex = connectionDef->mInputPortIndex;
    }
    else
    {
        result = OS_NOT_FOUND;
    }
    return(result);
}

int MpResourceTopology::getNextLogicalPortNumber()
{
    return --mPriorLogicalPort;
}

void MpResourceTopology::initVirtualInputIterator(VirtualPortIterator &portIter)
{
   portIter = new UtlHashMapIterator(mVirtualInputs);
}

void MpResourceTopology::freeVirtualInputIterator(VirtualPortIterator &portIter)
{
   delete (UtlHashMapIterator*)portIter;
   portIter = NULL;
}

OsStatus MpResourceTopology::getNextVirtualInput(VirtualPortIterator &portIter,
                                                 UtlString& realResourceName,
                                                 int &realPortIndex,
                                                 UtlString& virtualResourceName,
                                                 int &virtualPortIndex)
{
   UtlContainablePair *pVirtPort = (UtlContainablePair*)(*portIter)();
   if (pVirtPort == NULL)
   {
      return OS_NO_MORE_DATA;
   }

   UtlContainablePair *pRealPort = (UtlContainablePair*)portIter->value();
   realResourceName = *(UtlString*)pRealPort->getFirst();
   realPortIndex = ((UtlInt*)pRealPort->getSecond())->getValue();
   virtualResourceName = *(UtlString*)pVirtPort->getFirst();
   virtualPortIndex = ((UtlInt*)pVirtPort->getSecond())->getValue();

   return OS_SUCCESS;
}

void MpResourceTopology::initVirtualOutputIterator(VirtualPortIterator &portIter)
{
   portIter = new UtlHashMapIterator(mVirtualOutputs);
}

void MpResourceTopology::freeVirtualOutputIterator(VirtualPortIterator &portIter)
{
   // Implementation is the same as for getNextVirtualInput().
   // Only difference between input and output iterator is in its construction.
   freeVirtualInputIterator(portIter);
}

OsStatus MpResourceTopology::getNextVirtualOutput(VirtualPortIterator &portIter,
                                                  UtlString& realResourceName,
                                                  int &realPortIndex,
                                                  UtlString& virtualResourceName,
                                                  int &virtualPortIndex)
{
   // Implementation is the same as for getNextVirtualInput().
   // Only difference between input and output iterator is in its construction.
   return getNextVirtualInput(portIter, realResourceName, realPortIndex,
                              virtualResourceName, virtualPortIndex);
}

int MpResourceTopology::dumpResources(UtlString& dumpString)
{
    dumpString.remove(0);
    int resourceCount = 0;

    UtlString resourceType; 
    UtlString resourceName;
    MpConnectionID connectionId;
    int streamId;

    while(getResource(resourceCount, resourceType, resourceName, connectionId, streamId) == OS_SUCCESS)
    {
        dumpString.appendFormat("Resource[%d] type=%s name=%s connectionId=%d streamId=%d\n", 
                                resourceCount, resourceType.data(), resourceName.data(), connectionId, streamId);
        
        resourceCount++;
    }

    return(resourceCount);
}

int MpResourceTopology::dumpConnections(UtlString& dumpString)
{
    dumpString.remove(0);
    int connectionCount = 0;

    UtlString outputResourceName;
    int outputPortIndex;
    UtlString inputResourceName;
    int inputPortIndex;

    while(getConnection(connectionCount, outputResourceName, outputPortIndex, inputResourceName, inputPortIndex) == OS_SUCCESS)
    {
        dumpString.appendFormat("connection[%d] (out) %s[%d] => (in) %s[%d]\n", 
                                connectionCount, outputResourceName.data(), outputPortIndex, inputResourceName.data(), inputPortIndex);
        connectionCount++;
    }

    return(connectionCount);
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

int MpResourceTopology::findResourceConnections(const UtlString& resourceName,
                                                UtlContainer& connections) const
{
   UtlDListIterator iterator(mConnections);
   MpResourceConnectionDefinition* connectionDef = NULL;
   int connectionsFound = 0;
//   printf("   findResourceConnections(%s)\n", resourceName.data());
   while((connectionDef = (MpResourceConnectionDefinition*) iterator()))
   {
      // if the connections input or output resource match
      if(resourceName.compareTo(*connectionDef) == 0 ||
         resourceName.compareTo(connectionDef->mInputResourceName) == 0)
      {
//         printf("      %s(%d) -> %s(%d)\n", connectionDef->data(), connectionDef->mOutputPortIndex,
//                connectionDef->mInputResourceName.data(), connectionDef->mInputPortIndex);
         connections.insert(connectionDef);
         connectionsFound++;
      }  
   }
   return connectionsFound;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

