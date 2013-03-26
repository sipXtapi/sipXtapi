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

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpRtpInputConnection.h"
#include "mp/MprFromNet.h"
#include "mp/MprDecode.h"
#include "mp/MprRtpDispatcher.h"
#include "mp/MprRtpDispatcherIpAffinity.h"
#include "mp/MprRtpDispatcherActiveSsrcs.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MpIntResourceMsg.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/INetDispatch.h"
#include "rtcp/IRTPDispatch.h"
#include "rtcp/ISetSenderStatistics.h"
#include "rtcp/IRTCPSession.h"
#include "rtcp/IRTCPConnection.h"
#else
#include "os/OsDateTime.h"
#endif /* INCLUDE_RTCP ] */
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpRtpInputConnection::MpRtpInputConnection(const UtlString& resourceName,
                                           MpConnectionID myID, 
                                           IRTCPSession *piRTCPSession, // ignored...
                                           int maxRtpStreams,
                                           MprRtpDispatcher::RtpStreamAffinity rtpStreamAffinity)
: MpResource(resourceName, 0, 0, 0, maxRtpStreams)
, mpFromNet(NULL)
, mpRtpDispatcher(NULL)
, mMaxRtpStreams(maxRtpStreams)
, mRtpStreamAffinity(rtpStreamAffinity)
, mIsRtpStarted(FALSE)
#ifdef INCLUDE_RTCP /* [ */
// , mpiRTCPSession(piRTCPSession)
, mpiRTCPConnection(NULL)
#endif /* INCLUDE_RTCP ] */
{
   mConnectionId = myID;

   // Create our resources
   {
      UtlString name = getName() + "-RtpDispatcher";
      switch (mRtpStreamAffinity)
      {
      case MprRtpDispatcher::ADDRESS_AND_PORT:
         // We may accept only one RTP stream in this case.
         assert(mMaxRtpStreams == 1);
         mpRtpDispatcher = new MprRtpDispatcherIpAffinity(name, mConnectionId);
         break;
      case MprRtpDispatcher::MOST_RECENT_SSRC:
         mpRtpDispatcher = new MprRtpDispatcherActiveSsrcs(name, mConnectionId,
                                                           mMaxRtpStreams);
         break;
      default:
         assert(false);
         break;
      }
   }
   mpFromNet = new MprFromNet();
   mpFromNet->setRtpDispatcher(mpRtpDispatcher);
}

// Destructor
MpRtpInputConnection::~MpRtpInputConnection()
{
   delete mpFromNet;
   delete mpRtpDispatcher;

#ifdef INCLUDE_RTCP /* [ */
// Let's free our RTCP Connection
//  Actually... let's don't...
// if(mpiRTCPSession)
// {
//     mpiRTCPSession->TerminateRTCPConnection(mpiRTCPConnection); 
// }
#endif /* INCLUDE_RTCP ] */
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean MpRtpInputConnection::processFrame()
{
   mpRtpDispatcher->checkRtpStreamsActivity();

   return TRUE;
}

void MpRtpInputConnection::setSockets(OsSocket& rRtpSocket,
                                      OsSocket& rRtcpSocket)
{
   mpFromNet->setSockets(rRtpSocket, rRtcpSocket);
   mIsRtpStarted = TRUE;
}

void MpRtpInputConnection::releaseSockets()
{
   mpFromNet->resetSockets();
   mIsRtpStarted = FALSE;
}


void MpRtpInputConnection::setConnectionId(MpConnectionID connectionId)
{
   // Set connection ID for this resource.
   MpResource::setConnectionId(connectionId);

   // Set connection ID to contained resources.
   mpRtpDispatcher->setConnectionId(connectionId);
}

OsStatus MpRtpInputConnection::setRtpInactivityTimeout(const UtlString& namedResource,
                                                       OsMsgQ& fgQ,
                                                       int timeoutMs)
{
   MpIntResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_SET_INACTIVITY_TIMEOUT,
                        namedResource, timeoutMs);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MpRtpInputConnection::enableSsrcDiscard(const UtlString& namedResource,
                                                 OsMsgQ& fgQ,
                                                 UtlBoolean enable, RtpSRC ssrc)
{
   MpIntResourceMsg msg(enable?(MpResourceMsg::MpResourceMsgType)MPRM_ENABLE_SSRC_DISCARD
                              :(MpResourceMsg::MpResourceMsgType)MPRM_DISABLE_SSRC_DISCARD,
                        namedResource, ssrc);
   return fgQ.send(msg, sOperationQueueTimeout);
}

/* ============================ ACCESSORS ================================= */

#ifdef INCLUDE_RTCP /* [ */
IRTCPConnection *MpRtpInputConnection::getRTCPConnection(void)
{
    return(mpiRTCPConnection);
}
#endif /* INCLUDE_RTCP ] */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MpRtpInputConnection::connectOutput(MpResource& rTo,
                                               int toPortIdx,
                                               int fromPortIdx)
{
   UtlBoolean res = MpResource::connectOutput(rTo, toPortIdx, fromPortIdx);
   if (res)
   {
      assert(rTo.getContainableType() == MprDecode::TYPE);
      MprDecode *pDecode = (MprDecode*)&rTo;
      res = mpRtpDispatcher->connectOutput(fromPortIdx, pDecode);
   }
   return res;
}

