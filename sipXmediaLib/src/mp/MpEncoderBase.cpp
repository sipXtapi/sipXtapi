//  
// Copyright (C) 2007-2018 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include <mp/MpEncoderBase.h>
//#define TEST_PRINT
#ifdef TEST_PRINT
#   include <os/OsSysLog.h>
#endif

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
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpEncoderBase::initEncode payload type: %d fmtp: %s",
      mPayloadType,
      fmt);
#endif

   MppCodecFmtpInfoV1_2 fmtpInfo;
   //fmtpInfo.cbSize = sizeof(MppCodecFmtpInfoV1_2);
   fmtpInfo.mSetMarker = FALSE;

   plgHandle = mCallInfo.mPlgInit(fmt, CODEC_ENCODER, &fmtpInfo);

   if (plgHandle != NULL) {
      mInitialized = TRUE;

      // Fill in fmtp part of codec information
      mCodecInfo = MpCodecInfo((MppCodecInfoV1_1&)mCodecInfo, fmtpInfo);
   } else {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MpEncoderBase::initEncode plgHandle NULL");
#endif
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
                               UtlBoolean& isPacketSilent,
                               UtlBoolean& shouldSetMarker) 
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpEncoderBase::encode begin codec: %s",
      mCodecInfo.getCodecName());
#endif

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
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MpEncoderBase::encode end failed codec: %s",
         mCodecInfo.getCodecName());
#endif
      return OS_FAILED;
   }
   isPacketReady = (usendNow) ? TRUE : FALSE;

   // Always set it to FALSE for now, codecs API should be updated to support
   // this.
   isPacketSilent = FALSE;

   // For now this is fairly static.  At some point this may need to be set on 
   // frame by frame basis.
   shouldSetMarker = mCodecInfo.shouldSetMarker();

#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpEncoderBase::encode end codec: %s samples consumed: %d %s",
      mCodecInfo.getCodecName(), rSamplesConsumed, isPacketReady ? "send packet" : "packet not ready");
#endif
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
