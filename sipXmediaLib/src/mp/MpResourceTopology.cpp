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
#include <utl/UtlString.h>
#include <utl/UtlDListIterator.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlHashBagIterator.h>
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
        const UtlString& resourceName) :
    UtlString(resourceName),
        mResourceType(resourceType)
    {
    };

    virtual ~MpResourceDefinition(){};

    UtlString mResourceType;

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
                                   int inputPortIndex) :
    UtlString(outputResourceName),
    mInputResourceName(inputResourceName),
    mOutputPortIndex(outputPortIndex),
    mInputPortIndex(inputPortIndex)
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
}

/* ============================ MANIPULATORS ============================== */
OsStatus MpResourceTopology::addResource(const UtlString& resourceType,
                                         const UtlString& resourceName)
{
    OsStatus result;

    if(mResources.find(&resourceName))
    {
        result = OS_NAME_IN_USE;
    }
    else
    {
        mResources.append(new MpResourceDefinition(resourceType, resourceName));
        result = OS_SUCCESS;
    }
    return(result);
}

    /// Add a new connection definition to the topology
OsStatus MpResourceTopology::addConnection(const UtlString& outputResourceName,
                                           int outputPortIndex,
                                           const UtlString& inputResourceName,
                                           int inputPortIndex)
{
    mConnections.append(new MpResourceConnectionDefinition(outputResourceName,
                                                           outputPortIndex,
                                                           inputResourceName,
                                                           inputPortIndex));
    return(OS_SUCCESS);
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
        // at one resource following its connections and the resouces
        // connected by those connections.  As we find connections to
        // resources we will eliminate the resources from the danglingResource
        // list.  At the end after we have pursued all the connections
        // and resources left in the dangling list are not connection.
        // There should be no dangling resources in a full topology
        // definition, hense an invalid topology if there are resources
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
                // If the output resource for the connection is still in the list
                // add its connections to the connection list to traverse and
                // reomve the resource from the list.
                if(danglingResources.remove(connectionDef))
                {
                    findResourceConnections(*connectionDef, connectionsToTraverse);
                }

                //  Do the same for the input resource for this connection
                if(danglingResources.remove(&(connectionDef->mInputResourceName)))
                {
                    findResourceConnections(connectionDef->mInputResourceName,
                                            connectionsToTraverse);
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

    return(result);
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
    return(result);
}

/* ============================ ACCESSORS ================================= */

OsStatus MpResourceTopology::getResource(int resourceIndex,
                                         UtlString& resourceType,
                                         UtlString& resourceName) const
{
    MpResourceDefinition* resourceDef =
        (MpResourceDefinition*) mResources.at(resourceIndex);
    OsStatus result;
    if(resourceDef)
    {
        result = OS_SUCCESS;
        resourceName = *resourceDef;
        resourceType = resourceDef->mResourceType;
    }
    else
    {
        result = OS_NOT_FOUND;
    }
    return(result);
}

    /// Get the connection definition indicated by the connectionIndex
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
    return((--mPriorLogicalPort));
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

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

int MpResourceTopology::findResourceConnections(const UtlString& resourceName,
                                                UtlContainer& connections) const
{
    UtlDListIterator iterator(mConnections);
    MpResourceConnectionDefinition* connectionDef = NULL;
    int connectionsFound = 0;
    while((connectionDef = (MpResourceConnectionDefinition*) iterator()))
    {
        // if the connections input or output resource match
        if(resourceName.compareTo(*connectionDef) == 0 ||
           resourceName.compareTo(connectionDef->mInputResourceName) == 0)
        {
            connections.insert(connectionDef);
            connectionsFound++;
        }  
    }
    return(connectionsFound);
}

/* ============================ FUNCTIONS ================================= */

