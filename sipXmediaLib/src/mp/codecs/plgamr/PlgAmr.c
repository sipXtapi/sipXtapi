//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

/**
*  @file
*
*  This is AMR (narrow-band) encoder/decoder wrapper.
*
*  @see Read RFC4867 for payload format description and 3gpp specifications
*       for more information about AMR codec.
*
*  @note fmtp string is not really parsed. We silently assume default settings:
*        * Octet Aligned mode;
*        * DTX disabled;
*        * all optional features disabled;
*        * only one frame per packet;
*        * encoder use 12.2kbps mode;
*        * encoder set CMR field in payload header to 15 as we have no preference
*          which mode to receive.
*/

// WIN32: Add libamrnb to linker input.
#ifdef WIN32 // [
#   pragma comment(lib, "libamrnb.lib")
#endif // WIN32 ]

// SYSTEM INCLUDES
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// CODEC LIBRARY INCLUDES
#undef IF2  ///< RTP payload use Interface Format 1 (RFC2867 section 4.3.3)
#undef ETSI ///< No need for exploded bits (ETSI format)
#include <interf_enc.h>
#include <interf_dec.h>

// LOCAL DATA TYPES
/// Storage for encoder data
struct amr_encoder_data
{
   audio_sample_t mpBuffer[160];    ///< Buffer used to store input samples
   unsigned mBufferLoad;            ///< Number of samples stored in the buffer

   void *encoder_state;             ///< Pointer to encoder/decoder state structure
   int octet_align;                 ///< 1 - Octet Aligned mode, 0 - Bandwidth Efficient mode
   enum Mode mode;                  ///< Encoder mode (bitrate).
};

/// Storage for decoder data
struct amr_decoder_data
{
   void *decoder_state;             ///< Pointer to encoder/decoder state structure
   int octet_align;                 ///< 1 - Octet Aligned mode, 0 - Bandwidth Efficient mode
};

// STATIC VARIABLES INITIALIZATON
/// Codec MIME-subtype
static const char codecMIMEsubtype[] = "amr";
/// Codec information
struct plgCodecInfoV1 codecAMR = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   codecMIMEsubtype,                //mimeSubtype
   "AMR",                           //codecName
   "AMR 7.0.0.0",                   //codecVersion
   8000,                            //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   160,                             //interleaveBlockSize
   13200,                           //bitRate
   33*8,                            //minPacketBits
   33*8,                            //avgPacketBits
   33*8,                            //maxPacketBits
   160,                             //numSamplesPerFrame
   6,                               //preCodecJitterBufferSize
   0                                //codecSupportPLC
};
/// Frame sizes for different codec modes
static const uint8_t sgFrameSizesMap[16] =
{ 13, 14, 16, 18, 20, 21, 27, 32, 6 , 0 , 0 , 0 , 0 , 0 , 0 , 1  };
/// Default (recommended) fmtp parameters
static const char* defaultFmtps[] =
{
   "octet-align=1"
      //   "" // octet-align=0 assumed
};

CODEC_API int PLG_ENUM_V1(amr)(const char** mimeSubtype,
                               unsigned int* pModesCount,
                               const char*** modes)
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

CODEC_API void *PLG_INIT_V1(amr)(const char* fmtp,
                                 int isDecoder,
                                 struct plgCodecInfoV1* pCodecInfo)
{
   if (pCodecInfo != NULL)
   {
      int octet_align = 1;    /* Octet Aligned mode / Bandwidth Efficient mode */
      int doDtx = 0;          /* DTX / no DTX */
      enum Mode mode = MR122; /* mode to be used by encoder (12.2kbps by default) */

      /*
      *             Here fmtp should be parsed.
      *
      * Now we silently assume that Octet Aligned mode is used and DTX is not
      * enabled.
      */

      // Fill in codec info structure
      memcpy(pCodecInfo, &codecAMR, sizeof(struct plgCodecInfoV1));

      // Allocate and fill in codec state structure
      if (isDecoder)
      {
         struct amr_decoder_data *pCodecData;

         pCodecData = (struct amr_decoder_data *)malloc(sizeof(struct amr_decoder_data));
         if (!pCodecData)
         {
            return NULL;
         }

         pCodecData->octet_align = octet_align;
         pCodecData->decoder_state = Decoder_Interface_init();
         return pCodecData;
      } 
      else
      {
         struct amr_encoder_data *pCodecData;

         pCodecData = (struct amr_encoder_data *)malloc(sizeof(struct amr_encoder_data));
         if (!pCodecData)
         {
            return NULL;
         }

         pCodecData->mBufferLoad = 0;
         pCodecData->octet_align = octet_align;
         pCodecData->mode = mode;
         pCodecData->encoder_state = Encoder_Interface_init(doDtx);
         return pCodecData;
      }
   }

   return NULL;
}


CODEC_API int PLG_FREE_V1(amr)(void* handle, int isDecoder)
{
   if (isDecoder)
   {
      struct amr_decoder_data *pCodecData = (struct amr_decoder_data *)handle;

      if (NULL != handle)
      {
         Decoder_Interface_exit(pCodecData->decoder_state);
         free(handle);
      }
   } 
   else
   {
      struct amr_encoder_data *pCodecData = (struct amr_encoder_data *)handle;

      if (NULL != handle)
      {
         Encoder_Interface_exit(pCodecData->encoder_state);
         free(handle);
      }
   }

   return 0;
}

