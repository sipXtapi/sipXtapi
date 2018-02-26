//  
// Copyright (C) 2006-2018 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>


#include "os/OsIntTypes.h"
#ifdef __pingtel_on_posix__
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifdef WIN32 /* [ */
#include <winsock2.h>
#endif /* WIN32 ] */

#include <string.h>

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <mp/MpMisc.h>
#include <mp/MpBuf.h>
#include <mp/MprEncode.h>
#include <mp/MprToNet.h>
#include <mp/MpEncoderBase.h>
#include <mp/dmaTask.h>
#include <mp/MpMediaTask.h>
#include <mp/MpCodecFactory.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MprnRtpStreamActivityMsg.h>
#include <mp/MprDecodeSelectCodecsMsg.h>
#include <mp/MpIntResourceMsg.h>

// DEFINES
#define DEBUG_DTMF_SEND
#undef  DEBUG_DTMF_SEND
#ifdef DEBUG_PRINT
#  define dprintf printf
#else
  static inline void dprintf(...) {};
#endif

//#define TEST_PRINT

//#define ALWAYS_SEND_SILENCE

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
   // At 10 ms each, 10 seconds.  We will send an RTP packet to each active
   // destination at least this often, even when muted.
   const int MprEncode::RTP_KEEP_ALIVE_FRAME_INTERVAL = 1000;
   const UtlContainableType MprEncode::TYPE = "MprEncode";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprEncode::MprEncode(const UtlString& rName)
:  MpAudioResource(rName, 1, 1, 1, 1),
   mpPrimaryCodec(NULL),
   mpPacket1Payload(NULL),
   mPacket1PayloadBytes(0),
   mPayloadBytesUsed(0),
   mSamplesPacked(0),
   mActiveAudio1(FALSE),
   mMarkNext1(TRUE),
   mConsecutiveInactive1(0),
   mConsecutiveActive1(0),
   mConsecutiveUnsentFrames1(0),
   mDoesVad1(FALSE),
   mDisableDTX(TRUE),
   mEnableG722Hack(TRUE),
   mDoG722Hack(FALSE),

   mNeedResample(FALSE),
   mpResampler(MpResamplerBase::createResampler(1, 8000, 8000)),
   mResampleBufLen(0),
   mpResampleBuf(NULL),

   mpDtmfCodec(NULL),
   mpPacket2Payload(NULL),
   mPacket2PayloadBytes(0),

   mCurrentTone(-1),
   mNumToneStops(-1),
   mTotalTime(0),
   mNewTone(FALSE),

   mCurrentTimestamp(0),
   mMaxPacketTime(20),

   mpToNet(NULL)
{
}

// Destructor
MprEncode::~MprEncode()
{
   delete[] mpPacket1Payload;
   delete[] mpResampleBuf;
   delete[] mpPacket2Payload;
   delete mpPrimaryCodec;
   delete mpDtmfCodec;
   delete mpResampler;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprEncode::selectCodecs(const UtlString& namedResource, OsMsgQ& fgQ,
                                 SdpCodec* pPrimary, SdpCodec* pDtmf)
{
   SdpCodec* newCodecs[2];
   newCodecs[0] = pPrimary;
   newCodecs[1] = pDtmf;
   MprDecodeSelectCodecsMsg msg(namedResource, newCodecs, 2);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprEncode::deselectCodecs(const UtlString& namedResource, OsMsgQ& fgQ)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_DESELECT_CODECS, namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

void MprEncode::setMyToNet(MprToNet* myToNet)
{
   mpToNet = myToNet;
}

OsStatus MprEncode::startTone(const UtlString& namedResource, OsMsgQ& fgQ,
                              int toneId)
{
   MpIntResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_START_TONE,
                        namedResource, toneId);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprEncode::stopTone(const UtlString& namedResource, OsMsgQ& fgQ)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_STOP_TONE, namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprEncode::enableDtx(const UtlString& namedResource, 
                              OsMsgQ& fgQ,
                              UtlBoolean dtx)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)(dtx?MPRM_ENABLE_DTX
                                                           :MPRM_DISABLE_DTX),
                     namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprEncode::setMaxPacketTime(const UtlString& namedResource, OsMsgQ& fgQ,
                                     unsigned int maxPacketTime)
{
   MpIntResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_SET_MAX_PACKET_TIME,
                        namedResource, maxPacketTime);
   return fgQ.send(msg, sOperationQueueTimeout);
}

