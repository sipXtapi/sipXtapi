// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

/* There used to be #ifdef's here to do the same thing on WIN32/VXWORKS, but I
* took them out because we do the same thing on every OS. -Mike */
#define PRINTF Zprintf

// SYSTEM INCLUDES
#include <assert.h>
#ifdef _PROFILE /* [ */
#include <sys/time.h>
#endif /* _PROFILE ] */

// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "os/OsMsgPool.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/MpMediaTask.h"
#include "mp/MpMediaTaskMsg.h"
#include "mp/MpBufferMsg.h"
#include "mp/MpCodecFactory.h"
#include "mp/MpCodec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#ifdef __pingtel_on_posix__ /* [ */
#define MPMEDIA_DEF_MAX_MSGS 1000
#else
#define MPMEDIA_DEF_MAX_MSGS OsServerTask::DEF_MAX_MSGS
#endif /* __pingtel_on_posix__ ] */

// STATIC VARIABLE INITIALIZATIONS
MpMediaTask* MpMediaTask::spInstance = 0;
OsBSem       MpMediaTask::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
int          MpMediaTask::mMaxFlowGraph = 0;


#ifdef _PROFILE /* [ */
   static unsigned long long sSignalTicks;  // Time (in microseconds) for the current
                                      //  frame start signal
   static unsigned long long sMinTicks;
   static unsigned long long sMaxTicks;

   static int  sHistogram[20];
   static int  sHistFrames;
   static int  sMaxHistFrames = 100000; // 16.6 minutes
#endif /* _PROFILE ] */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the media task, creating it if necessary
MpMediaTask* MpMediaTask::getMediaTask(int maxFlowGraph)
{
   UtlBoolean isStarted;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
       spInstance = new MpMediaTask(maxFlowGraph);

   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }
   sLock.release();

   return spInstance;
}

