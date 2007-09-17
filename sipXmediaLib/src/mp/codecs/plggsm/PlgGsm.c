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

// WIN32: Add libgsm to linker input.
#ifdef WIN32 // [
#   ifdef _DEBUG // [
#      pragma comment(lib, "gsmd.lib")
#   else // _DEBUG ][
#      pragma comment(lib, "gsm.lib")
#   endif // _DEBUG ]
#endif // WIN32 ]

// APPLICATION INCLUDES
#include <gsm.h>

// LOCAL DATA TYPES
struct libgsm_codec_data
{
   audio_sample_t mpBuffer[160];    ///< Buffer used to store input samples
   int mBufferLoad;                 ///< How much data there is in the buffer
   gsm mpGsmState;
};

// STATIC VARIABLES INITIALIZATON
static const char codecMIMEsubtype[] = "gsm";
static const struct plgCodecInfoV1 codecGSM = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   codecMIMEsubtype,                //mimeSubtype
   "libgsm",                        //codecName
   "GSM 6.10",                      //codecVersion
   8000,                            //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   160,                             //interleaveBlockSize
   13200,                           //bitRate
   33*8,                            //minPacketBits
   33*8,                            //avgPacketBits
   33*8,                            //maxPacketBits
   160,                             //numSamplesPerFrame
   6                                //preCodecJitterBufferSize
};

CODEC_API int PLG_ENUM_V1(libgsm)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
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

CODEC_API void *PLG_INIT_V1(libgsm)(const char* fmt, int bDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   struct libgsm_codec_data *mpGsm;
   if (pCodecInfo == NULL) {
      return NULL;
   }

   memcpy(pCodecInfo, &codecGSM, sizeof(struct plgCodecInfoV1));
   mpGsm = (struct libgsm_codec_data *)malloc(sizeof(struct libgsm_codec_data));
   if (!mpGsm) {
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

CODEC_API int PLG_DECODE_V1(libgsm)(void* handle, const void* pCodedData, unsigned cbCodedPacketSize, void* pAudioBuffer, unsigned cbBufferSize, unsigned *pcbDecodedSize, const struct RtpHeader* pRtpHeader)
{
   struct libgsm_codec_data *mpGsm = (struct libgsm_codec_data *)handle;
   assert(handle != NULL);
   /* Assert that available buffer size is enough for the packet. */
   if (cbCodedPacketSize != 33)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   if (cbBufferSize < 160)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   gsm_decode(mpGsm->mpGsmState, (gsm_byte*)pCodedData, (gsm_signal*)pAudioBuffer);
   *pcbDecodedSize = 160;

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(libgsm)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData,
                            unsigned cbMaxCodedData, int* pcbCodedSize, unsigned* pbSendNow)
{
   int size = 0;   
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
      size = 33;
      gsm_encode(mpGsm->mpGsmState, (gsm_signal*)mpGsm->mpBuffer, (gsm_byte*)pCodedData);
      mpGsm->mBufferLoad = 0;
      *pbSendNow = TRUE;
   } else {
      *pbSendNow = FALSE;
   }

   *rSamplesConsumed = cbAudioSamples;
   *pcbCodedSize = size;

   return RPLG_SUCCESS;
}

PLG_SINGLE_CODEC(libgsm);
