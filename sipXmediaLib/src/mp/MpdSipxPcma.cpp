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


// APPLICATION INCLUDES
#include "mp/MpdSipxPcma.h"
#include "mp/MpSipxDecoders.h"

const MpCodecInfo MpdSipxPcma::smCodecInfo(
         SdpCodec::SDP_CODEC_PCMA, "SIPfoundry 1.0",
         8000, 8, 1, 160, 64000, 1280, 1280, 1280, 160, 3);

MpdSipxPcma::MpdSipxPcma(int payloadType)
: MpDecoderBase(payloadType, &smCodecInfo)
{
}

MpdSipxPcma::~MpdSipxPcma()
{
   freeDecode();
}

OsStatus MpdSipxPcma::initDecode()
{
   return OS_SUCCESS;
}

OsStatus MpdSipxPcma::freeDecode()
{
   return OS_SUCCESS;
}

int MpdSipxPcma::decode(const MpRtpBufPtr &pPacket,
                        unsigned decodedBufferLength,
                        MpAudioSample *samplesBuffer) 
{
   // Assert that available buffer size is enough for the packet.
   if (pPacket->getPayloadSize() > decodedBufferLength)
   {
      osPrintf("MpdSipxPcma::decode: Jitter buffer overloaded. Glitch!\n");
   }

   if (decodedBufferLength == 0)
      return 0;

   int samples = min(pPacket->getPayloadSize(), decodedBufferLength);
   G711A_Decoder(samples,
                 (const uint8_t*)pPacket->getDataPtr(),
                 samplesBuffer);
   return samples;
}
