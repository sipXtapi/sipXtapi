//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#include <assert.h>
#include <utl/UtlInit.h> 
#include <mp/MpPlgDecoderWrap.h>

MpPlgDecoderWrapper::MpPlgDecoderWrapper(int payloadType, const MpCodecCallInfoV1& plgci, const char* permanentDefaultMode)
: MpDecoderBase(payloadType)
, mplgci(plgci)
, mInitialized(FALSE)
, mDefParamString(permanentDefaultMode)
, codecSupportPLC(FALSE)
{

}

const MpCodecInfo* MpPlgDecoderWrapper::getInfo(void) const
{
   if (mInitialized)
   {
      return &mCodecInfo;
   }

   return NULL;
}

UtlBoolean MpPlgDecoderWrapper::initializeWrapper(const char* fmt)
{
   plgCodecInfoV1 plgInfo;

   plgHandle = mplgci.mPlgInit(fmt, CODEC_DECODER, &plgInfo);

   if ((plgHandle != NULL) && (plgInfo.cbSize == sizeof(struct plgCodecInfoV1)))
   {
      mInitialized = TRUE;

      codecSupportPLC = plgInfo.codecSupportPLC;

      // Fill in codec information
      mCodecInfo.mCodecVersion = plgInfo.codecVersion;
      mCodecInfo.mSamplingRate = plgInfo.samplingRate;
      mCodecInfo.mNumBitsPerSample = plgInfo.numSamplesPerFrame;
      mCodecInfo.mNumSamplesPerFrame = plgInfo.numSamplesPerFrame;
      mCodecInfo.mNumChannels = plgInfo.numChannels;
      mCodecInfo.mInterleaveBlockSize = plgInfo.interleaveBlockSize;
      mCodecInfo.mBitRate = plgInfo.bitRate;
      mCodecInfo.mMinPacketBits = plgInfo.minPacketBits;
      mCodecInfo.mAvgPacketBits = plgInfo.avgPacketBits;
      mCodecInfo.mMaxPacketBits = plgInfo.maxPacketBits;
      mCodecInfo.mPreCodecJitterBufferSize = plgInfo.preCodecJitterBufferSize;
      mCodecInfo.mIsSignalingCodec = plgInfo.signalingCodec && (mplgci.mPlgSignaling != NULL);
      mCodecInfo.mDoesVadCng = FALSE;
   }
   else
   {
      mInitialized = FALSE;
   }

   return mInitialized;
}


MpPlgDecoderWrapper::~MpPlgDecoderWrapper()
{
   if (mInitialized)
   {
      freeDecode();
   }
}


OsStatus MpPlgDecoderWrapper::initDecode(const char *fmt)
{
   initializeWrapper(fmt);

   if (!mInitialized) 
      return OS_INVALID_STATE;

   return OS_SUCCESS;
}

OsStatus MpPlgDecoderWrapper::initDecode()
{
   return initDecode(mDefParamString);
}

OsStatus MpPlgDecoderWrapper::freeDecode()
{
   if (!mInitialized)
      return OS_INVALID_STATE;

   mplgci.mPlgFree(plgHandle, CODEC_DECODER);
   mInitialized = FALSE;

   return OS_SUCCESS;
}

int MpPlgDecoderWrapper::decode(const MpRtpBufPtr &pPacket,
                   unsigned decodedBufferLength,
                   MpAudioSample *samplesBuffer)
{
   unsigned decodedSize = 0;
   int res;

   if (!mInitialized)
   {
      return 0;
   }

   if (pPacket.isValid() || codecSupportPLC) 
   {   
      res = mplgci.mPlgDecode(plgHandle, 
                              (pPacket.isValid()) ? pPacket->getDataPtr() : NULL, 
                              pPacket->getPayloadSize(), 
                              samplesBuffer, 
                              decodedBufferLength, 
                              &decodedSize,
                              &pPacket->getRtpHeader());
   }
   else
   {
      //TODO: Add PLC for codec that doesn't support itself
   }

   if (res != RPLG_SUCCESS) {
      //Error during decoding
      return 0;
   }
   return decodedSize;
}


OsStatus MpPlgDecoderWrapper::getSignalingData(uint8_t &event,
                                               UtlBoolean &isStarted,
                                               UtlBoolean &isStopped,
                                               uint16_t &duration)
{
   if (!mInitialized)
      return OS_INVALID_STATE;

   if (!mCodecInfo.isSignalingCodec()) 
      return OS_NOT_SUPPORTED;

   uint32_t wEvent, wStartStatus, wStopStatus, wDuration;
   int res;

   res = mplgci.mPlgSignaling(plgHandle, SIGNALING_DEFAULT_TYPE,
                              &wEvent, &wDuration, &wStartStatus, &wStopStatus);

   switch (res)
   {
   case RPLG_SUCCESS:
      event = (uint8_t)wEvent;
      isStarted = (UtlBoolean)wStartStatus;
      isStopped = (UtlBoolean)wStopStatus;
      duration = (uint16_t)wDuration;
      return OS_SUCCESS;
   case RPLG_NO_MORE_DATA:
      return OS_NO_MORE_DATA;
   default:
      return OS_FAILED;
   }
}
