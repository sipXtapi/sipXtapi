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
#include "mp/video/MpdH263p.h"
#include "mp/MpMisc.h"
#include "mp/video/MpvoGdi.h"
#include "sdp/SdpCodec.h"

#include "os/OsTimer.h"
#include "os/OsEvent.h"
#include "os/OsCallback.h"
#include "os/OsPtrMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpRemoteVideoTask::MpRemoteVideoTask(MprDejitter* pDejitter, void *hwnd)
: mpDejitter(pDejitter)
, mpVideoOut(NULL)
, mTimestamp(0)
, mStreamInitialized(false)
, mpTimer(NULL)
, mDecoderCount(0)
, mLastPayload(-1)
, mDecoderIndex(size_t(-1))
{
   for (size_t i = 0; i < MAX_VIDEO_DECODERS; ++i)
   {
      mDecoders[i] = NULL;
      mPayloadTypes[i] = -1;
   }

   mpVideoOut = new MpvoGdi(NULL);
   assert(mpVideoOut != NULL);
}

MpRemoteVideoTask::~MpRemoteVideoTask()
{
   const size_t decoderCount = mDecoderCount;
   mDecoderCount = 0;
   for (size_t i = 0; i < decoderCount; ++i)
   {
      delete mDecoders[i];
      mDecoders[i] = NULL;
      mPayloadTypes[i] = -1;
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
   if (mpDejitter != NULL && 0 != mDecoderCount)
   {
      MpVideoBufPtr pFrame;
      bool packetConsumed;

      // If there was no cached packet, pull one from Dejitter.
      if (!mpRtpPacket.isValid())
      {
         //mpRtpPacket = mpDejitter->pullPacket(CODEC_TYPE_H264);
         mpRtpPacket = pullPacket(NULL);
      }

      // We may get no packets from Dejitter. Handle this.
      if (mpRtpPacket.isValid())
      {
         assert(-1 != mLastPayload);
         assert(mDecoderIndex < mDecoderCount);
         assert(NULL != mDecoders[mDecoderIndex]);

         // Initialize timestamp and decoder, if this this the first packet
         // in RTP session.
         if (!mStreamInitialized)
         {
            // Initialize timestamp
            mTimestamp = mpRtpPacket->getRtpTimestamp();

            // Pass first packet to decoder to initialize its internal stream
            // state
            // TODO: in some distant future we might support switching codec on the fly, 
            // we should store initialized flag for each decoder separately then.
            // for now we just assume no sender is sophisticated enough to switch 
            // encoders after session is established.
            mDecoders[mDecoderIndex]->initStream(mpRtpPacket);
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
            assert(mDecoderIndex < mDecoderCount);
            assert(NULL != mDecoders[mDecoderIndex]);
            pFrame = mDecoders[mDecoderIndex]->decode(mpRtpPacket, packetConsumed, false);

            // Pull next packet of this frame from Dejitter if previous packet
            // was consumed () by decoder.
            if (packetConsumed)
            {
               //mpRtpPacket = mpDejitter->pullPacket(CODEC_TYPE_H264, mTimestamp);
               mpRtpPacket = pullPacket(&mTimestamp);
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
            if (-1 != mLastPayload)
            {
               assert(mDecoderIndex < mDecoderCount);
               assert(NULL != mDecoders[mDecoderIndex]);

               pFrame = mDecoders[mDecoderIndex]->decode(mpRtpPacket, packetConsumed, true);
            }

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

MpRtpBufPtr MpRemoteVideoTask::pullPacket(UINT* timestamp, bool lockTimestamp)
{
   assert(NULL != mpDejitter);

   MpRtpBufPtr buf;
   if (-1 != mLastPayload)
   {
      if (NULL != timestamp)
         buf = mpDejitter->pullPacket(mLastPayload, *timestamp, lockTimestamp);
      else
         buf = mpDejitter->pullPacket(mLastPayload);

      if (buf.isValid())
         return buf;
   }

   for (size_t i = 0; i < mDecoderCount; ++i)
   {
      const int payloadType = mPayloadTypes[i];
      assert(-1 != payloadType);
      if (payloadType == mLastPayload)
         continue;

      if (NULL != timestamp)
         buf = mpDejitter->pullPacket(payloadType, *timestamp, lockTimestamp);
      else
         buf = mpDejitter->pullPacket(payloadType);

      if (buf.isValid())
      {
         mLastPayload = payloadType;
         mDecoderIndex = i;
         return buf;
      }
   }

   return buf;
}

OsStatus MpRemoteVideoTask::applyCodecs(const SdpCodec * const pCodecs[], const int numCodecs)
{
   if (0 == mDecoderCount)
   {
      size_t decoderCount = 0;
      for (int i = 0; i < numCodecs; ++i)
      {
         const SdpCodec* codec = pCodecs[i];
         UtlString mimeType;
         codec->getMediaType(mimeType);
         if (MIME_TYPE_VIDEO != mimeType)
            continue;

         const int payloadType = codec->getCodecPayloadFormat();
         UtlString mimeSubtype;
         codec->getEncodingName(mimeSubtype);
         if (MIME_SUBTYPE_H263 == mimeSubtype)
         {
            mDecoders[decoderCount] = new MpdH263(payloadType, MpMisc.VideoFramesPool);
         }
         else if (MIME_SUBTYPE_H264 == mimeSubtype)
         {
            mDecoders[decoderCount] = new MpdH264(payloadType, MpMisc.VideoFramesPool);
         }
         else if (MIME_SUBTYPE_H263_1998 == mimeSubtype || MIME_SUBTYPE_H263_2000 == mimeSubtype)
         {
            mDecoders[decoderCount] = new MpdH263p(payloadType, MpMisc.VideoFramesPool);
         }
         else
         {
            assert(!"Unsupported video decoder.");
            continue;
         }

         assert(NULL != mDecoders[decoderCount]);
         if (OS_SUCCESS != mDecoders[decoderCount]->initDecode())
         {
            delete mDecoders[decoderCount];
            mDecoders[decoderCount] = NULL;
            continue;
         }

         mPayloadTypes[decoderCount] = payloadType;
         ++decoderCount;
      }

      mDecoderCount = decoderCount;
      return OS_SUCCESS;
   }
   else
   {
      // TODO: we don't check codecs array now. this should be done, to verify, 
      // that new codec settings are identical with previous ones. 
      return OS_SUCCESS;
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */


#endif // SIPX_VIDEO ]
