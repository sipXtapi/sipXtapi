//  
// Copyright (C) 2006-2020 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


/* There used to be #ifdef's here to do the same thing on WIN32/VXWORKS, but I
* took them out because we do the same thing on every OS. -Mike */
#define PRINTF Zprintf

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsLock.h>
#include <os/OsMsgPool.h>
#include <os/OsDateTime.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpMisc.h>

// define _PROFILE in MpMediaTask.h to enable
#include <mp/MpMediaTask.h>
#include <mp/MpMediaTaskMsg.h>
#include <mp/MpBufferMsg.h>

#ifdef __pingtel_on_posix__ // [
#  include "mp/MpMMTimerPosix.h"
#endif // ] __pingtel_on_posix__

//#define RTL_ENABLED
#ifdef RTL_ENABLED // [
#  include <rtl_macro.h>
#else // RTL_ENABLED ][
#  define RTL_EVENT(x,y)
#endif // RTL_ENABLED ]

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#ifdef __pingtel_on_posix__ /* [ */
#define MPMEDIA_DEF_MAX_MSGS 1000
#else
#define MPMEDIA_DEF_MAX_MSGS OsServerTask::DEF_MAX_MSGS
#endif /* __pingtel_on_posix__ ] */
const OsTime MpMediaTask::smOperationQueueTimeout = OsTime::OS_INFINITY;

// STATIC VARIABLE INITIALIZATIONS
MpMediaTask* volatile MpMediaTask::spInstance = NULL;
OsBSem       MpMediaTask::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
int          MpMediaTask::mMaxFlowGraph = 0;
UtlBoolean   MpMediaTask::mIsBlockingReported = FALSE;
int sSignalStartsMissed = 0;
int sSignalStartsNotQueued = 0;
int sSignalStartsQueued = 0;

#ifdef _PROFILE /* [ */
   static long long sSignalTicks; // Time (in microseconds) for the current
                                  // frame start signal
   static unsigned long long sMinTicks;
   static unsigned long long sMaxTicks;
#endif /* _PROFILE ] */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpMediaTask* MpMediaTask::createMediaTask(int maxFlowGraph, UtlBoolean enableLocalAudio)
{
   UtlBoolean isStarted;

   assert(maxFlowGraph>0);

   OsLock lock(sLock);
   assert(spInstance == NULL);

   if (spInstance == NULL)
       spInstance = new MpMediaTask(maxFlowGraph, enableLocalAudio);

   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }

   return spInstance;
}

MpMediaTask* MpMediaTask::getMediaTask()
{
   OsLock lock(sLock);

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;
   else
      // If we can't find existing media task, return NULL.
      return NULL;
}

