//  
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
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
#ifdef ANDROID
   // Set to 1 to disable, 0 to enable verbose (LOGV) messages
#  define LOG_NDEBUG 1
#  define LOG_TAG "MprDecode"

#  include <utils/Log.h>
#endif

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
#include <os/OsDefs.h>
#include <os/OsSysLog.h>
#include <os/OsLock.h>
#include <os/OsNotification.h>
#include <utl/UtlSerialized.h>
#include <mp/MprDecode.h>
#include <mp/MpDspUtils.h>
#include <mp/MpBuf.h>
#include <mp/MprDejitter.h>
#include <mp/MpDecoderBase.h>
#include <mp/MpCodecFactory.h>
#include <mp/MpJitterBuffer.h>
#include <mp/MpPlcBase.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MprnDTMFMsg.h>
#include <mp/MprDecodeSelectCodecsMsg.h>
#include <mp/MpStringResourceMsg.h>
#include <mp/MpPackedResourceMsg.h>
#include <mp/MprDejitter.h>

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
const UtlContainableType MprDecode::TYPE = "MprDecode";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDecode::MprDecode(const UtlString& rName,
                     const UtlString &plcName)
: MpAudioResource(rName, 1, 1, 1, 1)
, mpJB(new MpJitterBuffer(NULL, rName))
, mIsJBInitialized(FALSE)
, mpMyDJ(NULL)
, mOwnDJ(FALSE)
, mIsStreamInitialized(FALSE)
, mpJbEstimationState(MpJitterBufferEstimation::createJbe())
, mpCurrentCodecs(NULL)
, mNumCurrentCodecs(0)
, mEnableG722Hack(TRUE)
, mG722HackPayloadType(-1)
, mpPrevCodecs(NULL)
, mNumPrevCodecs(0)
{
   assert(mpJB != NULL);
   mpJB->setPlc(plcName);
}

// Destructor
MprDecode::~MprDecode()
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "MprDecode::~MprDecode %s flowgraph: %p", data(), mpFlowGraph);

   // Release our codecs (if any), and the array of pointers to them.
   handleDeselectCodecs();

   // Free decoder buffer.
   delete mpJB;

   // Free JB estimation state.
   delete mpJbEstimationState;

   // Free dejitter if it is owned by this decoder.
   if (mOwnDJ)
   {
      delete mpMyDJ;
   }

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprDecode::~MprDecode %s flowgraph: %p mNumCurrentCodecs: %d mpCurrentCodecs: %p mNumPrevCodecs: %d mpPrevCodecs: %p",
                 data(), mpFlowGraph, mNumCurrentCodecs, mpCurrentCodecs, mNumPrevCodecs, mpPrevCodecs);

   // Generally should not be current codecs as the decoder should be deactivated first
   if(mNumCurrentCodecs > 0)
   {
       OsSysLog::add(FAC_MP, PRI_ERR, 
           "MprDecode::~MprDecode invoked with %d active codecs.  Decoder %s (flowgraph: %p) should have been stopped first.",
           mNumCurrentCodecs, data(), mpFlowGraph);

       for (int codecIndex = 0; codecIndex < mNumCurrentCodecs; codecIndex++)
       {
           delete mpCurrentCodecs[codecIndex];
           mpCurrentCodecs[codecIndex] = NULL;
       }

       OsSysLog::add(FAC_MP, PRI_DEBUG,
                     "MprDecode::~MprDecode deleting mpCurrentCodecs: %p %s flowgraph: %p", 
                     mpCurrentCodecs, data(), mpFlowGraph);
       delete[] mpCurrentCodecs;
       mpCurrentCodecs = NULL;
   }

   // Delete the list of codecs used in the past.
   deletePriorCodecs();

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprDecode::~MprDecode exit %s flowgraph: %p",
                 data(), mpFlowGraph);
}

