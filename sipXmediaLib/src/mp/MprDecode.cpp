//  
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// OsIntTypes.h must be first include to define correct macros before system
// inttypes.h will be included.
#include <os/OsIntTypes.h>

// SYSTEM INCLUDES
#include <assert.h>

#ifdef WIN32 /* [ */
#include <winsock2.h>
#endif /* WIN32 ] */

#if defined(_VXWORKS) || defined(__pingtel_on_posix__) /* [ */
#include <sys/types.h>
#include <netinet/in.h>
#endif /* _VXWORKS || __pingtel_on_posix__ ] */

#include <string.h>

// APPLICATION INCLUDES
#include "mp/MpMisc.h"
#include "mp/MpDspUtils.h"
#include "mp/MpBuf.h"
#include "mp/MpRtpInputAudioConnection.h"
#include "mp/MprDecode.h"
#include "mp/MprDejitter.h"
#include "mp/MpDecoderBase.h"
#include "mp/NetInTask.h"
#include "mp/dmaTask.h"
#include "mp/MpMediaTask.h"
#include "mp/MpCodecFactory.h"
#include "mp/MpJitterBuffer.h"
#include "mp/MpPlcBase.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MprnDTMFMsg.h"
#include "mp/MpStringResourceMsg.h"
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "os/OsNotification.h"
#include "mp/MprDejitter.h"

// DEFINES
//#define RTL_ENABLED

#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#else
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#endif

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT // [
#  define dprintf printf
#else // DEBUG_PRINT ][
static void dprintf(const char *, ...) {};
#endif // DEBUG_PRINT ]

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDecode::MprDecode(const UtlString& rName, MpConnectionID connectionId,
                     const UtlString &plcName)
: MpAudioResource(rName, 0, 0, 1, 1)
, mpJB(new MpJitterBuffer())
, mpDtmfNotication(NULL)
, mpMyDJ(NULL)
, mIsStreamInitialized(FALSE)
, mpJbEstimationState(MpJitterBufferEstimation::createJbe())
, mIsJBInitialized(FALSE)
, mpCurrentCodecs(NULL)
, mNumCurrentCodecs(0)
, mpPrevCodecs(NULL)
, mNumPrevCodecs(0)
, mConnectionId(connectionId)
{
   assert(mpJB != NULL);
   mpJB->setPlc(plcName);
}