// Destructor
MpMediaTask::~MpMediaTask()
{
   // $$$ need to figure out how to cleanly shut down this task after
   // $$$ unmanaging and destroying all of its flow graphs

   delete[] mManagedFGs;

   if (mpBufferMsgPool != NULL)
      delete mpBufferMsgPool;

   if (mpSignalMsgPool != NULL)
      delete mpSignalMsgPool;

   spInstance = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Directs the media processing task to add the flow graph to its 
// set of managed flow graphs.  The flow graph must be in the 
// MpFlowGraphBase::STOPPED state when this method is invoked.
// Returns OS_INVALID_ARGUMENT if the flow graph is not in the STOPPED state.
// Otherwise returns OS_SUCCESS to indicate that the flow graph will be added
// to the set of managed flow graphs at the start of the next frame
// processing interval.
OsStatus MpMediaTask::manageFlowGraph(MpFlowGraphBase& rFlowGraph)
{
   MpMediaTaskMsg msg(MpMediaTaskMsg::MANAGE, &rFlowGraph);
   OsStatus       res;

   if (rFlowGraph.getState() != MpFlowGraphBase::STOPPED) {
      // PRINTF("MpMediaTask::manageFlowGraph: error!\n", 0,0,0,0,0,0);
      return OS_INVALID_ARGUMENT;
   }

   res = postMessage(msg, smOperationQueueTimeout);
   assert(res == OS_SUCCESS);

   return OS_SUCCESS;
}

// Directs the media processing task to remove the flow graph from its 
// set of managed flow graphs.
// If the flow graph is not already in the MpFlowGraphBase::STOPPED state,
// then the flow graph will be stopped before it is removed from the set
// of managed flow graphs.
// Returns OS_SUCCESS to indicate that the media task will stop managing
// the indicated flow graph at the start of the next frame processing
// interval.
OsStatus MpMediaTask::unmanageFlowGraph(MpFlowGraphBase& rFlowGraph)
{
   MpMediaTaskMsg msg(MpMediaTaskMsg::UNMANAGE, &rFlowGraph);
   OsStatus       res;

   res = postMessage(msg, smOperationQueueTimeout);
   assert(res == OS_SUCCESS);

   return OS_SUCCESS;
}

// When "debug" mode is enabled, the "time limit" checking is 
// disabled and the wait for "frame start" timeout is set to "OS_INFINITY".
// For now, this method always returns OS_SUCCESS.
OsStatus MpMediaTask::setDebug(UtlBoolean enableFlag)
{
   // osPrintf("\nMpMediaTask::setDebug(%s)\n", enableFlag?"TRUE":"FALSE");
   mDebugEnabled = enableFlag;

   return OS_SUCCESS;
}

// Changes the focus to the indicated flow graph.
// At most one flow graph at a time can have focus.  Only the flow
// graph that has focus is allowed to access the audio resources
// (speaker and microphone) of the phone.
// The affected flow graphs will be modified to reflect the change of
// focus at the beginning of the next frame interval. For now, this method
// always returns OS_SUCCESS.
OsStatus MpMediaTask::setFocus(MpFlowGraphBase* pFlowGraph)
{
   if (mIsLocalAudioEnabled)
   {
      MpMediaTaskMsg msg(MpMediaTaskMsg::SET_FOCUS, pFlowGraph);
      OsStatus       res;

      res = postMessage(msg, smOperationQueueTimeout);
      assert(res == OS_SUCCESS);
   }

   return OS_SUCCESS;
}

// Sets the amount of time (in microseconds) allotted to the media 
// processing task for processing a frame's worth of media.
// If this time limit is exceeded, the media processing task increments
// an internal statistic.  The value of this statistic can be retrieved
// by calling the getLimitExceededCnt() method. For now, this method
// always returns OS_SUCCESS.
OsStatus MpMediaTask::setTimeLimit(int usecs)
{
#ifdef _PROFILE /* [ */
   // Convert to nanoseconds.
   mLimitTicks = usecs * 1000;
#endif /* _PROFILE ] */
   mLimitUsecs = usecs;

   return OS_SUCCESS;
}

// Sets the maximum time (in milliseconds) that the media processing 
// task will wait for a "frame start" signal. A value of -1 indicates 
// that the task should wait "forever".
// The new timeout will take effect at the beginning of the next frame
// interval. For now, this method always returns OS_SUCCESS.
OsStatus MpMediaTask::setWaitTimeout(int msecs)
{
   assert(msecs >= -1);

   if (msecs == -1)
      mSemTimeout = OsTime::OS_INFINITY;
   else
   {
      OsTime tmpTime(msecs/1000, (msecs % 1000) * 1000);
      mSemTimeout = tmpTime;
   }

   return OS_SUCCESS;
}

// (static) Release the "frame start" semaphore.  This signals the media 
// processing task that it should begin processing the next frame.
// Returns the result of releasing the binary semaphore that is used to send
// the signal.
OsStatus MpMediaTask::signalFrameStart(const OsTime &timeout)
{
   OsStatus ret = OS_TASK_NOT_STARTED;
   MpMediaTaskMsg* pMsg;

   // If the Media Task has been started
   if (spInstance != NULL) {
      pMsg = (MpMediaTaskMsg*) spInstance->mpSignalMsgPool->findFreeMsg();
      if (NULL == pMsg) {
          sSignalStartsMissed++;
         ret = OS_LIMIT_REACHED;
      } 
      else 
      {
         OsTime signalTime;
         OsDateTime::getCurTime(signalTime);
         pMsg->setInt1(signalTime.seconds());
         pMsg->setInt2(signalTime.usecs());
#ifdef _PROFILE /* [ */
         long long now = (signalTime.seconds() * 1000000) + signalTime.usecs();

         // Record the tick interval into the histogram.
         if (spInstance->mSignalTime.tally(now - sSignalTicks) >= 1000)
         {
            UtlString* print = spInstance->mSignalTime.show();
            OsSysLog::add(FAC_MP, PRI_NOTICE,
                          "MpMediaTask::signalFrameStart %-18s %d%s",
                          "mSignalTime",
                          spInstance->mSignalTime.getBinSize(),
                          print->data());
            delete print;
            spInstance->mSignalTime.clear();
         }
         sSignalTicks = now; // record the time
#endif /* _PROFILE, __pingtel_on_posix__ ] */
         RTL_EVENT("MpMediaTask::signalFrameStart", 1);
         ret = spInstance->postMessage(*pMsg, timeout);
         RTL_EVENT("MpMediaTask::signalFrameStart", 0);
         if(ret == OS_SUCCESS)
         {
            sSignalStartsQueued++;
         }
         else
         {
             sSignalStartsNotQueued++;
         }

      }
   }
   return ret;
}

// Directs the media processing task to start the specified flow 
// graph.  A flow graph must be started in order for it to process 
// the media stream.
// The flow graph state change will take effect at the beginning of the
// next frame interval. For now, this method always returns OS_SUCCESS.
OsStatus MpMediaTask::startFlowGraph(MpFlowGraphBase& rFlowGraph)
{
   MpMediaTaskMsg msg(MpMediaTaskMsg::START, &rFlowGraph);
   OsStatus       res;

   res = postMessage(msg, smOperationQueueTimeout);
   if (res != OS_SUCCESS)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, " MpMediaTask::startFlowGraph - post"
         " returned %d, try again, will block", res);
         res = postMessage(msg);
      OsSysLog::add(FAC_MP, PRI_DEBUG, " MpMediaTask::startFlowGraph -"
         " re-post returned %d", res);
   }
   assert(res == OS_SUCCESS);

   return OS_SUCCESS;
}

