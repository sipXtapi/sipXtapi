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


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsLock.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpFlowGraphMsg.h>
#include <mp/MpResourceMsg.h>
#include <mp/MpResource.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType MpResource::TYPE = "MpResource";
const OsTime MpResource::sOperationQueueTimeout = OsTime::NO_WAIT_TIME;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpResource::MpResource(const UtlString& rName, int minInputs, int maxInputs,
                       int minOutputs, int maxOutputs)
: UtlString(rName) //,mName(rName) -- resource name is now stored in parent utlString
, mpFlowGraph(NULL)
, mIsEnabled(FALSE)
, mRWMutex(OsRWMutex::Q_PRIORITY)
, mMaxInputs(maxInputs)
, mMaxOutputs(maxOutputs)
, mMinInputs(minInputs)
, mMinOutputs(minOutputs)
, mNumActualInputs(0)
, mNumActualOutputs(0)
, mVisitState(NOT_VISITED)
, mNotificationsEnabled(FALSE)
, mLock(OsBSem::Q_FIFO, OsBSem::FULL)
{
   int i;   

   // Perform a sanity check on the input arguments
   assert((minInputs >= 0) && (minOutputs >= 0) &&
          (maxInputs >= 0) && (maxOutputs >= 0) &&
          (minInputs <= maxInputs) && (minOutputs <= maxOutputs));

   // Allocate arrays for input/output link objects and input/output
   // buffer.  Size the arrays so as to accommodate the maximum number of
   // input and output links supported for this resource.
   OsLock lock(mLock);
   mpInConns  = new Conn[maxInputs];
   mpOutConns = new Conn[maxOutputs];
   mpInBufs   = new MpBufPtr[maxInputs];
   mpOutBufs  = new MpBufPtr[maxOutputs];

   for (i=0; i < maxInputs; i++)       // initialize the input port storage
   {
      mpInConns[i].pResource = NULL;
      mpInConns[i].portIndex = -1;
      mpInConns[i].reserved = FALSE;
      mpInBufs[i].release();
   }

   for (i=0; i < maxOutputs; i++)      // initialize the output port storage
   {
      mpOutConns[i].pResource = NULL;
      mpOutConns[i].portIndex = -1;
      mpOutConns[i].reserved = FALSE;
      mpOutBufs[i].release();
   }
}

// Destructor
MpResource::~MpResource()
{
   int i;

   for (i=0; i < mMaxInputs; i++)
      mpInBufs[i].release();       // free all input buffers

   for (i=0; i < mMaxOutputs; i++)
      mpOutBufs[i].release();      // free all output buffers

   delete[] mpInConns;
   mpInConns = NULL;
   delete[] mpOutConns;
   mpOutConns = NULL;

   delete[] mpInBufs;
   mpInBufs = NULL;
   delete[] mpOutBufs;
   mpOutBufs = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Disable this resource.
// Returns TRUE if successful, FALSE otherwise.
// The "enabled" flag is passed to the doProcessFrame() method
// and will likely affect the media processing that is performed by this
// resource.  Typically, if a resource is not enabled,
// doProcessFrame() will perform only minimal processing (for
// of a one input / one output resource).
UtlBoolean MpResource::disable(void)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_DISABLE, this);
   OsStatus       res;

   res = postMessage(msg);
   return (res == OS_SUCCESS);
}

OsStatus MpResource::disable(const UtlString& namedResource,
                             OsMsgQ& fgQ)
{
   MpResourceMsg msg(MpResourceMsg::MPRM_RESOURCE_DISABLE, namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

// Enable this resource.
// Returns TRUE if successful, FALSE otherwise.
// The "enabled" flag is passed to the doProcessFrame() method
// and will likely affect the media processing that is performed by this
// resource.  Typically, if a resource is not enabled,
// doProcessFrame() will perform only minimal processing (for
// example, passing the input straight through to the output in the case
// of a one input / one output resource).
UtlBoolean MpResource::enable(void)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_ENABLE, this);
   OsStatus       res;

   res = postMessage(msg);
   return (res == OS_SUCCESS);
}