/* ============================ ACCESSORS ================================= */

UtlContainableType MprEncode::getContainableType() const
{
   return TYPE;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsStatus MprEncode::allocPacketBuffer(const MpEncoderBase& rEncoder,
                                      unsigned char*& rpPacketPayload,
                                      int& rPacketPayloadBytes)
{
   OsStatus ret = OS_SUCCESS;

   // Set packet size to maximum possible size. Probably we could guess better,
   // but to do so we need:
   // 1) to know how much audio data we want to pack (10ms, 20ms, or more);
   // 2) somehow negotiate packet size for codecs that require special
   //    processing to pack several frames into one packet (like AMR, Speex, etc).
   // One of possible solution would be to implement function to ask for
   // packet size (not frame size!) for codecs. That is pass to it number of
   // audio samples we want to pack and get packet size back from it, like this:
   //    int get_packet_size(int numSamples, int* packetSize) const;
   rPacketPayloadBytes = RTP_MTU;

   // Allocate buffer for RTP packet data
   rpPacketPayload = new unsigned char[rPacketPayloadBytes];
   if (rpPacketPayload == NULL )
   {
      // No free memory. Return error.
      ret = OS_NO_MEMORY;
   }

   return ret;
}

void MprEncode::handleDeselectCodecs(void)
{
   if (NULL != mpPrimaryCodec) {
      delete mpPrimaryCodec;
      mpPrimaryCodec = NULL;
      if (NULL != mpPacket1Payload) {
         delete[] mpPacket1Payload;
         mpPacket1Payload = NULL;
         mPacket1PayloadBytes = 0;
         mPayloadBytesUsed = 0;
         mSamplesPacked = 0;
      }
      if (mNeedResample)
      {
         mNeedResample = FALSE;
         mpResampler->resetStream();
         mResampleBufLen = 0;
         delete[] mpResampleBuf;
         mpResampleBuf = NULL;
      }
      if (mMarkNext1 == FALSE)
      {
         // Codecs were deselected in the middle of the stream. Send notification.
         notifyStopTx();
      }
   }
   if (NULL != mpDtmfCodec) {
      delete mpDtmfCodec;
      mpDtmfCodec = NULL;
      if (NULL != mpPacket2Payload) {
         delete[] mpPacket2Payload;
         mpPacket2Payload = NULL;
         mPacket2PayloadBytes = 0;
      }
   }
}

void MprEncode::handleSelectCodecs(int newCodecsCount, SdpCodec** newCodecs)
{
   SdpCodec* pPrimary;
   SdpCodec* pDtmf;
   MpEncoderBase* pNewEncoder;
   MpCodecFactory* pFactory = MpCodecFactory::getMpCodecFactory();
   UtlString mime;
   UtlString fmtp;
   OsStatus ret;
   int payload;

   // We should be only given 2 codecs here right now --
   // a data codec, and a signaling codec (for tones).
   assert(newCodecsCount == 2);
   pPrimary = newCodecs[0];
   pDtmf = newCodecs[1];

   handleDeselectCodecs();  // cleanup the old ones, if any

   if (NULL != pPrimary) 
   {
      pPrimary->getEncodingName(mime);
      pPrimary->getSdpFmtpField(fmtp);
      int sampleRate = pPrimary->getSampleRate();
      int numChannels = pPrimary->getNumChannels();
      payload = pPrimary->getCodecPayloadFormat();
      ret = pFactory->createEncoder(mime, fmtp, sampleRate, numChannels,
                                    payload, pNewEncoder);
      assert(OS_SUCCESS == ret);
      assert(NULL != pNewEncoder);
      pNewEncoder->initEncode();
      mpPrimaryCodec = pNewEncoder;
      mDoesVad1 = pNewEncoder->getInfo()->doesVadCng();
      allocPacketBuffer(*mpPrimaryCodec, mpPacket1Payload, mPacket1PayloadBytes);
      mPayloadBytesUsed = 0;
      mSamplesPacked = 0;

      // Setup resampling
      unsigned codecSamplesPerSec = mpPrimaryCodec->getInfo()->getSampleRate();
      unsigned flowgraphSamplesPerSec = mpFlowGraph->getSamplesPerSec();
      mNeedResample = UtlBoolean(flowgraphSamplesPerSec != codecSamplesPerSec);
      if (mNeedResample)
      {
         mpResampler->setInputRate(flowgraphSamplesPerSec);
         mpResampler->setOutputRate(codecSamplesPerSec);
         mResampleBufLen = mpFlowGraph->getSamplesPerFrame()
                           * codecSamplesPerSec/flowgraphSamplesPerSec;
         // If there is round off, leave room for one more sample for resampler
         if((mpFlowGraph->getSamplesPerFrame()                                                                               * codecSamplesPerSec) % flowgraphSamplesPerSec)
         {
            mResampleBufLen++;
         }
         mpResampleBuf = new MpAudioSample[mResampleBufLen];
      }

      // Check is G.722 workaround needed
      if (mEnableG722Hack && pPrimary->getCodecType() == SdpCodec::SDP_CODEC_G722)
      {
         mDoG722Hack = TRUE;
      }
      else
      {
         mDoG722Hack = FALSE;
      }

      mMaxPacketSamples = mMaxPacketTime*codecSamplesPerSec/1000;
      mMarkNext1 = TRUE;

      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprEncode::handleSelectCodecs "
                    "pPrimary->getEncodingName() = %s, "
                    "pPrimary->getSdpFmtpField() = %s, "
                    "pPrimary->getCodecPayloadFormat() = %d",
                    mime.data(), fmtp.data(),
                    pPrimary->getCodecPayloadFormat());
   }
   else 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprEncode::handleSelectCodecs "
                    "pPrimary == NULL");
   }

   if (NULL != pDtmf) 
   {
      pDtmf->getEncodingName(mime);
      pDtmf->getSdpFmtpField(fmtp);
      int sampleRate = pDtmf->getSampleRate();
      int numChannels = pDtmf->getNumChannels();
      payload = pDtmf->getCodecPayloadFormat();
      ret = pFactory->createEncoder(mime, fmtp, sampleRate, numChannels,
                                    payload, pNewEncoder);
      assert(OS_SUCCESS == ret);
      assert(NULL != pNewEncoder);
      pNewEncoder->initEncode();
      mpDtmfCodec = pNewEncoder;
      allocPacketBuffer(*mpDtmfCodec, mpPacket2Payload, mPacket2PayloadBytes);

      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprEncode::handleSelectCodecs "
                    "pDtmf->getEncodingName() = %s, "
                    "pDtmf->getSdpFmtpField() = %s, "
                    "pDtmf->getCodecPayloadFormat() = %d",
                    mime.data(), fmtp.data(),
                    pDtmf->getCodecPayloadFormat());
   }
}

