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

#ifndef _MpResourceTopology_h_
#define _MpResourceTopology_h_

// SYSTEM INCLUDES
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include <utl/UtlDList.h>

// APPLICATION INCLUDES
// DEFINES
#define DEFAULT_FROM_FILE_RESOURCE_NAME "FromFile1"
#define DEFAULT_FROM_INPUT_DEVICE_RESOURCE_NAME "FromMic1"
#define DEFAULT_BRIDGE_RESOURCE_NAME "Bridge1"
#define DEFAULT_TONE_GEN_RESOURCE_NAME "ToneGen1"
#define DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME "ToSpeaker1"
#define DEFAULT_NULL_RESOURCE_NAME "Null1"
#define DEFAULT_RTP_INPUT_RESOURCE_NAME "InRtp-%d"
#define DEFAULT_RTP_OUTPUT_RESOURCE_NAME "OutRtp-%d"
#define DEFAULT_BUFFER_RECORDER_RESOURCE_NAME "BufferRecorder1"
#define DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME "ToOutputSplitter1"
#define DEFAULT_AEC_RESOURCE_NAME "Aec1"

#define AEC_OUTPUT_BUFFER_RESOURCE_NAME_SUFFIX "-outBuffer"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpResourceFactory;

/**
*  @brief MpResourceTopology is used to define a set of resources and how they
*         are to be connected in a flowgraph.
*
*  MpResourceTopology is defines a list of resources to be created and a list of
*  connections defining how the resources are to be connected in a flowgraph.
*  There are two types of MpResourceTopologies that may be defined:
*  
*  1) Full topologies which define all the resources and connections for a
*  new, empty flowgraph.  In a full topology the connections must reference
*  only resources defined in the MpResourceTopology.
*
*  2) Incremental topologies which define a set of resources to be added to
*  a flowgraph and a set of connections defining how the new resources are
*  connected amongst themselves and the existing resources in the flowgraph.
*  An incremental topology the connections may reference resources which
*  already exist in the flowgraph or resources defined in the MpResourceTopology.
*
*  The MpResourceTopology only defines the resources and connections to
*  be created.  The actual creation and linking of resources is performed
*  by the flowgraph itself.  The flowgraph uses a MpResourceFactory to
*  create the resources listed in the MpResourceTopology.  The list of
*  resources in the MpResourceTopology are consist of pairs of a resource
*  type and name.  The type is used by the flowgraph to instruct the
*  MpResourceFactory to create a specific resource type.  The name is
*  the name the resource is to assume in the flowgraph.
*
*/
class MpResourceTopology
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum MpLogicalPortTypes
    {
        MP_TOPOLOGY_NEXT_AVAILABLE_PORT = -1
    };

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MpResourceTopology();

    /** Destructor
     */
    virtual ~MpResourceTopology();

/* ============================ MANIPULATORS ============================== */

    /// Add a new resource definition to the topology
    /** 
      * @param resourceType - type for MpResourceFactory construction
      * @param resourceName - unique name for resource in this topology and
      *        ultimately in flowgraph.
      * @returns OS_NAME_IN_USE if resourceName already exists in this topology
      */
    OsStatus addResource(const UtlString& resourceType,
                         const UtlString& resourceName);

    /// Add a new connection definition to the topology
    OsStatus addConnection(const UtlString& outputResourceName,
                           int outputPortIndex,
                           const UtlString& inputResourceName,
                           int inputPortIndex);

    /// Validate that the connections are connect the resources
    /**
      * a full topology MUST NOT have connections that reference resources
      * outside this topology AND all resource MUST be connected to such that
      * there are no dangling resources.  An incremental topology MAY have
      * connections that reference resources outside of this topology AND
      * all resources in this topology MUST have at least one connection.
      *
      * @param firstUnconnectedResourceName - first resource found to not
      *        have any connections.
      * @param firstDanglingResourceName - first resource found to not be
      *        connected topologically to the rest of the resources.  The
      *        can only be tested in full topologies.  We do not know the
      *        pre-existing topology in the flowgraph for incremental topologies.
      * @param allowExternalResources - FALSE = full topology, TRUE = increment
      *        topology (see definitions at top of this class)
      */
    OsStatus validateConnections(UtlString& firstUnconnectedResourceName,
                                 UtlString&  firstDanglingResourceName,
                                 UtlBoolean allowExternalResources = FALSE) const;

    /// Validate that all the resource types in this topology have a constructor in the factory
    OsStatus validateResourceTypes(MpResourceFactory& resourceFactory,
                                   int& firstInvalidResourceIndex) const;


    /// Utility to replace "%d" in resource name with an integer
    static void replaceNumInName(UtlString& resourceName,
                          int resourceNum);

/* ============================ ACCESSORS ================================= */

    /// Get the resource definition indicated by the resourceIndex
    OsStatus getResource(int resourceIndex,
                         UtlString& resourceType,
                         UtlString& resourceName) const;

    /// Get the connection definition indicated by the connectionIndex
    OsStatus getConnection(int connectionIndex,
                           UtlString& outputResourceName,
                           int& outputPortIndex,
                           UtlString& inputResourceName,
                           int& inputPortIndex);

    /// Get the next logical port, used when the same port number
    /** 
      * used when more than one connection should refer to the same
      * port number, but the port number is not yet known (e.g. use
      * the next available port).
      */
    int getNextLogicalPortNumber();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    int findResourceConnections(const UtlString& resourceName,
                                UtlContainer& connectionsToTraverse) const;

    /** Disabled copy constructor
     */
    MpResourceTopology(const MpResourceTopology& rMpResourceTopology);


    /** Disable assignment operator
     */
    MpResourceTopology& operator=(const MpResourceTopology& rhs);

    
    int mPriorLogicalPort;
    UtlDList mResources;
    UtlDList mConnections;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResourceTopology_h_
