//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpePtAVT.h"

const MpCodecInfo MpePtAVT::smCodecInfo(
         SdpCodec::SDP_CODEC_TONES, "Pingtel_1.0", false,
         8000, 0, 1, 0, 6400, 128, 128, 128, 160, TRUE);

MpePtAVT::MpePtAVT(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo)
{
}

MpePtAVT::~MpePtAVT()
{
   freeEncode();
}

OsStatus MpePtAVT::initEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpePtAVT::freeEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpePtAVT::encode(const short* pAudioSamples,
                             const int numSamples,
                             int& rSamplesConsumed,
                             unsigned char* pCodeBuf,
                             const int bytesLeft,
                             int& rSizeInBytes,
                             UtlBoolean& sendNow,
                             MpBufSpeech& rAudioCategory)
{
   assert(FALSE);
   rSizeInBytes = 0;
   rSamplesConsumed = numSamples;
   sendNow = FALSE;
   return OS_INVALID;
}
