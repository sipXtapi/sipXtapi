//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef SIPX_VIDEO // [

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpVideoCallFlowGraph.h"
#include "mp/video/MpCaptureDeviceBase.h"
#include "mp/MpConnection.h"
#include "mp/video/MpCaptureTask.h"
#include "mp/video/MpRemoteVideoTask.h"

#ifdef INCLUDE_RTCP /* [ */
#include "mp/NetInTask.h"  // For rand_timer32()
#endif /* INCLUDE_RTCP ] */

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
void *MpVideoCallFlowGraph::smpPreviewWindow=NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpVideoCallFlowGraph::MpVideoCallFlowGraph(MpCaptureDeviceBase *pCaptureDevice, const MpVideoStreamParams* pCaptureParams)
: mpCaptureDevice(pCaptureDevice)
, mpConnection(NULL)
, mpCaptureTask(NULL)
, mpRemoteVideoTask(NULL)
, mReceiving(false)
, mSending(false)
#ifdef INCLUDE_RTCP // [
, mpiRTCPSession(NULL)
, mulEventInterest(LOCAL_SSRC_COLLISION | REMOTE_SSRC_COLLISION)
#endif // INCLUDE_RTCP ]
{
#ifdef INCLUDE_RTCP /* [ */
   // Let's now create an RTCP Session so that we may be prepared to
   // report on the RTP connections that shall eventually be associated
   // with this flow graph

   // Let's get the  RTCP Control interface
   IRTCPControl *piRTCPControl = CRTCManager::getRTCPControl();
   assert(piRTCPControl);

   // Create an RTCP Session for this Flow Graph.  Pass the SSRC ID to be
   // used to identify our audio source uniquely within this RTP/RTCP Session.
   mpiRTCPSession = piRTCPControl->CreateSession(rand_timer32());

   // Subscribe for Events associated with this Session
   piRTCPControl->Advise((IRTCPNotify *)this);

   // Release Reference to RTCP Control Interface
   piRTCPControl->Release();

   mpConnection = new MpConnection(0, mpiRTCPSession);
#else // INCLUDE_RTCP ][
   mpConnection = new MpConnection(0, NULL);
#endif // INCLUDE_RTCP ]
   assert(mpConnection != NULL);

   mpRemoteVideoTask = new MpRemoteVideoTask(mpConnection->getDejitter());
   assert(mpRemoteVideoTask != NULL);

   if (mpCaptureDevice != NULL) {
      // Create sink for captured frames and connect it to capture device.
      mpCaptureTask = new MpCaptureTask(mpCaptureDevice->getFramesQueue(),
                                        mpConnection->getRtpWriter(),
                                        mpRemoteVideoTask, 
                                        pCaptureParams);
   }
}

MpVideoCallFlowGraph::~MpVideoCallFlowGraph()
{
   if (mpRemoteVideoTask != NULL)
   {
      delete mpRemoteVideoTask;
      mpRemoteVideoTask = NULL;
   }

   if (mpCaptureTask != NULL) {
      delete mpCaptureTask;
      mpCaptureTask = NULL;
   }

   if (mpCaptureDevice != NULL) {
      mpCaptureDevice->unInitialize();
      delete mpCaptureDevice;
      mpCaptureDevice = NULL;
   }

   if (mpConnection != NULL) {
      delete mpConnection;
      mpConnection = NULL;
   }

#ifdef INCLUDE_RTCP /* [ */
   // Let's terminate the RTCP Session in preparation for call teardown

   // Let's get the  RTCP Control interface
   IRTCPControl *piRTCPControl = CRTCManager::getRTCPControl();
   assert(piRTCPControl);

   // Unsubscribe for Events associated with this Session
   piRTCPControl->Unadvise((IRTCPNotify *)this);

   // Terminate the RTCP Session
   piRTCPControl->TerminateSession(mpiRTCPSession);

   // Release Reference to RTCP Control Interface
   piRTCPControl->Release();
#endif /* INCLUDE_RTCP ] */
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpVideoCallFlowGraph::startSendRtp(SdpCodec& rPrimaryCodec,
                                            OsSocket& rRtpSocket, OsSocket& rRtcpSocket)
{
   OsStatus status;

   if (mSending)
   {
      return OS_SUCCESS;
   }

   if (mpCaptureDevice == NULL)
   {
      return OS_FAILED;
   }

   // Prepare RTP session.
   mpConnection->prepareStartSendRtp(rRtpSocket, rRtcpSocket);

   // Start consuming captured frames
   if ( (mpCaptureTask == NULL)
      ||(!mpCaptureTask->isStarted() && mpCaptureTask->start() != TRUE) )
   {
      mpCaptureDevice->stopCapture();
      mpConnection->prepareStopSendRtp();
      return OS_FAILED;
   }

   // Start producing video frames
   status = mpCaptureDevice->startCapture();
   if (status != OS_SUCCESS)
   {
      mpConnection->prepareStopSendRtp();
      return status;
   }

   mSending = true;

   return OS_SUCCESS;
}

void MpVideoCallFlowGraph::stopSendRtp()
{
   if (mSending && mpCaptureDevice != NULL)
   {
      // Stop producing video frames
      mpCaptureDevice->stopCapture();

      // Stop consuming captured frames
      mpCaptureTask->stop();

      // Stop RTP session
      mpConnection->prepareStopSendRtp();

      mSending = false;
   }
}

OsStatus MpVideoCallFlowGraph::startReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                                               OsSocket& rRtpSocket, OsSocket& rRtcpSocket)
{
   if (mReceiving)
   {
      return OS_SUCCESS;
   }

   if (mpConnection != NULL)
   {
      mpConnection->prepareStartReceiveRtp(rRtpSocket, rRtcpSocket);

      if (  (mpRemoteVideoTask != NULL)
         && (mpRemoteVideoTask->isStarted() || mpRemoteVideoTask->startProcessing()) )
      {
         mReceiving = true;
         return OS_SUCCESS;
      }
   }
   
   return OS_FAILED;
}

