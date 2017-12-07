//  
// Copyright (C) 2008-2017 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT sipez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>
#include "CodecL16.h"

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// EXTERNAL FUNCTIONS
// DEFINES
#define DECODER_HANDLE     ((void*)1)
#define ENCODER_HANDLE     ((void*)2)

// STATIC VARIABLE INITIALIZATIONS
/// Exported codec information.
static const struct MppCodecInfoV1_1 sgCodecInfo_8000 = 
{
///////////// Implementation and codec info /////////////
   "SIPFoundry",                // codecManufacturer
   "L16",                       // codecName
   "1.0",                       // codecVersion
   CODEC_TYPE_SAMPLE_BASED,     // codecType

/////////////////////// SDP info ///////////////////////
   "L16",                       // mimeSubtype
   0,                           // fmtpsNum
   NULL,                        // fmtps
   8000,                        // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_NONE     // framePacking
};

DECLARE_FUNCS_V1(sipxL16_8000)

/* ============================== FUNCTIONS =============================== */

CODEC_API int PLG_GET_INFO_V1_1(sipxL16_8000)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo_8000;
   }
   return RPLG_SUCCESS;
}


CODEC_API void *PLG_INIT_V1_2(sipxL16_8000)(const char* fmtp, int isDecoder,
                                             struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
   if (pCodecInfo == NULL)
   {
      return NULL;
   }
   pCodecInfo->signalingCodec = FALSE;
   pCodecInfo->minBitrate = 8000*16;
   pCodecInfo->maxBitrate = 8000*16;
   pCodecInfo->numSamplesPerFrame = 1;
   pCodecInfo->minFrameBytes = 2;
   pCodecInfo->maxFrameBytes = 2;
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
   pCodecInfo->vadCng = CODEC_CNG_NONE;
   pCodecInfo->algorithmicDelay = 0;

   if (isDecoder)
      return DECODER_HANDLE;
   else
      return ENCODER_HANDLE;
}

CODEC_API int PLG_FREE_V1(sipxL16_8000)(void* handle, int isDecoder)
{
   return RPLG_SUCCESS;
}

CODEC_API int PLG_DECODE_V1(sipxL16_8000)(void* handle, const void* pCodedData, 
                                           unsigned cbCodedPacketSize, void* pAudioBuffer, 
                                           unsigned cbBufferSize, unsigned *pcbCodedSize, 
                                           const struct RtpHeader* pRtpHeader)
{
   if (handle != DECODER_HANDLE)
      return RPLG_BAD_HANDLE;

   // Assert that available buffer size is enough for the packet.
   if (cbCodedPacketSize/sizeof(audio_sample_t) > cbBufferSize)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   if (cbBufferSize == 0)
      return RPLG_INVALID_ARGUMENT;

   sipxL16decode(pCodedData, cbCodedPacketSize, pAudioBuffer, cbBufferSize,
                 pcbCodedSize);

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(sipxL16_8000)(void* handle, const void* pAudioBuffer, 
                                           unsigned cbAudioSamples, int* rSamplesConsumed, 
                                           void* pCodedData, unsigned cbMaxCodedData, 
                                           int* pcbCodedSize, unsigned* pbSendNow)
{
   if (handle != ENCODER_HANDLE)
      return RPLG_BAD_HANDLE;

   sipxL16encode(pAudioBuffer, cbAudioSamples, rSamplesConsumed, pCodedData,
                 cbMaxCodedData, pcbCodedSize);

   *pbSendNow = FALSE;

   return RPLG_SUCCESS;
}
