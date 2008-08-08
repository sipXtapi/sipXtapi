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
#include "os/OsDefs.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprEncode.h"
#include "mp/MprToNet.h"
#include "mp/MpEncoderBase.h"
#include "mp/dmaTask.h"
#include "mp/MpMediaTask.h"
#include "mp/MpCodecFactory.h"
#include "mp/MpFlowGraphBase.h"

// DEFINES
#define DEBUG_DTMF_SEND
#undef  DEBUG_DTMF_SEND

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
   // At 10 ms each, 10 seconds.  We will send an RTP packet to each active
   // destination at least this often, even when muted.
   const int MprEncode::RTP_KEEP_ALIVE_FRAME_INTERVAL = 1000;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprEncode::MprEncode(const UtlString& rName)
:  MpAudioResource(rName, 1, 1, 0, 0),
   mpPrimaryCodec(NULL),
   mpPacket1Payload(NULL),
   mPacket1PayloadBytes(0),
   mPayloadBytesUsed(0),
   mSamplesPacked(0),
   mActiveAudio1(FALSE),
   mMarkNext1(FALSE),
   mConsecutiveInactive1(0),
   mConsecutiveActive1(0),
   mConsecutiveUnsentFrames1(0),
   mDoesVad1(FALSE),
   mDisableDTX(TRUE),

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

void MprEncode::setMyToNet(MprToNet* myToNet)
{
   mpToNet = myToNet;
}

OsStatus MprEncode::startTone(int toneId)
{
   MpFlowGraphMsg msg(START_TONE, this, NULL, NULL, toneId, 0);
   return postMessage(msg);
}

OsStatus MprEncode::stopTone(void)
{
   MpFlowGraphMsg msg(STOP_TONE, this, NULL, NULL, 0, 0);
   return postMessage(msg);
}

OsStatus MprEncode::enableDTX(UtlBoolean dtx)
{
   MpFlowGraphMsg msg(ENABLE_DTX, this, NULL, NULL, dtx, 0);
   return postMessage(msg);
}

OsStatus MprEncode::setMaxPacketTime(unsigned int maxPacketTime)
{
   MpFlowGraphMsg msg(SET_MAX_PACKET_TIME, this, NULL, NULL, maxPacketTime, 0);
   return postMessage(msg);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

OsStatus MprEncode::deselectCodecs(void)
{
   MpFlowGraphMsg msg(DESELECT_CODECS, this, NULL, NULL, 0, 0);

   return postMessage(msg);
}

OsStatus MprEncode::selectCodecs(SdpCodec* pPrimary, SdpCodec* pDtmf)
{
   OsStatus res = OS_SUCCESS;
   MpFlowGraphMsg msg(SELECT_CODECS, this, NULL, NULL, 2, 0);

   int numNewCodecs = 2;
   SdpCodec** newCodecs = new SdpCodec*[numNewCodecs];

   newCodecs[0] = (NULL == pPrimary) ? NULL : new SdpCodec(*pPrimary);
   newCodecs[1] = (NULL == pDtmf) ? NULL : new SdpCodec(*pDtmf);
   msg.setPtr1(newCodecs);
   msg.setInt1(numNewCodecs);
   res = postMessage(msg);

   return res;
}

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
         mpResampleBuf = new MpAudioSample[mResampleBufLen];
      }

      mMaxPacketSamples = mMaxPacketTime*codecSamplesPerSec/1000;

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

// Handle messages for this resource.
UtlBoolean MprEncode::handleMessage(MpFlowGraphMsg& rMsg)
{
   if (rMsg.getMsg() == SELECT_CODECS)
   {
      int pNewCodecArrSz = rMsg.getInt1();
      SdpCodec** pNewCodecArr = (SdpCodec**)rMsg.getPtr1();
      // Note: handleSelectCodecs now does not free the data given to it.
      handleSelectCodecs(pNewCodecArrSz, pNewCodecArr);

      // Free the contents of the array we were sent, if not null.
      int i;
      for (i = 0; i < pNewCodecArrSz; i++)
      {
         if(NULL != pNewCodecArr[i])
         {
            delete pNewCodecArr[i];
         }
      }
      // free the array we were sent
      delete[] pNewCodecArr;
      return TRUE;
   } 
   else if (rMsg.getMsg() == DESELECT_CODECS) {
      handleDeselectCodecs();
      return TRUE;
   } else if (rMsg.getMsg() == START_TONE) {
      handleStartTone(rMsg.getInt1());
      return TRUE;
   } else if (rMsg.getMsg() == STOP_TONE) {
      handleStopTone();
      return TRUE;
   } else if (rMsg.getMsg() == ENABLE_DTX) {
      handleEnableDTX(rMsg.getInt1());
      return TRUE;
   } else if (rMsg.getMsg() == SET_MAX_PACKET_TIME) {
      handleSetMaxPacketTime(rMsg.getInt1());
      return TRUE;
   }
   else
      return MpAudioResource::handleMessage(rMsg);
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
   unsigned int codecFrameSamples;

   if (mpPrimaryCodec == NULL)
      return;

   if (!in.isValid())
      return;

   // Do resampling if needed.
   if (mNeedResample)
   {
      uint32_t samplesConsumed;
      mpResampler->resample(0,
                            in->getSamplesPtr(), in->getSamplesNumber(), samplesConsumed,
                            mpResampleBuf, mResampleBufLen, numSamplesIn);
      assert(samplesConsumed == in->getSamplesNumber());
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
                                   isPacketReady, isPacketSilent);
      mPayloadBytesUsed += bytesAdded;
      assert (mPacket1PayloadBytes >= mPayloadBytesUsed);

      // In case the encoder does silence suppression (e.g. G.729 Annex B)
      mMarkNext1 = mMarkNext1 || isPacketSilent;

      mSamplesPacked += numSamplesOut;
      pSamplesIn += numSamplesOut;
      numSamplesIn -= numSamplesOut;
      mCurrentTimestamp += numSamplesOut;

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
         && (isPacketReady || mSamplesPacked+codecFrameSamples > mMaxPacketSamples))
      {
         if (mActiveAudio1)
         {
            mConsecutiveInactive1 = 0;
         } else {
            mConsecutiveInactive1++;
         }
         if ((mConsecutiveInactive1 < HANGOVER_PACKETS) ||
             (mConsecutiveUnsentFrames1 >= RTP_KEEP_ALIVE_FRAME_INTERVAL))
         {
            mpToNet->writeRtp(mpPrimaryCodec->getPayloadType(),
                              mMarkNext1,
                              mpPacket1Payload,
                              mPayloadBytesUsed,
                              mStartTimestamp1,
                              NULL);
            mMarkNext1 = FALSE;
            mConsecutiveUnsentFrames1 = 0;
         } else {
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

         numSampleTimes = (mCurrentTimestamp + samplesPerFrame) - mStartTimestamp2;
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

