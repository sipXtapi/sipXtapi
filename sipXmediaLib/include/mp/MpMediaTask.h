//  
// Copyright (C) 2006-2021 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpMediaTask_h_
#define _MpMediaTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHistogram.h"
#include "os/OsDefs.h"
#include "os/OsRWMutex.h"
#include "os/OsServerTask.h"
#include "os/OsMsgPool.h"
#include "os/OsCallback.h"
#include "mp/MpMediaTaskMsg.h"

// DEFINES

//#define _PROFILE

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpFlowGraphBase;
class OsNotification;

/**
*  @brief Object responsible for coordinating the execution of media processing
*  flow graphs.
* 
*  <H3>Key Concepts</H3>
*  Flow graphs are created outside of this class and are initially in the
*  @link MpFlowGraphBase::STOPPED STOPPED @endlink state.  Once a flow graph has
*  been created and while it is still in the stopped state, the media processing
*  task is informed of the new flow graph via the <i>manageFlowGraph()</i>
*  method.<br>
*  <br>
*  A flow graph must be in the @link MpFlowGraphBase::STARTED STARTED @endlink
*  state before it will process media streams.  The <i>startFlowGraph()</i> and
*  <i>stopFlowGraph()</i> methods are used to start and stop flow graphs,
*  respectively.<br>
*  <br>
*  The <i>unmanageFlowGraph()</i> method informs the media processing task
*  that it should no longer execute the indicated flow graph.  If the flow
*  graph is not in the @link MpFlowGraphBase::STOPPED STOPPED @endlink state,
*  the media processing task stops the flow graph before unmanaging it.<br>
*  <br>
*  Changes to the flow graphs <i>startFlowGraph(), stopFlowGraph(),
*  manageFlowGraph(), unmanageFlowGraph()</i> and <i>setFocus()</i> all take
*  effect at frame processing boundaries.<br>
*  <br>
*  The media processing task expects to receive a notification every frame
*  interval indicating that it is time to process the next frame's worth of
*  media.  This signal is conveyed by calling the static
*  <i>signalFrameStart()</i> method.
* 
*  <H3>Locking</H3>
*  For the most part, this class relies on the atomicity of reads and writes
*  to appropriately aligned 32-bit data to avoid the need for locks.  However,
*  a single-writer/multiple-reader lock is used to protect the data structure
*  that maintains the set of flow graphs currently being managed by the media
*  processing task.  The media processing task takes the write lock while
*  modifying the data structure.  The <i>getManagedFlowGraphs()</i> method
*  acquires the read lock as part of getting the array of flow graphs.
* 
*  <H3>Messaging</H3>
*  Several of the methods in this class post @link ::MpMediaTaskMsg MpMediaTask
*  messages @endlink to the media processing task. These messages are processed
*  at the beginning of the next frame processing interval.  The methods that
*  cause messages to be sent are: <i>manageFlowGraph(), unmanageFlowGraph(),
*  startFlowGraph(), stopFlowGraph()</i> and <i>setFocus()</i>.<br>
*  <br>
*  At the beginning of the frame processing interval, the media processing
*  task sends itself a @link MpMediaTaskMsg::WAIT_FOR_SIGNAL WAIT_FOR_SIGNAL
*  @endlink message.  When the task receives that message, it knows that it is
*  time to finish the frame processing for the current interval and then wait
*  for the "start" signal for the next frame before processing any more messages.
*
*  @nosubgrouping
*/
class MpMediaTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum {
       DEF_TIME_LIMIT_USECS    = 6000,  ///< processing limit  = 6 msecs
       DEF_SEM_WAIT_MSECS      = 500,   ///< semaphore timeout = 0.5 secs
       MEDIA_TASK_PRIORITY     = 0      ///< media task execution priority
   };


