//  
// Copyright (C) 2007-2018 SIPez LLC. All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT sipez DOT com>

// SYSTEM INCLUDES
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

// APPLICATION INCLUDES
// CODEC LIBRARY INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// APPLICATION INCLUDES
#include <g729a_typedef.h>
#include <g729a_interface.h>

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// EXTERNAL FUNCTIONS
DECLARE_FUNCS_V1(itu_g729)

// DEFINES
#define COMPRESSED_G729_FRAME_BYTES 10
#define G729_FRAME_SAMPLES 80

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
   "ITU G.729", // codecManufacturer
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

CODEC_API int PLG_GET_INFO_V1_1(itu_g729)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1_2(itu_g729)(const char* fmtp, int isDecoder,
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
   pCodecInfo->minFrameBytes = COMPRESSED_G729_FRAME_BYTES;
   pCodecInfo->maxFrameBytes = COMPRESSED_G729_FRAME_BYTES;
   // TODO:  Not sure this implementation of G729 supports PLC
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
   pCodecInfo->vadCng = CODEC_CNG_NONE;

   void* coderContext = NULL;
   if (isDecoder)
   {
      pCodecInfo->algorithmicDelay = 0;

      G729A_Dec_state decoderStatePtr = malloc(G729A_Decoder_Get_Size());
      G729A_Decoder_Init(decoderStatePtr);
      coderContext = decoderStatePtr;
   } 
   else
   {
      pCodecInfo->algorithmicDelay = 5 /* ms */ * 8 /* samples/ms */;

      G729A_Enc_state encoderStatePtr = malloc(G729A_Encoder_Get_Size());
      G729A_Encoder_Init(encoderStatePtr);
      coderContext = encoderStatePtr;
   }

   return (void*)coderContext;
}


CODEC_API int PLG_FREE_V1(itu_g729)(void* handle, int isDecoder)
{
   free(handle);
   return RPLG_SUCCESS;
}

CODEC_API int PLG_DECODE_V1(itu_g729)(void* handle,
                                     const void* pCodedData, 
                                     unsigned cbCodedPacketSize,
                                     void* pAudioBuffer,
                                     unsigned cbBufferSize,
                                     unsigned* pcbDecodedSize,
                                     const struct RtpHeader* pRtpHeader)
{
   if(handle)
   {
      G729A_Dec_state decoderContext = (G729A_Dec_state) handle;
      int16_t* audioSamples = pAudioBuffer;

      if (pCodedData != NULL)
      {
         int8_t* encodedData = pCodedData;
         unsigned i;

         // Check that packet have integer number of frames in it.
         if (cbCodedPacketSize % COMPRESSED_G729_FRAME_BYTES != 0)
         {
            return RPLG_INVALID_ARGUMENT;
         }

         // Reset decoder pointer
         *pcbDecodedSize = 0;

         // Decode all frames.
         for (i=cbCodedPacketSize / COMPRESSED_G729_FRAME_BYTES; i>0; i--)
         {
            // Check that available buffer size is enough for decoded data
            if (cbBufferSize < G729_FRAME_SAMPLES)
            {
               return RPLG_INVALID_ARGUMENT;
            }

            // Decode
            G729A_Decoder_Process(decoderContext, encodedData, audioSamples);

            // Move pointers to next frame
            *pcbDecodedSize += G729_FRAME_SAMPLES;
            audioSamples += G729_FRAME_SAMPLES;
            encodedData += COMPRESSED_G729_FRAME_BYTES;
            cbBufferSize -= G729_FRAME_SAMPLES;
         }
      } 
      else
      {
         // Check that available buffer size is enough for decoded data
         if (cbBufferSize < G729_FRAME_SAMPLES)
         {
            return RPLG_INVALID_ARGUMENT;
         }

         // Decode
         // TODO:  Not sure this implementation of G729 supports PLC
         assert(pCodedData);
         G729A_Decoder_Process(decoderContext, (void*)pCodedData, audioSamples);

         // Return number of decoded samples.
         *pcbDecodedSize = G729_FRAME_SAMPLES;
      }
   }
   else
   {
      mppLogError("PlgItuG729 NULL decoder context");
   }

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(itu_g729)(void* handle,
                                     const void* pAudioBuffer,
                                     unsigned cbAudioSamples,
                                     int* rSamplesConsumed,
                                     void* pCodedData,
                                     unsigned cbMaxCodedData,
                                     int* pcbCodedSize,
                                     unsigned* pbSendNow)
{
   *pbSendNow = FALSE;
   *rSamplesConsumed = 0;
   *pcbCodedSize = 0;

   if (cbMaxCodedData < COMPRESSED_G729_FRAME_BYTES)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   // Check for necessary number of samples
   if (cbAudioSamples < G729_FRAME_SAMPLES)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   if(handle)
   {
      G729A_Enc_state encoderContext = (G729A_Enc_state) handle;
      G729A_Encoder_Process(encoderContext, (void*)pAudioBuffer, pCodedData);

      *rSamplesConsumed = G729_FRAME_SAMPLES;
      *pcbCodedSize = COMPRESSED_G729_FRAME_BYTES;
   }
   else
   {
      mppLogError("PlgItuG729 encoder context NULL");
   }

   return RPLG_SUCCESS;
}

PLG_ENUM_CODEC_START(itu_g729)
  PLG_ENUM_CODEC(itu_g729)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(itu_g729)
  PLG_ENUM_CODEC_NO_SIGNALING(itu_g729)
PLG_ENUM_CODEC_END 
