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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

// APPLICATION INCLUDES
// CODEC LIBRARY INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// APPLICATION INCLUDES
#include <va_g729.h>

// LOCAL DATA TYPES
// STATIC VARIABLES INITIALIZATON
/// Codec MIME-subtype
static const char codecMIMEsubtype[] = "g729";
/// Codec information
static const struct plgCodecInfoV1 codecG729 = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   codecMIMEsubtype,                //mimeSubtype
   "G.729",                         //codecName
   "VoiceAge Open G.729",           //codecVersion
   8000,                            //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   160,                             //interleaveBlockSize
   8000,                            //bitRate
   10*8,                            //minPacketBits
   20*8,                            //avgPacketBits
   20*8,                            //maxPacketBits
   160,                             //numSamplesPerFrame
   6,                               //preCodecJitterBufferSize
   1,                               //codecSupportPLC
   0                                //signalingCodec
};
/// Default (recommended) fmtp parameters
static const char* defaultFmtps[] =
{
   "annexb=no"
};

CODEC_API int PLG_ENUM_V1(va_g729)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
{
   if (mimeSubtype) {
      *mimeSubtype = codecMIMEsubtype;
   }
   if (pModesCount) {
      *pModesCount = (sizeof(defaultFmtps)/sizeof(defaultFmtps[0]));
   }
   if (modes) {
      *modes = defaultFmtps;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1(va_g729)(const char* fmt, int isDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   if (pCodecInfo == NULL)
   {
      return NULL;
   }

   memcpy(pCodecInfo, &codecG729, sizeof(struct plgCodecInfoV1));

   if (isDecoder)
   {
      va_g729a_init_decoder();
   } 
   else
   {
      va_g729a_init_encoder();
   }

   return 1;
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

PLG_SINGLE_CODEC(va_g729);