/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Create the media processing task
   static MpMediaTask* createMediaTask(int maxFlowGraph,
                                       UtlBoolean enableLocalAudio = true);

     /// Return a pointer to the media processing task if exists.
   static MpMediaTask* getMediaTask();

     /// Destructor
   virtual
   ~MpMediaTask();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @brief Directs the media processing task to add the flow graph to its 
     /// set of managed flow graphs.
   OsStatus manageFlowGraph(MpFlowGraphBase& rFlowGraph);
     /**<
     *  The flow graph must be in the 
     *  @link MpFlowGraphBase::STOPPED STOPPED @endlink state when this method
     *  is invoked.
     *
     *  @returns <b>OS_SUCCESS</b> - the flow graph will be added at the start
     *                               of the next frame processing interval.
     *  @returns <b>OS_INVALID_ARGUMENT</b> - flow graph is not in the STOPPED
     *                                        state
     */

     /// @brief Directs the media processing task to remove the flow graph from
     /// its set of managed flow graphs.
   OsStatus unmanageFlowGraph(MpFlowGraphBase& rFlowGraph);
     /**<
     *  If the flow graph is not already in the @link MpFlowGraphBase::STOPPED
     *  STOPPED @endlink state, then the flow graph will be stopped before it is
     *  removed from the set of managed flow graphs.
     * 
     *  @returns <b>OS_SUCCESS</b> - indicates that the media task will stop
     *                               managing the indicated flow graph
     */

     /// @brief When "debug" mode is enabled, the "time limit" checking is 
     /// disabled and the wait for "frame start" timeout is set to "INFINITY".
   OsStatus setDebug(UtlBoolean enableFlag);
     /**<
     *  For now, this method always returns OS_SUCCESS.
     */

     /// Changes the focus to the indicated flow graph.
   OsStatus setFocus(MpFlowGraphBase* pFlowGraph);
     /**<
     *  At most one flow graph at a time can have focus.  Only the flow
     *  graph that has focus is allowed to access the audio resources
     *  (speaker and microphone) of the phone.
     *  The affected flow graphs will be modified to reflect the change of
     *  focus at the beginning of the next frame interval.
     *  For now, this method always returns OS_SUCCESS.
     */

     /// @brief Sets the amount of time (in microseconds) allotted to the media 
     /// processing task for processing a frame's worth of media.
   OsStatus setTimeLimit(int usecs);
     /**<
     *  If this time limit is exceeded, the media processing task increments
     *  an internal statistic.  The value of this statistic can be retrieved
     *  by calling the getLimitExceededCnt() method. For now, this method
     *  always returns OS_SUCCESS.
     */

     /// @brief Sets the maximum time (in milliseconds) that the media processing 
     /// task will wait for a "frame start" signal.
   OsStatus setWaitTimeout(int msecs);
     /**<
     *  A value of -1 indicates that the task should wait "forever".
     *
     *  The new timeout will take effect at the beginning of the next frame
     *  interval. For now, this method always returns OS_SUCCESS.
     */

     /// Directs the media processing task to start the specified flow graph.
   OsStatus startFlowGraph(MpFlowGraphBase& rFlowGraph);
     /**<
     *  A flow graph must be started in order for it to process the media stream.
     *
     *  The flow graph state change will take effect at the beginning of the
     *  next frame interval. For now, this method always returns OS_SUCCESS.
     */

     /// Directs the media processing task to stop the specified flow graph.
   OsStatus stopFlowGraph(MpFlowGraphBase& rFlowGraph);
     /**<
     *  When a flow graph is stopped it no longer processes the media stream.
     *
     *  The flow graph state change will take effect at the beginning of the
     *  next frame interval. For now, this method always returns OS_SUCCESS.
     */

     /// @brief Signal the media processing task that it should begin processing
     /// the next frame.
   static OsStatus signalFrameStart(const OsTime &timeout = OsTime::OS_INFINITY);
     /**<
     *  @see getTickerNotification() for an alternative method of signaling
     *       frame start.
     */

   /// Queue the given messages to each of the managed flowgraphs
   OsStatus sendToAllFlowgraphs(OsMsgQ& queuedMessages);
   /**<
    *   @param queuedMessagees - message queue containing one or more messages that
    *          are to be queued to all of the managed flowgraphs
    */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    ///  Debug aid for tracking state. See MpMediaTaskTest
   int numHandledMsgErrs();

     /// Returns TRUE if debug mode is enabled, FALSE otherwise.
   UtlBoolean getDebugMode(void) const;

     /// @brief Returns the flow graph that currently has focus (access to the
     /// audio apparatus) or NULL if there is no flow graph with focus.
   MpFlowGraphBase* getFocus(void) const;

     /// @brief Returns the number of times that the frame processing time limit 
     /// has been exceeded.
   int getLimitExceededCnt(void) const;

     /// @brief Returns an array of MpFlowGraphBase pointers that are presently
     /// managed by the media processing task.
   OsStatus getManagedFlowGraphs(MpFlowGraphBase* flowGraphs[], const int size,
                                 int& numItems);
     /**<
     *  The caller is responsible for allocating the flowGraphs array
     *  containing room for <i>size</i> pointers.  The number of items
     *  actually filled in is passed back via the <i>nItems</i> argument.
     */

     /// @brief Returns the amount of time (in microseconds) allotted to the
     /// media processing task for processing a frame's worth of media.
   int getTimeLimit(void) const;

     /// @brief Returns the maximum time (in milliseconds) that the media
     /// processing task will wait for the "frame start" signal.
   int getWaitTimeout(void) const;
     /**<
     *  A value of -1 indicates that the task will wait "forever".
     */

     /// @brief Returns the number of times that the wait timeout associated with 
     /// "frame start" signal has been exceeded.
   int getWaitTimeoutCnt(void) const;

     /// @brief Returns the number of flow graphs currently being managed by the 
     /// media processing task.
   int numManagedFlowGraphs(void) const;

     /// @brief Returns the number of frames that the media processing task has 
     /// processed. This count is maintained as an unsigned, 32-bit value.
   int numProcessedFrames(void) const;
     /**<
     *  Note: If the frame period is 10 msecs, then it will take
     *  2^32 / (100 * 3600 * 24 * 365) = 1.36 years before this count wraps.
     */

     /// @brief Returns the number of flow graphs that have been started by the
     /// media processing task.
   int numStartedFlowGraphs(void) const;
     /**<
     *  This value should always be <= the number of managed flow graphs.
     */

     /// Returns pointer to pool of reusable buffer messages
   OsMsgPool* getBufferMsgPool(void) const;

     /// @brief Get OsNotification to signal for the next frame processing
     /// interval start.
   inline OsNotification *getTickerNotification();
     /**<
     *  Returned notification is owned by MediaTask and should not be freed
     *  by a caller.
     *
     *  @see signalFrameStart() for an alternative method of signaling frame start.
     */

     /// Displays information on the console about the media processing task.
   static MpFlowGraphBase* mediaInfo(void);

     /// @brief Returns the maximum number of flow graphs that can be managed by
     /// the media processing task.
   static int maxNumManagedFlowGraphs(void);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// Is flow graph managed by the media processing task?
   UtlBoolean isManagedFlowGraph(MpFlowGraphBase* pFlowGraph);
     /**<
     *  @returns <b>TRUE</b> - if the indicated flow graph is presently being
     *                         managed by the media processing task,
     *  @returns <b>FALSE</b> - otherwise.
     */

     /// Return usage information on the Media Task's message queue.
   void getQueueUsage(int& numMsgs, int& softLimit,
                      int& hardLimit);

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Default constructor
   MpMediaTask(int maxFlowGraph, UtlBoolean enableLocalAudio);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsMutex   mMutex;         ///< lock for synchronization
   UtlBoolean mDebugEnabled; ///< TRUE if debug mode is enabled, FALSE otherwise

   int       mTimeLimitCnt;  ///< Number of frames where time limit was exceeded
   unsigned  mProcessedCnt;  ///< Number of frames that have been processed
   int       mManagedCnt;    ///< Number of flow graphs presently managed
   int       mStartedCnt;    ///< Number of flow graphs presently started
   OsTime    mSemTimeout;    ///< Timeout value for acquiring the start semaphore
   int       mSemTimeoutCnt; ///< Number of times the mSemTimeOut was exceeded 
   UtlBoolean mWaitForSignal;///< @brief If TRUE, then don't handle any incoming
                             ///< msgs until a FrameStart signal has been received
   MpFlowGraphBase* mpFocus; ///< FlowGraph that has the focus (may be NULL)
   MpFlowGraphBase** mManagedFGs; ///< The set of flow graphs presently managed
   static int mMaxFlowGraph;
   int       mLimitUsecs;    ///< Frame processing time limit (in usecs)
   int       mHandleMsgErrs; ///< @brief Number of message handling problems
                             ///< during the last frame processing interval
   OsMsgPool* mpBufferMsgPool; ///< Pool of reusable buffer messages
   OsMsgPool* mpSignalMsgPool; ///< Pool of reusable frame signal messages
   static const OsTime smOperationQueueTimeout; ///< Timeout for posting messages
                             ///< to the flowgraph queue.
   OsCallback mFlowgraphTicker; ///< OsNotification to call flowgraphTickerCallback()
   UtlBoolean mIsLocalAudioEnabled; ///< Affects setFocus().
   static UtlBoolean mIsBlockingReported; ///< Is message about MediaTask being blocked
                             ///< for too long already reported?

   //  Static data members used to enforce Singleton behavior
   static MpMediaTask* volatile  spInstance;  ///< @brief pointer to the single instance
                                    ///< of the MpMediaTask class
   static OsBSem       sLock;       ///< @brief semaphore used to ensure that
                                    ///< there is only one instance of this class