// Directs the media processing task to stop the specified flow 
// graph.  When a flow graph is stopped it no longer processes the 
// media stream.
// The flow graph state change will take effect at the beginning of the
// next frame interval. For now, this method always returns OS_SUCCESS.
OsStatus MpMediaTask::stopFlowGraph(MpFlowGraphBase& rFlowGraph)
{
   MpMediaTaskMsg msg(MpMediaTaskMsg::STOP, &rFlowGraph);
   OsStatus       res;

   res = postMessage(msg, smOperationQueueTimeout);
   assert(res == OS_SUCCESS);

   return OS_SUCCESS;
}

OsStatus MpMediaTask::sendToAllFlowgraphs(OsMsgQ& queuedMessages)
{
    OsMsg* aMessage = NULL;
    int messageCount = 0;
    OsStatus status = OS_NOT_FOUND;

    OsLock lock(mMutex);
    while(queuedMessages.receive(aMessage, OsTime::NO_WAIT_TIME) == OS_SUCCESS)
    {
        if(aMessage)
        {
            messageCount++;
            int successfulSends = 0;
            for(int flowgraphIndex = 0; flowgraphIndex < mManagedCnt; flowgraphIndex++)
            {
                status = mManagedFGs[flowgraphIndex]->getMsgQ()->send(*aMessage);
                if(status == OS_SUCCESS)
                {
                    successfulSends++;
                }
                else
                {
                    OsSysLog::add(FAC_MP, PRI_DEBUG,
                        "MpMediaTask::sendToAllFlowgraphs failed to send message[%d] to flowgraph[%d] error: %d",
                        messageCount, flowgraphIndex, status);
                }
            }
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpMediaTask::sendToAllFlowgraphs sent message[%d] to %d flowgraphs",
                messageCount, successfulSends);

            delete aMessage;
            aMessage = NULL;
        }
        else
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                "MpMediaTask::sendToAllFlowgraphs dequeued NULL message[%d]",
                messageCount);
        }
    }

    return(status);
}

/* ============================ ACCESSORS ================================= */

// Debug aid for tracking state. See MpMediaTaskTest
int MpMediaTask::numHandledMsgErrs()
{
    int ret = mHandleMsgErrs;
    // reset the handleMessage error count
    mHandleMsgErrs = 0;
    return ret;
}

// Returns TRUE if debug mode is enabled, FALSE otherwise.
UtlBoolean MpMediaTask::getDebugMode(void) const
{
   return mDebugEnabled;
}

// Returns the flow graph that currently has focus (access to the audio 
// apparatus) or NULL if there is no flow graph with focus.
MpFlowGraphBase* MpMediaTask::getFocus(void) const
{
   return mpFocus;
}

// Returns the number of times that the frame processing time limit 
// has been exceeded.
int MpMediaTask::getLimitExceededCnt(void) const
{
   return mTimeLimitCnt;
}

// Returns an array of MpFlowGraphBase pointers that are presently managed 
// by the media processing task.
// The caller is responsible for allocating the flowGraphs array
// containing room for "size" pointers.  The number of items
// actually filled in is passed back via the "nItems" argument.
OsStatus MpMediaTask::getManagedFlowGraphs(MpFlowGraphBase* flowGraphs[],
                                           const int size, int& numItems)
{
   int        i;
   OsLock lock(mMutex);

   numItems = (mManagedCnt > size) ? size : mManagedCnt;
   for (i=0; i < numItems; i++)
   {
      flowGraphs[i] = mManagedFGs[i];
   }

   return OS_SUCCESS;
}

// Returns the amount of time (in microseconds) allotted to the media 
// processing task for processing a frame's worth of media.
int MpMediaTask::getTimeLimit(void) const
{
   return mLimitUsecs;
}

// Returns the maximum time (in milliseconds) that the media processing 
// task will wait for the "frame start" signal. A value of -1 indicates 
// that the task will wait "forever".
int MpMediaTask::getWaitTimeout(void) const
{
   if (mSemTimeout.isInfinite())
      return -1;
   else
      return mSemTimeout.cvtToMsecs();
}

// Returns the number of times that the wait timeout associated with 
// "frame start" signal has been exceeded.
int MpMediaTask::getWaitTimeoutCnt(void) const
{
   return mSemTimeoutCnt;
}