// Destructor
MpMediaTask::~MpMediaTask()
{
   // $$$ need to figure out how to cleanly shut down this task after
   // $$$ unmanaging and destroying all of its flow graphs

   delete[] mManagedFGs;
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

   res = postMessage(msg, OsTime::NO_WAIT);
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

   res = postMessage(msg, OsTime::NO_WAIT);
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
   MpMediaTaskMsg msg(MpMediaTaskMsg::SET_FOCUS, pFlowGraph);
   OsStatus       res;

   res = postMessage(msg, OsTime::NO_WAIT);
   assert(res == OS_SUCCESS);

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
OsStatus MpMediaTask::signalFrameStart(void)
{
   OsStatus ret = OS_TASK_NOT_STARTED;
   MpMediaTaskMsg* pMsg;

   if (0 != spInstance) {

      pMsg = (MpMediaTaskMsg*) spInstance->mpSignalMsgPool->findFreeMsg();
      if (NULL == pMsg) {
         ret = OS_LIMIT_REACHED;
      } else {
         ret = spInstance->postMessage(*pMsg, OsTime::NO_WAIT);
#ifdef _PROFILE /* [ */
         if (OS_SUCCESS == ret) {
            timeval t;
            gettimeofday(&t, NULL);
            sSignalTicks = (t.tv_sec * 1000000) + t.tv_usec; // record the time
         }
#endif /* _PROFILE, __pingtel_on_posix__ ] */
      }
   }
   return ret;
}

#ifdef _PROFILE /* [ */
int MpMediaTask::setHistInterval(int frames)
{
   int save = sMaxHistFrames;
   if (frames > 49) sMaxHistFrames = frames;
   return save;
}

int MpMediaTask::showHistogram(void)
{
   long mini, minf, maxi, maxf;
   char buf[500];
   int  s0,  s1,  s2,  s3,  s4,  s5,  s6,  s7,  s8,  s9;
   int s10, s11, s12, s13, s14, s15, s16, s17, s18, s19;

   s0 = sHistogram[0];
   s1 = sHistogram[1];
   s2 = sHistogram[2];
   s3 = sHistogram[3];
   s4 = sHistogram[4];
   s5 = sHistogram[5];
   s6 = sHistogram[6];
   s7 = sHistogram[7];
   s8 = sHistogram[8];
   s9 = sHistogram[9];
   s10 = sHistogram[10];
   s11 = sHistogram[11];
   s12 = sHistogram[12];
   s13 = sHistogram[13];
   s14 = sHistogram[14];
   s15 = sHistogram[15];
   s16 = sHistogram[16];
   s17 = sHistogram[17];
   s18 = sHistogram[18];
   s19 = sHistogram[19];

   mini = sMinTicks;
   minf = mini % 1000;
   mini = mini / 1000;

   maxi = sMaxTicks;
   maxf = maxi % 1000;
   maxi = maxi / 1000;

   memset(&sHistogram[0], 0, 20*sizeof(int));
   sprintf(buf,
   "%d %d %d %d %d  %d %d %d %d %d * %d %d %d %d %d",
      s0,  s1,  s2,  s3,  s4,  s5,  s6,  s7,  s8,  s9,
      s10 + s11, s12 + s13, s14 + s15, s16 + s17, s18 + s19);

   osPrintf("Histogram (%d frames, range:%lu.%03lu msec. - %lu.%03lu msec.): %s\n",
            sHistFrames, mini, minf, maxi, maxf, buf);
   sHistFrames = 0;
   sMinTicks = 100000000;
   sMaxTicks = 0;
}
#endif /* _PROFILE ] */

// Directs the media processing task to start the specified flow 
// graph.  A flow graph must be started in order for it to process 
// the media stream.
// The flow graph state change will take effect at the beginning of the
// next frame interval. For now, this method always returns OS_SUCCESS.
OsStatus MpMediaTask::startFlowGraph(MpFlowGraphBase& rFlowGraph)
{
   MpMediaTaskMsg msg(MpMediaTaskMsg::START, &rFlowGraph);
   OsStatus       res;

   res = postMessage(msg, OsTime::NO_WAIT);
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

   res = postMessage(msg, OsTime::NO_WAIT);
   assert(res == OS_SUCCESS);

   return OS_SUCCESS;
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

   pMediaTask = MpMediaTask::getMediaTask(0);

   printf("\nMedia processing task information\n");
   printf("  Debug mode:                      %s\n",
             pMediaTask->getDebugMode() ? "TRUE" : "FALSE");

   printf("  Processed Frame Count:           %d\n",
             pMediaTask->numProcessedFrames());

   printf("  Processing Time Limit:           %d usecs\n",
             pMediaTask->getTimeLimit());

   printf("  Processing Limit Exceeded Count: %d\n",
             pMediaTask->getLimitExceededCnt());

   i = pMediaTask->getWaitTimeout();
   if (i < 0)
      printf("  Frame Start Wait Timeout:        INFINITE\n");
   else
      printf("  Frame Start Wait Timeout:        %d\n", i);

   printf("  Wait Timeout Exceeded Count:     %d\n",
             pMediaTask->getWaitTimeoutCnt());

   printf("\n  Flow Graph Information\n");
   printf("    Managed:      %d\n", pMediaTask->numManagedFlowGraphs());
   printf("    Started:      %d\n", pMediaTask->numStartedFlowGraphs());

   pFlowGraph = pMediaTask->getFocus();
   if (pFlowGraph == NULL)
      printf("    Focus:        NULL\n");
   else
      printf("    Focus:        %p\n", pFlowGraph);

   res = pMediaTask->getManagedFlowGraphs(flowGraphs, 20, numItems);
   for (i=0; i < numItems; i++)
      printf("    FlowGraph[%d]: %p\n", i, flowGraphs[i]);
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

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getMediaTask())
MpMediaTask::MpMediaTask(int maxFlowGraph)
:  OsServerTask("MpMedia", NULL, MPMEDIA_DEF_MAX_MSGS,
                MEDIA_TASK_PRIO_NORMAL),
   mMutex(OsMutex::Q_PRIORITY),  // create mutex for protecting data
   mDebugEnabled(FALSE),
   mTimeLimitCnt(0),
   mProcessedCnt(0),
   mManagedCnt(0),
   mStartedCnt(0),
   mSemTimeout(DEF_SEM_WAIT_MSECS / 1000, (DEF_SEM_WAIT_MSECS % 1000) * 1000),
   mSemTimeoutCnt(0),
   mWaitForSignal(TRUE),
   mpFocus(NULL),
   mHandleMsgErrs(0),
   mpBufferMsgPool(NULL),
   // numQueuedMsgs(0)
   mpSignalMsgPool(NULL)
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
      int totalNumBufs = MpBuf_getTotalBufferCount();
      int soft;
      int incr;
      MpBufferMsg* pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);

      soft = totalNumBufs/20;
      if (soft < 8) soft = 8;
      incr = soft / 2 + 1;
      mpBufferMsgPool = new OsMsgPool("MediaBuffers", (*(OsMsg*)pMsg),
                          incr, soft, totalNumBufs, incr,
                          OsMsgPool::MULTIPLE_CLIENTS);
   }

   {
      MpMediaTaskMsg* pMsg =
         new MpMediaTaskMsg(MpMediaTaskMsg::WAIT_FOR_SIGNAL);
      mpSignalMsgPool = new OsMsgPool("MediaSignals", (*(OsMsg*)pMsg),
                          2, 2*mMaxFlowGraph, 4*mMaxFlowGraph, 1,
                          OsMsgPool::MULTIPLE_CLIENTS);
   }

   mpCodecFactory = MpCodecFactory::getMpCodecFactory();
