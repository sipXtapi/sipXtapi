//  
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
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

#include "rtcp/RtcpConfig.h"

// APPLICATION INCLUDES
#include <utl/UtlHashMap.h>

#include "utl/UtlString.h"
#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include "os/OsStatus.h"
#include "os/OsRWMutex.h"
#include "os/OsTime.h"
#include "mp/MpResource.h"
#include "os/OsMsgDispatcher.h"
#include "mp/MpResNotificationMsg.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTCManager.h"
#endif /* INCLUDE_RTCP ] */

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
#ifdef INCLUDE_RTCP /* [ */
                      : public CBaseClass,
                        public IRTCPNotify
#endif /* INCLUDE_RTCP ] */
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   friend OsStatus MpResource::postMessage(MpFlowGraphMsg& rMsg);

   typedef enum
   {
      STARTED,   ///< flow graph has been started (is processing media streams)
      STOPPED    ///< flow graph is stopped (not processing media streams)
   } FlowGraphState;

   typedef enum 
   {
      BASE_FLOWGRAPH       = 0,
      CALL_FLOWGRAPH       = 1,
      TOPOLOGY_FLOWGRAPH   = 2
   } FlowGraphType;
   //: FlowGraph Types

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpFlowGraphBase(int samplesPerFrame = 0, ///< Zero takes default
                   int samplesPerSec = 0, ///< Zero takes default
                   OsMsgDispatcher *pNotifDispatcher = NULL);

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
   OsMsgDispatcher*  
   setNotificationDispatcher(OsMsgDispatcher* notifyDispatcher);
     /**<
     *  Seta a notification dispatcher to the flowgraph, for use in
     *  letting the resources or the flowgraph tell the application 
     *  any interesting events.
     *
     *  @param[in] notifyDispatcher - a pointer to a notification dispatcher that 
     *             should be used to post media event notifications.
     *  
     *  @retval NULL if no previous notification dispatcher was set.
     *  @retval pointer to previous notification dispatcher if one was set previously.
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
     *  the audio hardware.  This may only be called if this flow graph
     *  is managed and started. Also this call is meaningful only if local
     *  audio is enabled for this flowgraph.
     *
     *  @retval  OS_SUCCESS on success
     *  @retval  OS_FAILED if flowgraph does not support setting focus.
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
     *  the audio hardware.  This may only be called if this flow graph
     *  is managed and started. Also this call is meaningful only if local
     *  audio is enabled for this flowgraph.
     *
     *  @retval  OS_SUCCESS on success
     *  @retval  OS_FAILED if flowgraph does not support setting focus.
     */

     /// @brief posts a resource notification message to the Notification dispatcher.
   virtual OsStatus postNotification(const MpResNotificationMsg& msg);
     /**<
     *  If there is a notification dispatcher, this message is posted
     *  to it.  Otherwise, the message is dropped.
     *
     *  The Notification Dispatcher is used to hold and notify users of 
     *  notification messages.  This is first being created to be used by
     *  resources, held here in the flowgraph, and used to pass notification up
     *  to the application level.  In the future, this could be extended to
     *  allow filtering of notification messages - The one setting up the 
     *  notification dispatcher could set properties on it to enable only
     *  certain types of messages to be sent up through it's framework.
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

     /// @brief Removes the indicated media processing object from the flowgraph
     /// and destroys it.
   OsStatus destroyResource(const UtlString& resourceName);
     /**<
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @retval OS_SUCCESS - success, resource has been removed and destroyed.
     *  @retval OS_UNSPECIFIED - destroy resource operation failed.
     */

     /// @copydoc CpMediaInterface::setMediaNotificationsEnabled()
   OsStatus setNotificationsEnabled(bool enabled, 
                                    const UtlString& resourceName = NULL);

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
     /// Returns the type of this flow graph.
   virtual FlowGraphType getType();

     /// Returns the number of samples expected per frame.
   inline int getSamplesPerFrame(void) const;

     /// Returns the number of samples expected per second.
   inline int getSamplesPerSec(void) const;

     /// Returns the current state of flow graph.
   inline int getState(void) const;

     /// Returns the current notification dispatcher, if any.  If none, returns NULL.
   inline OsMsgDispatcher* getNotificationDispatcher() const;

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
   inline int numLinks(void) const;

     /// Returns the number of frames this flow graph has processed.
   inline int numFramesProcessed(void) const;

     /// Returns the number of resources in the flow graph.
   inline int numResources(void) const;

     /// Returns the message queue used by the flow graph. 
   inline OsMsgQ* getMsgQ(void) ;

     /// Get flowgraph's sequence number (use for debug purposes only!)
   inline int getFlowgraphNum() const;

     /// Get flowgraph's name based on its sequence number (use for debug purposes only!)
   inline UtlString getFlowgraphName() const;

     /// Get latency for main path from one resource to another in forward direction.
   OsStatus getLatencyForPath(MpResource *pStartResource,
                              int startResourceInput,
                              const UtlString &endResourceName,
                              int endResourceOutput,
                              UtlBoolean includeEndResourceLatency,
                              int &latency);
     /**<
     *  This method traverses flowgraph from \p pStartResource following by
     *  first outputs (i.e. output 0) to next resources until resource with
     *  the name of \p endResourceName is reached. It sums up latencies of
     *  all encountered resources including latency of \p pStartResource and
     *  conditionally including latency of \p endResourceName.
     *
     *  @param[in]  pStartResource - pointer to a resource to start traverse from.
     *  @param[in]  startResourceInput - input number on start resource from
     *              which to calculate latency.
     *  @param[in]  endResourceName - name of a resource to end with.
     *  @param[in]  endResourceOutput - output number on end resource to which
     *              we should calculate latency. Value is meaningful only if
     *              \p includeEndResourceLatency is TRUE.
     *  @param[in]  includeEndResourceLatency - should the latency of
     *              the \p endResourceName be included in the path latency or not?
     *  @param[out] latency - calculated latency for the path (in samples).
     *
     *  @retval OS_SUCCESS if path was traversed successfully and latency was
     *          calculated.
     *  @retval OS_NOT_FOUND if either start or end resource is not found.
     */

     /// Get latency for main path from one resource to another in reverse direction.
   OsStatus getLatencyForPathReverse(MpResource *pStartResource,
                                     int startResourceOutput,
                                     const UtlString &endResourceName,
                                     int endResourceInput,
                                     UtlBoolean includeEndResourceLatency,
                                     int &latency);
     /**<
     *  Works exactly in the same way as getLatencyForPath(), but traverse
     *  resources from downstream to upstream ones.
     *
     *  @see getLatencyForPath()
     */

