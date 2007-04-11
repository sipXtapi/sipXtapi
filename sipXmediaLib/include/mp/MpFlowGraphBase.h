//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpFlowGraphBase_h_
#define _MpFlowGraphBase_h_

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <utl/UtlHashMap.h>

#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include "os/OsStatus.h"
#include "os/OsRWMutex.h"
#include "os/OsTime.h"
#include "mp/MpResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpFlowGraphMsg;
class OsMsg;

/**
*  @brief Flow graph for coordinating the execution of media processing resources.
*
*  The media processing for a call is modeled as a directed graph of media
*  processing resources (e.g., dejitter, mixer, encode, decode).
* 
*  <H3>Frame Interval Processing</H3>
*  The media processing subsystem works on blocks of data.  Every frame
*  processing interval (typically 10 milliseconds), one frame interval's
*  worth of media is processed.<br>
*  <br>
*  At the start of the interval, the media task calls the processNextFrame()
*  method for the flow graph.  This method first calls processMessages() to
*  handle any messages that have been posted to the flow graph.  Next, if
*  any resources or links have been added or removed since the last
*  frame processing interval, the computeOrder() method is invoked.
*  computeOrder() performs a topological sort on the resources in the flow
*  graph to determine the correct resource execution order.  This is done to
*  ensure that resources producing output buffers are executed before other
*  resources in the flow graph that expect to consume those buffers.  Once
*  the execution order has been determined, the @link MpResource::processFrame()
*  processFrame() @endlink method for each of the resources in the flow graph is
*  executed.
* 
*  <H3>Stopped vs. Started States</H3>
*  A flow graph must be in the @link MpFlowGraphBase::STARTED STARTED @endlink
*  state in order to process media streams.  For safety, the methods that modify
*  the state of the flow graph only take effect when the flow graph is in a
*  quiescent state.  If the flow graph is in the @link MpFlowGraphBase::STOPPED
*  STOPPED @endlink state, such methods take effect immediately.  However, when
*  the flow graph is in the @link MpFlowGraphBase::STARTED STARTED @endlink
*  state, changes take effect at the start of the next frame processing interval.
* 
*  <H3>Synchronization</H3>
*  A reader/writer lock is used to seialize access to the object's internal
*  data.  A write lock is taken before executing any method that changes the
*  object's internal state or posts a message to the flow graph's message
*  queue.  A read lock is taken before invoking any method that reads the
*  object's internal state information.  When reading internal state that
*  is stored in 32 bits or less, the read lock is not acquired since such
*  reads are assumed to be atomic.
*/
class MpFlowGraphBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   friend OsStatus MpResource::postMessage(MpFlowGraphMsg& rMsg);

   typedef enum
   {
      STARTED,   ///< flow graph has been started (is processing media streams)
      STOPPED    ///< flow graph is stopped (not processing media streams)
   } FlowGraphState;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpFlowGraphBase(int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MpFlowGraphBase();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// Creates a link between resources.
   OsStatus addLink(MpResource& rFrom, int outPortIdx,
                    MpResource& rTo,   int inPortIdx);
     /**<
     *  Creates a link between the <i>outPortIdx</i> port of the 
     *  <i>rFrom</i> resource to the <i>inPortIdx</i> port of the <i>rTo</i> 
     *  resource.
     *
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @returns <b>OS_SUCCESS</b> - successfully added the new link
     *  @returns <b>OS_INVALID_ARGUMENT</b> - invalid port index
     *  @returns <b>OS_UNSPECIFIED</b> - add link attempt failed
     */

     /// Adds the indicated media processing object to the flow graph.
   OsStatus addResource(MpResource& rResource, UtlBoolean makeNameUnique=TRUE);
     /**<
     *  If <i>makeNameUnique</i> is TRUE, then if a resource with the same name 
     *  already exists in the flow graph, the name for <i>rResource</i> will 
     *  be changed (by adding a numeric suffix) to make it unique within the 
     *  flow graph.
     *
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @returns <b>OS_SUCCESS</b> - success
     *  @returns <b>OS_UNSPECIFIED</b> - add resource attempt failed
     */

     /// @brief Stops the flow graph, removes all of the resources in the flow
     /// graph and destroys them.
   OsStatus destroyResources(void);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *  @returns <b>OS_SUCCESS</b> - for now, this method always returns success
     */

     /// Invokes the <i>disable()</i> method for each resource in the flow graph.
   OsStatus disable(void);
     /**<
     *  Resources must be enabled before they will perform any meaningful
     *  processing on the media stream.
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *  @returns <b>OS_SUCCESS</b> - for now, this method always returns success
     */

     /// Invokes the <i>enable()</i> method for each resource in the flow graph.
   OsStatus enable(void);
     /**<
     *  Resources must be enabled before they will perform any meaningful
     *  processing on the media stream.
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *  @returns <b>OS_SUCCESS</b> - for now, this method always returns success
     */

     /// Informs the flow graph that it now has the MpMediaTask focus.
   virtual OsStatus gainFocus(void);
     /**<
     *  Only the flow graph that has the focus is permitted to access
     *  the audio hardware.
     *  @returns <b>OS_INVALID</b> - only a MpBasicFlowGraph can have focus.
     */

     /// @brief Inserts <i>rResource</i> into the flow graph downstream of the
     /// designated <i>rUpstreamResource</i> resource.
   OsStatus insertResourceAfter(MpResource& rResource,
                                MpResource& rUpstreamResource,
                                int outPortIdx);
     /**<
     *  The new resource will be inserted on the <i>outPortIdx</i> output
     *  link of <i>rUpstreamResource</i>.
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *  @returns <b>OS_SUCCESS</b> - success
     *  @returns <b>OS_INVALID_ARGUMENT</b> - invalid port index
     */

     /// Inserts <i>rResource</i> into the flow graph upstream of the
     /// designated <i>rDownstreamResource</i> resource.
   OsStatus insertResourceBefore(MpResource& rResource,
                                 MpResource& rDownstreamResource,
                                 int inPortIdx);
     /**<
     *  The new resource will be inserted on the <i>inPortIdx</i> input
     *  link of <i>rDownstreamResource</i>.
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *  @returns <b>OS_SUCCESS</b> - success
     *  @returns <b>OS_INVALID_ARGUMENT</b> - invalid port index
     */

     /// Informs the flow graph that it has lost the MpMediaTask focus.
   virtual OsStatus loseFocus(void);
     /**<
     *  Only the flow graph that has the focus is permitted to access
     *  the audio hardware.
     *  @returns <b>OS_INVALID</b> - only a MpBasicFlowGraph can have focus.
     */

     /// Processes the next frame interval's worth of media for the flow graph.
   virtual OsStatus processNextFrame(void);
     /**<
     *  @returns <b>OS_SUCCESS</b> - for now, this method always returns success
     */

     /// @brief Removes the link between the <i>outPortIdx</i> port of the
     /// <i>rFrom</i> resource and its downstream counterpart.
   OsStatus removeLink(MpResource& rFrom, int outPortIdx);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *  @returns <b>OS_SUCCESS</b> - link has been removed
     *  @returns <b>OS_INVALID_ARGUMENT</b> - invalid port index
     */

     /// Removes the indicated media processing object from the flow graph.
   OsStatus removeResource(MpResource& rResource);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *  @returns <b>OS_SUCCESS</b> - success, resource has been removed
     *  @returns <b>OS_UNSPECIFIED</b> - remove resource operation failed
     */

     /// Sets the number of samples expected per frame.
   OsStatus setSamplesPerFrame(int samplesPerFrame);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *  @returns <b>OS_SUCCESS</b> - success
     *  @returns <b>OS_INVALID_ARGUMENT</b> - specified duration is not supported
     */

     /// Sets the number of samples expected per second.
   OsStatus setSamplesPerSec(int samplesPerSec);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *  @returns <b>OS_SUCCESS</b> - success
     *  @returns <b>OS_INVALID_ARGUMENT</b> - specified duration is not supported
     */

     /// Start this flow graph.
   OsStatus start(void);
     /**<
     *  A flow graph must be "started" before it will process media streams.
     *  This call takes effect immediately.
     *  @returns <b>OS_SUCCESS</b> - for now this method always returns success
     */

     /// Stop this flow graph.
   OsStatus stop(void);
     /**<
     *  Stop processing media streams with this flow graph.  This call takes
     *  effect at the start of the next frame processing interval.
     *  @returns <b>OS_SUCCESS</b> - for now this method always returns success
     */

//@}

     /// sends a message to self, and waits for reply.
   void synchronize(const char* tag=NULL, int val=0);

/* ============================ ACCESSORS ================================= */

     /// Displays information on the console about the specified flow graph.
   static void flowGraphInfo(MpFlowGraphBase* pFlowGraph);

///@name Accessors
//@{

     /// Returns the number of samples expected per frame.
   int getSamplesPerFrame(void) const;

     /// Returns the number of samples expected per second.
   int getSamplesPerSec(void) const;

     /// Returns the current state of flow graph.
   int getState(void) const;

     /// @brief Sets <i>rpResource</i> to point to the resource that corresponds
     /// to <i>name</i> or to NULL if no matching resource is found.
   OsStatus lookupResource(const UtlString& name,
                           MpResource*& rpResource);
     /**<
     *  @param name - the name of the resource to look up.
     *  @param rpResource - [out] the resource pointer to store the resource in.
     *  @returns <b>OS_SUCCESS</b> - success
     *  @returns <b>OS_NOT_FOUND</b> - no resource with the specified name
     *           rpResource is NULL if OS_NOT_FOUND.
     */

     /// Returns the number of links in the flow graph.
   int numLinks(void) const;

     /// Returns the number of frames this flow graph has processed.
   int numFramesProcessed(void) const;

     /// Returns the number of resources in the flow graph.
   int numResources(void) const;

     /// Returns the message queue used by the flow graph. 
   OsMsgQ* getMsgQ(void) ;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// Is flow graph in @link MpFlowGraphBase::STARTED STARTED @endlink state.
   UtlBoolean isStarted(void) const;
     /**<
     *  @returns <b>TRUE</b> - if the flow graph has been starte,
     *  @returns <b>FALSE</b> - otherwise.
     */

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsRWMutex        mRWMutex;      // reader/writer lock for synchronization

     /// Posts a message to this flow graph.
   virtual OsStatus postMessage(const MpFlowGraphMsg& rMsg,
                                const OsTime& rTimeout=OsTime::NO_WAIT_TIME);
     /**<
     *  Returns the result of the message send operation.
     */

     /// Handles an incoming message for the flow graph.
   virtual UtlBoolean handleMessage(OsMsg& rMsg);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_REMOVE_LINK FLOWGRAPH_REMOVE_LINK @endlink message.
   UtlBoolean handleRemoveLink(MpResource* pFrom, int outPortIdx);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_REMOVE_RESOURCE FLOWGRAPH_REMOVE_RESOURCE @endlink message.
   UtlBoolean handleRemoveResource(MpResource* pResource);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum { MAX_FLOWGRAPH_MESSAGES  = 150};
   enum { MAX_FLOWGRAPH_RESOURCES = 50};

   UtlHashMap mResourceDict;   ///< resource dictionary
   MpResource* mExecOrder[MAX_FLOWGRAPH_RESOURCES]; ///< resource execution order
   MpResource* mUnsorted[MAX_FLOWGRAPH_RESOURCES];  ///< unsorted resources
   int       mCurState;        ///< current flow graph state
   OsMsgQ    mMessages;        ///< message queue for this flow graph
   int       mPeriodCnt;       ///< number of frames processed by this flow graph
   int       mLinkCnt;         ///< number of links in this flow graph
   int       mResourceCnt;     ///< number of resources in this flow graph
   UtlBoolean mRecomputeOrder; ///< TRUE ==> the execution order needs computing
   int       mSamplesPerFrame; ///< number of samples per frame
   int       mSamplesPerSec;   ///< number of samples per second
   MpResource* mpResourceInProcess; ///< @brief For debugging, keep track of what
                                    ///< resource we are working on in
                                    ///< processNextFrame().

     /// @brief Computes the execution order for the flow graph by performing a 
     /// topological sort on the resource graph.
   OsStatus computeOrder(void);
     /**<
     *  @returns <b>OS_SUCCESS</b> - successfully computed an execution order
     *  @returns <b>OS_LOOP_DETECTED</b> - detected a loop in the flow graph
     */

     /// @brief Disconnects all inputs (and the corresponding upstream outputs)
     /// for the indicated resource. 
   UtlBoolean disconnectAllInputs(MpResource* pResource);
     /**<
     *  @returns <b>TRUE</b> - if successful,
     *  @returns <b>FALSE</b> - otherwise.
     */

     /// Disconnects all outputs (and the corresponding downstream inputs) for 
     /// the indicated resource.
   UtlBoolean disconnectAllOutputs(MpResource* pResource);
     /**<
     *  @returns <b>TRUE</b> - if successful,
     *  @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_ADD_LINK FLOWGRAPH_ADD_LINK @endlink message.
   UtlBoolean handleAddLink(MpResource* pFrom, int outPortIdx,
                            MpResource* pTo,   int inPortIdx);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_ADD_RESOURCE FLOWGRAPH_ADD_RESOURCE @endlink message.
   UtlBoolean handleAddResource(MpResource* pResource,
                               UtlBoolean makeNameUnique);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_DESTROY_RESOURCES FLOWGRAPH_DESTROY_RESOURCES @endlink message.
   UtlBoolean handleDestroyResources(void);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_DISABLE FLOWGRAPH_DISABLE @endlink message.
   UtlBoolean handleDisable(void);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_ENABLE FLOWGRAPH_ENABLE @endlink message.
   UtlBoolean handleEnable(void);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_SET_SAMPLES_PER_FRAME FLOWGRAPH_SET_SAMPLES_PER_FRAME @endlink message.
   UtlBoolean handleSetSamplesPerFrame(int samplesPerFrame);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_SET_SAMPLES_PER_SEC FLOWGRAPH_SET_SAMPLES_PER_SEC @endlink message.
   UtlBoolean handleSetSamplesPerSec(int samplesPerSec);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_START FLOWGRAPH_START @endlink message.
   UtlBoolean handleStart(void);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_STOP FLOWGRAPH_STOP @endlink message.
   UtlBoolean handleStop(void);
     /**<
     * @returns <b>TRUE</b> - if the message was handled,
     * @returns <b>FALSE</b> - otherwise.
     */

     /// @brief Sets <i>rpResource</i> to point to the resource that corresponds
     /// to <i>name</i> or to NULL if no matching resource is found.
   OsStatus lookupResourcePrivate(const UtlString& name,
                                  MpResource*& rpResource);
     /**<
     *  Does a lookup of name->resource, and sets \c rpResource to
     *  point to the resource that corresponds to \c name, or to 
     *  \c NULL if no matching resource is found.
     *  This private version does no locking.
     *  @param name - the name of the resource to look up.
     *  @param rpResource - [out] the resource pointer to store the resource in.
     *  @returns <b>OS_SUCCESS</b> - success
     *  @returns <b>OS_NOT_FOUND</b> - no resource with the specified name
     *           rpResource is NULL if OS_NOT_FOUND.
     */

     /// Processes all of the messages currently queued for this flow graph.
   OsStatus processMessages(void);
     /**<
     *  For now, this method always returns OS_SUCCESS.
     */

     /// Copy constructor (not implemented for this task)
   MpFlowGraphBase(const MpFlowGraphBase& rMpFlowGraph);

     /// Assignment operator (not implemented for this task)
   MpFlowGraphBase& operator=(const MpFlowGraphBase& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpFlowGraphBase_h_