void MprDecode::deletePriorCodecs()
{
   // Delete the list of codecs used in the past.
   if (mNumPrevCodecs > 0)
   {
      for (int i=0; i<mNumPrevCodecs; i++)
      {
         delete mpPrevCodecs[i];
         mpPrevCodecs[i] = NULL;
      }

      mNumPrevCodecs = 0;

      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprDecode::~MprDecode deleting mpPrevCodecs: %p %s flowgraph: %p", 
                    mpPrevCodecs, data(), mpFlowGraph);
      delete[] mpPrevCodecs;
      mpPrevCodecs = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprDecode::reset(const UtlString& namedResource,
                          OsMsgQ& fgQ)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_RESET, namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

void MprDecode::reset()
{
   reset(this->getName(), *mpFlowGraph->getMsgQ());
}

OsStatus MprDecode::selectCodecs(const UtlString& namedResource,
                                 OsMsgQ& fgQ,
                                 SdpCodec* codecs[],
                                 int numCodecs)
{
   MprDecodeSelectCodecsMsg msg(namedResource, codecs, numCodecs);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprDecode::deselectCodecs(const UtlString& namedResource,
                                   OsMsgQ& fgQ)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_DESELCT_CODECS, namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprDecode::setPlc(const UtlString& namedResource,
                           OsMsgQ& fgQ,
                           const UtlString& plcName)
{
   MpStringResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_SET_PLC, 
                           namedResource, plcName);
   return fgQ.send(msg, sOperationQueueTimeout);
}

void MprDecode::setMyDejitter(MprDejitter* pDJ, UtlBoolean ownDj)
{
   mpMyDJ = pDJ;
   mOwnDJ = ownDj;

   // Set connectionID and streamID on dejitter.
   mpMyDJ->setConnectionId(mConnectionId);
   mpMyDJ->setStreamId(mStreamId);
}

OsStatus MprDecode::pushBuffer(int inPortIndex, MpBufPtr& inputBuffer)
{
   OsStatus status = OS_INVALID_ARGUMENT;
   assert(inPortIndex == 0);
   assert(inputBuffer->getType() == MP_BUF_RTP);

   if(inPortIndex == 0 && inputBuffer->getType() == MP_BUF_RTP)
   {
      MpRtpBufPtr rtpBuffer = inputBuffer;
      assert(rtpBuffer.isValid() == inputBuffer.isValid());
      status = pushPacket(rtpBuffer);
   }

   return(status);
}

OsStatus MprDecode::pushPacket(MpRtpBufPtr &pRtp)
{
   // Lock access to dejitter and m*Codecs data
   OsLock lock(mLock);

#ifdef RTL_ENABLED // [
   UtlString str_fg(getFlowGraph()->getFlowgraphName());
   str_fg.append("_");
   str_fg.append(getName());
#endif // RTL_ENABLED ]
   RTL_EVENT(str_fg+"_pushPacket_seq", pRtp->getRtpSequenceNumber());
   RTL_EVENT(str_fg+"_pushPacket_TS", pRtp->getRtpTimestamp());
   dprintf("> %" PRIu16 " %" PRIu32 " %d",
           pRtp->getRtpSequenceNumber(), pRtp->getRtpTimestamp(),
           pRtp->getRtpPayloadType());

   // Get decoder info for this packet.
   int pt = pRtp->getRtpPayloadType();
   const MpDecoderBase *pDecoder = mDecoderMap.mapPayloadType(pt);
   if (pDecoder == NULL)
   {
      // No decoder for this packet - just return error.
      dprintf(" No decoder for payload type %d!\n", pt);
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprDecode::pushPacket() dropping RTP packet, no decoder for payload type %d %s flowgraph: %p",
                    pt, data(), mpFlowGraph);
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
      // Apply G.722 clock rate workaround if requested
      mStreamState.sampleRate = (pt!=mG722HackPayloadType)?pDecoderInfo->getSampleRate():8000;
      mStreamState.playbackFrameSize = getFlowGraph()->getSamplesPerFrame()
                                       * mStreamState.sampleRate
                                       / getFlowGraph()->getSamplesPerSec();
      mStreamState.playbackStreamPosition = pRtp->getRtpTimestamp();
      mStreamState.dejitterLength = 0;

//      RTL_EVENT(str_fg+"_PF_stream_position", mStreamState.playbackStreamPosition);

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
      // Sample rate must not change during the active stream.
      if (mStreamState.sampleRate !=
          ((pt!=mG722HackPayloadType)?pDecoderInfo->getSampleRate():8000))
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprDecode::pushPacket() dropping RTP packet, "
                       "dynamic samplerate change is not supported. pt=%d, rate=%d, stream rate=%d %s flowgraph: %p",
                       pt, pDecoderInfo->getSampleRate(), mStreamState.sampleRate, data(), mpFlowGraph);
         return OS_SUCCESS;
      }

      // RFC4733 states that timestamp always refers to the beginning of a tone,
      // so it will remain constant among a lot of packets. We don't want to
      // confuse our jitter buffer estimation with this.
      // TODO: Later, we will want to handle this correctly inside JBE itself,
      // because non-straightforward timestamps are also the case for, e.g.
      // redundant payloads.
      if (!pDecoderInfo->isSignalingCodec())
      {
         // Update jitter state data
         mpJbEstimationState->update(&pRtp->getRtpHeader(),
                                     mStreamState.rtpStreamPosition,
                                     mStreamState.playbackStreamPosition,
                                     &mStreamState.rtpStreamHint);
         // Update dejitter virtual length
         mStreamState.dejitterLength = pRtp->getRtpTimestamp() - mStreamState.rtpStreamPosition;
      }
   }

   RTL_EVENT(str_fg+"_pushPacket_rtp_stream_hint",
             mStreamState.rtpStreamHint);
   RTL_EVENT(str_fg+"_pushPacket_stream_position",
             mStreamState.playbackStreamPosition);
   dprintf("\n");

   return mpMyDJ->pushPacket(pRtp);
}