#ifdef _PROFILE /* [ */
   /// Start time (in microseconds) for the current frame processing interval
   long long mStartTicks;
   /// Stop time (in microseconds) for the current frame processing interval
   long long mStopTicks;
   /// Frame processing time limit (in nanoseconds)
   unsigned long long mLimitTicks;

   /// @brief Histograms for handleWaitForSignal() processing time.
   /// Time from start of execution to end.
   UtlHistogram mStartToEndTime;
   /// Time from start of execution to start of next execution.
   UtlHistogram mStartToStartTime;
   /// Time from end of execution to start of next execution.
   UtlHistogram mEndToStartTime;
   /// Time from one signal time to the next.
   UtlHistogram mSignalTime;
   /// Time from signal time to the execution it starts.
   UtlHistogram mSignalToStartTime;
   /// Time to process messages other than WAIT_FOR_SIGNAL.
   UtlHistogram mOtherMessages;
#endif /* _PROFILE ] */

     /// @copydoc OsServerTask::run()
   int run(void* pArg);

     /// Handle an incoming message
   UtlBoolean handleMessage(OsMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> - if the message was handled,
     *  @returns <b>FALSE</b> - otherwise.
     */

     /// Handles the @link MpMediaTaskMsg::MANAGE MANAGE @endlink message.
   UtlBoolean handleManage(MpFlowGraphBase* pFlowGraph);
     /**<
     *  @returns <b>TRUE</b> - if the message was handled,
     *  @returns <b>FALSE</b> - otherwise.
     */

     /// Handles the @link MpMediaTaskMsg::SET_FOCUS SET_FOCUS @endlink message.
   UtlBoolean handleSetFocus(MpFlowGraphBase* pFlowGraph);
     /**<
     *  @returns <b>TRUE</b> - if the message was handled,
     *  @returns <b>FALSE</b> - otherwise.
     */

     /// Handles the @link MpMediaTaskMsg::START START @endlink message.
   UtlBoolean handleStart(MpFlowGraphBase* pFlowGraph);
     /**<
     *  @returns <b>TRUE</b> - if the message was handled,
     *  @returns <b>FALSE</b> - otherwise.
     */

     /// Handles the @link MpMediaTaskMsg::STOP STOP @endlink message.
   UtlBoolean handleStop(MpFlowGraphBase* pFlowGraph);
     /**<
     *  @returns <b>TRUE</b> - if the message was handled,
     *  @returns <b>FALSE</b> - otherwise.
     */

     /// Handles the @link MpMediaTaskMsg::UNMANAGE UNMANAGE @endlink message.
   UtlBoolean handleUnmanage(MpFlowGraphBase* pFlowGraph);
     /**<
     *  @returns <b>TRUE</b> - if the message was handled,
     *  @returns <b>FALSE</b> - otherwise.
     */

     /// Handles the @link MpMediaTaskMsg::WAIT_FOR_SIGNAL WAIT_FOR_SIGNAL @endlink message.
   UtlBoolean handleWaitForSignal(MpMediaTaskMsg*);
     /**<
     *  @returns <b>TRUE</b> - if the message was handled,
     *  @returns <b>FALSE</b> - otherwise.
     */

     /// Callback for flowgraph ticker.
   static
   void flowgraphTickerCallback(const intptr_t userData, const  intptr_t eventData);
     /**<
     *  @param[in] userData - contains 0 for now.
     *  @param[in] eventData - contains 0 for now.
     */

     /// Copy constructor (not implemented for this task)
   MpMediaTask(const MpMediaTask& rMpMediaTask);

     /// Assignment operator (not implemented for this task)
   MpMediaTask& operator=(const MpMediaTask& rhs);

};

/* ============================ INLINE METHODS ============================ */

OsNotification *MpMediaTask::getTickerNotification()
{
   return &mFlowgraphTicker;
}

#endif  // _MpMediaTask_h_
