//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

/**
*  @file
*
*  This is AMR-WB (wide-band) encoder/decoder wrapper.
*
*  @see Read RFC4867 for payload format description and 3gpp specifications
*       for more information about AMR-WB codec.
*
*  @note fmtp string is not really parsed. We silently assume default settings:
*        * Octet Aligned mode;
*        * DTX disabled;
*        * all optional features disabled;
*        * only one frame per packet;
*        * encoder use 23.85kbps mode;
*        * encoder set CMR field in payload header to 15 as we have no preference
*          which mode to receive.
*/

// WIN32: Add libamrwb to linker input.
#ifdef WIN32 // [
#   pragma comment(lib, "libamrwb.lib")
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
#include <enc_if.h>
#include <dec_if.h>

// DEFINES
/// AMR-WB modes
//@{
#define MODE_7k       0
#define MODE_9k       1
#define MODE_12k      2
#define MODE_14k      3
#define MODE_16k      4
#define MODE_18k      5
#define MODE_20k      6
#define MODE_23k      7
#define MODE_24k      8
//@}

/// Number of samples in one frame
#define FRAME_SIZE    320

// LOCAL DATA TYPES
/// Storage for encoder data
struct amrwb_encoder_data
{
   audio_sample_t mpBuffer[FRAME_SIZE]; ///< Buffer used to store input samples
   unsigned mBufferLoad;            ///< Number of samples stored in the buffer

   void *encoder_state;             ///< Pointer to encoder/decoder state structure
   int octet_align;                 ///< 1 - Octet Aligned mode, 0 - Bandwidth Efficient mode
   int doDtx;                       ///< 1- use DTX, 0 - no DTX
   int mode;                        ///< Encoder mode (bitrate).
};

/// Storage for decoder data
struct amrwb_decoder_data
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
   "AMR-WB",                    // codecName
   "7.0.0.2",                   // codecVersion
   CODEC_TYPE_FRAME_BASED,      // codecType

/////////////////////// SDP info ///////////////////////
   "AMR-WB",                       // mimeSubtype
   sizeof(defaultFmtps)/sizeof(defaultFmtps[0]), // fmtpsNum
   defaultFmtps,                // fmtps
   16000,                       // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_SPECIAL  // framePacking
};
/// Frame sizes for different codec modes
static const uint8_t sgFrameSizesMap[16] =
{18, 24, 33, 37, 41, 47, 51, 59, 61, 6, 6, 0, 0, 0, 1, 1};


CODEC_API int PLG_GET_INFO_V1_1(amrwb)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1_1(amrwb)(const char* fmtp, int isDecoder,
                                     struct MppCodecFmtpInfoV1_1* pCodecInfo)
{
   if (pCodecInfo != NULL)
   {
      int octet_align = 1;   /* Octet Aligned mode / Bandwidth Efficient mode */
      int mode = MODE_24k;   /* mode to be used by encoder (23.85kbps by default) */

      /*
      *             Here fmtp should be parsed.
      *
      * Now we silently assume that Octet Aligned mode is used and DTX is not
      * enabled.
      */

      // Fill in codec info structure with general values.
      pCodecInfo->signalingCodec = FALSE;
      pCodecInfo->numSamplesPerFrame = FRAME_SIZE;
      //   we have PLC, but code should be fixed to really support it.
      pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
      pCodecInfo->vadCng = CODEC_CNG_INTERNAL;

      // Allocate and fill in codec state structure
      if (isDecoder)
      {
         struct amrwb_decoder_data *pCodecData;

         // Decoder should be able to decode all modes, so set to full range
         // of bitrates.
         // Note, we support only Octet-Aligned mode and no DTX at the moment.
         pCodecInfo->minFrameBytes = 1 /*CMR*/ + sgFrameSizesMap[MODE_7k] /* frame + ToC */;
         pCodecInfo->maxFrameBytes = 1 /*CMR*/ + sgFrameSizesMap[MODE_24k] /* frame + ToC */;
         pCodecInfo->minBitrate = pCodecInfo->minFrameBytes * 8/*bits/byte*/ * 50/*frames/sec*/;
         pCodecInfo->maxBitrate = pCodecInfo->maxFrameBytes * 8/*bits/byte*/ * 50/*frames/sec*/;

         pCodecData = (struct amrwb_decoder_data *)malloc(sizeof(struct amrwb_decoder_data));
         if (!pCodecData)
         {
            return NULL;
         }

         pCodecData->octet_align = octet_align;
         pCodecData->decoder_state = D_IF_init();
         return pCodecData;
      } 
      else
      {
         struct amrwb_encoder_data *pCodecData;

         // We will send only packets of requested mode, while we do not support
         // changing mode on the fly and CMR.
         // Note, we support only Octet-Aligned mode and no DTX at the moment.
         pCodecInfo->minFrameBytes = 1 /*CMR*/ + sgFrameSizesMap[mode] /* frame + ToC */;
         pCodecInfo->maxFrameBytes = 1 /*CMR*/ + sgFrameSizesMap[mode] /* frame + ToC */;
         pCodecInfo->minBitrate = pCodecInfo->minFrameBytes * 8/*bits/byte*/ * 50/*frames/sec*/;
         pCodecInfo->maxBitrate = pCodecInfo->maxFrameBytes * 8/*bits/byte*/ * 50/*frames/sec*/;

         pCodecData = (struct amrwb_encoder_data *)malloc(sizeof(struct amrwb_encoder_data));
         if (!pCodecData)
         {
            return NULL;
         }

         pCodecData->mBufferLoad = 0;
         pCodecData->octet_align = octet_align;
         pCodecData->doDtx = 0; // No DTX supported
         pCodecData->mode = mode;
         pCodecData->encoder_state = E_IF_init();
         return pCodecData;
      }
   }

