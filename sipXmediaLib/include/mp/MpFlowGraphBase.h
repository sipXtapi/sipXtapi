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
#include "MpNotificationDispatcher.h"

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
     *  Creates a link between the \p outPortIdx port of the 
     *  \p rFrom resource to the \p inPortIdx port of the \p rTo 
     *  resource.
     *
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - successfully added the new link.
     *  @retval OS_INVALID_ARGUMENT - invalid port index.
     *  @retval OS_UNSPECIFIED - add link attempt failed.
     */

     /// Adds a dispatcher for notifications to the flowgraph.
   OsStatus 
   addNotificationDispatcher(MpNotificationDispatcher* notifyDispatcher);
     /**<
     *  Adds a notification dispatcher to the flowgraph, for use in
     *  letting the resources or the flowgraph tell the application 
     *  any interesting events.
     *
     *  @retval OS_SUCCESS - successfully added the dispatcher.
     *  @retval OS_LIMIT_REACHED - if we have a dispatcher already.
     */

     /// Adds the indicated media processing object to the flow graph.
   OsStatus addResource(MpResource& rResource, UtlBoolean makeNameUnique=TRUE);
     /**<
     *  If \p makeNameUnique is TRUE, then if a resource with the same name 
     *  already exists in the flow graph, the name for \p rResource will 
     *  be changed (by adding a numeric suffix) to make it unique within the 
     *  flow graph.
     *
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - success.
     *  @retval OS_UNSPECIFIED - add resource attempt failed.
     */

     /// @brief Stops the flow graph, removes all of the resources in the flow
     /// graph and destroys them.
   OsStatus destroyResources(void);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - for now, this method always returns success.
     */

     /// @brief Invokes the \link MpResource::disable() disable()\endlink
     /// method for each resource in the flow graph.
   OsStatus disable(void);
     /**<
     *  Resources must be enabled before they will perform any meaningful
     *  processing on the media stream.
     *
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - for now, this method always returns success.
     */

     /// @brief Invokes the \link MpResource::enable() enable()\endlink
     /// method for each resource in the flow graph.
   OsStatus enable(void);
     /**<
     *  Resources must be enabled before they will perform any meaningful
     *  processing on the media stream.
     *
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - for now, this method always returns success.
     */

     /// Informs the flow graph that it now has the MpMediaTask focus.
   virtual OsStatus gainFocus(void);
     /**<
     *  Only the flow graph that has the focus is permitted to access
     *  the audio hardware.
     *
     *  @retval OS_INVALID - only a MpBasicFlowGraph can have focus.
     */

     /// @brief Inserts \p rResource into the flow graph downstream of the
     /// designated \p rUpstreamResource resource.
   OsStatus insertResourceAfter(MpResource& rResource,
                                MpResource& rUpstreamResource,
                                int outPortIdx);
     /**<
     *  The new resource will be inserted on the \p outPortIdx output
     *  link of \p rUpstreamResource.
     *
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - success.
     *  @retval OS_INVALID_ARGUMENT - invalid port index.
     */

     /// Inserts \p rResource into the flow graph upstream of the
     /// designated \p rDownstreamResource resource.
   OsStatus insertResourceBefore(MpResource& rResource,
                                 MpResource& rDownstreamResource,
                                 int inPortIdx);
     /**<
     *  The new resource will be inserted on the \p inPortIdx input
     *  link of \p rDownstreamResource.
     *
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - success.
     *  @retval OS_INVALID_ARGUMENT - invalid port index.
     */

     /// Informs the flow graph that it has lost the MpMediaTask focus.
   virtual OsStatus loseFocus(void);
     /**<
     *  Only the flow graph that has the focus is permitted to access
     *  the audio hardware.
     *
     *  @retval OS_INVALID - only a MpBasicFlowGraph can have focus.
     */

     /// @brief posts a resource notification message to the Notification dispatcher.
   virtual OsStatus postNotification(const MpResourceNotificationMsg& msg);
     /**<
     *  If there is a notification dispatcher, this message is posted
     *  to it.  Otherwise, the message is dropped.
     *
     *  @param[in] msg - the notification message to post to the dispatcher.
     *
     *  @retval OS_SUCCESS if the message was successfully added to the dispatcher.
     *  @retval OS_LIMIT_REACHED if the queue is full, and no more 
     *          messages can be accepted.
     *  @retval OS_NOT_FOUND if there's no dispatcher.
     */

     /// Processes the next frame interval's worth of media for the flow graph.
   virtual OsStatus processNextFrame(void);
     /**<
     *  @retval OS_SUCCESS - for now, this method always returns success
     */

     /// @brief Removes the link between the \p outPortIdx port of the
     /// \p rFrom resource and its downstream counterpart.
   OsStatus removeLink(MpResource& rFrom, int outPortIdx);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - link has been removed.
     *  @retval OS_INVALID_ARGUMENT - invalid port index.
     */

     /// Clears the pointer to notification dispatcher from the flow graph.
   OsStatus removeNotificationDispatcher();
     /**<
     *  Clears the pointer to the notification dispatcher held in the flowgraph.
     *  If there is no notification dispatcher set, then it returns failure.
     *
     *  @retval OS_SUCCESS - success, dispatcher has been removed.
     *  @retval OS_NOT_FOUND - no dispatcher to remove.
     */

     /// Removes the indicated media processing object from the flow graph.
   OsStatus removeResource(MpResource& rResource);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - success, resource has been removed.
     *  @retval OS_UNSPECIFIED - remove resource operation failed.
     */

     /// Sets the number of samples expected per frame.
   OsStatus setSamplesPerFrame(int samplesPerFrame);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - success.
     *  @retval OS_INVALID_ARGUMENT - specified duration is not supported.
     */

     /// Sets the number of samples expected per second.
   OsStatus setSamplesPerSec(int samplesPerSec);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - success.
     *  @retval OS_INVALID_ARGUMENT - specified duration is not supported.
     */

     /// Start this flow graph.
   OsStatus start(void);
     /**<
     *  A flow graph must be "started" before it will process media streams.
     *  This call takes effect immediately.
     *
     *  @retval OS_SUCCESS - for now this method always returns success.
     */

     /// Stop this flow graph.
   OsStatus stop(void);
     /**<
     *  Stop processing media streams with this flow graph.  This call takes
     *  effect at the start of the next frame processing interval.
     *
     *  @retval OS_SUCCESS - for now this method always returns success.
     */

