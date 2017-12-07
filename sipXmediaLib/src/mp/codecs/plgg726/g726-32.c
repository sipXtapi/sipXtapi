//  
// Copyright (C) 2007-2017 SIPez LLC.  All right reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

// SYSTEM INCLUDES
#include <memory.h>

// APPLICATION INCLUDES
#include "plgg726.h"

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// EXTERNAL FUNCTIONS
// DEFINES
// STATIC VARIABLES INITIALIZATON
/// Exported codec information.
static const struct MppCodecInfoV1_1 sgCodecInfo = 
{
///////////// Implementation and codec info /////////////
   "SpanDSP",                   // codecManufacturer
   "G.726-32",                  // codecName
   "1.15",                      // codecVersion
   CODEC_TYPE_SAMPLE_BASED,     // codecType

/////////////////////// SDP info ///////////////////////
   "G726-32",                   // mimeSubtype
   0,                           // fmtpsNum
   NULL,                        // fmtps
   8000,                        // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_NONE     // framePacking
};

DECLARE_FUNCS_V1(g726_32)

/* ============================== FUNCTIONS =============================== */

CODEC_API int PLG_GET_INFO_V1_1(g726_32)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1_2(g726_32)(const char* fmtp, int isDecoder,
                                       struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
   if (pCodecInfo == NULL)
   {
      return NULL;
   }
   pCodecInfo->signalingCodec = FALSE;
   pCodecInfo->minBitrate = 32000;
   pCodecInfo->maxBitrate = 32000;
   pCodecInfo->numSamplesPerFrame = 2;
   pCodecInfo->minFrameBytes = 1;
   pCodecInfo->maxFrameBytes = 1;
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
   pCodecInfo->vadCng = CODEC_CNG_NONE;
   pCodecInfo->algorithmicDelay = 0;

   return g726_init(NULL, 32000, G726_ENCODING_LINEAR, G726_PACKING_LEFT);
}

CODEC_API int PLG_FREE_V1(g726_32)(void* handle, int isDecoder)
{
   g726_release((g726_state_t*)handle);
   return RPLG_SUCCESS;
}

CODEC_API int PLG_DECODE_V1(g726_32)(void* handle, const void* pCodedData, 
                                     unsigned cbCodedPacketSize, void* pAudioBuffer, 
                                     unsigned cbBufferSize, unsigned *pcbCodedSize, 
                                     const struct RtpHeader* pRtpHeader)
{
   return internal_decode_g726(handle, pCodedData, cbCodedPacketSize, pAudioBuffer,
      cbBufferSize, pcbCodedSize, pRtpHeader);
}

CODEC_API int PLG_ENCODE_V1(g726_32)(void* handle, const void* pAudioBuffer, 
                                     unsigned cbAudioSamples, int* rSamplesConsumed, 
                                     void* pCodedData, unsigned cbMaxCodedData, 
                                     int* pcbCodedSize, unsigned* pbSendNow)
{
   return internal_encode_g726(handle, pAudioBuffer, cbAudioSamples, rSamplesConsumed, 
      pCodedData, cbMaxCodedData, pcbCodedSize, pbSendNow);
}
