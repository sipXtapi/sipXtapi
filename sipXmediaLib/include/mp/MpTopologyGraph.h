//  
// Copyright (C) 2006-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006-2008 SIPez LLC. 
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
                   MpResourceFactory& resourceFactory,
                   OsMsgDispatcher *pNotifDispatcher = NULL);

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
     *  @param[in] incrementalTopology - defines the resources to be added to the
     *             flowgraph and the order in which they are connected.
     *  @param[in] resourceFactory - factory to construct the resources added named
     *             in the incrementalTopology
     *  @param[in] resourceInstanceId - instance ID to be used to make resource names
     *             unique in the flowgraph.
     *
     *  @returns OS_SUCCESS always.
     */


     /// @brief Delete resources from existing flowgraph as defined by
     /// given topology.
   OsStatus destroyResources(MpResourceTopology& resourceTopology,
                            int resourceInstanceId);
     /**<
     *  @warning At the moment \p resourceTopology must not contain "paired"
     *           resources, that is resource, whose constructor returns two or
     *           more resource instances.
     *
     *  @param[in] incrementalTopology - defines the resources to be deleted from
     *             the flowgraph.
     *  @param[in] resourceInstanceId - instance ID used to make resource names
     *             unique in the flowgraph.
     *
     *  @returns OS_SUCCESS always.
     */

     /// Extended processNextFrame() for diagnostic reasons.
   virtual OsStatus processNextFrame();

     /// @copydoc MpFlowGraphBase::gainFocus()
   virtual OsStatus gainFocus();

     /// @copydoc MpFlowGraphBase::loseFocus()
   virtual OsStatus loseFocus();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @copydoc MpResource::getType()
   MpFlowGraphBase::FlowGraphType getType();

     /// Lookup real input port corresponding to given virtual one.
   OsStatus lookupVirtualInput(const UtlString& virtualName,
                               int virtualPortIdx,
                               MpResource*& rpResource,
                               int &portIdx);
     /**<
     *  @param[in]  virtualName - resource name of the virtual input port.
     *  @param[in]  virtualPortIdx - port index of the virtual input port.
     *  @param[out] rpResource - pointer to a resource, corresponding to the
     *              virtual port. Value is undefined if virtual port is not
     *              found.
     *  @param[out] portIdx - input port index on \p rpResource corresponding
     *              to the virtual port. Value is undefined if virtual port is
     *              not found.
     *
     *  @retval OS_SUCCESS if virtual port is found.
     *  @retval OS_NOT_FOUND if virtual resource is not found.
     */

     /// Lookup real output port corresponding to given virtual one.
   OsStatus lookupVirtualOutput(const UtlString& virtualName,
                                int virtualPortIdx,
                                MpResource*& rpResource,
                                int &portIdx);
     /**<
     *  @param[in]  virtualName - resource name of the virtual output port.
     *  @param[in]  virtualPortIdx - port index of the virtual output port.
     *  @param[out] rpResource - pointer to a resource, corresponding to the
     *              virtual port. Value is undefined if virtual port is not
     *              found.
     *  @param[out] portIdx - output port index on \p rpResource corresponding
     *              to the virtual port. Value is undefined if virtual port is
     *              not found.
     *
     *  @retval OS_SUCCESS if virtual port is found.
     *  @retval OS_NOT_FOUND if virtual resource is not found.
     */

     /// Lookup real or virtual input port.
   OsStatus lookupInput(const UtlString& resourceName,
                        int portIdx,
                        MpResource*& pFoundResource,
                        int &foundPortIdx);
     /**<
     *  This method tries to search for real resource with given name. If real
     *  resource is not found it then tries to search for virtual port with
     *  given name/portIdx pair.
     *
     *  @param[in]  resourceName - resource name to look for.
     *  @param[in]  portIdx - input port index to look for.
     *  @param[out] pFoundResource - found resource. Value is undefined if port
     *              is not found.
     *  @param[out] foundPortIdx - found port index. Value is undefined if port
     *              is not found.
     *
     *  @retval OS_SUCCESS if port is found.
     *  @retval OS_NOT_FOUND if port is not found.
     */

     /// Lookup real or virtual output port.
   OsStatus lookupOutput(const UtlString& resourceName,
                         int portIdx,
                         MpResource*& pFoundResource,
                         int &foundPortIdx);
     /**<
     *  This method tries to search for real resource with given name. If real
     *  resource is not found it then tries to search for virtual port with
     *  given name/portIdx pair.
     *
     *  @param[in]  resourceName - resource name to look for.
     *  @param[in]  portIdx - output port index to look for.
     *  @param[out] pFoundResource - found resource. Value is undefined if port
     *              is not found.
     *  @param[out] foundPortIdx - found port index. Value is undefined if port
     *              is not found.
     *
     *  @retval OS_SUCCESS if port is found.
     *  @retval OS_NOT_FOUND if port is not found.
     */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Post a message to be handled by this flowgraph.
   virtual OsStatus postMessage(const MpFlowGraphMsg& message,
                                const OsTime& waitTime = OsTime::OS_INFINITY);


     /// Handle a message for this flowgraph.
   virtual UtlBoolean handleMessage(OsMsg& message);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpResourceFactory* mpResourceFactory; ///< Factory for resources.
   UtlHashMap         mVirtualInputs;    ///< Virtual inputs mapping to real inputs.
   UtlHashMap         mVirtualOutputs;   ///< Virtual outputs mapping to real outputs.

     /// Adds all new resources defined in a topology.
   int addTopologyResources(MpResourceTopology& resourceTopology,
                            MpResourceFactory& resourceFactory,
                            UtlHashBag& newResources,
                            UtlBoolean replaceNumInName = FALSE,
                            int resourceNum = -1);

     /// Adds all virtual inputs defined in a topology.
   int addVirtualInputs(MpResourceTopology& resourceTopology,
                        UtlHashBag& newResources,
                        UtlBoolean replaceNumInName = FALSE,
                        int resourceNum = -1);

     /// Remove all virtual inputs defined in a topology from the flowgraph.
   int removeVirtualInputs(MpResourceTopology& resourceTopology,
                           UtlBoolean replaceNumInName,
                           int resourceNum);

     /// Adds all virtual outputs defined in a topology.
   int addVirtualOutputs(MpResourceTopology& resourceTopology,
                         UtlHashBag& newResources,
                         UtlBoolean replaceNumInName = FALSE,
                         int resourceNum = -1);

     /// Remove all virtual outputs defined in a topology from the flowgraph.
   int removeVirtualOutputs(MpResourceTopology& resourceTopology,
                            UtlBoolean replaceNumInName,
                            int resourceNum);

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