void MprDecode::setConnectionId(MpConnectionID connectionId)
{
   MpAudioResource::setConnectionId(connectionId);
   if (mpMyDJ)
   {
      mpMyDJ->setConnectionId(connectionId);
   }
}

void MprDecode::setStreamId(int StreamId)
{
   MpAudioResource::setStreamId(StreamId);
   if (mpMyDJ)
   {
      mpMyDJ->setStreamId(StreamId);
   }
}

/* ============================ ACCESSORS ================================= */

UtlContainableType MprDecode::getContainableType() const
{
   return TYPE;
}

OsStatus MprDecode::getCurrentLatency(int &latency, int input, int output) const
{
   if (input == ASSOCIATED_LATENCY && output == 0)
   {
      if (mIsStreamInitialized && isEnabled())
      {
         // Calculate JB latency
         latency = mStreamState.dejitterLength*mStreamState.sampleRate/getFlowGraph()->getSamplesPerSec();
         // Then we should add decoder algorithmic delay, but we don't know
         // which decoder we're using now - we should store last decoder used
         // and refer to it here. But most decoders have 0 latency. Notable
         // exclusion is Speex, which has 5ms latency in decoder, but it's
         // not too much for us now.
//         latency += pDecoder->getInfo()->getAlgorithmicDelay();
         return OS_SUCCESS;
      }
      else
      {
         latency = INF_LATENCY;
         return OS_SUCCESS;
      }
   }

   return OS_NOT_FOUND;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

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
   str_fg.append("_");
   str_fg.append(getName());
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

   // Pull packets from JB queue, until we get enough samples to playback
   dprintf("# pos:%" PRIu32, mStreamState.playbackStreamPosition);
   RTL_EVENT(str_fg+"_PF_stream_position", mStreamState.playbackStreamPosition);
   dprintf(" buf=%-3d", mpJB->getSamplesNum());
   RTL_EVENT(str_fg+"_PF_buffered_samples", mpJB->getSamplesNum());
   while (mpJB->getSamplesNum() < mpFlowGraph->getSamplesPerFrame())
   {
      MpRtpBufPtr rtp;
      UtlBoolean nextPacketAvailable=FALSE;

      RTL_EVENT(str_fg+"_PF_stream_position_inloop",
                mStreamState.playbackStreamPosition);

      // Step 1. Pull next packet from JB queue (MprDejitter)
      RTL_EVENT(str_fg+"_PF_dej_pull_position",
                mStreamState.rtpStreamPosition);
      dprintf(" pull[?%" PRIu32, mStreamState.rtpStreamPosition);
      dprintf(" JBlength=%d/%d",
              mpMyDJ->getNumLatePackets(), mpMyDJ->getNumPackets());
      RTL_EVENT(str_fg+"_PF_JB_length", mpMyDJ->getNumPackets());
      RTL_EVENT(str_fg+"_PF_JB_late_packets", mpMyDJ->getNumLatePackets());
      rtp = mpMyDJ->pullPacket(mStreamState.rtpStreamPosition,
                               &nextPacketAvailable);

      // We did not get a packet, but there are some in there.  Check if we have a discontinuity
      // in the time stamp
      if(!rtp.isValid())
      {
         int numPacketInDejitter = mpMyDJ->getNumPackets();
         if(mStreamState.isFirstRtpPulled && numPacketInDejitter > 2)
         {
            RtpSeq nextSeq;
            RtpTimestamp nextTime;
            OsStatus nextStatus = mpMyDJ->getFirstPacketInfo(nextSeq, nextTime);
            // Sequentially the next packet is available, so use its time stamp to get it
            // This can occur if the remote side introduces a discontinuity in the time stamp
            if(nextStatus == OS_SUCCESS &&
               MpDspUtils::compareSerials(mLastPulledSeq + 1, nextSeq) >= 0)
            // TODO: allow more slop in sequence to account for signalling or if there are a lot of packet already
            // in dejitter
            {
               rtp = mpMyDJ->pullPacket(nextTime, &nextPacketAvailable);
#ifdef ANDROID
               LOGV("doProcessFrame pullPacket second attempt due to sequential packets, possible time stamp discontinuity nextTime: %u",
                    nextTime);
#endif
            }
         }
      }
      else
      {
            mLastPulledSeq = rtp->getRtpSequenceNumber();
      }

      if (rtp.isValid())
      {
         dprintf(" <%" PRIu32 " n=%d", rtp->getRtpTimestamp(), nextPacketAvailable);
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
            mStreamState.rtpStreamPosition += mStreamState.playbackFrameSize;
            dprintf("]\n");
            return TRUE;
         }
      }
      if (rtp.isValid())
      {
         // Update stream payload type
         mStreamState.rtpPayloadType = rtp->getRtpPayloadType();
      }
      dprintf("]");

      // Step 2. Push packet to decoder buffer (MpJitterBuffer)
      int wantedBufferSamples = (int)( mStreamState.rtpStreamPosition
                                     - mStreamState.playbackStreamPosition)
                              + mStreamState.rtpStreamHint;
#ifdef ANDROID
      LOGV("doProcessFrame wantedBufferSamples: %d rtpStreamPosition: %u playbackStreamPosition: %u rtpStreamHint: %d JB getSamplesNum(): %d",
           wantedBufferSamples, mStreamState.rtpStreamPosition, mStreamState.playbackStreamPosition, mStreamState.rtpStreamHint, mpJB->getSamplesNum());
#endif

      int adjustment;
      int decodedLength;
      UtlBoolean isPlayed;
      OsStatus res = mpJB->pushPacket(rtp,
                                      nextPacketAvailable?0:mpFlowGraph->getSamplesPerFrame(),
                                      wantedBufferSamples,
                                      decodedLength,
                                      adjustment,
                                      isPlayed);
      if (mStreamState.rtpPayloadType == mG722HackPayloadType)
      {
         decodedLength /= 2;
      }
      dprintf(" decoded=%d", decodedLength);
      dprintf(" adj[h=%" PRId32 " ?%d %d p?%d",
              mStreamState.rtpStreamHint, wantedBufferSamples, adjustment,
              int(isPlayed));
      RTL_EVENT(str_fg+"_PF_rtp_stream _hint", mStreamState.rtpStreamHint);
      RTL_EVENT(str_fg+"_PF_wanted_buffer_samples", wantedBufferSamples);
      RTL_EVENT(str_fg+"_PF_adjustment", adjustment);
      RTL_EVENT(str_fg+"_PF_is_played", isPlayed);

      // We should not adjust stream position if we've just decoded a pure
      // signaling packet. According to RFC4733 its timestamp is set to
      // the beginning of a tone, so we will constantly jump back in time
      // if we set our stream position to its timestamp.
      if (decodedLength>0)
      {
         if (isPlayed)
         {
            // Discontiuous change in RTP time stamp, need to make similar update to
            // playback time.
            if(mStreamState.rtpStreamPosition + 5000 < rtp->getRtpTimestamp())
            {
               mStreamState.playbackStreamPosition += (rtp->getRtpTimestamp() - mStreamState.rtpStreamPosition);
            }
            mStreamState.rtpStreamPosition = rtp->getRtpTimestamp() + decodedLength;
         } 
         else
         {
            mStreamState.rtpStreamPosition += decodedLength;
         }
      }
#ifdef ANDROID
      LOGV("doProcessFrame decodedLength: %d isPlayed: %s rtpStreamPosition: %u",
           decodedLength, isPlayed ? "true" : "false", mStreamState.rtpStreamPosition);
#endif

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
      // If DTMF packet is received before any audio packet, PLC is not ready
      // and does not produce any samples. We need to gracefully handle this
      // situation instead of infinite looping.
      if (!rtp.isValid() && decodedLength==0)
      {
         break;
      }

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
         MprnDTMFMsg dtmfMsg(getName(),
                             (MprnDTMFMsg::KeyCode)event,
                             MprnDTMFMsg::KEY_DOWN);
         sendNotification(dtmfMsg);
         OsSysLog::add(FAC_MP, PRI_INFO,
                       "%s: DTMF KEY_DOWN event received for key %d\n",
                        getName().data(), event);
      }

      if (sigRes == OS_SUCCESS && isStopped)
      {
         // Post DTMF notification message to indicate key up.
         MprnDTMFMsg dtmfMsg(getName(),
                             (MprnDTMFMsg::KeyCode)event,
                             MprnDTMFMsg::KEY_UP, duration);
         sendNotification(dtmfMsg);
         OsSysLog::add(FAC_MP, PRI_INFO,
                       "%s: DTMF KEY_UP event received for key %d, duration %d\n",
                       getName().data(), event, duration);
      }
   } while (sigRes == OS_SUCCESS);

   return TRUE;
}

