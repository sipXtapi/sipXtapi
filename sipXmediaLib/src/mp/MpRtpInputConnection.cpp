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
#include "mp/MpRtpInputConnection.h"
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
MpRtpInputConnection::MpRtpInputConnection(MpConnectionID myID, IRTCPSession *piRTCPSession)
: mpFromNet(NULL)
, mpDejitter(NULL)
, mMyID(myID)
, mInEnabled(FALSE)
, mInRtpStarted(FALSE)
, mLock(OsMutex::Q_PRIORITY|OsMutex::INVERSION_SAFE)
#ifdef INCLUDE_RTCP /* [ */
, mpiRTCPSession(piRTCPSession)
, mpiRTCPConnection(NULL)
#endif /* INCLUDE_RTCP ] */
{
   char         name[50];

#ifdef INCLUDE_RTCP /* [ */
// Let's create an RTCP Connection to accompany the MP Connection just created.
   if(mpiRTCPSession)
   {
       mpiRTCPConnection = mpiRTCPSession->CreateRTCPConnection();

       assert(mpiRTCPConnection != NULL);
   }

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
   }
#endif /* INCLUDE_RTCP ] */

   // Create our resources
   sprintf(name, "Dejitter-%d", myID);
   mpDejitter  = new MprDejitter();
   sprintf(name, "FromNet-%d", myID);
   mpFromNet   = new MprFromNet();

#ifdef INCLUDE_RTCP /* [ */

// The MprFromNet object needs the RTP and RTCP Dispatch interfaces of the
// associated RTCP connection so that RTP and RTCP packets may be forwarded
// to the correct location.
   mpFromNet->setDispatchers(piRTPDispatch, piRTCPDispatch);

#else /* INCLUDE_RTCP ] [ */

#endif /* INCLUDE_RTCP ] */

   //////////////////////////////////////////////////////////////////////////
   // connect FromNet -> Dejitter
   mpFromNet->setMyDejitter(mpDejitter);
}

// Destructor
MpRtpInputConnection::~MpRtpInputConnection()
{
   if (mpDejitter != NULL)
      delete mpDejitter;

   if (mpFromNet != NULL)
      delete mpFromNet;

#ifdef INCLUDE_RTCP /* [ */
// Let's free our RTCP Connection
   if(mpiRTCPSession)
   {
       mpiRTCPSession->TerminateRTCPConnection(mpiRTCPConnection); 
   }
#endif /* INCLUDE_RTCP ] */
}

/* ============================ MANIPULATORS ============================== */

// Disables the input path of the connection.
// Resources on the path(s) will also be disabled by these calls.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
//!retcode: OS_SUCCESS - for now, these methods always return success



/*OsStatus MpRtpInputConnection::disable(void)
{
   mInEnabled = FALSE;
   return OS_SUCCESS;
}*/

// Enables the input path of the connection.
// Resources on the path(s) will also be enabled by these calls.
// Resources may allocate needed data (e.g. output path reframe buffer)
//  during this operation.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
//!retcode: OS_SUCCESS - for now, these methods always return success

OsStatus MpRtpInputConnection::enable(void)
{
   mInEnabled = TRUE;
   return OS_SUCCESS;
}

// Start receiving RTP and RTCP packets.
void MpRtpInputConnection::prepareStartReceiveRtp(OsSocket& rRtpSocket,
                                          OsSocket& rRtcpSocket)
{
   mpFromNet->setSockets(rRtpSocket, rRtcpSocket);
   mInRtpStarted = TRUE;
}

// TODO:  mpFromNet->setDestIp(rRtpSocket);

// Stop receiving RTP and RTCP packets.
void MpRtpInputConnection::prepareStopReceiveRtp()
{
   mpFromNet->resetSockets();
   mInRtpStarted = FALSE;
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

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