#ifdef INCLUDE_RTCP /* [ */
     /// Returns the RTCP Session interface pointer associated with this call's flow graph.
   IRTCPSession* getRTCPSessionPtr(void);

   IRTCPConnection* getRTCPConnectionPtr(MpConnectionID connId, int mediaType, int streamId);

#endif /* INCLUDE_RTCP ] */

   

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

/* ============================ CALLBACKS ================================= */
#ifdef INCLUDE_RTCP /* [ */

/**
 *
 * Method Name:  GetEventInterest()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Mask of Event Interests
 *
 * Description: The GetEventInterest() event method shall allow the dispatcher
 *              of notifications to access the event interests of a subscriber
 *              and use these wishes to dispatch RTCP event notifications
 *
 * Usage Notes:
 *
 */
    unsigned long GetEventInterest(void);

/**
 *
 * Method Name:  LocalSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to
 *                                                   associated RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                   RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The LocalSSRCCollision() event method shall inform the
 *              recipient of a collision between the local SSRC and one
 *              used by one of the remote participants.
 *
 * Usage Notes:
 *
 */
    void LocalSSRCCollision(IRTCPConnection    *piRTCPConnection,
                            IRTCPSession       *piRTCPSession);


/**
 *
 * Method Name:  RemoteSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to associated
 *                                                    RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                    RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The RemoteSSRCCollision() event method shall inform the
 *              recipient of a collision between two remote participants.
 *              .
 *
 * Usage Notes:
 *
 */
    void RemoteSSRCCollision(IRTCPConnection    *piRTCPConnection,
                             IRTCPSession       *piRTCPSession);


/**
 *
 * Macro Name:  DECLARE_IBASE_M
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 *
 * Usage Notes:
 *
 *
 */
DECLARE_IBASE_M
#endif /* INCLUDE_RTCP ] */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsRWMutex        mRWMutex;      // reader/writer lock for synchronization

     /// Posts a message to this flow graph.
   virtual OsStatus postMessage(const MpFlowGraphMsg& rMsg,
                                const OsTime& rTimeout=OsTime::OS_INFINITY);
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

     /// Handle the @link MpFlowGraphMsg::FLOWGRAPH_DESTROY_RESOURCE FLOWGRAPH_DESTROY_RESOURCE @endlink message.
   UtlBoolean handleDestroyResource(const UtlString &name);
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

   static int       gFgMaxNumber; ///< Flowgraph counter, used to generate
                               ///< mFgNumber (for debug purposes only!)
   int              mFgNumber; ///< Sequence number of this flowgraph (for debug
                               ///< purposes only!)

   UtlHashMap mResourceDict;   ///< resource dictionary
   MpResource* mExecOrder[MAX_FLOWGRAPH_RESOURCES]; ///< resource execution order
   MpResource* mUnsorted[MAX_FLOWGRAPH_RESOURCES];  ///< unsorted resources
   int       mCurState;        ///< current flow graph state
   OsMsgQ    mMessages;        ///< message queue for this flow graph
   OsMsgDispatcher* mNotifyDispatcher; ///< Dispatcher for notification messages
   int       mPeriodCnt;       ///< number of frames processed by this flow graph
   int       mLinkCnt;         ///< number of links in this flow graph
   int       mResourceCnt;     ///< number of resources in this flow graph
   UtlBoolean mRecomputeOrder; ///< TRUE ==> the execution order needs computing
   int       mSamplesPerFrame; ///< number of samples per frame
   int       mSamplesPerSec;   ///< number of samples per second
   MpResource* mpResourceInProcess; ///< @brief For debugging, keep track of what
                               ///< resource we are working on in processNextFrame().
   static const OsTime smProcessMessagesTimeout; ///< Timeout for receiving messages
                               ///< from the flowgraph queue.

