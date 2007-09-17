//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
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
#include <spandsp/g722.h>

// By default set 64,000 
#define BITRATE      64000

// STATIC VARIABLE INITIALIZATIONS
static const char codecMIMEsubtype[] = "g722";

static const struct plgCodecInfoV1 sipxCodecInfog722 = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   codecMIMEsubtype,                //mimeSubtype
   "g722",                          //codecName
   "g722_spansdp",                  //codecVersion
   16000,                           //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   160,                             //interleaveBlockSize
   BITRATE,                         //bitRate
   640,                             //minPacketBits
   640,                             //avgPacketBits
   640,                             //maxPacketBits
   160,                             //numSamplesPerFrame
   3                                //preCodecJitterBufferSize
};


// Set USE_8K_SAMPLES to G722_SAMPLE_RATE_8000 if sipX uses 8k samplerate
#define USE_8K_SAMPLES   0

CODEC_API int PLG_ENUM_V1(g722)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
{
   if (mimeSubtype) {
      *mimeSubtype = codecMIMEsubtype;
   }
   if (pModesCount) {
      *pModesCount = 0;
   }
   if (modes) {
      *modes = NULL;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1(g722)(const char* fmtps, int isDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   if (pCodecInfo == NULL) {
      return NULL;
   }
   memcpy(pCodecInfo, &sipxCodecInfog722, sizeof(struct plgCodecInfoV1));

   if (isDecoder)
      return g722_decode_init(NULL, BITRATE, USE_8K_SAMPLES);
   else
      return g722_encode_init(NULL, BITRATE, USE_8K_SAMPLES);
}


CODEC_API int PLG_FREE_V1(g722)(void* handle, int isDecoder)
{
   if (isDecoder)
      g722_decode_release((g722_decode_state_t *)handle);
   else
      g722_encode_release((g722_encode_state_t *)handle);
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
   *pcbCodedSize = cbMaxCodedData;

   *pbSendNow = FALSE;
   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}



PLG_SINGLE_CODEC(g722);
