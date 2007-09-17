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
#include "mp/video/MpdH263.h"
#include "mp/MpMisc.h"
#include "mp/video/MpvoGdi.h"

#include "os/OsTimer.h"
#include "os/OsEvent.h"
#include "os/OsCallback.h"
#include "os/OsPtrMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern void *ghVideo;

// CONSTANTS
#define CODEC_TYPE_H264 114
#define CODEC_TYPE_H263 34

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpRemoteVideoTask::MpRemoteVideoTask(MprDejitter* pDejitter, void *hwnd)
: mpDejitter(pDejitter)
//, mpDecoder(new MpdH264(CODEC_TYPE_H264, MpMisc.VideoFramesPool))
, mpDecoder(new MpdH263(CODEC_TYPE_H263, MpMisc.VideoFramesPool))
, mpVideoOut(NULL)
, mTimestamp(0)
, mStreamInitialized(false)
, mpTimer(NULL)
{
   if (mpDecoder != NULL)
   {
      if (OS_SUCCESS != mpDecoder->initDecode())
      {
         delete mpDecoder;
         mpDecoder = NULL;
      }
   }

   mpVideoOut = new MpvoGdi(NULL);
   assert(mpVideoOut != NULL);
}

MpRemoteVideoTask::~MpRemoteVideoTask()
{
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
   OsPtrMsg msg(SET_REMOTE_VIDEO_WINDOW, 0, (void*)hwnd);
   postMessage(msg);
}

OsStatus MpRemoteVideoTask::startProcessing()
{
   if (NULL == mpDecoder)
      return OS_FAILED;

   // Create frame tick timer
   mpTimer = new OsTimer(getMessageQueue(), 0);
   if (mpTimer == NULL)
      return OS_FAILED;

   // Start processing thread
   if (!start())
      return OS_SUCCESS;

   if (mpTimer->periodicEvery(OsTime(0), OsTime(20)) != OS_SUCCESS)
   {
      printf("MpRemoteVideoTask::startProcessing(): timer start failed!\n");
      fflush(stdout);
      stopProcessing();
      return OS_FAILED;
   }

   return OS_SUCCESS;
}

OsStatus MpRemoteVideoTask::stopProcessing()
{
   OsStatus ret=OS_SUCCESS;

   // Remove tick timer
   if (mpTimer != NULL)
   {
      // Synchronously stop frame tick timer
      printf("MpRemoteVideoTask::stopProcessing(): mpTimer=%X\n", mpTimer);
      fflush(stdout);
      mpTimer->stop(true);
      printf("MpRemoteVideoTask::stopProcessing(): stop ok.\n", mpTimer);
      fflush(stdout);

      // Delete timer
      delete mpTimer;
      mpTimer = NULL;
   }

   // Stop processing thread
   requestShutdown();
   if (!waitUntilShutDown())
   {
      ret = OS_FAILED;
   }

   // Reset timestamp
   mTimestamp = 0;
   mStreamInitialized = false;

   return ret;
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
   UtlBoolean handled = false;

   switch (rMsg.getMsgType())
   {
      case OsMsg::OS_EVENT:
         // Time to process next frame
         {
            handled = true;
            handleFrameTick();
            break;
         }
      case MpRemoteVideoTask::SET_REMOTE_VIDEO_WINDOW:
         // Set remote display video handle
         {
            handled = true;
            const void *hwnd = ((OsPtrMsg*)&rMsg)->getPtr();
            handleSetRemoteVideoWindow(hwnd);
            break;
         }
   }

   return handled;
}

OsStatus MpRemoteVideoTask::handleSetRemoteVideoWindow(const void *hwnd)
{
   return mpVideoOut->setWindow((HWND)hwnd);
}

OsStatus MpRemoteVideoTask::handleFrameTick()
{
   if (mpDejitter != NULL && mpDecoder != NULL)
   {
      MpVideoBufPtr pFrame;
      bool packetConsumed;

      // If there was no cached packet, pull one from Dejitter.
      if (!mpRtpPacket.isValid())
      {
         //mpRtpPacket = mpDejitter->pullPacket(CODEC_TYPE_H264);
         mpRtpPacket = mpDejitter->pullPacket(CODEC_TYPE_H263);
      }

      // We may get no packets from Dejitter. Handle this.
      if (mpRtpPacket.isValid())
      {

         // Initialize timestamp and decoder, if this this the first packet
         // in RTP session.
         if (!mStreamInitialized)
         {
            // Initialize timestamp
            mTimestamp = mpRtpPacket->getRtpTimestamp();

            // Pass first packet to decoder to initialize its internal stream
            // state
            mpDecoder->initStream(mpRtpPacket);

            mStreamInitialized = true;
         }

         // Update timestamp, if we get packet with greater timestamp. If
         // timestamp is lesser (earlier packet) we pass this packet to decoder,
         // but does not update timestamp -- this packet will likely be
         // discarded and we do not mess up with it.
         if (compare(mpRtpPacket->getRtpTimestamp(), mTimestamp)>0)
         {
            mTimestamp = mpRtpPacket->getRtpTimestamp();
            mStreamInitialized = true;
         }

         while (mpRtpPacket.isValid())
         {
            pFrame = mpDecoder->decode(mpRtpPacket, packetConsumed, false);

            // Pull next packet of this frame from Dejitter if previous packet
            // was consumed () by decoder.
            if (packetConsumed)
            {
               //mpRtpPacket = mpDejitter->pullPacket(CODEC_TYPE_H264, mTimestamp);
               mpRtpPacket = mpDejitter->pullPacket(CODEC_TYPE_H263, mTimestamp);
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

   return OS_SUCCESS;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */


#endif // SIPX_VIDEO ]
