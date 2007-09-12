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
#include <mp/MpPlgDecoderWrap.h>

MpPlgDecoderWrapper::MpPlgDecoderWrapper(int payloadType, const MpCodecCallInfoV1& plgci, const char* permanentDefaultMode)
: MpDecoderBase(payloadType)
, mplgci(plgci)
, mInitialized(FALSE)
, mSDPNumAssigned(FALSE)
, mDefParamString(permanentDefaultMode)
, codecSupportPLC(FALSE)
{

}

const MpCodecInfo* MpPlgDecoderWrapper::getInfo(void) const
{
   if (mInitialized)
   {
      assert(mSDPNumAssigned == TRUE);
      return &mpTmpInfo;
   }
   return NULL;
}

OsStatus MpPlgDecoderWrapper::setAssignedSDPNum(SdpCodec::SdpCodecTypes sdpNum)
{
   mSDPNumAssigned = TRUE;
   mpTmpInfo.mCodecType = sdpNum;
   return OS_SUCCESS;
}


UtlBoolean MpPlgDecoderWrapper::initializeWrapper(const char* fmt)
{
   //Currently signed 16bit LE format, one channel
   struct plgCodecInfoV1 plgInfo;
   plgHandle = mplgci.mPlgInit(fmt, PREPARE_DECODER, &plgInfo);

   if ((plgHandle != NULL) && (plgInfo.cbSize == sizeof(struct plgCodecInfoV1))) {
      mInitialized = TRUE;

      //Filling codec information
      //mpTmpInfo.mCodecType = mpTmpInfo.mCodecType = SdpCodec::SDP_CODEC_UNKNOWN; //(SdpCodec::SdpCodecTypes)plgInfo.codecSDPType;
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

      codecSupportPLC = plgInfo.codecSupportPLC;
   } else {
      mInitialized = FALSE;
   }
   return mInitialized;
}


MpPlgDecoderWrapper::~MpPlgDecoderWrapper()
{
   if (mInitialized) {
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

   mplgci.mPlgFree(plgHandle);
   mInitialized = FALSE;

   return OS_SUCCESS;
}

int MpPlgDecoderWrapper::decode(const MpRtpBufPtr &pPacket,
                   unsigned decodedBufferLength,
                   MpAudioSample *samplesBuffer)
{
   unsigned decodedSize = 0;
   int res;

   if (!mInitialized) {
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
      //TODO: Add PLC for codec that dosen't support itself
   }

   if (res != RPLG_SUCCESS) {
      //Error during decoding
      return 0;
   }
   return decodedSize;
}


