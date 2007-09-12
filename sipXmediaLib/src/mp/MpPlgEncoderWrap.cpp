//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: 

#include <assert.h>
#include <utl/UtlInit.h> 
#include <mp/MpPlgEncoderWrap.h>
#include <sdp/SdpCodec.h>

MpPlgEncoderWrapper::MpPlgEncoderWrapper(int payloadType, const MpCodecCallInfoV1& plgci, const char* permanentDefaultMode)
: MpEncoderBase(payloadType, &mpTmpInfo)
, mplgci(plgci)
, mInitialized(FALSE)
, mSDPNumAssigned(FALSE)
, mDefParamString(permanentDefaultMode)
{

}

const MpCodecInfo* MpPlgEncoderWrapper::getInfo(void) const
{
   if (mInitialized)
   {
         assert(mSDPNumAssigned == TRUE);
         return &mpTmpInfo;
   }
   return NULL;
}

OsStatus MpPlgEncoderWrapper::setAssignedSDPNum(SdpCodec::SdpCodecTypes sdpNum)
{
   mSDPNumAssigned = TRUE;
   mpTmpInfo.mCodecType = sdpNum;
   return OS_SUCCESS;
}

UtlBoolean MpPlgEncoderWrapper::initializeWrapper(const char* fmt)
{
   //Currently assuming 8000 hz, and signed 16bit LE format, one channel

   struct plgCodecInfoV1 plgInfo;
   plgHandle = mplgci.mPlgInit(fmt, PREPARE_ENCODER, &plgInfo);

   if ((plgHandle != NULL) && (plgInfo.cbSize == sizeof(struct plgCodecInfoV1))) {
      mInitialized = TRUE;

      //Filling codec information
      //mpTmpInfo.mCodecType = (SdpCodec::SdpCodecTypes)plgInfo.codecSDPType;
      //mpTmpInfo.mCodecType = SdpCodec::SDP_CODEC_UNKNOWN;
      mpTmpInfo.mCodecVersion = plgInfo.codecVersion;
      mpTmpInfo.mSamplingRate = plgInfo.samplingRate;
      mpTmpInfo.mNumBitsPerSample = plgInfo.numSamplesPerFrame;
      mpTmpInfo.mNumSamplesPerFrame = plgInfo.numSamplesPerFrame;
      mpTmpInfo.mNumChannels = plgInfo.numChannels;
      mpTmpInfo.mInterleaveBlockSize = plgInfo.interleaveBlockSize;
      mpTmpInfo.mBitRate = plgInfo.bitRate;
      mpTmpInfo.mMinPacketBits = plgInfo.minPacketBits;
      mpTmpInfo.mAvgPacketBits = plgInfo.avgPacketBits;
      mpTmpInfo.mMaxPacketBits = plgInfo.maxPacketBits;
      mpTmpInfo.mPreCodecJitterBufferSize = plgInfo.preCodecJitterBufferSize;
      mpTmpInfo.mIsSignalingCodec = FALSE;
      mpTmpInfo.mDoesVadCng = FALSE;
   } else {
      mInitialized = FALSE;
   }
   return mInitialized;
}


MpPlgEncoderWrapper::~MpPlgEncoderWrapper()
{
   if (mInitialized) {
      freeEncode();
   }
}


OsStatus MpPlgEncoderWrapper::initEncode(const char *fmt)
{
   initializeWrapper(fmt);

   if (!mInitialized) 
      return OS_INVALID_STATE;

   return OS_SUCCESS;
}

OsStatus MpPlgEncoderWrapper::initEncode()
{
   return initEncode(mDefParamString);
}

OsStatus MpPlgEncoderWrapper::freeEncode()
{
   if (!mInitialized)
      return OS_INVALID_STATE;
      
   mplgci.mPlgFree(plgHandle);
   mInitialized = FALSE;
   return OS_SUCCESS;
}

OsStatus MpPlgEncoderWrapper::encode(const MpAudioSample* pAudioSamples,
                const int numSamples,
                int& rSamplesConsumed,
                unsigned char* pCodeBuf,
                const int bytesLeft,
                int& rSizeInBytes,
                UtlBoolean& sendNow,
                MpAudioBuf::SpeechType& rAudioCategory) 
{
   int res;
   unsigned usendNow;

   if (!mInitialized) {
      return OS_INVALID_STATE;
   }

   res = mplgci.mPlgEncode(plgHandle, pAudioSamples, numSamples, &rSamplesConsumed, pCodeBuf, bytesLeft, &rSizeInBytes, &usendNow);   
   if (res) {
      //Error during encoding
      return OS_FAILED;
   }
   sendNow = (usendNow)  ? TRUE : FALSE;
   rAudioCategory = MpAudioBuf::MP_SPEECH_UNKNOWN;
   return OS_SUCCESS;
}


