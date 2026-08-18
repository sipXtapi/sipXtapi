//
// Copyright (C) 2022-2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
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
#include "mp/MpIntResourceMsg.h"
#include "mp/MpSetSrtpParamsMsg.h"
#include "mp/MpSetDtlsParamsMsg.h"
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

   //mpToNet->setSRAdjustUSecs(12345); // DEBUG: just to test/demo this, set to 12.345 milliseconds

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
void MpRtpOutputConnection::reassignSSRC(ssrc_t newSSRC)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG, "MpRtpOutputConnection::reassignSSRC: new SSRC=0x%08X", newSSRC);
    mpToNet->setSSRC(newSSRC);
}
#endif /* INCLUDE_RTCP ] */

UtlBoolean MpRtpOutputConnection::handleMessage(MpResourceMsg& message)
{
    UtlBoolean handled = FALSE;

    switch (message.getMsg())
    {
    case MprToNet::MPRM_SET_SR_ADJUST_USECS:
    {
        handled = TRUE;
        MpIntResourceMsg *pMsg = (MpIntResourceMsg*)&message;
        mpToNet->setSRAdjustUSecs(pMsg->getData());
    }
    break;

    case MpResourceMsg::MPRM_SET_SRTP_PARAMS:
    {
       handled = TRUE;
       MpSetSrtpParamsMsg* pMsg = (MpSetSrtpParamsMsg*)&message;
       const MpSrtpKeyUse keyUse = pMsg->getKeyUse();

       // RTP and RTCP are protected by separate contexts on separate threads:
       // MprToNet on the media thread, CRTCPRender on the CRTCManager thread.
       // Which of them a given key belongs to depends on how it was
       // negotiated, so route by the key use rather than handing every key to
       // both.
       //
       //   SDES        -> RTP_AND_RTCP; one master key covers the media line
       //                  (RFC 4568), whatever the port layout.
       //   DTLS-SRTP   -> RTP_ONLY or RTCP_ONLY; without rtcp-mux each
       //                  transport runs its own association with its own
       //                  master key (RFC 5764 section 3), and each discards
       //                  the half it does not need (section 4.2).  Feeding
       //                  the RTP association's key to the RTCP renderer, as
       //                  this used to do unconditionally, produces SRTCP no
       //                  conformant peer can authenticate.
       if (keyUse != MP_SRTP_KEY_USE_RTCP_ONLY)
       {
          mpToNet->setSrtpParams(pMsg->getCryptoSuite(), pMsg->getCryptoKey());
       }

#ifdef INCLUDE_RTCP /* [ */
       if (keyUse != MP_SRTP_KEY_USE_RTP_ONLY && mpiRTCPConnection)
       {
           mpiRTCPConnection->SetSrtpParams(pMsg->getCryptoSuite(), pMsg->getCryptoKey());
       }
#endif /* INCLUDE_RTCP ] */

       OsSysLog::add(FAC_MP, PRI_DEBUG,
          "MpRtpOutputConnection::handleMessage: installed outbound %s key, cryptoSuite=%d",
          MpSrtpKeyUseString(keyUse), pMsg->getCryptoSuite());
    }
    break;

    case MpResourceMsg::MPRM_SET_DTLS_PARAMS:
    {
       handled = TRUE;
       MpSetDtlsParamsMsg* pMsg = (MpSetDtlsParamsMsg*)&message;

       // MprToNet gates outbound MEDIA on its engine being active, so only the
       // RTP transport's engine belongs here. The RTCP association is driven
       // entirely through MprFromNet and the RTCP renderer.
       if (!pMsg->isForRtcpTransport())
       {
          mpToNet->setDtls(pMsg->getDtls());
       }

#ifdef INCLUDE_RTCP /* [ */
       // With DTLS-SRTP the keys do not exist until the handshake completes,
       // which can be several reporting intervals after the renderer starts.
       // Tell the RTCP connection to hold its reports until then instead of
       // putting unprotected ones on the wire -- the same rule
       // MprToNet::writeRtp applies to media during that window.
       //
       // Either engine implies the connection is DTLS-keyed, and the RTCP
       // renderer is unlatched only by the key that actually protects RTCP.
       if(mpiRTCPConnection && pMsg->getDtls() != NULL)
       {
           mpiRTCPConnection->SetSrtpRequired(true);
       }
#endif /* INCLUDE_RTCP ] */
    }
    break;


    default:
        handled = MpResource::handleMessage(message);
    }

    return(handled);
}


/* ============================ ACCESSORS ================================= */

OsStatus MpRtpOutputConnection::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
    OsStatus status = MpResource::setFlowGraph(pFlowGraph);

    if(mpToNet)
    {
        mpToNet->setFlowGraph(pFlowGraph);

#ifdef INCLUDE_RTCP /* [ */
        if (pFlowGraph != NULL)
        {
            // Get the RTCP Connection object for this flowgraph connection
            mpiRTCPConnection = pFlowGraph->getRTCPConnectionPtr(getConnectionId(), 'A', getStreamId());
            OsSysLog::add(FAC_MP, PRI_DEBUG, "MpRtpOutConn::setFlowGraph(%p) CID=%d, TC=%p", pFlowGraph, getConnectionId(), mpiRTCPConnection);

            if(mpiRTCPConnection)
            {
                // Let's use the Connection interface to acquire the constituent interfaces
                // required for dispatching RTP and RTCP packets received from the network as
                // well as the statistics interface tabulating RTP packets going to the network.
                INetDispatch* piRTCPDispatch = NULL;
                IRTPDispatch* piRTPDispatch = NULL;
                ISetSenderStatistics* piRTPAccumulator = NULL;

                mpiRTCPConnection->GetDispatchInterfaces(&piRTCPDispatch, &piRTPDispatch, &piRTPAccumulator);
    
                // Set the Statistics interface to be used by the RTP stream to increment
                // packet and octet statistics
                mpToNet->setRTPAccumulator(piRTPAccumulator);

                // The RTP Stream associated with the MprToNet object must have its SSRC ID
                // set to the value generated from the Connection.
                mpToNet->setSSRC(mpiRTCPConnection->GetLocalSSRC());
            }
        }
#else /* INCLUDE_RTCP ] */

        // Assign new SSRC value to MprToNet object
        mpToNet->setSSRC(rand_timer32());

#endif /* INCLUDE_RTCP ] */
    }

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