// Destructor
MprDecode::~MprDecode()
{
   // Release our codecs (if any), and the array of pointers to them.
   handleDeselectCodecs();

   // Free decoder buffer.
   delete mpJB;

   // Free JB estimation state.
   delete mpJbEstimationState;

   // Delete the list of codecs used in the past.
   OsLock lock(mLock);
   if (mNumPrevCodecs > 0)
   {
      for (int i=0; i<mNumPrevCodecs; i++)
      {
         mpPrevCodecs[i]->freeDecode();
         delete mpPrevCodecs[i];
      }
      delete[] mpPrevCodecs;
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprDecode::selectCodecs(SdpCodec* codecs[], int numCodecs)
{
   OsStatus ret = OS_SUCCESS;
   SdpCodec** codecArray;
   int i;
   int audioCodecsNum=0;
   UtlString codecMediaType;
   MpFlowGraphMsg msg(SELECT_CODECS, this, NULL, NULL, 0, 0);

   codecArray = new SdpCodec*[numCodecs];

   // Copy all audio codecs to new array
   for (i=0; i<numCodecs; i++) {
      codecs[i]->getMediaType(codecMediaType);
      if (codecMediaType.compareTo("audio") == 0)
      {
         codecArray[audioCodecsNum] = new SdpCodec(*codecs[i]);
         audioCodecsNum++;
      }
   }

   msg.setPtr1(codecArray);
   msg.setInt1(audioCodecsNum);
   ret = postMessage(msg);

   return ret;
}

OsStatus MprDecode::deselectCodec()
{
   MpFlowGraphMsg msg(DESELECT_CODECS, this, NULL, NULL, 0, 0);
   OsStatus ret = OS_SUCCESS;

//   osPrintf("MprDecode::deselectCodec\n");
   ret = postMessage(msg);

   return ret;
}

OsStatus MprDecode::setPlc(const UtlString& namedResource,
                           OsMsgQ& fgQ,
                           const UtlString& plcName)
{
   MpStringResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_SET_PLC, 
                           namedResource, plcName);
   return fgQ.send(msg, sOperationQueueTimeout);
}

void MprDecode::setMyDejitter(MprDejitter* pDJ)
{
   mpMyDJ = pDJ;
}

OsStatus MprDecode::pushPacket(const MpRtpBufPtr &pRtp)
{
   // Lock access to dejitter and m*Codecs data
   OsLock lock(mLock);

#ifdef RTL_ENABLED // [
   UtlString str_fg(getFlowGraph()->getFlowgraphName());
#endif // RTL_ENABLED ]
   RTL_EVENT(str_fg+"_MprDecode_pushPacket_seq", pRtp->getRtpSequenceNumber());
   RTL_EVENT(str_fg+"_MprDecode_pushPacket_TS", pRtp->getRtpTimestamp());
   dprintf("> %"PRIu16" %"PRIu32,
           pRtp->getRtpSequenceNumber(), pRtp->getRtpTimestamp());

   // Get decoder info for this packet.
   int pt = pRtp->getRtpPayloadType();
   const MpDecoderBase *pDecoder = mDecoderMap.mapPayloadType(pt);
   if (pDecoder == NULL)
   {
      // No decoder for this packet - just return error.
      return OS_NOT_FOUND;
   }
   const MpCodecInfo* pDecoderInfo = pDecoder->getInfo();

   // Initialize stream, if not initialized yet.
   if (mIsStreamInitialized == FALSE)
   {
      // Initialize stream with this packet.
      mStreamState.isFirstRtpPulled = FALSE;
      mStreamState.rtpStreamPosition = pRtp->getRtpTimestamp();
      mStreamState.rtpStreamHint = 0;
      mStreamState.sampleRate = pDecoderInfo->getSampleRate();
      mStreamState.playbackFrameSize = getFlowGraph()->getSamplesPerFrame()
                                       * mStreamState.sampleRate
                                       / getFlowGraph()->getSamplesPerSec();
      mStreamState.playbackStreamPosition = pRtp->getRtpTimestamp();

//      RTL_EVENT(str_fg+"_MprDecode_PF_stream_position", mStreamState.playbackStreamPosition);

      // Initialize JB estimator.
      mpJbEstimationState->init(mStreamState.sampleRate);

      // Update jitter state data
      mpJbEstimationState->update(&pRtp->getRtpHeader(),
                                  mStreamState.rtpStreamPosition,
                                  mStreamState.playbackStreamPosition,
                                  &mStreamState.rtpStreamHint);

      mStreamState.rtpStreamPosition += mStreamState.rtpStreamHint;

      mIsStreamInitialized = TRUE;
   }
   else
   {
      // Sample rate mustn't change during the live stream.
      assert(pDecoderInfo->getSampleRate() == mStreamState.sampleRate);

      // Update jitter state data
      mpJbEstimationState->update(&pRtp->getRtpHeader(),
                                  mStreamState.rtpStreamPosition,
                                  mStreamState.playbackStreamPosition,
                                  &mStreamState.rtpStreamHint);
   }

   RTL_EVENT(str_fg+"_MprDecode_pushPacket_rtp_stream_hint",
             mStreamState.rtpStreamHint);
   RTL_EVENT(str_fg+"_MprDecode_pushPacket_stream_position",
             mStreamState.playbackStreamPosition);
   dprintf("\n");

   return mpMyDJ->pushPacket(pRtp);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprDecode::handleSetDtmfNotify(OsNotification* pNotify)
{
   OsLock lock(mLock);

   mpDtmfNotication = pNotify;

   return TRUE;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprDecode::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
   MpAudioBufPtr out;
#ifdef RTL_ENABLED // [
   UtlString str_fg(getFlowGraph()->getFlowgraphName());
#endif // RTL_ENABLED ]

   if (outBufsSize == 0)
      return FALSE;

   if (!isEnabled) {
      return TRUE;
   }

   // If mIsJBInitialized is FALSE, we haven't got selectCodecs() called.
   // This is fine, but as long we do not know anything about codecs,
   // we could not continue.
   if (!mIsJBInitialized)
   {
      return TRUE;
   }

   // Lock access to dejitter and m*Codecs data
   OsLock lock(mLock);
   assert(mpMyDJ != NULL);

   // Initialize stream, if not initialized yet
   // and get latest RTP packet from dejitter queue.
   if (mIsStreamInitialized == FALSE)
   {
      return TRUE;
   }

   // Codecs are deselected, so call is going to be ended. Stop processing.
   if (mNumCurrentCodecs == 0)
   {
      return TRUE;
   }

   // Update playback stream pointer
   mStreamState.playbackStreamPosition += mStreamState.playbackFrameSize;

   // Pull packets from JB queue, till we won't get enough samples to playback
   dprintf("# pos:%"PRIu32, mStreamState.playbackStreamPosition);
   RTL_EVENT(str_fg+"_MprDecode_PF_stream_position", mStreamState.playbackStreamPosition);
   dprintf(" buf=%-3d", mpJB->getSamplesNum());
   RTL_EVENT(str_fg+"_MprDecode_PF_buffered_samples", mpJB->getSamplesNum());
   while (mpJB->getSamplesNum() < mpFlowGraph->getSamplesPerFrame())
   {
      MpRtpBufPtr rtp;
      UtlBoolean nextPacketAvailable=FALSE;

      RTL_EVENT(str_fg+"_MprDecode_PF_stream_position_inloop",
                mStreamState.playbackStreamPosition);

      // Step 1. Pull next packet from JB queue (MprDejitter)
      RTL_EVENT(str_fg+"_MprDecode_PF_dej_pull_position",
                mStreamState.rtpStreamPosition);
      dprintf(" pull[?%"PRIu32, mStreamState.rtpStreamPosition);
      dprintf(" JBlength=%d/%d",
              mpMyDJ->getNumLatePackets(), mpMyDJ->getNumPackets());
      RTL_EVENT(str_fg+"_MprDecode_PF_JB_length", mpMyDJ->getNumPackets());
      RTL_EVENT(str_fg+"_MprDecode_PF_JB_late_packets", mpMyDJ->getNumLatePackets());
      rtp = mpMyDJ->pullPacket(mStreamState.rtpStreamPosition,
                               &nextPacketAvailable);
      if (rtp.isValid())
      {
         dprintf(" <%"PRIu32" n=%d", rtp->getRtpTimestamp(), nextPacketAvailable);
      }
      else
      {
         dprintf(" <-");
      }
      // We must be sure to pass valid packet with first call to
      // MpJitterBuffer::pushPacket().
      if (!mStreamState.isFirstRtpPulled)
      {
         if (rtp.isValid())
         {
            // We've got first stream packet. Remember this.
            mStreamState.isFirstRtpPulled = TRUE;
         }
         else
         {
            // No packets have come out from JB queue yet. We'll wait.
            /// TODO:: Convert to stream samplerate.
            mStreamState.rtpStreamPosition += mStreamState.playbackFrameSize;
            dprintf("]\n");
            return TRUE;
         }
      }
      dprintf("]");

      // Step 2. Push packet to decoder buffer (MpJitterBuffer)
      int wantedBufferSamples = (int)( mStreamState.rtpStreamPosition
                                     - mStreamState.playbackStreamPosition)
                              + mStreamState.rtpStreamHint;
      int adjustment;
      int decodedSamples;
      UtlBoolean isPlayed;
      OsStatus res = mpJB->pushPacket(rtp,
                                      nextPacketAvailable?0:mpFlowGraph->getSamplesPerFrame(),
                                      wantedBufferSamples,
                                      decodedSamples,
                                      adjustment,
                                      isPlayed);
      dprintf(" adj[h=%"PRId32" ?%d %d p?%d",
              mStreamState.rtpStreamHint, wantedBufferSamples, adjustment,
              int(isPlayed));
      RTL_EVENT(str_fg+"_MprDecode_PF_rtp_stream _hint", mStreamState.rtpStreamHint);
      RTL_EVENT(str_fg+"_MprDecode_PF_wanted_buffer_samples", wantedBufferSamples);
      RTL_EVENT(str_fg+"_MprDecode_PF_adjustment", adjustment);
      RTL_EVENT(str_fg+"_MprDecode_PF_is_played", isPlayed);
      if (isPlayed)
      {
         mStreamState.rtpStreamPosition = rtp->getRtpTimestamp() + decodedSamples;
      } 
      else
      {
         mStreamState.rtpStreamPosition += decodedSamples;
      }
      if (res != OS_SUCCESS)
      {
         osPrintf("\n\n *** MpJitterBuffer::pushPacket() returned %d\n", res);
         if (rtp.isValid())
         {
            osPrintf(" pt=%d, Ts=%d, Seq=%d\n\n",
                     rtp->getRtpPayloadType(),
                     rtp->getRtpTimestamp(), rtp->getRtpSequenceNumber());
         }
      }
      dprintf("]");

      // Step 3. Get signaling data from packet if any.
      if (rtp.isValid())
      {
         tryDecodeAsSignalling(rtp);
      }
   }
   dprintf("\n");

   // Get next decoded frame from decoding buffer
   int numOriginalSamples;
   mpJB->getFrame(out, numOriginalSamples);
//   mStreamState.rtpStreamPosition += numOriginalSamples;

   // Return decoded frame
   outBufs[0].swap(out);
   return TRUE;
}

UtlBoolean MprDecode::tryDecodeAsSignalling(const MpRtpBufPtr &rtp)
{
   MpDecoderBase* pCurDec = mDecoderMap.mapPayloadType(rtp->getRtpPayloadType());

   // The codec is null or is not signaling. Do not continue.
   if (pCurDec == NULL || !pCurDec->getInfo()->isSignalingCodec())
      return FALSE;

   uint8_t event;
   UtlBoolean isStarted;
   UtlBoolean isStopped;
   uint16_t duration;
   OsStatus sigRes;

   do {
      sigRes = pCurDec->getSignalingData(event,
                                         isStarted,
                                         isStopped,
                                         duration);
      assert(sigRes != OS_NOT_SUPPORTED);
      if (sigRes == OS_SUCCESS && isStarted)
      {
         // Post DTMF notification message to indicate key down.
         MprnDTMFMsg dtmfMsg(getName(), mConnectionId,
                             (MprnDTMFMsg::KeyCode)event,
                             MprnDTMFMsg::KEY_DOWN);
         sendNotification(dtmfMsg);

         // Old way to indicate DTMF event. Will be removed soon, I hope.
         if (mpDtmfNotication)
         {
            mpDtmfNotication->signal( duration
                                    | (uint32_t)(event) << 16
                                    | 0<<31);
         }
      }

      if (sigRes == OS_SUCCESS && isStopped)
      {
         // Post DTMF notification message to indicate key up.
         MprnDTMFMsg dtmfMsg(getName(), mConnectionId,
                             (MprnDTMFMsg::KeyCode)event,
                             MprnDTMFMsg::KEY_UP, duration);
         sendNotification(dtmfMsg);

         // Old way to indicate DTMF event. Will be removed soon, I hope.
         if (mpDtmfNotication)
         {
            mpDtmfNotication->signal( duration
                                    | (uint32_t)(event) << 16
                                    | 1<<31);
         }
      }
   } while (sigRes == OS_SUCCESS);

   return TRUE;
}

// Handle old style messages for this resource.
UtlBoolean MprDecode::handleMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean ret = FALSE;

   switch (rMsg.getMsg()) {
   case DESELECT_CODECS:
      handleDeselectCodecs();
      ret = TRUE;
      break;
   case SELECT_CODECS:
      {
         SdpCodec** pCodecs = (SdpCodec**)rMsg.getPtr1();
         int numCodecs = rMsg.getInt1();
         handleSelectCodecs(pCodecs, numCodecs);
         // Delete the list pCodecs.
         for (int i=0; i<numCodecs; i++) {
            delete pCodecs[i];
         }
         delete[] pCodecs;
         ret = TRUE;
      }
      break;
   default:
      ret = MpAudioResource::handleMessage(rMsg);
      break;
   }
   return ret;
}

// Handle new style messages for this resource.
UtlBoolean MprDecode::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   switch (rMsg.getMsg()) 
   {
   case MPRM_SET_PLC:
      msgHandled = handleSetPlc(((MpStringResourceMsg*)&rMsg)->getData());
      break;

   default:
      // If we don't handle the message here, let our parent try.
      msgHandled = MpResource::handleMessage(rMsg); 
      break;
   }
   return msgHandled;
}

