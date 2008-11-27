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

// WIN32: Add libgsm to linker input.
#ifdef WIN32 // [
#   ifdef _DEBUG // [
#      pragma comment(lib, "gsmd.lib")
#   else // _DEBUG ][
#      pragma comment(lib, "gsm.lib")
#   endif // _DEBUG ]
#endif // WIN32 ]

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// CODEC LIBRARY INCLUDES
#include <gsm.h>

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// LOCAL DATA TYPES
struct libgsm_codec_data
{
   audio_sample_t mpBuffer[160];    ///< Buffer used to store input samples
   int mBufferLoad;                 ///< How much data there is in the buffer
   gsm mpGsmState;
};

// EXTERNAL FUNCTIONS
// DEFINES
// STATIC VARIABLES INITIALIZATON
/// Exported codec information.
static const struct MppCodecInfoV1_1 sgCodecInfo = 
{
///////////// Implementation and codec info /////////////
   "libgsm",                    // codecManufacturer
   "GSM-FR (06.10)",            // codecName
   "12",                        // codecVersion
   CODEC_TYPE_FRAME_BASED,      // codecType

/////////////////////// SDP info ///////////////////////
   "GSM",                       // mimeSubtype
   0,                           // fmtpsNum
   NULL,                        // fmtps
   8000,                        // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_NONE     // framePacking
};

/* ============================== FUNCTIONS =============================== */

CODEC_API int PLG_GET_INFO_V1_1(libgsm)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1_1(libgsm)(const char* fmtp, int isDecoder,
                                      struct MppCodecFmtpInfoV1_1* pCodecInfo)
{
   struct libgsm_codec_data *mpGsm;
   if (pCodecInfo == NULL)
   {
      return NULL;
   }
   pCodecInfo->signalingCodec = FALSE;
   pCodecInfo->minBitrate = 13200;
   pCodecInfo->maxBitrate = 13200;
   pCodecInfo->numSamplesPerFrame = 160;
   pCodecInfo->minFrameBytes = 33;
   pCodecInfo->maxFrameBytes = 33;
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
   pCodecInfo->vadCng = CODEC_CNG_NONE;

   mpGsm = (struct libgsm_codec_data *)malloc(sizeof(struct libgsm_codec_data));
   if (!mpGsm)
   {
      return NULL;
   }

   mpGsm->mBufferLoad = 0;
   mpGsm->mpGsmState = gsm_create();

   return mpGsm;
}


CODEC_API int PLG_FREE_V1(libgsm)(void* handle, int isDecoder)
{
   struct libgsm_codec_data *mpGsm = (struct libgsm_codec_data *)handle;

   if (NULL != handle)
   {
      gsm_destroy(mpGsm->mpGsmState);
      free(handle);
   }
   return RPLG_SUCCESS;
}

CODEC_API int PLG_DECODE_V1(libgsm)(void* handle,
                                    const void* pCodedData,
                                    unsigned cbCodedPacketSize,
                                    void* pAudioBuffer,
                                    unsigned cbBufferSize,
                                    unsigned *pcbDecodedSize,
                                    const struct RtpHeader* pRtpHeader)
{
   struct libgsm_codec_data *mpGsm = (struct libgsm_codec_data *)handle;
   gsm_byte *pIn = (gsm_byte * ) pCodedData;
   gsm_signal *pOut = ( gsm_signal * ) pAudioBuffer;
   unsigned outSize   = ( cbCodedPacketSize / 33 ) * 160;
   unsigned remaining = cbCodedPacketSize; 

   assert(handle != NULL);
   /* Assert that payload have correct size. */
   if ( (cbCodedPacketSize % 33) != 0 )
   {
      return RPLG_CORRUPTED_DATA;
   }

   /* Assert that available buffer size is enough for the packet. */
   if (cbBufferSize < outSize )
   {
      return RPLG_BUFFER_TOO_SMALL;
   }

   while ( remaining > 0 )
   {
       gsm_decode(mpGsm->mpGsmState, pIn, pOut);
       pIn += 33;
       pOut +=160;
       remaining -= 33; 
   }

   *pcbDecodedSize = outSize; 

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(libgsm)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData,
                            unsigned cbMaxCodedData, int* pcbCodedSize, unsigned* pbSendNow)
{
   struct libgsm_codec_data *mpGsm = (struct libgsm_codec_data *)handle;
   assert(handle != NULL);
   if (cbMaxCodedData < 33)
   {
      return RPLG_INVALID_ARGUMENT;
   }
   memcpy(&mpGsm->mpBuffer[mpGsm->mBufferLoad], pAudioBuffer, SIZE_OF_SAMPLE*cbAudioSamples);
   mpGsm->mBufferLoad = mpGsm->mBufferLoad + cbAudioSamples;

   assert(mpGsm->mBufferLoad <= 160);

   /* Check for necessary number of samples */
   if (mpGsm->mBufferLoad == 160)
   {
      gsm_encode(mpGsm->mpGsmState, (gsm_signal*)mpGsm->mpBuffer, (gsm_byte*)pCodedData);
      mpGsm->mBufferLoad = 0;
      *pcbCodedSize = 33;
      *pbSendNow = TRUE;
   } else {
      *pcbCodedSize = 0;
      *pbSendNow = FALSE;
   }

   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}

PLG_ENUM_CODEC_START(libgsm)
  PLG_ENUM_CODEC(libgsm)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(libgsm)
  PLG_ENUM_CODEC_NO_SIGNALING(libgsm)
PLG_ENUM_CODEC_END 