// (static) Displays information on the console about the media processing
// task.
MpFlowGraphBase* MpMediaTask::mediaInfo(void)
{
   MpFlowGraphBase* flowGraphs[20];
   int              i;
   int              numItems;
   MpMediaTask*     pMediaTask;
   MpFlowGraphBase* pFlowGraph;
   OsStatus         res;

   pMediaTask = MpMediaTask::getMediaTask();

   osPrintf("\nMedia processing task information\n");
   osPrintf("  Debug mode:                      %s\n",
             pMediaTask->getDebugMode() ? "TRUE" : "FALSE");

   osPrintf("  Processed Frame Count:           %d\n",
             pMediaTask->numProcessedFrames());

   osPrintf("  Processing Time Limit:           %d usecs\n",
             pMediaTask->getTimeLimit());

   osPrintf("  Processing Limit Exceeded Count: %d\n",
             pMediaTask->getLimitExceededCnt());

   i = pMediaTask->getWaitTimeout();
   if (i < 0)
      osPrintf("  Frame Start Wait Timeout:        INFINITE\n");
   else
      osPrintf("  Frame Start Wait Timeout:        %d\n", i);

   osPrintf("  Wait Timeout Exceeded Count:     %d\n",
             pMediaTask->getWaitTimeoutCnt());

   osPrintf("\n  Flow Graph Information\n");
   osPrintf("    Managed:      %d\n", pMediaTask->numManagedFlowGraphs());
   osPrintf("    Started:      %d\n", pMediaTask->numStartedFlowGraphs());

   pFlowGraph = pMediaTask->getFocus();
   if (pFlowGraph == NULL)
      osPrintf("    Focus:        NULL\n");
   else
      osPrintf("    Focus:        %p\n", pFlowGraph);

   res = pMediaTask->getManagedFlowGraphs(flowGraphs, 20, numItems);
   SIPX_UNUSED(res);
   for (i=0; i < numItems; i++)
      osPrintf("    FlowGraph[%d]: %p\n", i, flowGraphs[i]);
   return pFlowGraph;
}

// Returns the number of flow graphs currently being managed by the 
// media processing task.
int MpMediaTask::numManagedFlowGraphs(void) const
{
   return mManagedCnt;
}

//:Returns the maximum number of flow graphs that can be managed by the 
//:media processing task.
int MpMediaTask::maxNumManagedFlowGraphs(void)
{
   /* return one less to allow for the "global flowgraph" ... someday */
   return mMaxFlowGraph - 1;
}


// Returns the number of frames that the media processing task has 
// processed. This count is maintained as an unsigned, 32-bit value.
// Note: If the frame period is 10 msecs, then it will take
// 2^32 / (100 * 3600 * 24 * 365) = 1.36 years before this count wraps.
int MpMediaTask::numProcessedFrames(void) const
{
   return mProcessedCnt;
}

// Returns the number of flow graphs that have been started by the media 
// processing task.
// This value should always be <= the number of managed flow graphs.
int MpMediaTask::numStartedFlowGraphs(void) const
{
   return mStartedCnt;
}

// Returns pointer to pool of reusable buffer messages
OsMsgPool* MpMediaTask::getBufferMsgPool(void) const
{
   return mpBufferMsgPool;
}


/* ============================ INQUIRY =================================== */

void MpMediaTask::getQueueUsage(int& numMsgs, int& softLimit, int& hardLimit)
{
   numMsgs = mIncomingQ.numMsgs();
   softLimit = mpSignalMsgPool->getSoftLimit();
   hardLimit = mpSignalMsgPool->getHardLimit();
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpMediaTask::getQueueUsage "
                 "numMsgs = %d, softLimit = %d, hardLimit = %d",
                 numMsgs, softLimit, hardLimit);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getMediaTask())
MpMediaTask::MpMediaTask(int maxFlowGraph, UtlBoolean enableLocalAudio)
: OsServerTask("MpMedia", NULL, MPMEDIA_DEF_MAX_MSGS,
               MEDIA_TASK_PRIORITY)