#ifdef _PROFILE /* [ */
   for (i=0; i < 20; i++) {
      sHistogram[i] = 0;
   }
   mStartTicks = 0;
   mStopTicks = 0;
   sSignalTicks = 0;
   sHistFrames = 0;
   sMinTicks = 0;
   sMaxTicks = 0;
#endif /* _PROFILE ] */
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

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
      if (!handleWaitForSignal())
         mHandleMsgErrs++;
      break;
   default:
      handled = FALSE; // we didn't handle the message after all
      break;
   }

   return handled;
}

// Handles the MANAGE message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpMediaTask::handleManage(MpFlowGraphBase* pFlowGraph)
{
   OsLock lock(mMutex);

   if (mManagedCnt >= mMaxFlowGraph) {
      // PRINTF("MpMediaTask::handleManage: ERROR: too many flow graphs!\n", 0,0,0,0,0,0);
      return FALSE;
   }

   if (isManagedFlowGraph(pFlowGraph)) { // we are already managing
      // PRINTF("MpMediaTask::handleManage: ERROR: flow graph already managed!\n", 0,0,0,0,0,0);
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
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpMediaTask::handleWaitForSignal(void)
{
   int              i;
   MpFlowGraphBase* pFlowGraph;
   OsStatus         res;

#ifdef MEDIA_VERBOSE /* [ */
   static int lastmMCnt = -1;
#endif /* MEDIA_VERBOSE ] */

 ///////////////////////////////////////////////////////////////////
   {
      MpCodec_doProcessFrame();

#ifdef _PROFILE /* [ */
      timeval t;
      gettimeofday(&t, NULL);
      mStartTicks = (t.tv_sec * 1000000) + t.tv_usec; // record the processing start time
#endif /* _PROFILE ] */

      // reset the handleMessage error count
      // mHandleMsgErrs = 0;

      mWaitForSignal = FALSE;
   }
 ///////////////////////////////////////////////////////////////////

#ifdef MEDIA_VERBOSE /* [ */
   if (0 && (0 == (mProcessedCnt % 100))) {
      PRINTF("handleWaitForSignal: %d frames, %d managed flow graphs\n",
         mProcessedCnt, mManagedCnt, 0,0,0,0);
   }

   if (mManagedCnt != lastmMCnt) {
      // PRINTF("handleWaitForSignal: %d frames, %d managed flow graphs\n",
         mProcessedCnt, mManagedCnt, 0,0,0,0);
      lastmMCnt = mManagedCnt;
   }
#endif /* MEDIA_VERBOSE ] */

   // When this message is received we know that:
   // 1) We have received a frame start signal
   // 2) All of the messages that had been queued for this task at the
   //    time the frame start signal occurred have been processed.

   // Call processNextFrame() for each of the "started" flow graphs
   for (i=0; i < mManagedCnt; i++)
   {
      pFlowGraph = mManagedFGs[i];
      if (pFlowGraph->isStarted())
      {
         res = pFlowGraph->processNextFrame();
         assert(res == OS_SUCCESS);
      }
   }

#ifdef _PROFILE /* [ */
   timeval t;
   gettimeofday(&t, NULL);
   mStopTicks = (t.tv_sec * 1000000) + t.tv_usec; // record the processing stop time
   // if not debugging, determine whether the processing limit was exceeded
   if (!mDebugEnabled)
   {
      if ((mStopTicks - mStartTicks) >= mLimitTicks) {
         mTimeLimitCnt++;
      }
   }
   unsigned long long delta = (mStopTicks - sSignalTicks);
   if (delta < sMinTicks) sMinTicks = delta;
   if (delta > sMaxTicks) sMaxTicks = delta;
   delta = delta / 1000;
   if (delta < 1) delta = 0;
   if (delta > 18) delta = 19;
   sHistogram[delta]++;
   sHistFrames++;
   if (sHistFrames >= sMaxHistFrames) showHistogram();
#endif /* _PROFILE ] */

   assert(!mWaitForSignal);
   mProcessedCnt++;
#ifdef MEDIA_VERBOSE /* [ */
   if (0 && (0 == (mProcessedCnt % 125))) {
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

/* ============================ FUNCTIONS ================================= */

