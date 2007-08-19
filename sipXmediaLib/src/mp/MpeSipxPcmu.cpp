//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpeSipxPcmu.h"
#include "mp/MpSipxDecoders.h"

const MpCodecInfo MpeSipxPcmu::smCodecInfo(
         SdpCodec::SDP_CODEC_PCMU, "SIPfoundry 1.0", true,
         8000, 8, 1, 160, 64000, 1280, 1280, 1280, 160);

MpeSipxPcmu::MpeSipxPcmu(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo)
{

}

MpeSipxPcmu::~MpeSipxPcmu()
{
   freeEncode();
}

OsStatus MpeSipxPcmu::initEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpeSipxPcmu::freeEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpeSipxPcmu::encode(const MpAudioSample* pAudioSamples,
                             const int numSamples,
                             int& rSamplesConsumed,
                             unsigned char* pCodeBuf,
                             const int bytesLeft,
                             int& rSizeInBytes,
                             UtlBoolean& sendNow,
                             MpAudioBuf::SpeechType& rAudioCategory)
{
   G711U_Encoder(numSamples, pAudioSamples, (uint8_t*)pCodeBuf);
   rSizeInBytes = numSamples;
   rAudioCategory = MpAudioBuf::MP_SPEECH_UNKNOWN;
   sendNow = FALSE;
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}
