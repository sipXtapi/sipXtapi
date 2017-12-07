//  
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

// SYSTEM INCLUDES
#include <memory.h>

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>
#ifndef USE_BUGGY_G711 // [
#  ifdef _MSC_VER // [
#     define __inline__ __inline // For gcc compatibility
#   endif // _MSC_VER ]
#  include <spandsp/g711.h>
#endif // !USE_BUGGY_G711 ]

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// EXTERNAL FUNCTIONS
#ifdef USE_BUGGY_G711 // [
   extern int G711A_Decoder(int numSamples, const uint8_t* inBuff, MpAudioSample* outBuf);
   extern int G711A_Encoder(int numSamples, const MpAudioSample* inBuff, uint8_t* outBuf);
#endif // USE_BUGGY_G711 ]

// DEFINES
#define DECODER_HANDLE     ((void*)1)
#define ENCODER_HANDLE     ((void*)2)

// STATIC VARIABLE INITIALIZATIONS
/// Exported codec information.
static const struct MppCodecInfoV1_1 sgCodecInfo = 
{
///////////// Implementation and codec info /////////////
   "SpanDSP",                   // codecManufacturer
   "G.711A",                    // codecName
   "0.0.5pre4",                 // codecVersion
   CODEC_TYPE_SAMPLE_BASED,     // codecType

/////////////////////// SDP info ///////////////////////
   "PCMA",                      // mimeSubtype
   0,                           // fmtpsNum
   NULL,                        // fmtps
   8000,                        // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_NONE     // framePacking
};

DECLARE_FUNCS_V1(sipxPcma)

/* ============================== FUNCTIONS =============================== */

CODEC_API int PLG_GET_INFO_V1_1(sipxPcma)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo;
   }
   return RPLG_SUCCESS;
}


CODEC_API void *PLG_INIT_V1_2(sipxPcma)(const char* fmtp, int isDecoder,
                                        struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
   if (pCodecInfo == NULL)
   {
      return NULL;
   }
   pCodecInfo->signalingCodec = FALSE;
   pCodecInfo->minBitrate = 64000;
   pCodecInfo->maxBitrate = 64000;
   pCodecInfo->numSamplesPerFrame = 1;
   pCodecInfo->minFrameBytes = 1;
   pCodecInfo->maxFrameBytes = 1;
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
   pCodecInfo->vadCng = CODEC_CNG_NONE;
   pCodecInfo->algorithmicDelay = 0;

   if (isDecoder)
      return DECODER_HANDLE;
   else
      return ENCODER_HANDLE;
}

CODEC_API int PLG_FREE_V1(sipxPcma)(void* handle, int isDecoder)
{
   return RPLG_SUCCESS;
}

CODEC_API int PLG_DECODE_V1(sipxPcma)(void* handle, const void* pCodedData, 
                                      unsigned cbCodedPacketSize, void* pAudioBuffer, 
                                      unsigned cbBufferSize, unsigned *pcbCodedSize, 
                                      const struct RtpHeader* pRtpHeader)
{
   int samples;
   if (handle != DECODER_HANDLE)
      return RPLG_BAD_HANDLE;

   // Assert that available buffer size is enough for the packet.
   if (cbCodedPacketSize > cbBufferSize)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   if (cbBufferSize == 0)
      return RPLG_INVALID_ARGUMENT;

   samples = PLG_MIN(cbCodedPacketSize, cbBufferSize);
#ifdef USE_BUGGY_G711 // [
   G711A_Decoder(samples, (uint8_t*)pCodedData, (MpAudioSample *)pAudioBuffer);
#else // USE_BUGGY_G711 ][
   {
      int16_t *pSamples = (int16_t*)pAudioBuffer;
      uint8_t *pEncoded = (uint8_t*)pCodedData;
      int i;
      for (i=0; i<samples; i++)
      {
         pSamples[i] = alaw_to_linear(pEncoded[i]);
      }
   }
#endif // USE_BUGGY_G711 ]
   *pcbCodedSize = samples;

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(sipxPcma)(void* handle, const void* pAudioBuffer, 
                                      unsigned cbAudioSamples, int* rSamplesConsumed, 
                                      void* pCodedData, unsigned cbMaxCodedData, 
                                      int* pcbCodedSize, unsigned* pbSendNow)
{
   if (handle != ENCODER_HANDLE)
      return RPLG_BAD_HANDLE;

#ifdef USE_BUGGY_G711 // [
   G711A_Encoder(cbAudioSamples, (MpAudioSample *)pAudioBuffer, (uint8_t*)pCodedData);
#else // USE_BUGGY_G711 ][
   {
      int16_t *pSamples = (int16_t*)pAudioBuffer;
      uint8_t *pEncoded = (uint8_t*)pCodedData;
      unsigned i;
      for (i=0; i<cbAudioSamples; i++)
      {
         pEncoded[i] = linear_to_alaw(pSamples[i]);
      }
   }
#endif // USE_BUGGY_G711 ]
   *pcbCodedSize = cbAudioSamples;

   *pbSendNow = FALSE;
   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}