UtlBoolean MprDecode::handleSelectCodecs(SdpCodec* pCodecs[], int numCodecs)
{
   int i;
   SdpCodec* pCodec;
   int payload;
   UtlString mime;
   UtlString fmtp;
   OsStatus ret;
   MpDecoderBase* pNewDecoder;
   MpDecoderBase* pOldDecoder;
   MpCodecFactory* pFactory = MpCodecFactory::getMpCodecFactory();
   int allReusable = 0;
   int canReuse;
#if 0
   osPrintf("MprDecode::handleSelectCodecs(%d codec%s):\n",
      numCodecs, ((1 == numCodecs) ? "" : "s"));
#endif
   if (OsSysLog::willLog(FAC_MP, PRI_DEBUG))
   {
      for (i=0; i<numCodecs; i++) {
         pCodec = pCodecs[i];
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprDecode::handleSelectCodecs "
                       "pCodecs[%d]->getCodecType() = %d, "
                       "pCodecs[%d]->getCodecPayloadFormat() = %d",
                       i, pCodec->getCodecType(),
                       i, pCodec->getCodecPayloadFormat());
            }
   }

#if 0 // [
   // Check to see if all codecs in pCodecs can be handled by codecs
   // in mpCurrentCodecs.
   for (i=0; i<numCodecs; i++) {
      pCodec = pCodecs[i];
      pCodec->getEncodingName(mime);
      pCodec->getSdpFmtpField(fmtp);
      payload = pCodec->getCodecPayloadFormat();
#if 0
      osPrintf("  #%d: New=0x%X/i:%d/x:%d, ",
         i, (int)pCodec, ourCodec, payload);
#endif
      pOldDecoder = mDecoderMap.mapPayloadType(payload);
      if (NULL != pOldDecoder) {
         UtlString oldMime;
         UtlString oldFmtp;
         pOldDecoder->getInfo()->getCodecType();
#if 0
         osPrintf("  Old=0x%X/i:%d", (int)pOldDecoder, oldSdpType);
#endif
         canReuse = (ourCodec == oldSdpType);
      } else {
         // osPrintf("  no Old");
         canReuse = 0;
      }
      allReusable &= canReuse;
#if 0
      osPrintf(" i:%d/x:%d (%sreusable%s)\n", ourCodec, payload,
         (canReuse ? "" : "not "),
         (canReuse && (ourCodec != oldSdpType) ? "[*]" : ""));
#endif
   }
