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
#include "mp/MpAudioConnection.h"
#include "mp/JB/JB_API.h"
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
    160,                        // interleaveBlockSize
    15200,                      // bitRate
    NO_OF_BYTES_20MS * 8,       // minPacketBits
    NO_OF_BYTES_20MS * 8,       // avgPacketBits
    NO_OF_BYTES_20MS * 8,       // maxPacketBits
    160,                        // numSamplesPerFrame
    6);                         // preCodecJitterBufferSize (should be adjusted)



MpdSipxILBC::MpdSipxILBC(int payloadType)
: MpDecoderBase(payloadType, &smCodecInfo)
, mpJBState(NULL)
, mpState(NULL)
{
}

MpdSipxILBC::~MpdSipxILBC()
{
   freeDecode();
}

OsStatus MpdSipxILBC::initDecode(MpAudioConnection* pConnection)
{
   mpJBState = pConnection->getJBinst();
   JB_initCodepoint(mpJBState, "iLBC", 8000, getPayloadType());

   if (mpState == NULL) 
   {
      mpState = new iLBC_Dec_Inst_t();
      memset(mpState, 0, sizeof(*mpState));
      ::initDecode(mpState, 20, 1);
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
   if (decodedBufferLength < 160)
   {
      osPrintf("MpdSipxILBC::decode: Jitter buffer overloaded. Glitch!\n");
      return 0;
   }

   // Decode incoming packet to temp buffer. If no packet - do PLC.
   float buffer[160];
   if (pPacket.isValid())
   {
      // Assert that available buffer size is enough for the packet.
      if (NO_OF_BYTES_20MS != pPacket->getPayloadSize())
         return 0;

      // Packet data available. Decode it.
      iLBC_decode(buffer, (unsigned char*)pPacket->getDataPtr(), mpState, 1);
   }
   else
   {
      // Packet data does not available. Do PLC.
      iLBC_decode(buffer, NULL, mpState, 0);
   }

   for (int i = 0; i < 160; ++i)
   {
      float tmp = buffer[i];
      if (tmp > SHRT_MAX)
         tmp = SHRT_MAX;
      if (tmp < SHRT_MIN)
         tmp = SHRT_MIN;

      samplesBuffer[i] = MpAudioSample(tmp + 0.5f);
   }

   return 160;
}

#endif // HAVE_ILBC ]