OsStatus MprDecode::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   OsStatus stat = MpResource::setFlowGraph(pFlowGraph);

   // If the parent's call was successful, then call
   // setFlowGraph on any child resources we have.
   if(stat == OS_SUCCESS)
   {
      if (pFlowGraph != NULL)
      {
         mpJB->setFlowGraph(pFlowGraph);
         mpMyDJ->setFlowgrapName(pFlowGraph->getFlowgraphName());
      }
      else
      {
         mpJB->setFlowGraph(NULL);
         mpMyDJ->setFlowgrapName("None");
      }
   }
   return stat;
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

   case MpResourceMsg::MPRM_DECODE_SELECT_CODECS:
      {
         MprDecodeSelectCodecsMsg *pMsg = (MprDecodeSelectCodecsMsg*)&rMsg;
         msgHandled = handleSelectCodecs(pMsg->getCodecs(), pMsg->getNumCodecs());
      }
      break;

   case MPRM_DESELCT_CODECS:
      msgHandled = handleDeselectCodecs();
      break;

   case MPRM_RESET:
      msgHandled = handleReset();
      break;

   case MPRM_SET_VAD_PARAM:
      {
          MpPackedResourceMsg packedMessage = (MpPackedResourceMsg&) rMsg;
          UtlSerialized& serialData = packedMessage.getData();
          msgHandled = handleSetVadParam(serialData);
      }
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
#if 0
   MpDecoderBase* pOldDecoder;
   int canReuse;