, mMutex(OsMutex::Q_PRIORITY)  // create mutex for protecting data
, mDebugEnabled(FALSE)
, mTimeLimitCnt(0)
, mProcessedCnt(0)
, mManagedCnt(0)
, mStartedCnt(0)
, mSemTimeout(DEF_SEM_WAIT_MSECS / 1000, (DEF_SEM_WAIT_MSECS % 1000) * 1000)
, mSemTimeoutCnt(0)
, mWaitForSignal(TRUE)
, mpFocus(NULL)
, mHandleMsgErrs(0)
, mpBufferMsgPool(NULL)
//, numQueuedMsgs(0)
, mpSignalMsgPool(NULL)
, mFlowgraphTicker(0, flowgraphTickerCallback)
, mIsLocalAudioEnabled(enableLocalAudio)
#ifdef _PROFILE /* [ */
, mStartToEndTime(20, 0, 1000, " %4d", 5)
, mStartToStartTime(20, 0, 1000, " %4d", 5)
, mEndToStartTime(20, 0, 1000, " %4d", 5)
, mSignalTime(20, 0, 1000, " %4d", 5)
, mSignalToStartTime(20, 0, 1000, " %4d", 5)
, mOtherMessages(20, 0, 1000, " %4d", 5)
#endif /* _PROFILE ] */
{
   mMaxFlowGraph = maxFlowGraph;

   int      i;
   OsStatus res;

   res = setTimeLimit(DEF_TIME_LIMIT_USECS);
   assert(res == OS_SUCCESS);

   assert(mMaxFlowGraph > 0); // mMaxFlowGraph must be greater than zero
   if (mMaxFlowGraph > 0)
   {
      mManagedFGs = new MpFlowGraphBase*[mMaxFlowGraph];
      if (mManagedFGs)
      {
         for (i=0; i < mMaxFlowGraph; i++)
         {
            mManagedFGs[i] = NULL;
         }
      }
   }
   {
      int totalNumBufs = MpMisc.AudioHeadersPool->getNumBlocks()
                       + MpMisc.RtpHeadersPool->getNumBlocks()
#ifdef REAL_RTCP // [
                       + MpMisc.RtcpHeadersPool->getNumBlocks()
#endif // ] TINY_MEDIALIB
                       ;
      int soft;
      int incr;
      MpBufferMsg msg(MpBufferMsg::AUD_RECORDED);

      soft = totalNumBufs/20;
      if (soft < 8) soft = 8;
      incr = soft / 2 + 1;
      mpBufferMsgPool = new OsMsgPool("MediaBuffers", msg,
                          incr, soft, totalNumBufs, incr,
                          OsMsgPool::MULTIPLE_CLIENTS);
   }

   {
      MpMediaTaskMsg msg(MpMediaTaskMsg::WAIT_FOR_SIGNAL);
      mpSignalMsgPool = new OsMsgPool("MediaSignals", msg,
                          2, 2, 4*mMaxFlowGraph, 1,
                          OsMsgPool::MULTIPLE_CLIENTS);
   }

#ifdef _PROFILE /* [ */
   mStartTicks = 0;
   mStopTicks = 0;
   sSignalTicks = 0;
   sMinTicks = 0;
   sMaxTicks = 0;
#endif /* _PROFILE ] */
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

int MpMediaTask::run(void* pArg)
{
   UtlBoolean doShutdown = FALSE;
   OsMsg*    pMsg = NULL;
   OsStatus  res;

   do
   {
      res = receiveMessage((OsMsg*&) pMsg);          // wait for a message
      assert(res == OS_SUCCESS);

      doShutdown = isShuttingDown();
      if (!doShutdown)
      {                                              // comply with shutdown
         if (!handleMessage(*pMsg))                  // process the message
            OsServerTask::handleMessage(*pMsg);
      }

      if (!pMsg->getSentFromISR())
         pMsg->releaseMsg();                         // free the message
   }
   while (!doShutdown);

   return 0;        // and then exit
}

// Handle an incoming message
// Return TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpMediaTask::handleMessage(OsMsg& rMsg)
{
   UtlBoolean           handled;
   MpFlowGraphBase*    pFlowGraph;
   MpMediaTaskMsg*     pMsg;

   if (rMsg.getMsgType() != OsMsg::MP_TASK_MSG)
      return FALSE;    // the method only handles MP_TASK_MSG messages

   pMsg = (MpMediaTaskMsg*) &rMsg;
   pFlowGraph = (MpFlowGraphBase*) pMsg->getPtr1();

   handled = TRUE;     // until proven otherwise, assume we'll handle the msg
#ifdef _PROFILE /* [ */
   // Log the time it takes to handle messages other than WAIT_FOR_SIGNAL.
   long long start_time;
   if (pMsg->getMsg() != MpMediaTaskMsg::WAIT_FOR_SIGNAL)
   {
      OsTime now;
      OsDateTime::getCurTime(now);
      start_time = (now.seconds() * 1000000) + now.usecs();
   }
#endif /* _PROFILE ] */
   RTL_EVENT("MpMediaTask::handleMessage", 1 + pMsg->getMsg());
   switch (pMsg->getMsg())
   {
   case MpMediaTaskMsg::MANAGE:
      if (!handleManage(pFlowGraph))
         mHandleMsgErrs++;
      break;
   case MpMediaTaskMsg::SET_FOCUS:
      if (!handleSetFocus(pFlowGraph))
         mHandleMsgErrs++;
      break;
   case MpMediaTaskMsg::START:
      if (!handleStart(pFlowGraph))
         mHandleMsgErrs++;
      break;
   case MpMediaTaskMsg::STOP:
      if (!handleStop(pFlowGraph))
         mHandleMsgErrs++;
      break;
   case MpMediaTaskMsg::UNMANAGE:
      if (!handleUnmanage(pFlowGraph))
         mHandleMsgErrs++;
      break;
   case MpMediaTaskMsg::WAIT_FOR_SIGNAL:
      if (!handleWaitForSignal(pMsg))
         mHandleMsgErrs++;
      break;
   default:
      handled = FALSE; // we didn't handle the message after all
      break;
   }
#ifdef _PROFILE /* [ */
   // Log the time it takes to handle messages other than WAIT_FOR_SIGNAL.
   if (pMsg->getMsg() != MpMediaTaskMsg::WAIT_FOR_SIGNAL)
   {
      OsTime now;
      OsDateTime::getCurTime(now);
      long long end_time = (now.seconds() * 1000000) + now.usecs();
      mOtherMessages.tally(end_time - start_time);
   }
#endif /* _PROFILE ] */
   RTL_EVENT("MpMediaTask::handleMessage", 0);

   return handled;
}

// Handles the MANAGE message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpMediaTask::handleManage(MpFlowGraphBase* pFlowGraph)
{
   OsLock lock(mMutex);

   if (mManagedCnt >= mMaxFlowGraph) {
      // PRINTF("MpMediaTask::handleManage: ERROR: too many flow graphs!\n", 0,0,0,0,0,0);
      syslog(FAC_MP, PRI_ERR, "MpMediaTask::handleManage: too many flow graphs!\n");
      return FALSE;
   }

   if (isManagedFlowGraph(pFlowGraph)) { // we are already managing
      // PRINTF("MpMediaTask::handleManage: ERROR: flow graph already managed!\n", 0,0,0,0,0,0);
      syslog(FAC_MP, PRI_WARNING, "MpMediaTask::handleManage: flow graph already managed!!\n");
      return FALSE;                      // the flow graph, return FALSE
   }

   // PRINTF("MpMediaTask::handleManage: Adding flow graph # %d!\n", mManagedCnt, 0,0,0,0,0);
   mManagedFGs[mManagedCnt] = pFlowGraph;
   mManagedCnt++;

   return TRUE;
}

// Handles the SET_FOCUS message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpMediaTask::handleSetFocus(MpFlowGraphBase* pFlowGraph)
{
   if (pFlowGraph != NULL)
   {
      if (!isManagedFlowGraph(pFlowGraph) ||
          !pFlowGraph->isStarted())
      {
         Nprintf("MpMT::handleSetFocus(0x%X) INVALID: %smanaged, %sstarted\n",
            (int) pFlowGraph,
            (int) (isManagedFlowGraph(pFlowGraph)? "" : "NOT "),
            (int) (pFlowGraph->isStarted() ? "" : "NOT "), 0,0,0);
         return FALSE; // we aren't managing this flow graph, return FALSE
      }
   }

   if (mpFocus != NULL)
   {
      // remove focus from the flow graph that currently has it
      Nprintf("MpMT::handleSetFocus(0x%X): removing old focus (0x%X)\n",
         (int) pFlowGraph, (int) mpFocus, 0,0,0,0);
      mpFocus->loseFocus();
   }

   mpFocus = pFlowGraph;
   if (mpFocus != NULL)
   {
      // try to give focus to the indicated flow graph
      if (OS_SUCCESS != mpFocus->gainFocus())
      {
         Nprintf("MpMT::handleSetFocus(0x%X): attempt to give focus FAILED\n",
            (int) pFlowGraph, 0,0,0,0,0);
         mpFocus = NULL;
         return FALSE; // the flow graph did not accept focus.
      }
      Nprintf("MpMT::handleSetFocus(0x%X): attempt to give focus SUCCEEDED\n",
         (int) pFlowGraph, 0,0,0,0,0);
   }

   return TRUE;
}

// Handles the START message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpMediaTask::handleStart(MpFlowGraphBase* pFlowGraph)
{
   OsStatus res;

   if (!isManagedFlowGraph(pFlowGraph))
      return FALSE;  // flow graph is not presently managed, return FALSE

   if (pFlowGraph->isStarted())   // if already started, return FALSE
      return FALSE;

   res = pFlowGraph->start();
   assert(res == OS_SUCCESS);

   mStartedCnt++;
   return TRUE;
}

// Handles the STOP message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpMediaTask::handleStop(MpFlowGraphBase* pFlowGraph)
{
   OsStatus res;

   if (pFlowGraph == mpFocus)
            handleSetFocus(NULL);

   if (!isManagedFlowGraph(pFlowGraph))
      return FALSE;  // flow graph is not presently managed, return FALSE

   if (pFlowGraph->getState() == MpFlowGraphBase::STOPPED)
      return FALSE;               // if already stopped, return FALSE

   res = pFlowGraph->stop();
   assert(res == OS_SUCCESS);

   mStartedCnt--;
   return TRUE;
}

// Handles the UNMANAGE message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpMediaTask::handleUnmanage(MpFlowGraphBase* pFlowGraph)
{
   OsLock lock(mMutex);
   UtlBoolean   found;
   int         i;
   OsStatus    res;

   if (pFlowGraph == mpFocus)
            handleSetFocus(NULL);

   if (!isManagedFlowGraph(pFlowGraph)) {
      return FALSE;  // flow graph is not presently managed, return FALSE
   }

   if (pFlowGraph->getState() != MpFlowGraphBase::STOPPED)
   {
      handleStop(pFlowGraph);

      // since we have "unmanaged" this flow graph, we need to coerce the
      // flow graph into processing its messages so that it gets the
      // indication that it has been stopped.
      res = pFlowGraph->processNextFrame();
      assert(res == OS_SUCCESS);
   }

   found = FALSE;
   for (i=0; i < mManagedCnt; i++)
   {
      if (found)
      {                           // compact the managed flow graphs array
         mManagedFGs[i-1] = mManagedFGs[i];
      }

      if (mManagedFGs[i] == pFlowGraph)
      {
         // PRINTF("MpMediaTask::handleUnmanage: Removing flow graph # %d!\n", i, 0,0,0,0,0);
         found = TRUE;
         mManagedFGs[i] = NULL;
      }
   }

   if (!found) {                  // we aren't managing this flow graph,
      return FALSE;               //  return FALSE
   }

   mManagedCnt--;
   return TRUE;
}

// Handles the WAIT_FOR_SIGNAL message.
// Performs the one-per-tick media processing as directed by the flow graph.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpMediaTask::handleWaitForSignal(MpMediaTaskMsg* pMsg)
{
   OsTime startTime;
   OsDateTime::getCurTime(startTime);
   OsTime signaledTime(pMsg->getInt1(), pMsg->getInt2());
   int              i;
   MpFlowGraphBase* pFlowGraph;
   OsStatus         res;

#ifdef MEDIA_VERBOSE /* [ */
   static int lastmMCnt = -1;
#endif /* MEDIA_VERBOSE ] */

#ifdef _PROFILE /* [ */
   {
      // Get the current time.
      long long now = (startTime.seconds() * 1000000) + startTime.usecs();
      // From the message, get the time that it was sent.
      long long signal_time =
         (pMsg->getInt1() * 1000000) + pMsg->getInt2();
      // Record intervals.
      mStartToStartTime.tally(now - mStartTicks);
      mEndToStartTime.tally(now - mStopTicks);
      mSignalToStartTime.tally(now - signal_time);
      // Record the processing start time.
      mStartTicks = now;
   }
#endif /* _PROFILE ] */

   // reset the handleMessage error count
   // mHandleMsgErrs = 0;

   mWaitForSignal = FALSE;

#ifdef MEDIA_VERBOSE /* [ */
   if (0 && (0 == (mProcessedCnt % 100))) {
      PRINTF("handleWaitForSignal: %d frames, %d managed flow graphs\n",
             mProcessedCnt, mManagedCnt, 0,0,0,0);
   }

   if (mManagedCnt != lastmMCnt)
   {
      // PRINTF("handleWaitForSignal: %d frames, %d managed flow graphs\n",
      mProcessedCnt, mManagedCnt, 0,0,0,0);
      lastmMCnt = mManagedCnt;
   }
#endif /* MEDIA_VERBOSE ] */

   // When this message is received we know that:
   // 1) We have received a frame start signal
   // 2) All of the messages that had been queued for this task at the
   //    time the frame start signal occurred have been processed.

#ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MpMediaTask::handleWaitForSignal starting frame processing for flowgraphs: %d",
         mManagedCnt);
