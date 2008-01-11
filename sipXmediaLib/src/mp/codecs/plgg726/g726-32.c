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

// APPLICATION INCLUDES
#include "plgg726.h"

// STATIC VARIABLE INITIALIZATIONS
static const char codecMIMEsubtype[] = "g726-32";

static const struct plgCodecInfoV1 sipxCodecInfog726_32 = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   codecMIMEsubtype,                //mimeSubtype
   "G726-32",                       //codecName
   "G726-32-spandsp",               //codecVersion
   8000,                            //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   160,                             //interleaveBlockSize
   32000,                           //bitRate
   320,                             //minPacketBits
   320,                             //avgPacketBits
   320,                             //maxPacketBits
   160,                             //numSamplesPerFrame
   3                                //preCodecJitterBufferSize
};

CODEC_API int PLG_ENUM_V1(g726_32)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
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

CODEC_API void *PLG_INIT_V1(g726_32)(const char* fmtps, int isDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   if (pCodecInfo == NULL) {
      return NULL;
   }
   memcpy(pCodecInfo, &sipxCodecInfog726_32, sizeof(struct plgCodecInfoV1));
   
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