UtlBoolean MpRtpInputConnection::disconnectOutput(int outPortIdx)
{
   UtlBoolean res = MpResource::disconnectOutput(outPortIdx);
   if (res)
   {
      res = mpRtpDispatcher->disconnectOutput(outPortIdx);
   }
   return res;
}

OsStatus MpRtpInputConnection::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   OsStatus stat = MpResource::setFlowGraph(pFlowGraph);

   // If the parent's call was successful, then call
   // setFlowGraph on any child resources we have.
   if(stat == OS_SUCCESS)
   {
      if (pFlowGraph != NULL)
      {
         mpRtpDispatcher->setNotificationDispatcher(pFlowGraph->getNotificationDispatcher());
         if(mpFromNet)
         {
             mpFromNet->setFlowGraph(pFlowGraph);
         }
#ifdef INCLUDE_RTCP /* [ */
      // Get the RTCP Connection object for this flowgraph connection
         mpiRTCPConnection = pFlowGraph->getRTCPConnectionPtr(getConnectionId(), 'A', getStreamId());
         OsSysLog::add(FAC_MP, PRI_DEBUG, " MpRtpInConn::setFlowGraph(%p) CID=%d, TC=%p", pFlowGraph, getConnectionId(), mpiRTCPConnection);

         // Let's use the Connection interface to acquire the constituent interfaces
         // required for dispatching RTP and RTCP packets received from the network as
         // well as the statistics interface tabulating RTP packets going to the network.
         INetDispatch         *piRTCPDispatch = NULL;
         IRTPDispatch         *piRTPDispatch = NULL;
         ISetSenderStatistics *piRTPAccumulator = NULL;

         if(mpiRTCPConnection)
         {
             mpiRTCPConnection->GetDispatchInterfaces(&piRTCPDispatch, &piRTPDispatch, &piRTPAccumulator);
         }
         // The MprFromNet object needs the RTP and RTCP Dispatch interfaces of the
         // associated RTCP connection so that RTP and RTCP packets may be forwarded
         // to the correct location.
         mpFromNet->setDispatchers(piRTPDispatch, piRTCPDispatch);

#endif /* INCLUDE_RTCP ] */
      }
      else
      {
         mpRtpDispatcher->setNotificationDispatcher(NULL);
      }
   }

   return stat;
}


UtlBoolean MpRtpInputConnection::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   switch (rMsg.getMsg()) 
   {
   case MPRM_SET_INACTIVITY_TIMEOUT:
      {
         MpIntResourceMsg *pMsg = (MpIntResourceMsg*)&rMsg;
         OsTime timeout(pMsg->getData());
         handleSetInactivityTimeout(timeout);
      }
      msgHandled = TRUE;
      break;

   case MPRM_ENABLE_SSRC_DISCARD:
   case MPRM_DISABLE_SSRC_DISCARD:
      {
         MpIntResourceMsg *pMsg = (MpIntResourceMsg*)&rMsg;
         RtpSRC ssrc = pMsg->getData();
         UtlBoolean enable = (rMsg.getMsg() == MPRM_ENABLE_SSRC_DISCARD);
         handleEnableSsrcDiscard(enable, ssrc);
      }
      msgHandled = TRUE;
      break;

   case MpResourceMsg::MPRM_DISABLE_ALL_NOTIFICATIONS:
   case MpResourceMsg::MPRM_ENABLE_ALL_NOTIFICATIONS:
      // Enable/disable all notifications sent out from this resource.
      msgHandled = MpResource::handleMessage(rMsg); 
      if (msgHandled)
      {
         UtlBoolean enabled = (rMsg.getMsg() == MpResourceMsg::MPRM_ENABLE_ALL_NOTIFICATIONS);
         mpRtpDispatcher->setNotificationsEnabled(enabled);
      }
      break;

   default:
      // If we don't handle the message here, let our parent try.
      msgHandled = MpResource::handleMessage(rMsg); 
      break;
   }
   return msgHandled;
}

void MpRtpInputConnection::handleSetInactivityTimeout(const OsTime &timeout)
{
   mpRtpDispatcher->setRtpInactivityTimeout(timeout);
}

void MpRtpInputConnection::handleEnableSsrcDiscard(UtlBoolean enable, RtpSRC ssrc)
{
   mpFromNet->enableSsrcDiscard(enable, ssrc);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