#endif

   // Call processNextFrame() for each of the "started" flow graphs
   for (i=0; i < mManagedCnt; i++)
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MpMediaTask::handleWaitForSignal about to processNextFrame on flowgraph: %d",
         i);
#endif
      RTL_EVENT("MpMediaTask::handleWaitForSignal", i+1);
      pFlowGraph = mManagedFGs[i];
      if (pFlowGraph->isStarted())
      {
         res = pFlowGraph->processNextFrame();
         assert(res == OS_SUCCESS);
      }
   }
   RTL_EVENT("MpMediaTask::handleWaitForSignal", 0);
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MpMediaTask::handleWaitForSignal done with all flowgraphs");
#endif

#ifdef TEST_PRINT
   OsTime finishTime;
   OsDateTime::getCurTime(finishTime);
   OsTime queueTime = startTime - signaledTime;
   OsTime processTime = finishTime - startTime;
   int messagesUsed = mpSignalMsgPool->getNoInUse();

   {
      static int prev_mU = 12345, prev_SSM = 12345, prev_SSNQ = 12345;
      if ((prev_mU!=messagesUsed) || (prev_SSM!=sSignalStartsMissed) || (prev_SSNQ!=sSignalStartsNotQueued) || (0 == (0xff & sSignalStartsQueued)) /* mod 256 */
          || queueTime.seconds() || processTime.seconds() || ( queueTime.usecs() + processTime.usecs() > mLimitUsecs))
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
            "Finished frame queued: %d.%06d processing: %d.%06d queued signals: %d missed: %d not queued: %d queued: %d",
            // Down cast the following as seconds should be very small and usecs should not exceed 999,999
            (int)queueTime.seconds(), (int)queueTime.usecs(), (int)processTime.seconds(), (int)processTime.usecs(),
            messagesUsed,
            sSignalStartsMissed, sSignalStartsNotQueued, sSignalStartsQueued);
            prev_mU = messagesUsed;
            prev_SSM = sSignalStartsMissed;
            prev_SSNQ = sSignalStartsNotQueued;
      }
   }
