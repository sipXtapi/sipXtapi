//  
// Copyright (C) 2007-2017 SIPez LLC. 
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
#include "plgg726.h"

int internal_decode_g726(void* handle, const void* pCodedData, 
                         unsigned cbCodedPacketSize, void* pAudioBuffer, 
                         unsigned cbBufferSize, unsigned *pcbCodedSize, 
                         const struct RtpHeader* pRtpHeader)
{
   int samples;
   int maxBytesToDecode;

   // TODO: fix this hack
   maxBytesToDecode = cbCodedPacketSize;
   // g726_state_t is now opaque.  Can no longer access bits_per_sample bits_per_sample 
   //maxBytesToDecode = PLG_MIN(cbCodedPacketSize, cbBufferSize*8/((g726_state_t*)handle)->bits_per_sample);
   //assert(maxBytesToDecode == cbCodedPacketSize);
   
   samples = g726_decode((g726_state_t*)handle, (int16_t*)pAudioBuffer, pCodedData, maxBytesToDecode);
//   printf("G726 decoded %d frames.\n", samples);
   *pcbCodedSize = samples;

   return RPLG_SUCCESS;
}


int internal_encode_g726(void* handle, const void* pAudioBuffer, 
                         unsigned cbAudioSamples, int* rSamplesConsumed, 
                         void* pCodedData, unsigned cbMaxCodedData, 
                         int* pcbCodedSize, unsigned* pbSendNow)
{
   int bytes;

   bytes = g726_encode((g726_state_t*)handle, pCodedData, pAudioBuffer, cbAudioSamples);
   assert(bytes <= cbMaxCodedData);
   *pcbCodedSize = bytes;

//   printf("G726 encoder: samples consumed: %d, bytes in a frame: %d, bytes wrote: %d\n",
//          cbAudioSamples, bytes, cbMaxCodedData);

   *pbSendNow = FALSE;
   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}


DECLARE_FUNCS_V1(g726_16)
DECLARE_FUNCS_V1(g726_24)
DECLARE_FUNCS_V1(g726_32)
DECLARE_FUNCS_V1(g726_40)

PLG_ENUM_CODEC_START(g726)
  PLG_ENUM_CODEC(g726_16)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(g726_16)
  PLG_ENUM_CODEC_NO_SIGNALING(g726_16)

  PLG_ENUM_CODEC(g726_24)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(g726_24)
  PLG_ENUM_CODEC_NO_SIGNALING(g726_24)

  PLG_ENUM_CODEC(g726_32)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(g726_32)
  PLG_ENUM_CODEC_NO_SIGNALING(g726_32)

  PLG_ENUM_CODEC(g726_40)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(g726_40)
  PLG_ENUM_CODEC_NO_SIGNALING(g726_40)
PLG_ENUM_CODEC_END 