//@}

     /// Sends a message to self, and waits for reply.
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

     /// @brief Sets \p rpResource to point to the resource that corresponds
     /// to \p name or to NULL if no matching resource is found.
   OsStatus lookupResource(const UtlString& name,
                           MpResource*& rpResource);
     /**<
     *  @param[in] name - the name of the resource to look up.
     *  @param[out] rpResource - the resource pointer to store the resource in.
     *
     *  @retval OS_SUCCESS - success
     *  @retval OS_NOT_FOUND - no resource with the specified name
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
     *  @retval TRUE if the flow graph has been started,
     *  @retval FALSE otherwise.
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
     * @retval TRUE if the message was handled.
     * @retval FALSE otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_REMOVE_LINK FLOWGRAPH_REMOVE_LINK @endlink message.
   UtlBoolean handleRemoveLink(MpResource* pFrom, int outPortIdx);
     /**<
     * @retval TRUE if the message was handled.
     * @retval FALSE otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_REMOVE_RESOURCE FLOWGRAPH_REMOVE_RESOURCE @endlink message.
   UtlBoolean handleRemoveResource(MpResource* pResource);
     /**<
     * @retval TRUE if the message was handled.
     * @retval FALSE otherwise.
     */

     /// Handle the FLOWGRAPH_SYNCHRONIZE message.
   UtlBoolean handleSynchronize(MpFlowGraphMsg& rMsg);
     /**<
     *  @retval TRUE if the message was handled.
     *  @retval FALSE otherwise.
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
   MpNotificationDispatcher* mNotifyDispatcher; ///< Dispatcher for notification messages
   int       mPeriodCnt;       ///< number of frames processed by this flow graph
   int       mLinkCnt;         ///< number of links in this flow graph
   int       mResourceCnt;     ///< number of resources in this flow graph
   UtlBoolean mRecomputeOrder; ///< TRUE ==> the execution order needs computing
   int       mSamplesPerFrame; ///< number of samples per frame
   int       mSamplesPerSec;   ///< number of samples per second
   MpResource* mpResourceInProcess; ///< @brief For debugging, keep track of what
                               ///< resource we are working on in processNextFrame().

     /// @brief Computes the execution order for the flow graph by performing a 
     /// topological sort on the resource graph.
   OsStatus computeOrder(void);
     /**<
     *  @retval OS_SUCCESS - successfully computed an execution order.
     *  @retval OS_LOOP_DETECTED - detected a loop in the flow graph.
     */

     /// @brief Disconnects all inputs (and the corresponding upstream outputs)
     /// for the indicated resource. 
   UtlBoolean disconnectAllInputs(MpResource* pResource);
     /**<
     *  @retval TRUE - if successful.
     *  @retval FALSE - otherwise.
     */

     /// @brief Disconnects all outputs (and the corresponding downstream inputs)
     /// for the indicated resource.
   UtlBoolean disconnectAllOutputs(MpResource* pResource);
     /**<
     *  @retval TRUE - if successful.
     *  @retval FALSE - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_ADD_LINK FLOWGRAPH_ADD_LINK @endlink message.
   UtlBoolean handleAddLink(MpResource* pFrom, int outPortIdx,
                            MpResource* pTo,   int inPortIdx);
     /**<
     * @retval TRUE - if the message was handled.
     * @retval FALSE - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_ADD_RESOURCE FLOWGRAPH_ADD_RESOURCE @endlink message.
   UtlBoolean handleAddResource(MpResource* pResource,
                               UtlBoolean makeNameUnique);
     /**<
     * @retval TRUE - if the message was handled.
     * @retval FALSE - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_DESTROY_RESOURCES FLOWGRAPH_DESTROY_RESOURCES @endlink message.
   UtlBoolean handleDestroyResources(void);
     /**<
     * @retval TRUE - if the message was handled.
     * @retval FALSE - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_DISABLE FLOWGRAPH_DISABLE @endlink message.
   UtlBoolean handleDisable(void);
     /**<
     * @retval TRUE - if the message was handled.
     * @retval FALSE - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_ENABLE FLOWGRAPH_ENABLE @endlink message.
   UtlBoolean handleEnable(void);
     /**<
     * @retval TRUE - if the message was handled.
     * @retval FALSE - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_SET_SAMPLES_PER_FRAME FLOWGRAPH_SET_SAMPLES_PER_FRAME @endlink message.
   UtlBoolean handleSetSamplesPerFrame(int samplesPerFrame);
     /**<
     * @retval TRUE - if the message was handled.
     * @retval FALSE - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_SET_SAMPLES_PER_SEC FLOWGRAPH_SET_SAMPLES_PER_SEC @endlink message.
   UtlBoolean handleSetSamplesPerSec(int samplesPerSec);
     /**<
     * @retval TRUE - if the message was handled.
     * @retval FALSE - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_START FLOWGRAPH_START @endlink message.
   UtlBoolean handleStart(void);
     /**<
     * @retval TRUE - if the message was handled.
     * @retval FALSE - otherwise.
     */

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_STOP FLOWGRAPH_STOP @endlink message.
   UtlBoolean handleStop(void);
     /**<
     * @retval TRUE - if the message was handled.
     * @retval FALSE - otherwise.
     */

     /// @brief Sets \p rpResource to point to the resource that corresponds
     /// to \p name or to NULL if no matching resource is found.
   OsStatus lookupResourcePrivate(const UtlString& name,
                                  MpResource*& rpResource);
     /**<
     *  Does a lookup of name->resource, and sets \p rpResource to
     *  point to the resource that corresponds to \p name, or to 
     *  \c NULL if no matching resource is found.
     *
     *  This private version does no locking.
     *
     *  @param[in] name - the name of the resource to look up.
     *  @param[out] rpResource - the resource pointer to store the resource in.
     *
     *  @retval OS_SUCCESS - success.
     *  @retval OS_NOT_FOUND - no resource with the specified name
     *          \p rpResource is \c NULL if \c OS_NOT_FOUND.
     */

     /// Processes all of the messages currently queued for this flow graph.
   OsStatus processMessages(void);
     /**<
     *  @returns Always OS_SUCCESS for nows.
     */

     /// Copy constructor (not implemented for this task)
   MpFlowGraphBase(const MpFlowGraphBase& rMpFlowGraph);

     /// Assignment operator (not implemented for this task)
   MpFlowGraphBase& operator=(const MpFlowGraphBase& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpFlowGraphBase_h_