void MpVideoCallFlowGraph::stopReceiveRtp()
{
   if (mReceiving)
   {
      if (mpRemoteVideoTask != NULL)
      {
         mpRemoteVideoTask->stopProcessing();
      }

      if (mpConnection != NULL)
      {
         mpConnection->prepareStopReceiveRtp();
         mReceiving = false;
      }
   }
}

void MpVideoCallFlowGraph::setVideoWindow(const void *hWnd)
{
   if (mpRemoteVideoTask != NULL)
   {
      mpRemoteVideoTask->setRemoteVideoWindow(hWnd);
   }
}

OsStatus MpVideoCallFlowGraph::setVideoPreviewWindow(void *hWnd)
{
   smpPreviewWindow = hWnd;

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

void *MpVideoCallFlowGraph::getVideoWindow() const
{
   if (mpRemoteVideoTask == NULL)
      return NULL;

   return mpRemoteVideoTask->getRemoteVideoWindow();
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

#ifdef INCLUDE_RTCP // [
void MpVideoCallFlowGraph::LocalSSRCCollision(IRTCPConnection  *piRTCPConnection, 
                                         IRTCPSession     *piRTCPSession)
{

//  Ignore those events that are for a session other than ours
    if(mpiRTCPSession != piRTCPSession)
    {
//      Release Interface References
        piRTCPConnection->Release();
        piRTCPSession->Release();
        return;
    }

// We have a collision with our local SSRC.  We will remedy this by
// generating a new SSRC
    mpiRTCPSession->ReassignSSRC(rand_timer32(),
                         (unsigned char *)"LOCAL SSRC COLLISION");

// We must inform connection associated with this session to change its SSRC
   if (mpConnection->getRTCPConnection()) 
   {
//       Set the new SSRC
      mpConnection->reassignSSRC((int)mpiRTCPSession->GetSSRC());
   }

// Release Interface References
   piRTCPConnection->Release();
   piRTCPSession->Release();

   return;
}

void MpVideoCallFlowGraph::RemoteSSRCCollision(IRTCPConnection  *piRTCPConnection, 
                                          IRTCPSession     *piRTCPSession)
{

//  Ignore those events that are for a session other than ours
    if(mpiRTCPSession != piRTCPSession)
    {
//      Release Interface References
        piRTCPConnection->Release();
        piRTCPSession->Release();
        return;
    }

// According to standards, we are supposed to ignore remote sites that
// have colliding SSRC IDS.
   if (mpConnection->getRTCPConnection() == piRTCPConnection) 
   {
// We are supposed to ignore the media of the latter of two terminals
// whose SSRC collides
      stopReceiveRtp();
   }

// Release Interface References
   piRTCPConnection->Release();
   piRTCPSession->Release();


}

unsigned long MpVideoCallFlowGraph::GetEventInterest(void) 
{ 
    return(mulEventInterest); 
}
#endif // INCLUDE_RTCP ]


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

#endif // SIPX_VIDEO ]
