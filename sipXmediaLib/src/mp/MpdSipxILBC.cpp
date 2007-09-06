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
// APPLICATION INCLUDES
#include "os/OsDefs.h" // for min macro
#include "mp/MpdSipxILBC.h"
extern "C" {
#include <iLBC_define.h>
#include <iLBC_decode.h>
}

const MpCodecInfo MpdSipxILBC::smCodecInfo20ms(
    SdpCodec::SDP_CODEC_ILBC,   // codecType
    "iLBC",                     // codecVersion
    8000,                       // samplingRate
    8,                          // numBitsPerSample (not used)
    1,                          // numChannels
    160,                        // interleaveBlockSize
    15200,                      // bitRate
    NO_OF_BYTES_20MS * 8,       // minPacketBits
    NO_OF_BYTES_20MS * 8,       // avgPacketBits
    NO_OF_BYTES_20MS * 8,       // maxPacketBits
    160,                        // numSamplesPerFrame
    6);                         // preCodecJitterBufferSize (should be adjusted)

const MpCodecInfo MpdSipxILBC::smCodecInfo30ms(
    SdpCodec::SDP_CODEC_ILBC,   // codecType
    "iLBC",                     // codecVersion
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



MpdSipxILBC::MpdSipxILBC(int payloadType, CodecMode mode)
: MpDecoderBase(payloadType)
, mMode(mode)
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
      // Allocate codec state structure
      mpState = new iLBC_Dec_Inst_t();

      // Make valgrind happy.
      memset(mpState, 0, sizeof(*mpState));

      // Initialize codec state.
      // Note, that our CodecMode type maps directly to expected mode number.
      ::initDecode(mpState, mMode, 1);
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
   if (decodedBufferLength < (unsigned)mpState->blockl)
   {
      osPrintf("MpdSipxILBC::decode: Jitter buffer overloaded. Glitch!\n");
   }

   // Allocate temporary buffer for decoded audio.
   // We allocate memory for 240 samples (30ms frame), because 20ms frame
   // (160 samples) will also fit.
   float buffer[240];

   // Decode incoming packet to temp buffer. If no packet - do PLC.
   if (pPacket.isValid())
   {
      if (pPacket->getPayloadSize() != mpState->no_of_bytes)
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

   // We should not overflow decode buffer.
   int decodedSamples = min(decodedBufferLength, (unsigned)mpState->blockl);

   // Convert samples from float to 16-bit signed integer.
   for (int i = 0; i < decodedSamples; ++i)
   {
      float tmp = buffer[i];
      if (tmp > MAX_SAMPLE)
         tmp = MAX_SAMPLE;
      if (tmp < MIN_SAMPLE)
         tmp = MIN_SAMPLE;

      samplesBuffer[i] = MpAudioSample(tmp + 0.5f);
   }

   return decodedSamples;
}


const MpCodecInfo* MpdSipxILBC::getInfo() const
{
   if (mMode == MODE_20MS)
   {
      return &smCodecInfo20ms;
   } 
   else
   {
      return &smCodecInfo30ms;
   }
}

#endif // HAVE_ILBC ]