#endif

#ifdef _PROFILE /* [ */
   {
      OsTime now;
      OsDateTime::getCurTime(now);
      // Record the processing stop time.
      mStopTicks = (now.seconds() * 1000000) + now.usecs();
      // if not debugging, determine whether the processing limit was exceeded
      if (!mDebugEnabled)
      {
         if ((mStopTicks - mStartTicks) >= mLimitTicks) {
            mTimeLimitCnt++;
         }
      }
      if (mStartToEndTime.tally(mStopTicks - mStartTicks) >= 1000)
      {
         UtlString* print;
         // Print mSignalToStartTime.
         print = mSignalToStartTime.show();
         OsSysLog::add(FAC_MP, PRI_NOTICE,
                       "MpMediaTask::handleWaitForSignal %-18s %d%s",
                       "mSignalToStartTime",
                       mSignalToStartTime.getBinSize(),
                       print->data());
         delete print;
         mSignalToStartTime.clear();
         // Print mStartToEndTime.
         print = mStartToEndTime.show();
         OsSysLog::add(FAC_MP, PRI_NOTICE,
                       "MpMediaTask::handleWaitForSignal %-18s %d%s",
                       "mStartToEndTime",
                       mStartToEndTime.getBinSize(),
                       print->data());
         delete print;
         mStartToEndTime.clear();
         // Print mStartToStartTime.
         print = mStartToStartTime.show();
         OsSysLog::add(FAC_MP, PRI_NOTICE,
                       "MpMediaTask::handleWaitForSignal %-18s %d%s",
                       "mStartToStartTime",
                       mStartToStartTime.getBinSize(),
                       print->data());
         delete print;
         mStartToStartTime.clear();
         // Print mEndToStartTime.
         print = mEndToStartTime.show();
         OsSysLog::add(FAC_MP, PRI_NOTICE,
                       "MpMediaTask::handleWaitForSignal %-18s %d%s",
                       "mEndToStartTime",
                       mEndToStartTime.getBinSize(),
                       print->data());
         delete print;
         mEndToStartTime.clear();
         // Print mOtherMessages.
         print = mOtherMessages.show();
         OsSysLog::add(FAC_MP, PRI_NOTICE,
                       "MpMediaTask::handleWaitForSignal %-18s %d%s",
                       "mOtherMessages",
                       mOtherMessages.getBinSize(),
                       print->data());
         delete print;
         mOtherMessages.clear();
      }
   }