#endif // 0 ]

   // Lock the m*Codecs members.
   OsLock lock(mLock);

   // If the new list is not a subset of the old list, we have to copy
   // pCodecs into mpCurrentCodecs.
   if (!allReusable)
   {
      // Delete the current codecs.
      handleDeselectCodecs(FALSE);

      mNumCurrentCodecs = 0;
      mpCurrentCodecs = new MpDecoderBase*[numCodecs];

      for (i=0; i<numCodecs; i++) {
         pCodec = pCodecs[i];
         pCodec->getEncodingName(mime);
         pCodec->getSdpFmtpField(fmtp);
         int sampleRate = pCodec->getSampleRate();
         int numChannels = pCodec->getNumChannels();
         payload = pCodec->getCodecPayloadFormat();
         ret = pFactory->createDecoder(mime, fmtp, sampleRate, numChannels,
                                       payload, pNewDecoder);
         assert(OS_SUCCESS == ret);
         assert(NULL != pNewDecoder);
         if (pNewDecoder->initDecode() == OS_SUCCESS)
         {
            // Add this codec to mpConnection's payload type decoding table.
            mDecoderMap.addPayloadType(payload, pNewDecoder);
            mpCurrentCodecs[mNumCurrentCodecs] = pNewDecoder;
            mNumCurrentCodecs++;
         }
         else
         {
            delete pNewDecoder;
         }
      }

      // Go back and add any signaling codecs to Jitter Buffer.
      for (i=0; i<mNumCurrentCodecs; i++) {
         if (mpCurrentCodecs[i]->getInfo()->isSignalingCodec()) {
            mpCurrentCodecs[i]->initDecode();
         }
      }
   }

   // If decoder buffer is not yet initialized - lets go initialize it.
   if (!mIsJBInitialized)
   {
      mpJB->init(mpFlowGraph->getSamplesPerSec(),
                 mpFlowGraph->getSamplesPerFrame());
      mIsJBInitialized= TRUE;
   }

   mpJB->setCodecList(&mDecoderMap);

   return TRUE;
}

