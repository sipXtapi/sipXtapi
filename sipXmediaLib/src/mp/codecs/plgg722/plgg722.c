//  
// Copyright (C) 2007-2017 SIPez LLC. All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

// SYSTEM INCLUDES
#include <memory.h>
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// CODEC LIBRARY INCLUDES
#include <spandsp/telephony.h>
#include <spandsp/g722.h>

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// EXTERNAL FUNCTIONS
// DEFINES
/// G.722 bitrate - we support only 64,000bps mode (as in RFC3551).
#define G722_MODE      64000
/// Set USE_8K_SAMPLES to G722_SAMPLE_RATE_8000 to accept 8kHz PCM data
/// as input/output instead of 16kHz data.
#define USE_8K_SAMPLES   0

// STATIC VARIABLES INITIALIZATON
/// Exported codec information.
static const struct MppCodecInfoV1_1 sgCodecInfo = 
{
///////////// Implementation and codec info /////////////
   "SpanDSP library",           // codecManufacturer
   "G.722",                     // codecName
   "1.15",                      // codecVersion
   CODEC_TYPE_SAMPLE_BASED,     // codecType

/////////////////////// SDP info ///////////////////////
   "G722",                      // mimeSubtype
   0,                           // fmtpsNum
   NULL,                        // fmtps
   16000,                       // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_NONE     // framePacking
};

DECLARE_FUNCS_V1(g722)

/* ============================== FUNCTIONS =============================== */

CODEC_API int PLG_GET_INFO_V1_1(g722)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1_2(g722)(const char* fmtp, int isDecoder,
                                    struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
   if (pCodecInfo == NULL)
   {
      return NULL;
   }
   pCodecInfo->signalingCodec = FALSE;
   if (G722_MODE == 64000)
   {
      pCodecInfo->minBitrate = 64000;
      pCodecInfo->maxBitrate = 64000;
      pCodecInfo->numSamplesPerFrame = 2;
      pCodecInfo->minFrameBytes = 1;
      pCodecInfo->maxFrameBytes = 1;
   } 
   else
   {
      // Bitrates other then 64kbps are not supported.
      return NULL;
   }
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
   pCodecInfo->vadCng = CODEC_CNG_NONE;

   if (isDecoder)
   {
      pCodecInfo->algorithmicDelay = 0;
      return g722_decode_init(NULL, G722_MODE, USE_8K_SAMPLES);
   }
   else
   {
      pCodecInfo->algorithmicDelay = 16 /* samples/ms */ * 3/2 /* ms */;
      return g722_encode_init(NULL, G722_MODE, USE_8K_SAMPLES);
   }
}


CODEC_API int PLG_FREE_V1(g722)(void* handle, int isDecoder)
{
   if (isDecoder)
   {
      g722_decode_release((g722_decode_state_t *)handle);
      g722_decode_free((g722_decode_state_t *)handle);
   }
   else
   {
      g722_encode_release((g722_encode_state_t *)handle);
      g722_encode_free((g722_encode_state_t *)handle);
   }
   return RPLG_SUCCESS;
}

CODEC_API int PLG_DECODE_V1(g722)(void* handle, const void* pCodedData, 
                                     unsigned cbCodedPacketSize, void* pAudioBuffer, 
                                     unsigned cbBufferSize, unsigned *pcbCodedSize, 
                                     const struct RtpHeader* pRtpHeader)
{
   int samples;

   samples = g722_decode((g722_decode_state_t*)handle, (int16_t*)pAudioBuffer, 
                         pCodedData, cbCodedPacketSize);
   *pcbCodedSize = samples;

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(g722)(void* handle, const void* pAudioBuffer, 
                                     unsigned cbAudioSamples, int* rSamplesConsumed, 
                                     void* pCodedData, unsigned cbMaxCodedData, 
                                     int* pcbCodedSize, unsigned* pbSendNow)
{
   int bytes;

   bytes = g722_encode((g722_encode_state_t*)handle, pCodedData, pAudioBuffer, cbAudioSamples);
   *pcbCodedSize = bytes;

   *pbSendNow = FALSE;
   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}

PLG_ENUM_CODEC_START(g722)
  PLG_ENUM_CODEC(g722)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(g722)
  PLG_ENUM_CODEC_NO_SIGNALING(g722)
PLG_ENUM_CODEC_END 