void MprEncode::handleStartTone(int toneId)
{
   if (NULL == mpDtmfCodec) return;
   if ((mCurrentTone == -1) && (mNumToneStops < 1)) {
      mCurrentTone = lookupTone(toneId);
      if (mCurrentTone != -1) {
         mNewTone = TRUE;
      }
   }
}

void MprEncode::handleStopTone(void)
{
   if ((mCurrentTone > -1) && (mNumToneStops < 1)) {
      mNumToneStops = TONE_STOP_PACKETS; // send TONE_STOP_PACKETS end packets
   }
}

void MprEncode::handleEnableDTX(UtlBoolean dtx)
{
   mDisableDTX = !dtx;
}

void MprEncode::handleSetMaxPacketTime(unsigned maxPacketTime)
{
   mMaxPacketTime = maxPacketTime;
}

UtlBoolean MprEncode::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   switch (rMsg.getMsg()) 
   {
   case MpResourceMsg::MPRM_DECODE_SELECT_CODECS:
      {
         MprDecodeSelectCodecsMsg *pMsg = (MprDecodeSelectCodecsMsg*)&rMsg;
         handleSelectCodecs(pMsg->getNumCodecs(), pMsg->getCodecs());
      }
      msgHandled = TRUE;
      break;

   case MPRM_DESELECT_CODECS:
      handleDeselectCodecs();
      msgHandled = TRUE;
      break;

   case MPRM_START_TONE:
      {
         MpIntResourceMsg *pMsg = (MpIntResourceMsg*)&rMsg;
         handleStartTone(pMsg->getData());
      }
      msgHandled = TRUE;
      break;

   case MPRM_STOP_TONE:
      handleStopTone();
      msgHandled = TRUE;
      break;

   case MPRM_SET_MAX_PACKET_TIME:
      {
         MpIntResourceMsg *pMsg = (MpIntResourceMsg*)&rMsg;
         handleSetMaxPacketTime(pMsg->getData());
      }
      msgHandled = TRUE;
      break;

   case MPRM_ENABLE_DTX:
      handleEnableDTX(TRUE);
      msgHandled = TRUE;
      break;

   case MPRM_DISABLE_DTX:
      handleEnableDTX(FALSE);
      msgHandled = TRUE;
      break;

   default:
      // If we don't handle the message here, let our parent try.
      msgHandled = MpResource::handleMessage(rMsg); 
      break;
   }
   return msgHandled;
}

