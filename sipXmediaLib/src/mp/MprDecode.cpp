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

#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#  ifdef RTL_AUDIO_ENABLED
#     include <SeScopeAudioBuffer.h>
#  endif
#else
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#endif

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
, mpPlc(MpPlcBase::createPlc(plcName))
, mIsPlcInitialized(FALSE)
, mIsJBInitialized(FALSE)
, mpCurrentCodecs(NULL)
, mNumCurrentCodecs(0)
, mpPrevCodecs(NULL)
, mNumPrevCodecs(0)
, mConnectionId(connectionId)
{
   assert(mpJB != NULL);
   assert(mpPlc != NULL);
}

// Destructor
MprDecode::~MprDecode()
{
   // Release our codecs (if any), and the array of pointers to them.
   handleDeselectCodecs();

   // Free decoder buffer.
   delete mpJB;
   delete mpPlc;

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

   // Get decoder info for this packet.
   int pt = pRtp->getRtpPayloadType();
   const MpDecoderBase *pDecoder = mDecoderMap.mapPayloadType(pt);
   if (pDecoder == NULL)
   {
      // No decoder for this packet - just return error.
      return OS_NOT_FOUND;
   }
   const MpCodecInfo* pDecoderInfo = pDecoder->getInfo();

   // Initialize stream, if not initialized yet
   // and get latest RTP packet from dejitter queue.
   if (mIsStreamInitialized == FALSE)
   {
      // Initialize stream with this packet.
      mLastPlayedSeq = pRtp->getRtpSequenceNumber();

      mStreamState.streamPosition = pRtp->getRtpTimestamp() - JB_LENGTH_SAMPLES;
      mStreamState.recommendedPosition = mStreamState.streamPosition;

      mIsStreamInitialized = TRUE;
   }
   else
   {
      // Update jitter state data
      uint32_t newPosition = pRtp->getRtpTimestamp() - JB_LENGTH_SAMPLES;
      if (MpDspUtils::compareSerials(newPosition, mStreamState.recommendedPosition) > 0)
      {
         mStreamState.recommendedPosition = newPosition;
      }
   }

#ifdef DEBUG_PRINT // [
   printf(">>> pushing                          position=%u recommended=%u\n",
           mStreamState.streamPosition, mStreamState.recommendedPosition);
   printf("PUSH: Seq#=%u   TS=%u\n",
          pRtp->getRtpSequenceNumber(), pRtp->getRtpTimestamp());
#endif // DEBUG_PRINT ]

   return mpMyDJ->pushPacket(pRtp, pDecoderInfo->isSignalingCodec());
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
   MpRtpBufPtr rtp;

   if (outBufsSize == 0)
      return FALSE;

   if (!isEnabled) {
      return TRUE;
   }

   // If mIsJBInitialized is FALSE, we haven't got selectCodecs() called.
   // This is fine, but a long we do not know anything about codecs,
   // we could not continue.
   if (!mIsJBInitialized)
   {
      return TRUE;
   }

   // Initialize PLC if it is not initialized yet.
   if (!mIsPlcInitialized)
   {
      mpPlc->init(mpFlowGraph->getSamplesPerSec(), mpFlowGraph->getSamplesPerFrame());
      mCurFrameNum = 0;
      mIsPlcInitialized = TRUE;
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

   if (mpJB->getSamplesNum() == 0)
   {
      int32_t lag = (int32_t)(mStreamState.recommendedPosition-mStreamState.streamPosition);

#ifdef DEBUG_PRINT // [
      printf("                                <<< pulling    position=%u recommended=%u lag=%d\n",
             mStreamState.streamPosition, mStreamState.recommendedPosition, lag);
#endif // DEBUG_PRINT ]

      if ( (lag > JB_LAG_SAMPLES) || (lag < -JB_ADVANCE_SAMPLES))
      {
         mStreamState.streamPosition = mStreamState.recommendedPosition;
#ifdef DEBUG_PRINT // [
         printf("                                ! position reset !\n");
#endif // DEBUG_PRINT ]
      }

      MpRtpBufPtr tmpRtp;
      while ((tmpRtp = mpMyDJ->pullPacket(mStreamState.streamPosition)).isValid())
      {
#ifdef DEBUG_PRINT // [
         printf("                                PULL: Seq#=%u   TS=%u\n",
                tmpRtp->getRtpSequenceNumber(), tmpRtp->getRtpTimestamp());
#endif // DEBUG_PRINT ]

         // If delayed packet, drop it
         if (MpDspUtils::compareSerials(mLastPlayedSeq, tmpRtp->getRtpSequenceNumber()) > 0)
         {
            // todo:: fix glitch!
            printf("Dropping delayed packet with seq#%u, TS%u\n",
                   tmpRtp->getRtpSequenceNumber(), tmpRtp->getRtpTimestamp());
            continue;
         }
         // If it is signaling packet - decode it now.
         //              !!!!  BUG::XMR-104  !!!!
         if (tryDecodeAsSignalling(tmpRtp))
         {
            continue;
         }
         // Accept packet, if it is later then one we already have, else drop it.
         if (  !rtp.isValid()
            || (MpDspUtils::compareSerials(rtp->getRtpSequenceNumber(),
                                           tmpRtp->getRtpSequenceNumber()) < 0))
         {
            rtp = tmpRtp;
         }
      }
   }

   // Decode RTP packet if we got one.
   if (rtp.isValid())
   {
      MpDecoderBase* pCurDec = mDecoderMap.mapPayloadType(rtp->getRtpPayloadType());

      // Update last played sequence number.
      mLastPlayedSeq = rtp->getRtpSequenceNumber();

      if (pCurDec != NULL)
      {
//         printf("decoding RTP seq# %u   TS %u\n",
//                rtp->getRtpSequenceNumber(), rtp->getRtpTimestamp());

         // Flush decoder buffer, if there were unplayed frames.

         // todo:: fix glitch?
         if (mpJB->getSamplesNum() > 0)
         {
            printf("Flushing decode buffer with %d samples. Glitch!\n",
                   mpJB->getSamplesNum());
            mpJB->flush();
         }

         OsStatus res = mpJB->pushPacket(rtp);
         if (res != OS_SUCCESS)
         {
            osPrintf("\n\n *** MprDecode::doProcessFrame() returned %d\n", res);
            osPrintf(" pt=%d, Ts=%d, Seq=%d\n\n",
                     rtp->getRtpPayloadType(),
                     rtp->getRtpTimestamp(), rtp->getRtpSequenceNumber());
         }
      }
   }

   // Get next decoded frame
   int numOriginalSamples;
   mpJB->getFrame(out, numOriginalSamples);

   // Update stream pointer
//   mStreamState.streamPosition += numOriginalSamples;

#ifdef DEBUG_PRINT // [
   if (!out.isValid())
   {
      printf("                                !!! PLC !!!!\n");
   }
#endif // DEBUG_PRINT ]

   // Run frame through PLC algorithm
   doPlc(out);

   /// BUG:: THIS IS WRONG IN CASE OF RESAMPLING !!!!!
   mStreamState.streamPosition += out->getSamplesNumber();

   outBufs[0] = out;
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

void MprDecode::doPlc(MpAudioBufPtr &pFrame)
{
   const MpAudioSample *plcSamplesIn = NULL;
   MpAudioSample *plcSamplesOut = NULL;
   UtlBoolean plcIsFrameModified;
   OsStatus plcResult;

   RTL_EVENT("MprDecode_doPlc_loss_patern", !pFrame.isValid());

   // Prepare pointers to input and output frames for PLC.
   if (pFrame.isValid())
   {
      plcSamplesIn = pFrame->getSamplesPtr();
   }
   if (!mTempPlcFrame.isValid())
   {
      mTempPlcFrame = MpMisc.RawAudioPool->getBuffer();
      assert(mTempPlcFrame.isValid());
      mTempPlcFrame->setSamplesNumber(mpFlowGraph->getSamplesPerFrame());
      mTempPlcFrame->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);
   }
   plcSamplesOut = mTempPlcFrame->getSamplesWritePtr();

   // Pass frame through PLC.
   plcResult = mpPlc->processFrame(mCurFrameNum, mCurFrameNum,
                                   plcSamplesIn, plcSamplesOut,
                                   &plcIsFrameModified);
   assert(plcResult == OS_SUCCESS);
   if (plcResult == OS_SUCCESS && plcIsFrameModified)
   {
      pFrame.release();
      pFrame.swap(mTempPlcFrame);
   }

#ifdef RTL_AUDIO_ENABLED
   UtlString outputLabel("MpJitterBuffer_getSamples");
   RTL_RAW_AUDIO(outputLabel,
                 mpFlowGraph->getSamplesPerSec(),
                 pFrame->getSamplesNumber(),
                 pFrame->getSamplesPtr(),
                 mCurFrameNum);
#endif

   // Advance frame number.
   mCurFrameNum++;
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
   // Free old PLC
   delete mpPlc;

   // Set PLC to a new one
   mpPlc = MpPlcBase::createPlc(plcName);
   mIsPlcInitialized = FALSE;

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
