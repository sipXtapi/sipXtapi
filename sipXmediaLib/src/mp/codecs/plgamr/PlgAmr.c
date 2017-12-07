//  
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
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

// DEFINES
/// Number of samples in one frame
#define FRAME_SIZE    160

// LOCAL DATA TYPES
/// Storage for encoder data
struct amr_encoder_data
{
   audio_sample_t mpBuffer[FRAME_SIZE]; ///< Buffer used to store input samples
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
/// Default (recommended) fmtp parameters
static const char* defaultFmtps[] =
{
   "octet-align=1"
//   "" // octet-align=0 assumed
};
/// Exported codec information.
static const struct MppCodecInfoV1_1 sgCodecInfo = 
{
///////////// Implementation and codec info /////////////
   "3GPP",                      // codecManufacturer
   "AMR",                       // codecName
   "7.0.0.0",                   // codecVersion
   CODEC_TYPE_FRAME_BASED,      // codecType

/////////////////////// SDP info ///////////////////////
   "AMR",                       // mimeSubtype
   sizeof(defaultFmtps)/sizeof(defaultFmtps[0]), // fmtpsNum
   defaultFmtps,                // fmtps
   8000,                        // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_SPECIAL  // framePacking
};
/// Frame sizes for different codec modes
static const uint8_t sgFrameSizesMap[16] =
{ 13, 14, 16, 18, 20, 21, 27, 32, 6 , 0 , 0 , 0 , 0 , 0 , 0 , 1  };

/* LOCAL FUNCTIONS DECLARATIONS */
static int parseFmtpParams(const char *fmtp, int *octet_align, int *dtx);
DECLARE_FUNCS_V1(amr)

/* ============================== FUNCTIONS =============================== */

CODEC_API int PLG_GET_INFO_V1_1(amr)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1_2(amr)(const char* fmtp, int isDecoder,
                                   struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
   if (pCodecInfo != NULL)
   {
      int octet_align = 0;    /* Octet Aligned mode / Bandwidth Efficient mode */
      int doDtx = 0;          /* DTX / no DTX */
      enum Mode mode = MR122; /* mode to be used by encoder (12.2kbps by default) */

      // Parse fmtp
      parseFmtpParams(fmtp, &octet_align, &doDtx);
      // Check for supported parameter values
      if (octet_align != 1 || doDtx != 0)
      {
         return NULL;
      }

      // Fill in codec info structure with general values.
      pCodecInfo->signalingCodec = FALSE;
      pCodecInfo->numSamplesPerFrame = FRAME_SIZE;
      //   we have PLC, but code should be fixed to really support it.
      pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
      pCodecInfo->vadCng = CODEC_CNG_INTERNAL;
      pCodecInfo->algorithmicDelay = 0;

      // Allocate and fill in codec state structure
      if (isDecoder)
      {
         struct amr_decoder_data *pCodecData;

         // Decoder should be able to decode all modes, so set to full range
         // of bitrates.
         // Note, we support only Octet-Aligned mode and no DTX at the moment.
         pCodecInfo->minFrameBytes = 1 /*CMR*/ + sgFrameSizesMap[MR475] /* frame + ToC */;
         pCodecInfo->maxFrameBytes = 1 /*CMR*/ + sgFrameSizesMap[MR122] /* frame + ToC */;
         pCodecInfo->minBitrate = pCodecInfo->minFrameBytes * 8/*bits/byte*/ * 50/*frames/sec*/;
         pCodecInfo->maxBitrate = pCodecInfo->maxFrameBytes * 8/*bits/byte*/ * 50/*frames/sec*/;

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

         // We will send only packets of requested mode, while we do not support
         // changing mode on the fly and CMR.
         // Note, we support only Octet-Aligned mode and no DTX at the moment.
         pCodecInfo->minFrameBytes = 1 /*CMR*/ + sgFrameSizesMap[mode] /* frame + ToC */;
         pCodecInfo->maxFrameBytes = 1 /*CMR*/ + sgFrameSizesMap[mode] /* frame + ToC */;
         pCodecInfo->minBitrate = pCodecInfo->minFrameBytes * 8/*bits/byte*/ * 50/*frames/sec*/;
         pCodecInfo->maxBitrate = pCodecInfo->maxFrameBytes * 8/*bits/byte*/ * 50/*frames/sec*/;

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

CODEC_API int PLG_GET_PACKET_SAMPLES_V1_2(amr)(void          *handle,
                                               const uint8_t *pPacketData,
                                               unsigned       packetSize,
                                               unsigned      *pNumSamples,
                                               const struct RtpHeader* pRtpHeader)
{
   struct amr_decoder_data *pCodecData = (struct amr_decoder_data *)handle;
   int haveMoreData;   // Payload ToC F bit
   int frameMode;      // Payload ToC FT field
   /*int frameQuality; */  // Payload ToC Q bit
   unsigned dataIndex; // Number of currently processing byte in a packet.

   assert(handle != NULL);

   // There should be at least 2 bytes in a packet - CMR and ToC fields.
   if (packetSize < 2)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   // We do not support Bandwidth Efficient mode now.
   assert(pCodecData->octet_align == 1);

   // Initialize number of decoded samples
   *pNumSamples = 0;

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
      haveMoreData =  (pPacketData[dataIndex]) >> 7;          // Payload ToC F bit
      frameMode    = ((pPacketData[dataIndex]) >> 3) & 0x0F;  // Payload ToC FT field
      /* frameQuality = ((pPacketData[dataIndex]) >> 2) & 0x01; */ // Payload ToC Q bit

      // Jump to the next speech frame and update samples number
      dataIndex += sgFrameSizesMap[frameMode];
      *pNumSamples += FRAME_SIZE;

      // Something is broken if we have gone over the packet data end or got
      // non-existent mode (frame size in sgFrameSizesMap for this mode
      // should not be zero).
      if (dataIndex > packetSize || sgFrameSizesMap[frameMode] == 0)
      {
         return RPLG_CORRUPTED_DATA;
      }
   } while(haveMoreData == 1);

   // Something is broken if packet size does not match its data
   if (dataIndex != packetSize)
   {
      return RPLG_CORRUPTED_DATA;
   }

   return RPLG_SUCCESS;
}

CODEC_API int PLG_DECODE_V1(amr)(void* handle, const  void* pCodedData,
                                 unsigned cbCodedPacketSize, void* pAudioBuffer,
                                 unsigned cbBufferSize, unsigned *pcbDecodedSize,
                                 const struct RtpHeader* pRtpHeader)
{
   struct amr_decoder_data *pCodecData = (struct amr_decoder_data *)handle;
   int haveMoreData;   // Payload ToC F bit
   int frameMode;      // Payload ToC FT field
   /* int frameQuality; */  // Payload ToC Q bit
   unsigned dataIndex; // Number of currently processing byte in a packet.

   assert(handle != NULL);

   // Assert that available buffer size is enough for the packet.
   if (cbBufferSize < FRAME_SIZE)
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

   // Initialize number of decoded samples
   *pcbDecodedSize = 0;

   // Decode all frames in packet.
   do 
   {
      // Parse Table of Contents field
      haveMoreData =  (((uint8_t*)pCodedData)[dataIndex]) >> 7;          // Payload ToC F bit
      frameMode    = ((((uint8_t*)pCodedData)[dataIndex]) >> 3) & 0x0F;  // Payload ToC FT field
      /* frameQuality = ((((uint8_t*)pCodedData)[dataIndex]) >> 2) & 0x01; */ // Payload ToC Q bit

      Decoder_Interface_Decode(pCodecData->decoder_state,       // Decoder state
                               &((uint8_t*)pCodedData)[dataIndex],          // ToC and Speech frame
                               &((int16_t*)pAudioBuffer)[*pcbDecodedSize],  // Decoder data buffer
                               0);                              // This is a good frame
      *pcbDecodedSize += FRAME_SIZE;

      // Jump to the next speech frame
      dataIndex += sgFrameSizesMap[frameMode];
   } while(haveMoreData == 1);

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(amr)(void* handle, const void* pAudioBuffer,
                                 unsigned cbAudioSamples, int* rSamplesConsumed,
                                 void* pCodedData, unsigned cbMaxCodedData,
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
   *rSamplesConsumed = PLG_MIN(cbAudioSamples, FRAME_SIZE-pCodecData->mBufferLoad);
   memcpy(&pCodecData->mpBuffer[pCodecData->mBufferLoad], pAudioBuffer,
          SIZE_OF_SAMPLE*(*rSamplesConsumed));
   pCodecData->mBufferLoad = pCodecData->mBufferLoad + cbAudioSamples;

   /* Check for necessary number of samples */
   if (pCodecData->mBufferLoad == FRAME_SIZE)
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

static const char sgOctetAlignParam[]="octet-align";
static const char sgDtx[]="dtx";

int parseFmtpParams(const char *fmtp, int *octet_align, int *dtx)
{
   const char *sep = fmtp;

   while (sep != NULL && *sep != '\0')
   {
      if (strncmp(sep, sgOctetAlignParam, sizeof(sgOctetAlignParam)-1) == 0)
      {
         if (sep[sizeof(sgOctetAlignParam)-1] != '=')
         {
            return 1;
         }
         *octet_align = atoi(&sep[sizeof(sgOctetAlignParam)]);
         sep += sizeof(sgOctetAlignParam);
      }
      else if (strncmp(sep, sgDtx, sizeof(sgDtx)-1) == 0)
      {
         if (sep[sizeof(sgDtx)-1] != '=')
         {
            return 1;
         }
         *dtx = atoi(&sep[sizeof(sgDtx)]);
         sep += sizeof(sgDtx);
      }

      sep = strchr(sep, ',');
   }

   return 0;
}

PLG_ENUM_CODEC_START(amr)
  PLG_ENUM_CODEC(amr)
  PLG_ENUM_CODEC_SPECIAL_PACKING(amr)
  PLG_ENUM_CODEC_NO_SIGNALING(amr)
PLG_ENUM_CODEC_END 