// Translate our tone ID into RFC2833 values.
int MprEncode::lookupTone(int toneId)
{
   int ret = -1;

   switch (toneId) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         ret = toneId - '0';
         break;
      case  0 :
      case  1 :
      case  2 :
      case  3 :
      case  4 :
      case  5 :
      case  6 :
      case  7 :
      case  8 :
      case  9 :
         ret = toneId;
         break;
      case 'a': case 'A':
      case 'b': case 'B':
      case 'c': case 'C':
      case 'd': case 'D':
         ret = ((toneId | ('a' ^ 'A')) - ('a' | ('a' ^ 'A'))) + 12;
         break;
      case '*':
         ret = 10;
         break;
      case '#':
         ret = 11;
         break;
   }
   return ret;
}

void MprEncode::doPrimaryCodec(MpAudioBufPtr in)
{
   uint32_t numSamplesIn;
   int numSamplesOut;
   const MpAudioSample* pSamplesIn;
   int payloadBytesLeft;
   unsigned char* pDest;
   int bytesAdded;
   OsStatus ret;
   UtlBoolean isPacketReady;
   UtlBoolean isPacketSilent;
   UtlBoolean codecWantsMarkerSet = FALSE;
   unsigned int codecFrameSamples;

   // TODO:: Here we have a bug, which will be visible when DTX is enabled.
   //        Consider codec with packet size bigger then our internal frame size.
   //        And consider we've just sent a packet to the net and pushed next
   //        frame of data to encoder. So we have some data in encoder and it's
   //        waiting for more data to complete the packet. And suddenly stream
   //        stops. This part of old data retains in the encoder's buffer until
   //        stream resumes and then it's gotten encoded into the packet with
   //        the new frame of data. Then on decoder side we have a piece of old
   //        data at the beginning of new data which may produce an audible click.

#ifdef ALWAYS_SEND_SILENCE
   // We get null audio buffers if there are no active inputs mixed at the bridge/upstream resource.
   // This forces there to be silence audio samples to be encoded and a RTP packet to be created
   // for silence.
   // TODO: this should be runtime configurable and ideally we should be able to state that a silence
   // packet should be sent periodically.
   if(! in.isValid())
   {
      in = MpMisc.mpFgSilence;
#  ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MprEncode::doPrimaryCodec MpMisc.mpFgSilence contains %d samples",
         in->getSamplesNumber());
#  endif
   }
