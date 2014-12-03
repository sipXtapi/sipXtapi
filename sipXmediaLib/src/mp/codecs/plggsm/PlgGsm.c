//  
// Copyright (C) 2007-2014 SIPez LLC. All rights reserved.
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
// CONSTANTS
#define GSM_FRAME_BYTES   33
#define GSM_FRAME_SAMPLES 160

// LOCAL DATA TYPES
struct libgsm_codec_data
{
   audio_sample_t mpBuffer[GSM_FRAME_SAMPLES]; ///< Buffer used to store input samples
   int mBufferLoad;                            ///< How much data there is in the buffer
   gsm mpGsmState;
   int mFrameCount;
};

// EXTERNAL FUNCTIONS
CODEC_API void* sipxGsmCommonInit(const char* fmtp, int isDecoder,
                                  struct MppCodecFmtpInfoV1_2* pCodecInfo);
CODEC_API int sipxGsmCommonFree(void* handle, int isDecoder);
CODEC_API int sipxGsmCommonEncode(void* handle, 
                                  const void* pAudioBuffer, 
                                  unsigned cbAudioSamples, 
                                  int* rSamplesConsumed, 
                                  void* pCodedData, 
                                  unsigned cbMaxCodedData, 
                                  int* pcbCodedSize, 
                                  unsigned* pbSendNow);
CODEC_API int sipxGsmCommonDecode(void* handle,
                                  int isWave,
                                  const void* pCodedData,
                                  unsigned cbCodedPacketSize,
                                  void* pAudioBuffer,
                                  unsigned cbBufferSize,
                                  unsigned *pcbDecodedSize,
                                  const struct RtpHeader* pRtpHeader);

DECLARE_FUNCS_V1(libgsm)
DECLARE_FUNCS_V1(libgsm_wave)

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
static const struct MppCodecInfoV1_1 sgCodecInfoWave = 
{
///////////// Implementation and codec info /////////////
   "libgsm",                    // codecManufacturer
   "GSM-FR (06.10)",            // codecName
   "12",                        // codecVersion
   CODEC_TYPE_FRAME_BASED,      // codecType

/////////////////////// SDP info ///////////////////////
   "GSM_WAVE",                  // mimeSubtype
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

CODEC_API int PLG_GET_INFO_V1_1(libgsm_wave)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfoWave;
   }
   return RPLG_SUCCESS;
}

CODEC_API void* sipxGsmCommonInit(const char* fmtp, int isDecoder,
                                      struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
   struct libgsm_codec_data *mpGsm;
   if (pCodecInfo == NULL)
   {
      return NULL;
   }
   pCodecInfo->signalingCodec = FALSE;
   pCodecInfo->minBitrate = 13200;
   pCodecInfo->maxBitrate = 13200;
   pCodecInfo->numSamplesPerFrame = GSM_FRAME_SAMPLES;
   pCodecInfo->minFrameBytes = GSM_FRAME_BYTES;
   pCodecInfo->maxFrameBytes = GSM_FRAME_BYTES;
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
   pCodecInfo->vadCng = CODEC_CNG_NONE;
   pCodecInfo->algorithmicDelay = 0;

   mpGsm = (struct libgsm_codec_data *)malloc(sizeof(struct libgsm_codec_data));
   if (!mpGsm)
   {
      return NULL;
   }

   mpGsm->mBufferLoad = 0;
   mpGsm->mpGsmState = gsm_create();
   mpGsm->mFrameCount = 0;

   return mpGsm;
}

CODEC_API void* PLG_INIT_V1_2(libgsm)(const char* fmtp, int isDecoder,
                                      struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
    return(sipxGsmCommonInit(fmtp,
                         isDecoder,
                         pCodecInfo));
}

CODEC_API void* PLG_INIT_V1_2(libgsm_wave)(const char* fmtp, int isDecoder,
                                      struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
    struct libgsm_codec_data* mpGsm = 
        sipxGsmCommonInit(fmtp,
                      isDecoder,
                      pCodecInfo);

    // All of the cloning in here is to set the different packing option for
    // GSM which is stored in a wave file.  The packing is different and the
    // frames are alternating 32 and 33 bytes where RTP is a constant 33 bytes.
    int packForWave = 1;
    int gsmStatus = gsm_option(mpGsm->mpGsmState, GSM_OPT_WAV49, &packForWave);
    assert(gsmStatus >= 0);

    return(mpGsm);
}


CODEC_API int sipxGsmCommonFree(void* handle, int isDecoder)
{
   struct libgsm_codec_data *mpGsm = (struct libgsm_codec_data *)handle;

   if (NULL != handle)
   {
      gsm_destroy(mpGsm->mpGsmState);
      free(handle);
   }
   return RPLG_SUCCESS;
}

CODEC_API int PLG_FREE_V1(libgsm)(void* handle, int isDecoder)
{
    return(sipxGsmCommonFree(handle, isDecoder));
}

CODEC_API int PLG_FREE_V1(libgsm_wave)(void* handle, int isDecoder)
{
    return(sipxGsmCommonFree(handle, isDecoder));
}

