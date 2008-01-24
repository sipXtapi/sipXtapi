//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: 

#include <assert.h>
#include <utl/UtlInit.h> 
#include <mp/MpPlgEncoderWrap.h>
#include <sdp/SdpCodec.h>

MpPlgEncoderWrapper::MpPlgEncoderWrapper(int payloadType,
                                         const MpCodecCallInfoV1& callInfo,
                                         const MppCodecInfoV1_1& codecInfo,
                                         const char* defaultFmtp)
: MpEncoderBase(payloadType)
, mCodecInfo(codecInfo) // This fills only first part of codec information.
, mCallInfo(callInfo)
, mInitialized(FALSE)
, mDefaultFmtp(defaultFmtp)
{

}

const MpCodecInfo* MpPlgEncoderWrapper::getInfo(void) const
{
   if (mInitialized)
   {
      return &mCodecInfo;
   }
   return NULL;
}

UtlBoolean MpPlgEncoderWrapper::initializeWrapper(const char* fmt)
{
   MppCodecFmtpInfoV1_1 fmtpInfo;

   plgHandle = mCallInfo.mPlgInit(fmt, CODEC_ENCODER, &fmtpInfo);

   if (plgHandle != NULL) {
      mInitialized = TRUE;

      // Fill in fmtp part of codec information
      mCodecInfo = MpCodecInfo((MppCodecInfoV1_1&)mCodecInfo, fmtpInfo);
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
   return initEncode(mDefaultFmtp);
}

OsStatus MpPlgEncoderWrapper::freeEncode()
{
   if (!mInitialized)
      return OS_INVALID_STATE;
      
   mCallInfo.mPlgFree(plgHandle, CODEC_ENCODER);
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

   res = mCallInfo.mPlgEncode(plgHandle, pAudioSamples, numSamples, &rSamplesConsumed, pCodeBuf, bytesLeft, &rSizeInBytes, &usendNow);   
   if (res) {
      //Error during encoding
      return OS_FAILED;
   }
   sendNow = (usendNow)  ? TRUE : FALSE;
   rAudioCategory = MpAudioBuf::MP_SPEECH_UNKNOWN;
   return OS_SUCCESS;
}