#endif

   if (mpPrimaryCodec == NULL || !in.isValid())
   {
      if (mMarkNext1 == FALSE)
      {
         // This is the first empty frame after active stream.
         notifyStopTx();
         mMarkNext1 = TRUE;
      }

      // Update current timestamp to maintain RTP clock.
      if (mNeedResample && mpPrimaryCodec != NULL)
      {
         mCurrentTimestamp += mpFlowGraph->getSamplesPerFrame()
                              *mpPrimaryCodec->getInfo()->getSampleRate()
                              /mpFlowGraph->getSamplesPerSec();
      }
      else
      {
         mCurrentTimestamp += mpFlowGraph->getSamplesPerFrame();
      }

#ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MprEncode::doPrimaryCodec invalid input buffer");
#endif

      return;
   }

   // Do resampling if needed.
   if (mNeedResample)
   {
      uint32_t samplesConsumed;
      mpResampler->resample(0,
                            in->getSamplesPtr(), in->getSamplesNumber(), samplesConsumed,
                            mpResampleBuf, mResampleBufLen, numSamplesIn);

      // If we are using silence, we do not care if we drop stuff on the floor.
      // TODO: optimize and don't resample silence.
      if(in != MpMisc.mpFgSilence &&
         samplesConsumed != in->getSamplesNumber())
      {
         int inSamples = in->getSamplesNumber();
         OsSysLog::add(FAC_MP, PRI_ERR,
            "MprEncode::doPrimaryCodec should be equal samplesConsumed = %d in->getSamplesNumber() = %d",
            samplesConsumed, inSamples);
         OsSysLog::flush();
         assert(samplesConsumed == in->getSamplesNumber());
      }
      pSamplesIn = mpResampleBuf;
   }
   else
   {
      numSamplesIn = in->getSamplesNumber();
      pSamplesIn = in->getSamplesPtr();
   }

   while (numSamplesIn > 0)
   {
      if (mPayloadBytesUsed == 0)
      {
         mStartTimestamp1 = mCurrentTimestamp;
         mActiveAudio1 = mDoesVad1 || mDisableDTX;
      }

      mActiveAudio1 = mActiveAudio1 || isActiveAudio(in->getSpeechType());

      payloadBytesLeft = mPacket1PayloadBytes - mPayloadBytesUsed;

      pDest = mpPacket1Payload + mPayloadBytesUsed;

      bytesAdded = 0;
      ret = mpPrimaryCodec->encode(pSamplesIn, numSamplesIn, numSamplesOut,
                                   pDest, payloadBytesLeft, bytesAdded,
                                   isPacketReady, isPacketSilent, codecWantsMarkerSet);
      mPayloadBytesUsed += bytesAdded;
      assert (mPacket1PayloadBytes >= mPayloadBytesUsed);

      // In case the encoder does silence suppression (e.g. G.729 Annex B)
      mMarkNext1 = mMarkNext1 || isPacketSilent;

      mSamplesPacked += numSamplesOut;
      pSamplesIn += numSamplesOut;
      numSamplesIn -= numSamplesOut;

      if (mDoG722Hack)
      {
         mCurrentTimestamp += numSamplesOut/2;
      }
      else
      {
         mCurrentTimestamp += numSamplesOut;
      }

      if (mpPrimaryCodec->getInfo()->getCodecType() == CODEC_TYPE_FRAME_BASED)
      {
         codecFrameSamples = mpPrimaryCodec->getInfo()->getNumSamplesPerFrame();
      }
      else if (mpPrimaryCodec->getInfo()->getCodecType() == CODEC_TYPE_SAMPLE_BASED)
      {
         codecFrameSamples = numSamplesOut;
      }
      else
      {
         assert(!"Unknown codec type!");
      }

      if (  (mPayloadBytesUsed > 0)
         && (isPacketReady || 
            (int)(mSamplesPacked + codecFrameSamples) > mMaxPacketSamples))
      {
         if (mActiveAudio1)
         {
            mConsecutiveInactive1 = 0;
         } else {
            mConsecutiveInactive1++;
         }
         if ((  mConsecutiveInactive1 < HANGOVER_PACKETS)
             || (  mConsecutiveUnsentFrames1
                && mConsecutiveUnsentFrames1 >= RTP_KEEP_ALIVE_FRAME_INTERVAL))
         {
            mpToNet->writeRtp(mpPrimaryCodec->getPayloadType(),
                              mMarkNext1 || codecWantsMarkerSet,
                              mpPacket1Payload,
                              mPayloadBytesUsed,
                              mStartTimestamp1,
                              NULL);

#ifdef TEST_PRINT
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MprEncode::doPrimaryCodec called writeRtp\n");
#endif

            if (mMarkNext1 == TRUE)
            {
               // This is the first packet in the stream. Send notification.
               notifyStartTx();
            }

            mMarkNext1 = FALSE;
            mConsecutiveUnsentFrames1 = 0;
         } else {
            if (mMarkNext1 == FALSE)
            {
               // This is the first packet after the stream. Send notification.
               notifyStopTx();
            }
            mMarkNext1 = TRUE;
         }
         mPayloadBytesUsed = 0;
         mSamplesPacked = 0;
      }
   }
}

