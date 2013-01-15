//  
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
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
#include "mp/MpRtpOutputConnection.h"
#include "mp/MprToNet.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MprEncode.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/INetDispatch.h"
#include "rtcp/IRTPDispatch.h"
#include "rtcp/ISetSenderStatistics.h"
#include "rtcp/IRTCPSession.h"
#include "rtcp/IRTCPConnection.h"
#endif /* INCLUDE_RTCP ] */
#include "os/OsDateTime.h"
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpRtpOutputConnection::MpRtpOutputConnection(const UtlString& resourceName,
                                             MpConnectionID myID, 
                                             IRTCPSession *piRTCPSession)
: MpResource(resourceName, 1, 1, 0, 0)
,mpToNet(NULL)
, mOutRtpStarted(FALSE)
#ifdef INCLUDE_RTCP /* [ */
, mpiRTCPConnection(NULL)
#endif /* INCLUDE_RTCP ] */
{
   // Save connection ID
   mConnectionId = myID;

   // Create ToNet resource
   mpToNet = new MprToNet();

#ifndef INCLUDE_RTCP /* [ */
   {
      OsDateTime date;
      OsTime now;
      int ssrc;
      OsDateTime::getCurTime(date);
      date.cvtToTimeSinceEpoch(now);
      ssrc = now.seconds() ^ now.usecs();
      mpToNet->setSSRC(ssrc);
   }
#endif /* INCLUDE_RTCP ] */

   //////////////////////////////////////////////////////////////////////////
   // connect ToNet -> FromNet for RTP synchronization
   // TODO: mpToNet->setRtpPal(mpFromNet);
}

// Destructor
MpRtpOutputConnection::~MpRtpOutputConnection()
{
   if (mpToNet != NULL)
      delete mpToNet;
}

/* ============================ MANIPULATORS ============================== */

void MpRtpOutputConnection::setSockets(OsSocket& rRtpSocket,
                                                OsSocket& rRtcpSocket)
{
   mpToNet->setSockets(rRtpSocket, rRtcpSocket);
   // TODO: mpFromNet->setDestIp(rRtpSocket);

#ifdef INCLUDE_RTCP /* [ */
// Associate the RTCP socket to be used by the RTCP Render portion of the
// connection to write reports to the network
   if(mpiRTCPConnection)
   {
       // OsSysLog::add(FAC_MP, PRI_DEBUG, "MpRtpOutputConnection::setSockets: call mpiRTCPConnection->StartRenderer(%p)", &rRtcpSocket);
       mpiRTCPConnection->StartRenderer(rRtcpSocket);
   }
#endif /* INCLUDE_RTCP ] */

   mOutRtpStarted = TRUE;
}

void MpRtpOutputConnection::releaseSockets()
{
#ifdef INCLUDE_RTCP /* [ */
// Terminate the RTCP Connection which shall include stopping the RTCP
// Render so that no additional reports are emitted
   if(mpiRTCPConnection)
   {
       mpiRTCPConnection->StopRenderer();
   }
#endif /* INCLUDE_RTCP ] */

   mpToNet->resetSockets();

   mOutRtpStarted = FALSE;
}

#ifdef INCLUDE_RTCP /* [ */
void MpRtpOutputConnection::reassignSSRC(int iSSRC)
{

//  Set the new SSRC
    mpToNet->setSSRC(iSSRC);

    return;

}
#endif /* INCLUDE_RTCP ] */

/* ============================ ACCESSORS ================================= */

OsStatus MpRtpOutputConnection::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
    OsStatus status = MpResource::setFlowGraph(pFlowGraph);

    if(mpToNet)
    {
        mpToNet->setFlowGraph(pFlowGraph);
    }

#ifdef INCLUDE_RTCP /* [ */
    if (pFlowGraph != NULL)
    {
        // Get the RTCP Connection object for this flowgraph connection
        mpiRTCPConnection = pFlowGraph->getRTCPConnectionPtr(getConnectionId());
     
     // Let's use the Connection interface to acquire the constituent interfaces
     // required for dispatching RTP and RTCP packets received from the network as
     // well as the statistics interface tabulating RTP packets going to the network.
        INetDispatch         *piRTCPDispatch = NULL;
        IRTPDispatch         *piRTPDispatch = NULL;
        ISetSenderStatistics *piRTPAccumulator = NULL;
     
        if(mpiRTCPConnection)
        {
            mpiRTCPConnection->GetDispatchInterfaces(&piRTCPDispatch,
               &piRTPDispatch, &piRTPAccumulator);
     
           // Set the Statistics interface to be used by the RTP stream to increment
           // packet and octet statistics
           mpToNet->setRTPAccumulator(piRTPAccumulator);
     
           // The RTP Stream associated with the MprToNet object must have its SSRC ID
           // set to the value generated from the Session.
           mpToNet->setSSRC(pFlowGraph->getRTCPSessionPtr()->GetSSRC());
        }
    }
#endif /* INCLUDE_RTCP ] */

    return(status);
}

#ifdef INCLUDE_RTCP /* [ */
IRTCPConnection *MpRtpOutputConnection::getRTCPConnection(void)
{
    return(mpiRTCPConnection);
}
#endif /* INCLUDE_RTCP ] */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MpRtpOutputConnection::processFrame()
{
   return TRUE;
}

UtlBoolean MpRtpOutputConnection::connectInput(MpResource& rFrom,
                                                int fromPortIdx,
                                                int toPortIdx)
{
   // TODO:: Move this to MprEncode and implement disconnect!
   UtlBoolean res = MpResource::connectInput(rFrom, fromPortIdx, toPortIdx);
   if (res)
   {
      assert(rFrom.getContainableType() == MprEncode::TYPE);
      MprEncode *pEncode = (MprEncode*)&rFrom;
      pEncode->setMyToNet(mpToNet);
   }
   return res;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