CODEC_API int PLG_DECODE_V1(libgsm)(void* handle,
                                    const void* pCodedData,
                                    unsigned cbCodedPacketSize,
                                    void* pAudioBuffer,
                                    unsigned cbBufferSize,
                                    unsigned *pcbDecodedSize,
                                    const struct RtpHeader* pRtpHeader)
{
    return(sipxGsmCommonDecode(handle,
                               FALSE,
                               pCodedData,
                               cbCodedPacketSize,
                               pAudioBuffer,
                               cbBufferSize,
                               pcbDecodedSize,
                               pRtpHeader));
}

CODEC_API int PLG_DECODE_V1(libgsm_wave)(void* handle,
                                         const void* pCodedData,
                                         unsigned cbCodedPacketSize,
                                         void* pAudioBuffer,
                                         unsigned cbBufferSize,
                                         unsigned *pcbDecodedSize,
                                         const struct RtpHeader* pRtpHeader)
{
    return(sipxGsmCommonDecode(handle,
                               TRUE,
                               pCodedData,
                               cbCodedPacketSize,
                               pAudioBuffer,
                               cbBufferSize,
                               pcbDecodedSize,
                               pRtpHeader));
}

CODEC_API int sipxGsmCommonDecode(void* handle,
                                  int isWave,
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
   unsigned outSize   = ( cbCodedPacketSize / GSM_FRAME_BYTES ) * GSM_FRAME_SAMPLES;
   unsigned remaining = cbCodedPacketSize; 

   assert(handle != NULL);
   /* Assert that payload have correct size. */
   if ( (cbCodedPacketSize % GSM_FRAME_BYTES) != 0 )
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
       (mpGsm->mFrameCount)++;
       if(isWave && ((mpGsm->mFrameCount & 0x1)) || !isWave)
       {
           pIn += GSM_FRAME_BYTES;
       }
       else
       {
           pIn += GSM_FRAME_BYTES - 1;
       }

       pIn += GSM_FRAME_BYTES;
       pOut += GSM_FRAME_SAMPLES;
       remaining -= GSM_FRAME_BYTES; 
   }

   *pcbDecodedSize = outSize; 

   return RPLG_SUCCESS;
}



CODEC_API int sipxGsmCommonEncode(void* handle, 
                                  const void* pAudioBuffer, 
                                  unsigned cbAudioSamples, 
                                  int* rSamplesConsumed, 
                                  void* pCodedData, 
                                  unsigned cbMaxCodedData, 
                                  int* pcbCodedSize, 
                                  unsigned* pbSendNow)
{
   struct libgsm_codec_data *mpGsm = (struct libgsm_codec_data *)handle;
   assert(handle != NULL);
   if (cbMaxCodedData < GSM_FRAME_BYTES)
   {
      return RPLG_INVALID_ARGUMENT;
   }
   memcpy(&mpGsm->mpBuffer[mpGsm->mBufferLoad], pAudioBuffer, SIZE_OF_SAMPLE*cbAudioSamples);
   mpGsm->mBufferLoad = mpGsm->mBufferLoad + cbAudioSamples;

   assert(mpGsm->mBufferLoad <= GSM_FRAME_SAMPLES);

   /* Check for necessary number of samples */
   if (mpGsm->mBufferLoad == GSM_FRAME_SAMPLES)
   {
      gsm_encode(mpGsm->mpGsmState, (gsm_signal*)mpGsm->mpBuffer, (gsm_byte*)pCodedData);
      mpGsm->mBufferLoad = 0;
      (mpGsm->mFrameCount)++;
      *pcbCodedSize = GSM_FRAME_BYTES;
      *pbSendNow = TRUE;
   } else {
      *pcbCodedSize = 0;
      *pbSendNow = FALSE;
   }

   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(libgsm)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData,
                            unsigned cbMaxCodedData, int* pcbCodedSize, unsigned* pbSendNow)
{
    return(sipxGsmCommonEncode(handle,
                               pAudioBuffer,
                               cbAudioSamples,
                               rSamplesConsumed,
                               pCodedData,
                               cbMaxCodedData,
                               pcbCodedSize,
                               pbSendNow));
}

CODEC_API int PLG_ENCODE_V1(libgsm_wave)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData,
                            unsigned cbMaxCodedData, int* pcbCodedSize, unsigned* pbSendNow)
{
    int status = (sipxGsmCommonEncode(handle,
                                      pAudioBuffer,
                                      cbAudioSamples,
                                      rSamplesConsumed,
                                      pCodedData,
                                      cbMaxCodedData,
                                      pcbCodedSize,
                                      pbSendNow));
    if(*pcbCodedSize)
    {
        assert(*pcbCodedSize == GSM_FRAME_BYTES);
        if(((struct libgsm_codec_data*)handle)->mFrameCount & 0x1)
        {
            // GSM wave file packing has alternate frames of 32
            *pcbCodedSize = GSM_FRAME_BYTES - 1;
        }
        // else framesize already set to 33
    }

    return(status);
}

PLG_ENUM_CODEC_START(libgsm)
  PLG_ENUM_CODEC(libgsm)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(libgsm)
  PLG_ENUM_CODEC_NO_SIGNALING(libgsm)
  PLG_ENUM_CODEC(libgsm_wave)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(libgsm_wave)
  PLG_ENUM_CODEC_NO_SIGNALING(libgsm_wave)
PLG_ENUM_CODEC_END 