CODEC_API int PLG_DECODE_V1(amr)(void* handle, const uint8_t* pCodedData,
                                 unsigned cbCodedPacketSize, uint16_t* pAudioBuffer,
                                 unsigned cbBufferSize, unsigned *pcbDecodedSize,
                                 const struct RtpHeader* pRtpHeader)
{
   struct amr_decoder_data *pCodecData = (struct amr_decoder_data *)handle;
   int haveMoreData;   // Payload ToC F bit
   int frameMode;      // Payload ToC FT field
   int frameQuality;   // Payload ToC Q bit
   unsigned dataIndex; // Number of currently processing byte in a packet.

   assert(handle != NULL);

   // Assert that available buffer size is enough for the packet.
   if (cbBufferSize < 160)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   // There should be at least 2 bytes in a packet - CMR and ToC fields.
   if (cbCodedPacketSize < 2)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   // We do not support Bandwidth Efficient mode now.
   assert(pCodecData->octet_align == 1);

   // Start parsing from the first byte
   dataIndex = 0;

   // Ignore Codec Mode Request (CMR) field, because our decoder could not
   // control our encoder at the moment.
//   int cmr = pCodedData[dataIndex] >> 4;
   dataIndex++;

   // Check packet length, as recommended in RFC 4867 section 4.5.1.
   // To do this we're going through all available speech frames and check
   // that their total size is equal to packet length minus 1 byte
   // for CMR field.
   do 
   {
      // Parse Table of Contents field
      haveMoreData =  (pCodedData[dataIndex]) >> 7;          // Payload ToC F bit
      frameMode    = ((pCodedData[dataIndex]) >> 3) & 0x0F;  // Payload ToC FT field
      frameQuality = ((pCodedData[dataIndex]) >> 2) & 0x01;  // Payload ToC Q bit

      // Jump to the next speech frame
      dataIndex += sgFrameSizesMap[frameMode];

      // Something is broken if we have gone over the packet data end
      if (dataIndex > cbCodedPacketSize)
      {
         return RPLG_INVALID_ARGUMENT;
      }
   } while(haveMoreData == 1);

   // Start over from first frame ToC
   dataIndex = 1;
   // Reset number of decoded samples
   *pcbDecodedSize = 0;

   // Decode all frames in packet.
   do 
   {
      // Parse Table of Contents field
      haveMoreData =  (pCodedData[dataIndex]) >> 7;          // Payload ToC F bit
      frameMode    = ((pCodedData[dataIndex]) >> 3) & 0x0F;  // Payload ToC FT field
      frameQuality = ((pCodedData[dataIndex]) >> 2) & 0x01;  // Payload ToC Q bit

      Decoder_Interface_Decode(pCodecData->decoder_state,       // Decoder state
                               &pCodedData[dataIndex],          // ToC and Speech frame
                               &pAudioBuffer[*pcbDecodedSize],  // Decoder data buffer
                               0);                              // This is a good frame
      *pcbDecodedSize += 160;

      // Jump to the next speech frame
      dataIndex += sgFrameSizesMap[frameMode];
   } while(haveMoreData == 1);

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(amr)(void* handle, const void* pAudioBuffer,
                                 unsigned cbAudioSamples, int* rSamplesConsumed,
                                 uint8_t* pCodedData, unsigned cbMaxCodedData,
                                 int* pcbCodedSize, unsigned* pbSendNow)
{
   struct amr_encoder_data *pCodecData = (struct amr_encoder_data *)handle;

   assert(handle != NULL);

   if (cbMaxCodedData < 33)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   // We do not support Bandwidth Efficient mode now.
   assert(pCodecData->octet_align == 1);

   // Calculate amount of audio data to be consumed and store it
   // to intermediate buffer.
   *rSamplesConsumed = min(cbAudioSamples, 160-pCodecData->mBufferLoad);
   memcpy(&pCodecData->mpBuffer[pCodecData->mBufferLoad], pAudioBuffer,
          SIZE_OF_SAMPLE*(*rSamplesConsumed));
   pCodecData->mBufferLoad = pCodecData->mBufferLoad + cbAudioSamples;

   /* Check for necessary number of samples */
   if (pCodecData->mBufferLoad == 160)
   {
      uint8_t *pPacketCurPtr = (uint8_t*)pCodedData;

      *pcbCodedSize = 0;

      // Add Codec Mode Request (CMR) field: always set to 15
      // as we have no preference which mode to receive.
      *pPacketCurPtr = (15 << 4);
      pPacketCurPtr++;
      (*pcbCodedSize)++;

      // Add actual frame data (TOC and speech frame)
      *pcbCodedSize += Encoder_Interface_Encode(pCodecData->encoder_state, // encoder state
                                                pCodecData->mode,          // encoder mode
                                                pCodecData->mpBuffer,      // speech data
                                                pPacketCurPtr,             // place for encoded data
                                                0);                        // do not force speech
      // Empty speech buffer
      pCodecData->mBufferLoad = 0;

      // Packet is ready to be sent
      *pbSendNow = TRUE;
   } else {
      // Packet is not yet ready to be sent
      *pcbCodedSize = 0;
      *pbSendNow = FALSE;
   }

   return RPLG_SUCCESS;
}

PLG_SINGLE_CODEC(amr);