UtlBoolean MprDecode::handleDeselectCodec(MpDecoderBase* pDecoder)
{
   int payload;

   if (NULL != pDecoder) {
      payload = pDecoder->getPayloadType();
      mDecoderMap.deletePayloadType(payload);
   }
   return TRUE;
}

UtlBoolean MprDecode::handleSetPlc(const UtlString &plcName)
{
   mpJB->setPlc(plcName);
   return TRUE;
}

UtlBoolean MprDecode::handleDeselectCodecs(UtlBoolean shouldLock)
{
   int i;
   MpDecoderBase** pCurrentCodecs;
   MpDecoderBase** pPrevCodecs;
   int newN;
   if(shouldLock)
   {
       mLock.acquire();
   }
   if (0 < mNumCurrentCodecs) {

      newN = mNumCurrentCodecs + mNumPrevCodecs;
      pPrevCodecs = new MpDecoderBase*[newN];
#if 0
      osPrintf("|handleDeselectCodecs(0x%X): (0x%X,%d) -> (0x%X,%d) (+%d)\n",
         (int) this, (int) mpPrevCodecs, mNumPrevCodecs, (int) pPrevCodecs,
         newN, mNumCurrentCodecs);
#endif
      if (mNumPrevCodecs > 0) {
         for (i=0; i<mNumPrevCodecs; i++) {
            pPrevCodecs[i] = mpPrevCodecs[i];
         }
         delete[] mpPrevCodecs;
      }

      i = mNumCurrentCodecs;
      mNumCurrentCodecs = 0;
      pCurrentCodecs = mpCurrentCodecs;
      mpCurrentCodecs = NULL;
      while (i>0) {
         i--;
         handleDeselectCodec(pCurrentCodecs[i]);
         pPrevCodecs[i+mNumPrevCodecs] = pCurrentCodecs[i];
         pCurrentCodecs[i] = NULL;
      }
      delete[] pCurrentCodecs;
      mpPrevCodecs = pPrevCodecs;
      mNumPrevCodecs = newN;
   }

   if (mIsJBInitialized)
   {
      mpJB->setCodecList(NULL);
   }

   if(shouldLock)
   {
       mLock.release();
   }
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
