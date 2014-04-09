//  
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <mp/MpDecoderBase.h>
#ifdef TEST_PRINT
#   include <os/OsSysLog.h>
#endif

/* ============================ CREATORS ================================== */

MpDecoderBase::MpDecoderBase(int payloadType,
                             const MpCodecCallInfoV1& callInfo,
                             const MppCodecInfoV1_1& codecInfo,
                             const char* defaultFmtp)
: mPayloadType(payloadType)
, mCodecInfo(codecInfo) // This fills only first part (non-fmtp) of codec information.
, mCallInfo(callInfo)
, plgHandle(NULL)
, mDefaultFmtp(defaultFmtp)
{
}

MpDecoderBase::~MpDecoderBase()
{
   freeDecode();
}

OsStatus MpDecoderBase::initDecode(const char *fmtp)
{
   MppCodecFmtpInfoV1_2 fmtpInfo;

   freeDecode();
   plgHandle = mCallInfo.mPlgInit(fmtp, CODEC_DECODER, &fmtpInfo);

   if (plgHandle == NULL)
   {
      return OS_INVALID_STATE;
   }

   // Fill in remaining (fmtp) part of codec information
   mCodecInfo = MpCodecInfo((MppCodecInfoV1_1&)mCodecInfo, fmtpInfo);

   return OS_SUCCESS;
}

OsStatus MpDecoderBase::initDecode()
{
   return initDecode(mDefaultFmtp);
}

OsStatus MpDecoderBase::freeDecode()
{
   OsStatus status = OS_INVALID_STATE;
   if (isInitialized())
   {
      mCallInfo.mPlgFree(plgHandle, CODEC_DECODER);
      plgHandle = NULL;
      status = OS_SUCCESS;
   }

   return(status);
}

/* ============================ MANIPULATORS ============================== */

int MpDecoderBase::decode(const MpRtpBufPtr &pPacket,
                          unsigned decodedBufferLength,
                          MpAudioSample *samplesBuffer)
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpDecoderBase::decode begin codec: %s",
      mCodecInfo.getCodecName());
#endif

   unsigned decodedSize = 0;
   int res = RPLG_FAILED;

   if (!isInitialized())
   {
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpDecoderBase::decode end not intitialized codec: %s",
      mCodecInfo.getCodecName());
#endif
      return 0;
   }

   if (pPacket.isValid())
   {   
      res = mCallInfo.mPlgDecode(plgHandle, 
                                 pPacket->getDataPtr(),
                                 pPacket->getPayloadSize(), 
                                 samplesBuffer, 
                                 decodedBufferLength, 
                                 &decodedSize,
                                 &pPacket->getRtpHeader());
   }
   else if (mCodecInfo.haveInternalPLC())
   {   
      res = mCallInfo.mPlgDecode(plgHandle, 
         NULL,
         0, 
         samplesBuffer, 
         decodedBufferLength, 
         &decodedSize,
         NULL);
   }
   else
   {
      assert(!"Codec does not have internal PLC. You MUST use external PLC!");
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MpDecoderBase::decode end no PLC  codec: %s",
         mCodecInfo.getCodecName());
#endif
      return 0;
   }

   if (res != RPLG_SUCCESS)
   {
      //Error during decoding
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MpDecoderBase::decode end error codec: %s",
         mCodecInfo.getCodecName());
#endif
      return 0;
   }
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpDecoderBase::decode end codec: %s",
      mCodecInfo.getCodecName());
#endif
   return decodedSize;
}

/* ============================ ACCESSORS ================================= */

const MpCodecInfo* MpDecoderBase::getInfo() const
{
   if (isInitialized())
   {
      return &mCodecInfo;
   }

   return NULL;
}

int MpDecoderBase::getPayloadType(void)
{
   return(mPayloadType);
}

OsStatus MpDecoderBase::getSignalingData(uint8_t &event,
                                         UtlBoolean &isStarted,
                                         UtlBoolean &isStopped,
                                         uint16_t &duration)
{
   if (!isInitialized())
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

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

