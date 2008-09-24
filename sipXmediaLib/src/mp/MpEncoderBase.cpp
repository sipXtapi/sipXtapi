//  
// Copyright (C) 2007-2008 SIPez LLC. 
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


#include "mp/MpEncoderBase.h"

/* ============================ CREATORS ================================== */

MpEncoderBase::MpEncoderBase(int payloadType,
                             const MpCodecCallInfoV1& callInfo,
                             const MppCodecInfoV1_1& codecInfo,
                             const char* defaultFmtp)
: mPayloadType(payloadType)
, mCodecInfo(codecInfo) // This fills only first part (non-fmtp) of codec information.
, mCallInfo(callInfo)
, mInitialized(FALSE)
, mDefaultFmtp(defaultFmtp)
{
}
    
MpEncoderBase::~MpEncoderBase()
{
   if (mInitialized)
   {
      freeEncode();
   }
}

OsStatus MpEncoderBase::initEncode(const char* fmt)
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

   if (!mInitialized) 
      return OS_FAILED;

   return OS_SUCCESS;
}

OsStatus MpEncoderBase::initEncode()
{
   return initEncode(mDefaultFmtp);
}

OsStatus MpEncoderBase::freeEncode()
{
   if (!mInitialized)
      return OS_INVALID_STATE;

   mCallInfo.mPlgFree(plgHandle, CODEC_ENCODER);
   mInitialized = FALSE;
   return OS_SUCCESS;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpEncoderBase::encode(const MpAudioSample* pAudioSamples,
                               const int numSamples,
                               int& rSamplesConsumed,
                               unsigned char* pCodeBuf,
                               const int bytesLeft,
                               int& rSizeInBytes,
                               UtlBoolean& isPacketReady,
                               UtlBoolean& isPacketSilent) 
{
   int res;
   unsigned usendNow;

   if (!mInitialized) {
      return OS_INVALID_STATE;
   }

   res = mCallInfo.mPlgEncode(plgHandle, pAudioSamples, numSamples,
      &rSamplesConsumed, pCodeBuf, bytesLeft,
      &rSizeInBytes, &usendNow);   
   if (res) {
      //Error during encoding
      return OS_FAILED;
   }
   isPacketReady = (usendNow) ? TRUE : FALSE;

   // Always set it to FALSE for now, codecs API should be updated to support
   // this.
   isPacketSilent = FALSE;

   return OS_SUCCESS;
}


/* ============================ ACCESSORS ================================= */

const MpCodecInfo* MpEncoderBase::getInfo() const
{
   if (mInitialized)
   {
      return &mCodecInfo;
   }
   return NULL;
}

int MpEncoderBase::getPayloadType()
{
   return(mPayloadType);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
