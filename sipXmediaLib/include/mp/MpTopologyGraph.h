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

#ifndef _MpTopologyGraph_h_
#define _MpTopologyGraph_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpFlowGraphBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpResourceTopology;
class MpResourceFactory;

/**
*  @brief Flowgraph with resources wired as defined in given topology and factory.
*
*  The MpTopologyGraph is a MpFlowGraphBase which is constructed with resources
*  connected as defined by the given MpResourceTopology.  The resources are
*  constructed using the given MpResourceFactory.  This allows for a flexible
*  construction of flowgraphs containing different resources, connected in
*  a custom (as opposed to hardcoded) graph topology.
*
*  To keep this flowgraph independent of specific resources and topologies
*  all operations on existing resources must be performed via message passing
*  (the message queue is obtained via getMsgQ).  The messages are either handled
*  by the flowgraph or dispatched to the resource named in the message.  See
*  the specific resource for the types of messages and operations that can be
*  performed on the resource.  See MpResource for more information on the
*  message format and performing operations on resources using message passing.
*/
class MpTopologyGraph : public MpFlowGraphBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpTopologyGraph(int samplesPerFrame, 
                   int samplesPerSec,
                   MpResourceTopology& initialResourceTopology,
                   MpResourceFactory& resourceFactory);

     /// Destructor
   virtual ~MpTopologyGraph();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @brief Add resource to the existing flowgraph as defined by given
     /// topology and optional factory.
   OsStatus addResources(MpResourceTopology& incrementalTopology,
                         MpResourceFactory* resourceFactory,
                         int resourceInstanceId);
     /**<
     *  If the resourceFactory is NULL, the factory provided when constructing this
     *  flowgraph is used as the default factory.
     *
     *  @param incrementalTopology - defines the resources to be added to the
     *         flowgraph and the in which they are connected.
     *  @param resourceFactory - factory to construct the resources added named in the incrementalTopology
     *  @param resourceInstanceId - instance ID to be used to make resource names
     *         unique in the flowgraph.
     */


     /// Extended processNextFrame() for diagnostic reasons.
   virtual OsStatus processNextFrame();

     /// Informs the flow graph that it now has the MpMediaTask focus.
   virtual OsStatus gainFocus();
     /**<
     *  Only the flow graph that has the focus is permitted to access
     *  the audio hardware.  This may only be called if this flow graph
     *  is managed and started!
     *
     *  @returns  OS_SUCCESS, always
     */

     /// Informs the flow graph that it has lost the MpMediaTask focus.
   virtual OsStatus loseFocus();
     /**<
     *  Only the flow graph that has the focus is permitted to access
     *  the audio hardware.  This should only be called if this flow graph
     *  is managed and started!
     *
     *  @returns  OS_SUCCESS, always
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Post a message to be handled by this flowgraph.
   virtual OsStatus postMessage(const MpFlowGraphMsg& message,
                                const OsTime& waitTime = OsTime::NO_WAIT_TIME);


     /// Handle a message for this flowgraph.
   virtual UtlBoolean handleMessage(OsMsg& message);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpResourceFactory* mpResourceFactory;

     /// Adds all new resources defined in a topology.
   int addTopologyResources(MpResourceTopology& resourceTopology,
                            MpResourceFactory& resourceFactory,
                            UtlHashBag& newResources,
                            UtlBoolean replaceNumInName = FALSE,
                            int resourceNum = -1);

     /// Adds links defined for resources in resource topology.
   int linkTopologyResources(MpResourceTopology& resourceTopology,
                             UtlHashBag& newResources,
                             UtlBoolean replaceNumInName = FALSE,
                             int resourceNum = -1);


     ///Disabled copy constructor.
   MpTopologyGraph(const MpTopologyGraph& rMpTopologyGraph);


     /// Disable assignment operator.
   MpTopologyGraph& operator=(const MpTopologyGraph& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpTopologyGraph_h_
