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
#include "mp/video/MpRemoteVideoTask.h"
#include "mp/MprDejitter.h"
#include "mp/video/MpdH264.h"
#include "mp/MpMisc.h"
#include "mp/video/MpvoGdi.h"

#include "os/OsTimer.h"
#include "os/OsEvent.h"
#include "os/OsCallback.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern void *ghVideo;

// CONSTANTS
#define CODEC_TYPE_H264 114

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpRemoteVideoTask::MpRemoteVideoTask(MprDejitter* pDejitter, void *hwnd)
: mpDejitter(pDejitter)
, mpDecoder(new MpdH264(CODEC_TYPE_H264, MpMisc.VideoFramesPool))
, mpVideoOut(NULL)
, mTimestamp(0)
, mTimestampInitialized(false)
, mpCallback(NULL)
, mpTimer(NULL)
{
   if (mpDecoder != NULL)
   {
      mpDecoder->initDecode();
   }

   mpVideoOut = new MpvoGdi(NULL);
   assert(mpVideoOut != NULL);


   // TODO:: move to initialization
   mpTimer = new OsTimer(getMessageQueue(), VIDEO_TICK);
}

MpRemoteVideoTask::~MpRemoteVideoTask()
{
   // TODO:: move to uninitialization
   if (mpTimer != NULL)
   {
      delete mpTimer;
      mpTimer = NULL;
   }

   if (mpDecoder != NULL)
   {
      delete mpDecoder;
      mpDecoder = NULL;
   }

   if (mpVideoOut != NULL)
   {
      delete mpVideoOut;
      mpVideoOut = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

void MpRemoteVideoTask::setRemoteVideoWindow(const void *hwnd)
{
   mpVideoOut->setWindow((HWND)hwnd);
}

void MpRemoteVideoTask::step()
{
   if (mpDejitter != NULL && mpDecoder != NULL)
   {
      MpVideoBufPtr pFrame;
      bool packetConsumed;

      // If there was no cached packet, pull one from Dejitter.
      if (!mpRtpPacket.isValid())
      {
         mpRtpPacket = mpDejitter->pullPacket(CODEC_TYPE_H264);
      }

      // We may get no packets from Dejitter. Handle this.
      if (mpRtpPacket.isValid())
      {

         // Initialize timestamp, if this this the first packet in RTP session.
         // Or update it, if we get packet with greater timestamp. If timestamp
         // is lesser (earlier packet) we pass this packet to decoder, but
         // does not update timestamp -- this packet will likely be discarded
         // and we do not mess up with it.
         if (!mTimestampInitialized || compare(mpRtpPacket->getRtpTimestamp(), mTimestamp)>0)
         {
            mTimestamp = mpRtpPacket->getRtpTimestamp();
            mTimestampInitialized = true;
         }

         while (mpRtpPacket.isValid())
         {
            pFrame = mpDecoder->decode(mpRtpPacket, packetConsumed, false);

            // Pull next packet of this frame from Dejitter if previous packet
            // was consumed () by decoder.
            if (packetConsumed)
            {
               mpRtpPacket = mpDejitter->pullPacket(CODEC_TYPE_H264, mTimestamp);
            }

            // End pulling packets from Dejitter if we got frame from Decoder.
            if (pFrame.isValid())
            {
               break;
            }
         }

         // If we pulled all packets, but did not get video frame from decoder,
         // force decoding - we want draw something on the screen.
         if (!pFrame.isValid())
         {
            // TODO:: decode with force flag here!
            pFrame = mpDecoder->decode(mpRtpPacket, packetConsumed, true);

            // Free packet if it was consumed (processed) by decoder.
            if (packetConsumed)
            {
               mpRtpPacket.release();
            }
         }

         if (pFrame.isValid() && mpVideoOut != NULL)
         {
            mpVideoOut->render(pFrame);
         }
      }
   }
}

OsStatus MpRemoteVideoTask::stop()
{
   requestShutdown();
   if (waitUntilShutDown())
      return OS_SUCCESS;
   else
      return OS_FAILED;

   mTimestamp = 0;
   mTimestampInitialized = false;
}

/* ============================ ACCESSORS ================================= */

void *MpRemoteVideoTask::getRemoteVideoWindow() const
{
   return mpVideoOut->getWindow();
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MpRemoteVideoTask::handleMessage(OsMsg& rMsg)
{

}

/*
int MpRemoteVideoTask::run(void* pArg)
{
   OsCallback callback((int)this, ReportingAlarm);
   OsTimer timer(callback);
   
   if (timer.periodicEvery(OsTime(100), OsTime(100)) != OS_SUCCESS)
      return -1;

   while (!isShuttingDown())
   {
      Sleep(100);
   }

   timer.stop();

   ackShutdown();

   return 0;
}

void MpRemoteVideoTask::ReportingAlarm(const int userData, const int eventData)
{
    MpRemoteVideoTask *remoteVideoTask = (MpRemoteVideoTask *) userData;
    remoteVideoTask->step();
}
*/

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */


#endif // SIPX_VIDEO ]
