//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_ILBC // [

// SYSTEM INCLUDES
#include <limits.h>

// APPLICATION INCLUDES
#include "mp/MpdSipxILBC.h"
extern "C" {
#include <iLBC_define.h>
#include <iLBC_decode.h>
}

const MpCodecInfo MpdSipxILBC::smCodecInfo(
    SdpCodec::SDP_CODEC_ILBC,   // codecType
    "iLBC",                     // codecVersion
    false,                      // usesNetEq
    8000,                       // samplingRate
    8,                          // numBitsPerSample (not used)
    1,                          // numChannels
    240,                        // interleaveBlockSize
    13334,                      // bitRate
    NO_OF_BYTES_30MS * 8,       // minPacketBits
    NO_OF_BYTES_30MS * 8,       // avgPacketBits
    NO_OF_BYTES_30MS * 8,       // maxPacketBits
    240,                        // numSamplesPerFrame
    6);                         // preCodecJitterBufferSize (should be adjusted)



MpdSipxILBC::MpdSipxILBC(int payloadType)
: MpDecoderBase(payloadType, &smCodecInfo)
, mpState(NULL)
{
}

MpdSipxILBC::~MpdSipxILBC()
{
   freeDecode();
}

OsStatus MpdSipxILBC::initDecode()
{
   if (mpState == NULL) 
   {
      mpState = new iLBC_Dec_Inst_t();
      memset(mpState, 0, sizeof(*mpState));
      ::initDecode(mpState, 30, 1);
   }
   return OS_SUCCESS;
}

OsStatus MpdSipxILBC::freeDecode(void)
{
   delete mpState;
   mpState = NULL;
   return OS_SUCCESS;
}

int MpdSipxILBC::decode(const MpRtpBufPtr &pPacket,
                        unsigned decodedBufferLength,
                        MpAudioSample *samplesBuffer)
{
   // Check if available buffer size is enough for the packet.
   if (decodedBufferLength < 240)
   {
      osPrintf("MpdSipxILBC::decode: Jitter buffer overloaded. Glitch!\n");
      return 0;
   }

   // Decode incoming packet to temp buffer. If no packet - do PLC.
   float buffer[240];
   if (pPacket.isValid())
   {
      if (NO_OF_BYTES_30MS != pPacket->getPayloadSize())
      {
         osPrintf("MpdSipxILBC::decode: Payload size: %d!\n", pPacket->getPayloadSize());
         return 0;
      }

      // Packet data available. Decode it.
      iLBC_decode(buffer, (unsigned char*)pPacket->getDataPtr(), mpState, 1);
   }
   else
   {
      // Packet data is not available. Do PLC.
      iLBC_decode(buffer, NULL, mpState, 0);
   }
   
   for (int i = 0; i < 240; ++i)
   {
      float tmp = buffer[i];
      if (tmp > SHRT_MAX)
         tmp = SHRT_MAX;
      if (tmp < SHRT_MIN)
         tmp = SHRT_MIN;

      samplesBuffer[i] = MpAudioSample(tmp + 0.5f);
   }

   return 240;
}

#endif // HAVE_ILBC ]