void MprEncode::doDtmfCodec(int samplesPerFrame, int samplesPerSecond)
{
   int numSampleTimes;
#ifdef DEBUG_DTMF_SEND /* [ */
   int skipped;
#endif /* DEBUG_DTMF_SEND ] */

   if (mCurrentTone == -1)
      return;

   if (mpDtmfCodec == NULL)
      return;

   if (mNewTone == TRUE) {
      mStartTimestamp2 = mCurrentTimestamp;
      mDtmfSampleInterval = samplesPerFrame * 2;
      mNumToneStops = -1;
   }

   if (TONE_STOP_PACKETS == mNumToneStops) {
      mTotalTime = mCurrentTimestamp - mStartTimestamp2;
   }

   if (mNumToneStops-- < 0) {
      if (mNewTone == TRUE ||
          ((mLastDtmfSendTimestamp + mDtmfSampleInterval) <= mCurrentTimestamp)) {

         numSampleTimes = (mCurrentTimestamp + mDtmfSampleInterval) - mStartTimestamp2;
         if (numSampleTimes > ((1<<16) - 1)) numSampleTimes = ((1<<16) - 1);

         mpPacket2Payload[0] = mCurrentTone;
         mpPacket2Payload[1] = 10; // -10 dBm0
         mpPacket2Payload[2] = (numSampleTimes >> 8) & 0xff; // Big Endian
         mpPacket2Payload[3] = numSampleTimes & 0xff; // Big Endian
         mpToNet->writeRtp(mpDtmfCodec->getPayloadType(),
                           (FALSE != mNewTone),  // set marker on first packet
                           mpPacket2Payload,
                           4,
                           mStartTimestamp2,
                           NULL);
         mLastDtmfSendTimestamp = mCurrentTimestamp;
         mNewTone = FALSE;
#ifdef DEBUG_DTMF_SEND /* [ */
         skipped = 0;
      } else {
         skipped = 1;
      }
      {
         osPrintf("doDtmfCodec(%p): %d + %d = %d, %d -- %s\n",
                  this,
                  mLastDtmfSendTimestamp, mDtmfSampleInterval,
                  (mLastDtmfSendTimestamp + mDtmfSampleInterval),
                  mCurrentTimestamp, (skipped ? "skipped" : "sent"));
#endif /* DEBUG_DTMF_SEND ] */
      }
   } else {
      numSampleTimes = mTotalTime;
      if (numSampleTimes > ((1<<16) - 1)) numSampleTimes = ((1<<16) - 1);

      mpPacket2Payload[0] = mCurrentTone;
      mpPacket2Payload[1] = (1<<7) + 10; // -10 dBm0, with E bit
      mpPacket2Payload[2] = (numSampleTimes >> 8) & 0xff; // Big Endian
      mpPacket2Payload[3] = numSampleTimes & 0xff; // Big Endian
      mpToNet->writeRtp(mpDtmfCodec->getPayloadType(),
                        FALSE,
                        mpPacket2Payload,
                        4,
                        mStartTimestamp2,
                        NULL);
      mLastDtmfSendTimestamp = mCurrentTimestamp;
      if (mNumToneStops < 1) { // all done, ready to start next tone.
         mCurrentTone = -1;
         mNumToneStops = -1;
         mTotalTime = 0;
      }
#ifdef DEBUG_DTMF_SEND /* [ */
      osPrintf("doDtmfCodec(%p): %d + %d = %d, %d -- stop sent\n",
               this,
               mLastDtmfSendTimestamp, mDtmfSampleInterval,
               (mLastDtmfSendTimestamp + mDtmfSampleInterval),
               mCurrentTimestamp);
#endif /* DEBUG_DTMF_SEND ] */
   }
}

UtlBoolean MprEncode::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
   MpBufPtr in;

   mConsecutiveUnsentFrames1++;

#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MprEncode::doProcessFrame inBufsSize: %d isEnabled: %s",
      inBufsSize, isEnabled ? "true" : "false");
#endif

   if (inBufsSize == 0)
      return FALSE;

   if (!isEnabled)
      return TRUE;

   in = inBufs[0];

   if (NULL != mpPrimaryCodec) {
      doPrimaryCodec(in);
   }

   if (NULL != mpDtmfCodec) {
      doDtmfCodec(samplesPerFrame, samplesPerSecond);
   }

   return TRUE;
}

void MprEncode::notifyStartTx()
{
   MprnRtpStreamActivityMsg msg(getName(),
                                MprnRtpStreamActivityMsg::STREAM_START,
                                mpToNet->getSSRC(), 0, -1);
   OsStatus stat = sendNotification(msg);
   if (stat != OS_SUCCESS && stat != OS_NOT_FOUND)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MprEncode::notifyStartTx() sendNotification() returns %d",
                    stat);
   }
   dprintf("Start TX\n");
}

void MprEncode::notifyStopTx()
{
   MprnRtpStreamActivityMsg msg(getName(),
                                MprnRtpStreamActivityMsg::STREAM_STOP,
                                mpToNet->getSSRC(), 0, -1);
   OsStatus stat = sendNotification(msg);
   if (stat != OS_SUCCESS && stat != OS_NOT_FOUND)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MprEncode::notifyStartTx() sendNotification() returns %d",
                    stat);
   }
   dprintf("Stop TX\n");
}