#endif
   MpCodecFactory* pFactory = MpCodecFactory::getMpCodecFactory();
   int allReusable = 0;
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
                       "pCodecs[%d]->getCodecPayloadFormat() = %d "
                       "%s flowgraph: %p",
                       i, pCodec->getCodecType(),
                       i, pCodec->getCodecPayloadFormat(),
                       data(), mpFlowGraph);
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
      // Reset G.722 clock rate workaround flag.
      mG722HackPayloadType = -1;

      mNumCurrentCodecs = 0;
      mpCurrentCodecs = new MpDecoderBase*[numCodecs];
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprDecode::handleSelectCodecs allocating mpCurrentCodecs: %p %s flowgraph: %p", 
                    mpCurrentCodecs, data(), mpFlowGraph);
      UtlString codecMediaType;

      for (i=0; i<numCodecs; i++)
      {
         pCodec = pCodecs[i];
         pCodec->getMediaType(codecMediaType);
         if(codecMediaType.compareTo(MIME_TYPE_AUDIO, UtlString::ignoreCase) == 0)
         {
            pCodec->getEncodingName(mime);
            pCodec->getSdpFmtpField(fmtp);
            int sampleRate = pCodec->getSampleRate();
            int numChannels = pCodec->getNumChannels();
            payload = pCodec->getCodecPayloadFormat();
            ret = pFactory->createDecoder(mime, fmtp, sampleRate, numChannels,
                                          payload, pNewDecoder);
            assert(OS_SUCCESS == ret);
            assert(NULL != pNewDecoder);
            OsStatus codecInitStatus = pNewDecoder->initDecode();
            if (codecInitStatus == OS_SUCCESS)
            {
               // Add this codec to mpConnection's payload type decoding table.
               mDecoderMap.addPayloadType(payload, pNewDecoder);
               mpCurrentCodecs[mNumCurrentCodecs] = pNewDecoder;
               mNumCurrentCodecs++;

               // Check should we apply G.722 clock rate workaround to this codec?
               if (mEnableG722Hack && pCodec->getCodecType() == SdpCodec::SDP_CODEC_G722)
               {
                  mG722HackPayloadType = payload;
               }
            }
            else
            {
               OsSysLog::add(FAC_MP, PRI_ERR,
                       "MprDecode::handleSelectCodecs codec decoder: %s SDP fmpt: %s payload ID: %d (internal ID: %d) failed to init with status: %d %s flowgraph: %p",
                       mime.data(), fmtp.data(), payload, pCodec->getCodecType(), codecInitStatus, data(), mpFlowGraph);
               delete pNewDecoder;
            }
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
   if (0 < mNumCurrentCodecs) 
   {
      // Not sure why previous codecs are kept.  I can think of the following reasons to do so:
      //  1) There is lag in the RTP change in payloads after a signalling payload renegotiation so
      //     so the prior codec set could still be in use for a short period of time.  However we are 
      //     not careful to avoid codec payload ID conflicts in the prior set.
      //  2) We could and probably should do some optimization here and reuse the codecs if they are the 
      //     same as the prior set.  In which case we should keep track of the prior payload IDs so that
      //     we can reuse the codec even if the payload ID changes.
      //  3) To avoid calling free while in the mediaTask and avoid the malloc/free locking/blocking in
      //     the realtime loop.
      //
      // However we certainly are not yet doing 1) or 2) yet.
      deletePriorCodecs();

      newN = mNumCurrentCodecs + mNumPrevCodecs;
      pPrevCodecs = new MpDecoderBase*[newN];
      OsSysLog::add(FAC_MP, PRI_DEBUG,
              "MprDecode::handleDeselectCodecs allocated pPrevCodecs(to be mpPrevCodecs): %p %s flowgraph: %p", 
              pPrevCodecs, data(), mpFlowGraph);
#if 0
      osPrintf("|handleDeselectCodecs(0x%X): (0x%X,%d) -> (0x%X,%d) (+%d)\n",
         (int) this, (int) mpPrevCodecs, mNumPrevCodecs, (int) pPrevCodecs,
         newN, mNumCurrentCodecs);
#endif
      if (mNumPrevCodecs > 0) {
         for (i=0; i<mNumPrevCodecs; i++) {
            pPrevCodecs[i] = mpPrevCodecs[i];
         }
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprDecode::handleDeselectCodecs deleting mpPrevCodecs: %p %s flowgraph: %p", 
                       mpPrevCodecs, data(), mpFlowGraph);
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
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprDecode::handleDeselectCodecs deleting pCurrentCodecs: %p %s flowgraph: %p", 
                    pCurrentCodecs, data(), mpFlowGraph);
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

UtlBoolean MprDecode::handleReset()
{
   OsLock lock(mLock);

   // Reset JB, JBE and dejitter
   mpJB->reset();
   mpJbEstimationState->reset();
   if (mpMyDJ != NULL)
   {
      mpMyDJ->reset();
   }

   mIsStreamInitialized = FALSE;

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprDecode::handleReset %s flowgraph: %p",
                 data(), mpFlowGraph);

   // Not sure why this was here.  mpPrevCodecs is not used and this
   // caused a huge leak.  If mNumPrevCodecs is to be set to zero, 
   // mpPrevCodecs needs to be freed up first.
   //mNumPrevCodecs = 0;

   return TRUE;
}

UtlBoolean MprDecode::handleDisable()
{
   if (isEnabled() && mpFlowGraph)
   {
      // Reset decoder and all algorithms to start fresh on enable.
      handleReset();
   }
   return MpResource::handleDisable();
}

UtlBoolean MprDecode::handleSetVadParam(UtlSerialized& serialData)
{
    assert(mpJB);
    if(mpJB)
    {
        UtlString name;
        int value;
        serialData.deserialize(name);
        serialData.deserialize(value);
        OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MprDecode setting VAD parameter: %s value: %d",
            name.data(), value);
        mpJB->setVadParam(name, value);
    }
    return(TRUE);
}

/* ============================ FUNCTIONS ================================= */
