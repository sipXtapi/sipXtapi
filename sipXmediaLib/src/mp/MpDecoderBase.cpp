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

#include <assert.h>
#include "mp/MpDecoderBase.h"

/* ============================ CREATORS ================================== */

MpDecoderBase::MpDecoderBase(int payloadType,
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

MpDecoderBase::~MpDecoderBase()
{
   if (mInitialized)
   {
      freeDecode();
   }
}

OsStatus MpDecoderBase::initDecode(const char *fmtp)
{
   MppCodecFmtpInfoV1_1 fmtpInfo;

   plgHandle = mCallInfo.mPlgInit(fmtp, CODEC_DECODER, &fmtpInfo);

   if (plgHandle == NULL)
   {
      mInitialized = FALSE;
      return OS_INVALID_STATE;
   }

   mInitialized = TRUE;

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
   if (!mInitialized)
   {
      return OS_INVALID_STATE;
   }

   mCallInfo.mPlgFree(plgHandle, CODEC_DECODER);
   mInitialized = FALSE;

   return OS_SUCCESS;
}

/* ============================ MANIPULATORS ============================== */

int MpDecoderBase::decode(const MpRtpBufPtr &pPacket,
                          unsigned decodedBufferLength,
                          MpAudioSample *samplesBuffer)
{
   unsigned decodedSize = 0;
   int res = RPLG_FAILED;

   if (!mInitialized)
   {
      return 0;
   }

   if (pPacket.isValid() || mCodecInfo.haveInternalPLC()) 
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

   if (res != RPLG_SUCCESS)
   {
      //Error during decoding
      return 0;
   }
   return decodedSize;
}

/* ============================ ACCESSORS ================================= */

const MpCodecInfo* MpDecoderBase::getInfo() const
{
   if (mInitialized)
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

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