#ifdef INCLUDE_RTCP /* [ */
   int hashRtcpConnection(MpConnectionID connId, int mediaType, int streamId);
   UtlBoolean createRtcpConnection(MpConnectionID connId, int mediaType, int streamId);
   UtlBoolean deleteRtcpConnection(MpConnectionID connId, int mediaType, int streamId);

   IRTCPSession* mpiRTCPSession;
   /// Event Interest Attribute for RTCP Notifications
   unsigned long mulEventInterest;
   OsMutex mRtcpConnMutex;
   UtlHashMap mRtcpConnMap;    ///< map of RTCPConnections by connection IDs
#endif /* INCLUDE_RTCP ] */


     /// @brief Computes the execution order for the flow graph by performing a 
     /// topological sort on the resource graph.
   OsStatus computeOrder(void);
     /**<
     *  @retval OS_SUCCESS - successfully computed an execution order.
     *  @retval OS_LOOP_DETECTED - detected a loop in the flow graph.
     */

     /// @brief Find the index of the given resource in the compute order array
   int getExecOrderIndex(const MpResource* resource) const;
     /**<
     *  @param[in] resource pointer to flowgraph resource to find index for
     *  @retval index of resource in execute order array or -1 if not found
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

     /// Do the real work for getLatencyForPath()
   OsStatus handleGetLatencyForPath(MpResource *pStartResource,
                                    int startResourceInput,
                                    const UtlString &endResourceName,
                                    int endResourceOutput,
                                    UtlBoolean includeEndResourceLatency,
                                    int &latency);

     /// Do the real work for getLatencyForPathReverse()
   OsStatus handleGetLatencyForPathReverse(MpResource *pStartResource,
                                           int startResourceOutput,
                                           const UtlString &endResourceName,
                                           int endResourceInput,
                                           UtlBoolean includeEndResourceLatency,
                                           int &latency);

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

int MpFlowGraphBase::getSamplesPerFrame(void) const
{
   return mSamplesPerFrame;
}

int MpFlowGraphBase::getSamplesPerSec(void) const
{
   return mSamplesPerSec;
}

int MpFlowGraphBase::getState(void) const
{
   return mCurState;
}

OsMsgDispatcher* MpFlowGraphBase::getNotificationDispatcher(void) const
{
   return mNotifyDispatcher;
}

int MpFlowGraphBase::numLinks(void) const
{
   return mLinkCnt;
}

int MpFlowGraphBase::numFramesProcessed(void) const
{
   return mPeriodCnt;
}

int MpFlowGraphBase::numResources(void) const
{
   return mResourceCnt;
}

OsMsgQ* MpFlowGraphBase::getMsgQ(void) 
{
   return &mMessages ;
}

int MpFlowGraphBase::getFlowgraphNum() const
{
   return mFgNumber;
}

UtlString MpFlowGraphBase::getFlowgraphName() const
{
   char tmpStr[10];
   snprintf(tmpStr, sizeof(tmpStr), "FG%d", getFlowgraphNum());
   return tmpStr;
}

#ifdef INCLUDE_RTCP /* [ */
inline IRTCPSession *MpFlowGraphBase::getRTCPSessionPtr(void)
{
    return(mpiRTCPSession);
}



/**
 *
 * Method Name:  GetEventInterest()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Mask of Event Interests
 *
 * Description: The GetEventInterest() event method shall allow the dispatcher
 *              of notifications to access the event interests of a subscriber
 *              and use these wishes to dispatch RTCP event notifications
 *
 * Usage Notes:
 *
 */
inline unsigned long MpFlowGraphBase::GetEventInterest(void)
{

    return(mulEventInterest);
}
#endif /* INCLUDE_RTCP ] */

#endif  // _MpFlowGraphBase_h_
