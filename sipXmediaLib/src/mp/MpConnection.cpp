//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
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
#include "mp/MpConnection.h"
#include "mp/MprToNet.h"
#include "mp/MprFromNet.h"
#include "mp/MprDejitter.h"
#include "os/OsLock.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RtcpConfig.h"
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
MpConnection::MpConnection(MpConnectionID myID, IRTCPSession *piRTCPSession)
: mpToNet(NULL)
, mpFromNet(NULL)
, mpDejitter(NULL)
, mMyID(myID)
, mInEnabled(FALSE)
, mOutEnabled(FALSE)
, mInRtpStarted(FALSE)
, mOutRtpStarted(FALSE)
, mLock(OsMutex::Q_PRIORITY|OsMutex::INVERSION_SAFE)
#ifdef INCLUDE_RTCP /* [ */
, mpiRTCPSession(piRTCPSession)
, mpiRTCPConnection(NULL)
#endif /* INCLUDE_RTCP ] */
{
   char         name[50];

#ifdef INCLUDE_RTCP /* [ */
// Let's create an RTCP Connection to accompany the MP Connection just created.
   
   mpiRTCPConnection = mpiRTCPSession->CreateRTCPConnection();

   assert(mpiRTCPConnection != NULL); 

// Let's use the Connection interface to acquire the constituent interfaces
// required for dispatching RTP and RTCP packets received from the network as
// well as the statistics interface tabulating RTP packets going to the network.
   INetDispatch         *piRTCPDispatch;
   IRTPDispatch         *piRTPDispatch;
   ISetSenderStatistics *piRTPAccumulator;

   mpiRTCPConnection-> GetDispatchInterfaces(&piRTCPDispatch,
      &piRTPDispatch, &piRTPAccumulator);
#endif /* INCLUDE_RTCP ] */

   // Create our resources
   sprintf(name, "Dejitter-%d", myID);
   mpDejitter  = new MprDejitter();
   sprintf(name, "FromNet-%d", myID);
   mpFromNet   = new MprFromNet(this);
   sprintf(name, "ToNet-%d", myID);
   mpToNet     = new MprToNet();

#ifdef INCLUDE_RTCP /* [ */

// The MprFromNet object needs the RTP and RTCP Dispatch interfaces of the
// associated RTCP connection so that RTP and RTCP packets may be forwarded
// to the correct location.
   mpFromNet->setDispatchers(piRTPDispatch, piRTCPDispatch);

// Set the Statistics interface to be used by the RTP stream to increment
// packet and octet statistics
   mpToNet->setRTPAccumulator(piRTPAccumulator);

// The RTP Stream associated with the MprToNet object must have its SSRC ID
// set to the value generated from the Session.
   mpToNet->setSSRC(mpiRTCPSession->GetSSRC());
#else /* INCLUDE_RTCP ] [ */
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
   // connect FromNet -> Dejitter
   mpFromNet->setMyDejitter(mpDejitter);

   //////////////////////////////////////////////////////////////////////////
   // connect ToNet -> FromNet for RTP synchronization
   mpToNet->setRtpPal(mpFromNet);
}

// Destructor
MpConnection::~MpConnection()
{
   if (mpDejitter != NULL)
      delete mpDejitter;

   if (mpFromNet != NULL)
      delete mpFromNet;

   if (mpToNet != NULL)
      delete mpToNet;

#ifdef INCLUDE_RTCP /* [ */
// Let's free our RTCP Connection
   mpiRTCPSession->TerminateRTCPConnection(mpiRTCPConnection); 
#endif /* INCLUDE_RTCP ] */
}

/* ============================ MANIPULATORS ============================== */

// Disables the input path, output path, or both paths, of the connection.
// Resources on the path(s) will also be disabled by these calls.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
//!retcode: OS_SUCCESS - for now, these methods always return success

OsStatus MpConnection::disableIn(void)
{
   mInEnabled = FALSE;
   return OS_SUCCESS;
}

OsStatus MpConnection::disableOut(void)
{
   mOutEnabled = FALSE;
   return OS_SUCCESS;
}

OsStatus MpConnection::disable(void)
{
   MpConnection::disableIn();
   MpConnection::disableOut();
   return OS_SUCCESS;
}

// Enables the input path, output path, or both paths, of the connection.
// Resources on the path(s) will also be enabled by these calls.
// Resources may allocate needed data (e.g. output path reframe buffer)
//  during this operation.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
//!retcode: OS_SUCCESS - for now, these methods always return success

OsStatus MpConnection::enableIn(void)
{
   mInEnabled = TRUE;
   return OS_SUCCESS;
}

OsStatus MpConnection::enableOut(void)
{
   mOutEnabled = TRUE;
   return OS_SUCCESS;
}

OsStatus MpConnection::enable(void)
{
   enableIn();
   enableOut();
   return OS_SUCCESS;
}

void MpConnection::prepareStartSendRtp(OsSocket& rRtpSocket,
                                       OsSocket& rRtcpSocket)
{
   mpToNet->setSockets(rRtpSocket, rRtcpSocket);
   mpFromNet->setDestIp(rRtpSocket);

#ifdef INCLUDE_RTCP /* [ */
// Associate the RTCP socket to be used by the RTCP Render portion of the
// connection to write reports to the network
   mpiRTCPConnection->StartRenderer(rRtcpSocket);
#endif /* INCLUDE_RTCP ] */

   mOutRtpStarted = TRUE;
}

void MpConnection::prepareStopSendRtp()
{
#ifdef INCLUDE_RTCP /* [ */
// Terminate the RTCP Connection which shall include stopping the RTCP
// Render so that no additional reports are emitted
   mpiRTCPConnection->StopRenderer();
#endif /* INCLUDE_RTCP ] */

   mpToNet->resetSockets();

   mOutRtpStarted = FALSE;
}

// Start receiving RTP and RTCP packets.
void MpConnection::prepareStartReceiveRtp(OsSocket& rRtpSocket,
                                          OsSocket& rRtcpSocket)
{
   mpFromNet->setSockets(rRtpSocket, rRtcpSocket);
   mInRtpStarted = TRUE;
}

// Stop receiving RTP and RTCP packets.
void MpConnection::prepareStopReceiveRtp()
{
   mpFromNet->resetSockets();
   mInRtpStarted = FALSE;
}

#ifdef INCLUDE_RTCP /* [ */
void MpConnection::reassignSSRC(int iSSRC)
{

//  Set the new SSRC
    mpToNet->setSSRC(iSSRC);

    return;

}
#endif /* INCLUDE_RTCP ] */

/* ============================ ACCESSORS ================================= */

#ifdef INCLUDE_RTCP /* [ */
IRTCPConnection *MpConnection::getRTCPConnection(void)
{
    return(mpiRTCPConnection);
}
#endif /* INCLUDE_RTCP ] */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
