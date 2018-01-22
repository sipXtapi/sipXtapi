//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

// SYSTEM INCLUDES
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

// APPLICATION INCLUDES
// CODEC LIBRARY INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// APPLICATION INCLUDES
#include <va_g729.h>

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// EXTERNAL FUNCTIONS
// DEFINES
// STATIC VARIABLES INITIALIZATON
/// Default (recommended) fmtp parameters
static const char* defaultFmtps[] =
{
   "annexb=no"
};
/// Exported codec information.
static const struct MppCodecInfoV1_1 sgCodecInfo = 
{
///////////// Implementation and codec info /////////////
   "VoiceAge Open G.729 Initiative", // codecManufacturer
   "G.729",                     // codecName
   "1.0",                       // codecVersion
   CODEC_TYPE_FRAME_BASED,      // codecType

/////////////////////// SDP info ///////////////////////
   "G729",                      // mimeSubtype
   sizeof(defaultFmtps)/sizeof(defaultFmtps[0]), // fmtpsNum
   defaultFmtps,                // fmtps
   8000,                        // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_NONE     // framePacking
};

/* ============================== FUNCTIONS =============================== */

CODEC_API int PLG_GET_INFO_V1_1(va_g729)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1_2(va_g729)(const char* fmtp, int isDecoder,
                                       struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
   if (pCodecInfo == NULL)
   {
      return NULL;
   }
   pCodecInfo->signalingCodec = FALSE;
   pCodecInfo->minBitrate = 8000;
   pCodecInfo->maxBitrate = 8000;
   pCodecInfo->numSamplesPerFrame = 80;
   pCodecInfo->minFrameBytes = 10;
   pCodecInfo->maxFrameBytes = 10;
   // G.729 have PLC actually, but our wrapper should be fixed to support it.
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
   pCodecInfo->vadCng = CODEC_CNG_NONE;

   if (isDecoder)
   {
      pCodecInfo->algorithmicDelay = 0;
      va_g729a_init_decoder();
   } 
   else
   {
      pCodecInfo->algorithmicDelay = 5 /* ms */ * 8 /* samples/ms */;
      va_g729a_init_encoder();
   }

   return (void*)1;
}


CODEC_API int PLG_FREE_V1(va_g729)(void* handle, int isDecoder)
{
   return RPLG_SUCCESS;
}

CODEC_API int PLG_DECODE_V1(va_g729)(void* handle,
                                     const uint8_t* pCodedData, 
                                     unsigned cbCodedPacketSize,
                                     uint16_t* pAudioBuffer,
                                     unsigned cbBufferSize,
                                     unsigned *pcbDecodedSize,
                                     const struct RtpHeader* pRtpHeader)
{
   if (pCodedData != NULL)
   {
      unsigned i;

      // Check that packet have integer number of frames in it.
      if (cbCodedPacketSize%L_FRAME_COMPRESSED != 0)
      {
         return RPLG_INVALID_ARGUMENT;
      }

      // Reset decoder pointer
      *pcbDecodedSize = 0;

      // Decode all frames.
      for (i=cbCodedPacketSize/L_FRAME_COMPRESSED; i>0; i--)
      {
         // Check that available buffer size is enough for decoded data
         if (cbBufferSize < L_FRAME)
         {
            return RPLG_INVALID_ARGUMENT;
         }

         // Decode
         va_g729a_decoder(pCodedData, pAudioBuffer, 0);

         // Move pointers to next frame
         *pcbDecodedSize += L_FRAME;
         pAudioBuffer += L_FRAME;
         pCodedData += L_FRAME_COMPRESSED;
         cbBufferSize -= L_FRAME;
      }
   } 
   else
   {
      // Check that available buffer size is enough for decoded data
      if (cbBufferSize < L_FRAME)
      {
         return RPLG_INVALID_ARGUMENT;
      }

      // Decode
      va_g729a_decoder(pCodedData, pAudioBuffer, 1);

      // Return number of decoded samples.
      *pcbDecodedSize = L_FRAME;
   }

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(va_g729)(void* handle,
                                     const void* pAudioBuffer,
                                     unsigned cbAudioSamples,
                                     int* rSamplesConsumed,
                                     void* pCodedData,
                                     unsigned cbMaxCodedData,
                                     int* pcbCodedSize,
                                     unsigned* pbSendNow)
{
   if (cbMaxCodedData < L_FRAME_COMPRESSED)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   // Check for necessary number of samples
   if (cbAudioSamples < L_FRAME)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   va_g729a_encoder(pAudioBuffer, pCodedData);

   *pbSendNow = FALSE;
   *rSamplesConsumed = L_FRAME;
   *pcbCodedSize = L_FRAME_COMPRESSED;

   return RPLG_SUCCESS;
}

PLG_ENUM_CODEC_START(va_g729)
  PLG_ENUM_CODEC(va_g729)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(va_g729)
  PLG_ENUM_CODEC_NO_SIGNALING(va_g729)
PLG_ENUM_CODEC_END 
