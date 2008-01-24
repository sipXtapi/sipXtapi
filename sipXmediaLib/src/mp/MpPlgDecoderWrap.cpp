//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#include <assert.h>
#include <utl/UtlInit.h> 
#include <mp/MpPlgDecoderWrap.h>

MpPlgDecoderWrapper::MpPlgDecoderWrapper(int payloadType,
                                         const MpCodecCallInfoV1& callInfo,
                                         const MppCodecInfoV1_1& codecInfo,
                                         const char* defaultFmtp)
: MpDecoderBase(payloadType)
, mCodecInfo(codecInfo) // This fills only first part of codec information.
, mCallInfo(callInfo)
, mInitialized(FALSE)
, mDefaultFmtp(defaultFmtp)
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
   MppCodecFmtpInfoV1_1 fmtpInfo;

   plgHandle = mCallInfo.mPlgInit(fmt, CODEC_DECODER, &fmtpInfo);

   if (plgHandle != NULL)
   {
      mInitialized = TRUE;

      // Fill in fmtp part of codec information
      mCodecInfo = MpCodecInfo((MppCodecInfoV1_1&)mCodecInfo, fmtpInfo);
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


OsStatus MpPlgDecoderWrapper::initDecode(const char *fmtp)
{
   initializeWrapper(fmtp);

   if (!mInitialized) 
      return OS_INVALID_STATE;

   return OS_SUCCESS;
}

OsStatus MpPlgDecoderWrapper::initDecode()
{
   return initDecode(mDefaultFmtp);
}

OsStatus MpPlgDecoderWrapper::freeDecode()
{
   if (!mInitialized)
      return OS_INVALID_STATE;

   mCallInfo.mPlgFree(plgHandle, CODEC_DECODER);
   mInitialized = FALSE;

   return OS_SUCCESS;
}

int MpPlgDecoderWrapper::decode(const MpRtpBufPtr &pPacket,
                   unsigned decodedBufferLength,
                   MpAudioSample *samplesBuffer)
{
   unsigned decodedSize = 0;
   int res = RPLG_FAILED;

   if (!mInitialized)
   {
      return 0;
   }

   if (pPacket.isValid() || codecSupportPLC) 
   {   
      res = mCallInfo.mPlgDecode(plgHandle, 
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

   res = mCallInfo.mPlgSignaling(plgHandle,&wEvent, &wDuration,
                              &wStartStatus, &wStopStatus);

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