OsStatus MpResource::enable(const UtlString& namedResource,
                            OsMsgQ& fgQ)
{
   MpResourceMsg msg(MpResourceMsg::MPRM_RESOURCE_ENABLE, namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MpResource::setAllNotificationsEnabled(UtlBoolean enable,
                                                const UtlString& namedResource, 
                                                OsMsgQ& fgQ)
{
   // Default to disabling all resource msgs -- then check if it is enable.
   MpResourceMsg msg(MpResourceMsg::MPRM_DISABLE_ALL_NOTIFICATIONS, namedResource);

   // If we're enabling instead, then set it to enable all resource notifications.
   if(enable)
   {
      msg.setMsgSubType(MpResourceMsg::MPRM_ENABLE_ALL_NOTIFICATIONS);
   }

   return fgQ.send(msg, sOperationQueueTimeout);
}

// Sets the visit state for this resource (used in performing a 
// topological sort on the resources contained within a flow graph).
void MpResource::setVisitState(int newState)
{
   assert(newState >= NOT_VISITED && newState <= FINISHED);

   mVisitState = newState;
}

/* ============================ ACCESSORS ================================= */

// (static) Displays information on the console about the specified flow
// graph.
void MpResource::resourceInfo(MpResource* pResource, int index)
{
   if(pResource)
   {
       int         i;
       const char*       name;

       name = pResource->getName();
       osPrintf("    Resource[%d]: %p, %s (%sabled)\n",
              index, pResource, name, pResource->mIsEnabled ? "En" : "Dis");

       OsLock lock(pResource->mLock);
       for (i=0; i<pResource->mMaxInputs; i++) 
       {
          if (NULL != pResource->mpInConns[i].pResource) 
          {
             name = pResource->mpInConns[i].pResource->getName();
             osPrintf("        Input %d from %s:%d\n", i, 
                name, pResource->mpInConns[i].portIndex);
          }
          else if(pResource->mpInConns[i].reserved)
          {
             osPrintf("        Input %d reserved", i);
          }
          else
          {
             osPrintf("        Input %d  not reserved", i);
          }
       }

       for (i=0; i<pResource->mMaxOutputs; i++) 
       {
          if (NULL != pResource->mpOutConns[i].pResource) 
          {
             name = pResource->mpOutConns[i].pResource->getName();
             osPrintf("        Output %d to %s:%d\n", i, 
                name, pResource->mpOutConns[i].portIndex);
          }
          else if(pResource->mpOutConns[i].reserved)
          {
             osPrintf("        Output %d reserved", i);
          }
          else
          {
             osPrintf("        Output %d  not reserved", i);
          }
       }
   }
}


// Returns the flow graph that contains this resource or NULL if the 
// resource is not presently part of any flow graph.
MpFlowGraphBase* MpResource::getFlowGraph(void) const
{
   return mpFlowGraph;
}

// Returns information about the upstream end of a link to the 
// "inPortIdx" input on this resource.  If "inPortIdx" is invalid or
// there is no link, then "rpUpstreamResource" will be set to NULL.
void MpResource::getInputInfo(int inPortIdx, MpResource*& rpUpstreamResource,
                              int& rUpstreamPortIdx)
{
   if (inPortIdx < 0 || inPortIdx >= mMaxInputs)
   {
      rpUpstreamResource = NULL;        // inPortIdx is out of range
      rUpstreamPortIdx   = -1;
   }
   else
   {
      OsLock lock(mLock);
      rpUpstreamResource = mpInConns[inPortIdx].pResource;
      rUpstreamPortIdx   = mpInConns[inPortIdx].portIndex;
   }
}

// Returns the name associated with this resource.
UtlString MpResource::getName(void) const
{
   return *this;
}

// Returns information about the downstream end of a link to the 
// "outPortIdx" output on this resource.  If "outPortIdx" is invalid or
// there is no link, then "rpDownstreamResource" will be set to NULL.
void MpResource::getOutputInfo(int outPortIdx,
                               MpResource*& rpDownstreamResource,
                               int& rDownstreamPortIdx)
{
   if (outPortIdx < 0 || outPortIdx >= mMaxOutputs)
   {
      rpDownstreamResource = NULL;      // outPortIdx is out of range
      rDownstreamPortIdx   = -1;
   }
   else
   {
      OsLock lock(mLock);
      rpDownstreamResource = mpOutConns[outPortIdx].pResource;
      rDownstreamPortIdx   = mpOutConns[outPortIdx].portIndex;
   }
}

// Returns the current visit state for this resource (used in performing 
// a topological sort on the resources contained within a flow graph).
int MpResource::getVisitState(void)
{
   return mVisitState;
}

// Returns the maximum number of inputs supported by this resource.
int MpResource::maxInputs(void) const
{
   return mMaxInputs;
}

// Returns the maximum number of outputs supported by this resource.
int MpResource::maxOutputs(void) const
{
   return mMaxOutputs;
}

// Returns the minimum number of inputs required by this resource.
int MpResource::minInputs(void) const
{
   return mMinInputs;
}

// Returns the minimum number of outputs required by this resource.
int MpResource::minOutputs(void) const
{
   return mMinOutputs;
}

// Returns the number of resource inputs that are currently connected.
int MpResource::numInputs(void) const
{
   return mNumActualInputs;
}

// Returns the number of resource outputs that are currently connected.
int MpResource::numOutputs(void) const
{
   return mNumActualOutputs;
}

int MpResource::reserveFirstUnconnectedInput()
{
   int i;
   int portIndex = -1;
   OsLock lock(mLock);
   for (i = 0; i < mMaxInputs; i++)       // initialize the input port storage
   {
      if(mpInConns[i].pResource == NULL && mpInConns[i].reserved == FALSE)
      {
          mpInConns[i].reserved = TRUE;
          portIndex = i;
          break;
      }
   }
   return(portIndex);
}

int MpResource::reserveFirstUnconnectedOutput()
{
   int i;
   int portIndex = -1;
   OsLock lock(mLock);
   for (i = 0; i < mMaxOutputs; i++)       // initialize the input port storage
   {
      if(mpOutConns[i].pResource == NULL && mpOutConns[i].reserved == FALSE)
      {
          mpOutConns[i].reserved = TRUE;
          portIndex = i;
          break;
      }
   }
   return(portIndex);
}

// Get the ContainableType for a UtlContainable derived class.
/*UtlContainableType MpResource::getContainableType() const
{
    return TYPE;
}*/

/* ============================ INQUIRY =================================== */

// Returns TRUE is this resource is currently enabled, FALSE otherwise.
UtlBoolean MpResource::isEnabled(void) const
{
   return mIsEnabled;
}

// Returns TRUE if portIdx is valid and the indicated input is connected,
// FALSE otherwise.
UtlBoolean MpResource::isInputConnected(int portIdx)
{
   if (portIdx < 0 || portIdx >= mMaxInputs)  // portIdx out of range
      return FALSE;

   OsLock lock(mLock);
   UtlBoolean isConnected = (mpInConns[portIdx].pResource != NULL);
   return(isConnected);
}

// Returns TRUE if portIdx is valid and the indicated input is not connected,
// FALSE otherwise.
UtlBoolean MpResource::isInputUnconnected(int portIdx)
{
   if (portIdx < 0 || portIdx >= mMaxInputs)  // portIdx out of range
      return FALSE;

   OsLock lock(mLock);
   UtlBoolean isUnconnected = (mpInConns[portIdx].pResource == NULL);
   return(isUnconnected);
}

// Returns TRUE if portIdx is valid and the indicated output is connected,
// FALSE otherwise.
UtlBoolean MpResource::isOutputConnected(int portIdx)
{
   if (portIdx < 0 || portIdx >= mMaxOutputs) // portIdx out of range
      return FALSE;

   OsLock lock(mLock);
   UtlBoolean isConnected = (mpOutConns[portIdx].pResource != NULL);
   return(isConnected);
}

// Returns TRUE if portIdx is valid and the indicated output is not connected,
// FALSE otherwise.
UtlBoolean MpResource::isOutputUnconnected(int portIdx)
{
   if (portIdx < 0 || portIdx >= mMaxOutputs) // portIdx out of range
      return FALSE;

   OsLock lock(mLock);
   UtlBoolean isUnconnected = (mpOutConns[portIdx].pResource == NULL);
   return(isUnconnected);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Handles an incoming flowgraph message for this media processing object.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpResource::handleMessage(MpFlowGraphMsg& fgMsg)
{
   UtlBoolean msgHandled = FALSE;

   msgHandled = TRUE; // assume we'll handle the msg
   switch (fgMsg.getMsg())
   {
   case MpFlowGraphMsg::RESOURCE_DISABLE:   // disable this resource
      msgHandled = handleDisable();
      break;
   case MpFlowGraphMsg::RESOURCE_ENABLE:    // enable this resource
      msgHandled = handleEnable();
      break;
   default:
      msgHandled = FALSE; // we didn't handle the msg after all
      break;
   }
   
   return msgHandled;
}

// Handles an incoming resource message for this media processing object.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpResource::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   // Do stuff for resource messages.
   msgHandled = TRUE; // assume we'll handle the msg
   switch (rMsg.getMsg())
   {
   case MpResourceMsg::MPRM_RESOURCE_DISABLE:   // disable this resource
      msgHandled = handleDisable();
      break;
   case MpResourceMsg::MPRM_RESOURCE_ENABLE:    // enable this resource
      msgHandled = handleEnable();
      break;
   case MpResourceMsg::MPRM_DISABLE_ALL_NOTIFICATIONS:
      // Disable all notifications sent out from this resource.
      mNotificationsEnabled = FALSE;
      break;
   case MpResourceMsg::MPRM_ENABLE_ALL_NOTIFICATIONS:
      // Enable all notifications sent out from this resource.
      mNotificationsEnabled = TRUE;
      break;
   default:
      msgHandled = FALSE; // we didn't handle the msg after all
      break;
   }

   return msgHandled;
}

UtlBoolean MpResource::handleMessages(OsMsgQ& msgQ)
{
   UtlBoolean handledAllMsgs = FALSE;
   while(!msgQ.isEmpty())
   {
      OsMsg* msg;
      OsStatus recvStat = msgQ.receive(msg, OsTime::NO_WAIT_TIME);
      UtlBoolean curMsgHandled = FALSE;
      if (recvStat == OS_SUCCESS)
      {
         if (msg->getMsgType() == OsMsg::MP_FLOWGRAPH_MSG)
         {
            MpFlowGraphMsg* fgMsg = dynamic_cast<MpFlowGraphMsg*>(msg);
            if (fgMsg != NULL)
            {
               curMsgHandled = handleMessage(*fgMsg);
            }
         }
         else if (msg->getMsgType() == OsMsg::MP_RESOURCE_MSG)
         {
            MpResourceMsg* rMsg = dynamic_cast<MpResourceMsg*>(msg);
            if (rMsg != NULL)
            {
               curMsgHandled = handleMessage(*rMsg);
            }
         }

         // TODO: If message received is not handled, it might be good
         //       to stuff the message back into the front of the queue
         //       with msgQ.sendUrgent(msg, OsTime::NO_WAIT_TIME);
         //       I haven't thought through the ramifications of this,
         //       so I haven't implemented it yet.
      }

      // Stop at the first message we encounter that is not handled.
      if (curMsgHandled == FALSE)
      {
         break;
      }

      // If the queue is empty at this point,
      // then all messages have been handled.
      if (msgQ.isEmpty())
      {
         handledAllMsgs = TRUE;
      }
   }
   return handledAllMsgs;
}

UtlBoolean MpResource::handleEnable()
{
    mIsEnabled = TRUE;
    return(TRUE);
}

UtlBoolean MpResource::handleDisable()
{
    mIsEnabled = FALSE;
    return(TRUE);
}

// If there already is a buffer stored for this input port, delete it.
// Then store pBuf for the indicated input port.
void MpResource::setInputBuffer(int inPortIdx, const MpBufPtr &pBuf)
{
   // make sure we have a valid port that is connected to a resource
   // Not locking mpInConns here as we only lock for setting/accessing
   // reservation state (the only thing set in mpInConns outside media task)
   assert((inPortIdx >= 0) && (inPortIdx < mMaxInputs) &&
          (mpInConns[inPortIdx].pResource != NULL));

   mpInBufs[inPortIdx] = pBuf;    // store the new buffer
}

// Makes pBuf available to resource connected to the outPortIdx output
// port of this resource.
// Returns TRUE if there is a resource connected to the specified output
// port, FALSE otherwise.
UtlBoolean MpResource::pushBufferDownsream(int outPortIdx, const MpBufPtr &pBuf)
{
   MpResource* pDownstreamInput;
   int         downstreamPortIdx;

   // Not locking mpOutConns here as we only lock for setting/accessing
   // reservation state (the only thing set in mpOutConns outside media task)

   if (outPortIdx < 0 || outPortIdx >= mMaxOutputs)  // port  out of range
      return FALSE;

   pDownstreamInput  = mpOutConns[outPortIdx].pResource;
   downstreamPortIdx = mpOutConns[outPortIdx].portIndex;
   if (pDownstreamInput == NULL)                     // no connected resource
      return FALSE;

   pDownstreamInput->setInputBuffer(downstreamPortIdx, pBuf);
   return TRUE;
}

// Post a message to this resource.
// If this resource is not part of a flow graph, then rMsg is
// immediately passed to the handleMessage() method for this
// resource.  If this resource is part of a flow graph, then
// rMsg will be sent to the message queue for the flow graph
// that this resource belongs to.  The handleMessage() method
// for this resource will be invoked at the start of the next frame
// processing interval.
OsStatus MpResource::postMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean res;

   if (mpFlowGraph == NULL)
   {
      res = handleMessage(rMsg);
      assert(res);
      return OS_SUCCESS;
   }
   else
   {
      return mpFlowGraph->postMessage(rMsg, OsTime::NO_WAIT_TIME);
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Connects the toPortIdx input port on this resource to the 
// fromPortIdx output port of the rFrom resource.
// Returns TRUE if successful, FALSE otherwise.
UtlBoolean MpResource::connectInput(MpResource& rFrom, int fromPortIdx,
                                   int toPortIdx)
{
   if (toPortIdx < 0 ||                // bad port index
       toPortIdx >= mMaxInputs)        // bad port index
      return FALSE;

   OsLock lock(mLock);
   mpInBufs[toPortIdx].release();
   mpInConns[toPortIdx].pResource = &rFrom;
   mpInConns[toPortIdx].portIndex = fromPortIdx;
   mpInConns[toPortIdx].reserved = FALSE;

   mNumActualInputs++;

   return TRUE;
}

// Connects the fromPortIdx output port on this resource to the 
// toPortIdx input port of the rTo resource.
// Returns TRUE if successful, FALSE otherwise.
UtlBoolean MpResource::connectOutput(MpResource& rTo, int toPortIdx,
                                    int fromPortIdx)
{
   if (fromPortIdx < 0 ||              // bad port index
       fromPortIdx >= mMaxOutputs)     // bad port index
      return FALSE;

   OsLock lock(mLock);
   mpOutBufs[fromPortIdx].release();
   mpOutConns[fromPortIdx].pResource = &rTo;
   mpOutConns[fromPortIdx].portIndex = toPortIdx;
   mpOutConns[fromPortIdx].reserved = FALSE;

   mNumActualOutputs++;

   return TRUE;
}

// Removes the link to the inPortIdx input port of this resource.
// Returns TRUE if successful, FALSE otherwise.
UtlBoolean MpResource::disconnectInput(int inPortIdx)
{
   OsLock lock(mLock);
   if (mpInConns[inPortIdx].pResource == NULL || // no connected resource
       inPortIdx < 0 ||                          // bad port index
       inPortIdx >= mMaxInputs)                  // bad port index
      return FALSE;

   mpInBufs[inPortIdx].release();
   mpInConns[inPortIdx].pResource = NULL;
   mpInConns[inPortIdx].portIndex = -1;
   mpInConns[inPortIdx].reserved = FALSE;

   mNumActualInputs--;

   return TRUE;
}

// Removes the link to the outPortIdx output port of this resource.
// Returns TRUE if successful, FALSE otherwise.
UtlBoolean MpResource::disconnectOutput(int outPortIdx)
{
   OsLock lock(mLock);
   if (mpOutConns[outPortIdx].pResource == NULL || // no connected resource
       outPortIdx < 0 ||                           // bad port index
       outPortIdx >= mMaxOutputs)                  // bad port index
      return FALSE;

   mpOutBufs[outPortIdx].release();
   mpOutConns[outPortIdx].pResource = NULL;
   mpOutConns[outPortIdx].portIndex = -1;
   mpOutConns[outPortIdx].reserved = FALSE;

   mNumActualOutputs--;

   return TRUE;
}

// Associates this resource with the indicated flow graph.
// For now, this method always returns success
OsStatus MpResource::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   mpFlowGraph = pFlowGraph;

   return OS_SUCCESS;
}

// Sets the name that is associated with this resource.
void MpResource::setName(const UtlString& rName)
{
   *((UtlString*)this) = rName;
}

/* ============================ FUNCTIONS ================================= */