#endif /* _PROFILE ] */

   assert(!mWaitForSignal);
   mProcessedCnt++;
#ifdef MEDIA_VERBOSE /* [ */
   if (0 && (0 == (mProcessedCnt % 125)))
   {
      PRINTF("handleWaitForSignal: %d frames\n", mProcessedCnt, 0,0,0,0,0);
   }
#endif /* MEDIA_VERBOSE ] */
   mWaitForSignal = TRUE;

   return TRUE;
}

// Returns TRUE if the indicated flow graph is presently being managed 
// by the media processing task, otherwise FALSE.
UtlBoolean MpMediaTask::isManagedFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   int i;
   OsLock lock(mMutex);

   for (i=0; i < mManagedCnt; i++)
   {
      if (mManagedFGs[i] == pFlowGraph) {
         return TRUE;
      }
   }

   return FALSE;
}

void MpMediaTask::flowgraphTickerCallback(const intptr_t userData,
                                          const intptr_t eventData)
{
   OsStatus result = MpMediaTask::signalFrameStart();
   if (result == OS_LIMIT_REACHED)
   {
      // OS_LIMIT_REACHED is returned when there are no more free messages
      // in the pool. This means that MediaTask is blocked for too long and
      // does not process messages and return them to pool. This is a normal
      // situation when you hit breakpoint during debugging, but in production
      // this is NOT normal and should be fixed.
      if (mIsBlockingReported == FALSE)
      {
         OsSysLog::add(FAC_MP, PRI_ERR,
                       "Something is blocking MediaTask for too long!");
         MpMediaTask::mIsBlockingReported = TRUE;
      }
   }
   else
   {
      assert(result == OS_SUCCESS);
      mIsBlockingReported = FALSE;
   }
}

/* ============================ FUNCTIONS ================================= */