   return NULL;
}


CODEC_API int PLG_FREE_V1(amrwb)(void* handle, int isDecoder)
{
   if (isDecoder)
   {
      struct amrwb_decoder_data *pCodecData = (struct amrwb_decoder_data *)handle;

      if (NULL != handle)
      {
         D_IF_exit(pCodecData->decoder_state);
         free(handle);
      }
   } 
   else
   {
      struct amrwb_encoder_data *pCodecData = (struct amrwb_encoder_data *)handle;

      if (NULL != handle)
      {
         E_IF_exit(pCodecData->encoder_state);
         free(handle);
      }
   }

   return 0;
}

CODEC_API int PLG_GET_PACKET_SAMPLES_V1_2(amrwb)(void          *handle,
                                                 const uint8_t *pPacketData,
                                                 unsigned       packetSize,
                                                 unsigned      *pNumSamples,
                                                 const struct RtpHeader* pRtpHeader)
{
   struct amrwb_decoder_data *pCodecData = (struct amrwb_decoder_data *)handle;
   int haveMoreData;   // Payload ToC F bit
   int frameMode;      // Payload ToC FT field
   int frameQuality;   // Payload ToC Q bit
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
      frameQuality = ((pPacketData[dataIndex]) >> 2) & 0x01;  // Payload ToC Q bit

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

CODEC_API int PLG_DECODE_V1(amrwb)(void* handle, const uint8_t* pCodedData,
                                   unsigned cbCodedPacketSize, uint16_t* pAudioBuffer,
                                   unsigned cbBufferSize, unsigned *pcbDecodedSize,
                                   const struct RtpHeader* pRtpHeader)
{
   struct amrwb_decoder_data *pCodecData = (struct amrwb_decoder_data *)handle;
   int haveMoreData;   // Payload ToC F bit
   int frameMode;      // Payload ToC FT field
   int frameQuality;   // Payload ToC Q bit
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
      haveMoreData =  (pCodedData[dataIndex]) >> 7;          // Payload ToC F bit
      frameMode    = ((pCodedData[dataIndex]) >> 3) & 0x0F;  // Payload ToC FT field
      frameQuality = ((pCodedData[dataIndex]) >> 2) & 0x01;  // Payload ToC Q bit

      GP3D_IF_decode(pCodecData->decoder_state,       // Decoder state
                     &pCodedData[dataIndex],          // ToC and Speech frame
                     &pAudioBuffer[*pcbDecodedSize],  // Decoder data buffer
                     _good_frame);                    // This is a good frame
      *pcbDecodedSize += FRAME_SIZE;

      // Jump to the next speech frame
      dataIndex += sgFrameSizesMap[frameMode];
   } while(haveMoreData == 1);

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(amrwb)(void* handle, const void* pAudioBuffer,
                                   unsigned cbAudioSamples, int* rSamplesConsumed,
                                   uint8_t* pCodedData, unsigned cbMaxCodedData,
                                   int* pcbCodedSize, unsigned* pbSendNow)
{
   struct amrwb_encoder_data *pCodecData = (struct amrwb_encoder_data *)handle;

   assert(handle != NULL);

   if (cbMaxCodedData < 62)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   // We do not support Bandwidth Efficient mode now.
   assert(pCodecData->octet_align == 1);

   // Calculate amount of audio data to be consumed and store it
   // to intermediate buffer.
   *rSamplesConsumed = min(cbAudioSamples, FRAME_SIZE-pCodecData->mBufferLoad);
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
      *pcbCodedSize += GP3E_IF_encode(pCodecData->encoder_state, // encoder state
                                      pCodecData->mode,          // encoder mode
                                      pCodecData->mpBuffer,      // speech data
                                      pPacketCurPtr,             // place for encoded data
                                      pCodecData->doDtx);        // DTX
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

PLG_ENUM_CODEC_START(amrwb)
  PLG_ENUM_CODEC(amrwb)
  PLG_ENUM_CODEC_SPECIAL_PACKING(amrwb)
  PLG_ENUM_CODEC_NO_SIGNALING(amrwb)
PLG_ENUM_CODEC_END 
