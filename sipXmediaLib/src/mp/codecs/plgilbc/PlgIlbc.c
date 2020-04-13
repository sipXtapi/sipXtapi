//  
// Copyright (C) 2007-2020 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

// SYSTEM INCLUDES
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// CODEC LIBRARY INCLUDES
#include <iLBC_define.h>
#include <iLBC_decode.h>
#include <iLBC_encode.h>

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// LOCAL DATA TYPES
struct iLBC_codec_data {
   int mMode;                           ///< 20ms frames or 30ms frames
   struct iLBC_Dec_Inst_t_ *mpStateDec; ///< Internal iLBC decoder state.
   audio_sample_t mpBuffer[240];        ///< Buffer used to store input samples
   int mBufferLoad;                     ///< How much data there is in the buffer
   struct iLBC_Enc_Inst_t_* mpStateEnc; ///< Internal iLBC decoder state.
};

// EXTERNAL FUNCTIONS
// MACROS
#define sipx_min(x,y) (((x)<(y))?(x):(y))

// DEFINES
#define DEBUG_PRINT
#undef  DEBUG_PRINT

// WIN32 dev does not facilitate circular shared lib dependencies
// So for now take a short cut
#if defined(WIN32)
#  define mppLogError printf
#endif

// STATIC VARIABLES INITIALIZATON
static const char* defaultFmtps[] = {
   "mode=30",
   "mode=20"
};
/// Exported codec information.
static const struct MppCodecInfoV1_1 sgCodecInfo = 
{
///////////// Implementation and codec info /////////////
   "The Internet Society",      // codecManufacturer
   "iLBC",                      // codecName
   "RFC3951",                   // codecVersion
   CODEC_TYPE_FRAME_BASED,      // codecType

/////////////////////// SDP info ///////////////////////
   "iLBC",                      // mimeSubtype
   sizeof(defaultFmtps)/sizeof(defaultFmtps[0]), // fmtpsNum
   defaultFmtps,                // fmtps
   8000,                        // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_NONE     // framePacking
};

DECLARE_FUNCS_V1(ilbc)

/* ============================== FUNCTIONS =============================== */

CODEC_API int PLG_GET_INFO_V1_1(ilbc)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1_2(ilbc)(const char* fmtp, int isDecoder,
                                    struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
   int mode;
   struct iLBC_codec_data *mpiLBC;

   if (pCodecInfo == NULL)
   {
      return NULL;
   }

   if(fmtp == NULL || fmtp[0] == '\0')
   {
      mode = 30;
   }
   else
   {
      mode = getFmtpValueRange(fmtp, "mode", 30, 20, 30);
      // If mode is unspecified, we assume 30
      if(mode == -1)
      {
          mode = 30;
      }
   }
   if (mode != 20 && mode != 30)
   {
      return NULL;
   }

   pCodecInfo->signalingCodec = FALSE;
   if (mode == 20)
   {
      pCodecInfo->minFrameBytes = NO_OF_BYTES_20MS;
      pCodecInfo->maxFrameBytes = NO_OF_BYTES_20MS;
      pCodecInfo->minBitrate = NO_OF_BYTES_20MS*8*1000/20;
      pCodecInfo->maxBitrate = NO_OF_BYTES_20MS*8*1000/20;
      pCodecInfo->numSamplesPerFrame = 160;
      if (isDecoder) {
         pCodecInfo->algorithmicDelay = 0;
      } else {
         pCodecInfo->algorithmicDelay = 5 /* ms */ * 8 /* samples/ms */;
      }
   } 
   else
   {
      pCodecInfo->minFrameBytes = NO_OF_BYTES_30MS;
      pCodecInfo->maxFrameBytes = NO_OF_BYTES_30MS;
      pCodecInfo->minBitrate = NO_OF_BYTES_30MS*8*1000/33;
      pCodecInfo->maxBitrate = NO_OF_BYTES_30MS*8*1000/33;
      pCodecInfo->numSamplesPerFrame = 240;
      if (isDecoder) {
         pCodecInfo->algorithmicDelay = 0;
      } else {
         pCodecInfo->algorithmicDelay = 10 /* ms */ * 8 /* samples/ms */;
      }
   }
   pCodecInfo->packetLossConcealment = CODEC_PLC_INTERNAL;
   pCodecInfo->vadCng = CODEC_CNG_NONE;

   mpiLBC = (struct iLBC_codec_data *)malloc(sizeof(struct iLBC_codec_data));
   if (!mpiLBC) {
      return NULL;
   }

   mpiLBC->mpStateDec = NULL;
   mpiLBC->mpStateEnc = NULL;

   mpiLBC->mBufferLoad = 0;
   mpiLBC->mMode = mode;

   if (isDecoder) {
      /* Preparing decoder */
      mpiLBC->mpStateDec = (struct iLBC_Dec_Inst_t_*)malloc(sizeof(struct iLBC_Dec_Inst_t_));
      if (mpiLBC->mpStateDec == NULL) {
         free(mpiLBC);
         return NULL;
      }
      memset(mpiLBC->mpStateDec, 0, sizeof(*mpiLBC->mpStateDec));
      initDecode(mpiLBC->mpStateDec, mpiLBC->mMode, 1);
   } else {
      /* Preparing encoder */
      mpiLBC->mpStateEnc = (struct  iLBC_Enc_Inst_t_*)malloc(sizeof(struct iLBC_Enc_Inst_t_));
      if (mpiLBC->mpStateEnc == NULL) {
         free(mpiLBC);
         return NULL;
      }
      memset(mpiLBC->mpStateEnc, 0, sizeof(*mpiLBC->mpStateEnc));
      initEncode(mpiLBC->mpStateEnc, mpiLBC->mMode);
   }

   return mpiLBC;
}


CODEC_API int PLG_FREE_V1(ilbc)(void* handle, int isDecoder)
{
   struct iLBC_codec_data *mpiLBC = (struct iLBC_codec_data *)handle;

   if (NULL != handle)
   {
      if (isDecoder) {
         /* UnPreparing decoder */
         free(mpiLBC->mpStateDec);
      } else {
         free(mpiLBC->mpStateEnc);
      }
      free(handle);
   }
   return 0;
}

CODEC_API int PLG_DECODE_V1(ilbc)(void* handle, const void* pCodedData, unsigned cbCodedPacketSize, void* pAudioBuffer, unsigned cbBufferSize, unsigned *pcbDecodedSize, const struct RtpHeader* pRtpHeader)
{
   int i;
   float buffer[240];
   audio_sample_t* samplesBuffer = (audio_sample_t*)pAudioBuffer;
   struct iLBC_codec_data *mpiLBC = (struct iLBC_codec_data *)handle;
   assert(handle != NULL);

#ifdef DEBUG_PRINT /* [ */
   printf("iLBC decoder(%p): ", handle);
#endif /* DEBUG_PRINT ] */

   // Check if available buffer size is enough for the packet.
   if (cbBufferSize < (unsigned)mpiLBC->mMode * 8)
   {
      mppLogError("iLBC decoder: Jitter buffer overloaded. Glitch!\n");
      return RPLG_FAILED;
   }

   // Decode incoming packet to temp buffer. If no packet - do PLC.  
   if (pCodedData) {
      if (((NO_OF_BYTES_30MS != cbCodedPacketSize) && (mpiLBC->mMode == 30)) ||
         ((NO_OF_BYTES_20MS != cbCodedPacketSize) && (mpiLBC->mMode == 20)))
      {
         mppLogError("iLBC decoder: wrong decoder type or packet size: %d mode: %d!\n",
               cbCodedPacketSize,
               mpiLBC->mMode);
         return RPLG_FAILED;
      }
      // Packet data available. Decode it.
      iLBC_decode(buffer, (unsigned char*)pCodedData, mpiLBC->mpStateDec, 1);
   }
   else
   {
      // Packet data is not available. Do PLC.
      iLBC_decode(buffer, NULL, mpiLBC->mpStateDec, 0);
   }

   for (i = 0; i <  mpiLBC->mMode * 8; ++i)
   {
      float tmp = buffer[i];
      if (tmp > SHRT_MAX)
         tmp = SHRT_MAX;
      if (tmp < SHRT_MIN)
         tmp = SHRT_MIN;

      samplesBuffer[i] = (audio_sample_t)(tmp + 0.5f);
   }

   *pcbDecodedSize = mpiLBC->mMode * 8;

#ifdef DEBUG_PRINT /* [ */
   printf("ok, %d samples decoded\n", *pcbDecodedSize);
#endif /* DEBUG_PRINT ] */

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(ilbc)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData,
                         unsigned cbMaxCodedData, int* pcbCodedSize, unsigned* pbSendNow)
{
   struct iLBC_codec_data *mpiLBC = (struct iLBC_codec_data *)handle;
   assert(handle != NULL);

   *rSamplesConsumed = sipx_min(cbAudioSamples, mpiLBC->mMode*8 - mpiLBC->mBufferLoad);

   memcpy(&mpiLBC->mpBuffer[mpiLBC->mBufferLoad], pAudioBuffer,
          *rSamplesConsumed*SIZE_OF_SAMPLE);
   mpiLBC->mBufferLoad += *rSamplesConsumed;
   assert(mpiLBC->mBufferLoad <= mpiLBC->mMode * 8);

   if (mpiLBC->mBufferLoad == mpiLBC->mMode * 8)
   {
      int i;
      float buffer[240];
      for (i = 0; i < mpiLBC->mMode * 8; ++i)
         buffer[i] =  (float)mpiLBC->mpBuffer[i];

      iLBC_encode((unsigned char*)pCodedData, buffer, mpiLBC->mpStateEnc);

      mpiLBC->mBufferLoad = 0;
      *pcbCodedSize = (mpiLBC->mMode == 30) ? NO_OF_BYTES_30MS : NO_OF_BYTES_20MS;
      *pbSendNow = TRUE;
   } 
   else 
   {
      *pcbCodedSize = 0;
      *pbSendNow = FALSE;
   }

   return RPLG_SUCCESS;
}

PLG_ENUM_CODEC_START(ilbc)
  PLG_ENUM_CODEC(ilbc)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(ilbc)
  PLG_ENUM_CODEC_NO_SIGNALING(ilbc)
PLG_ENUM_CODEC_END 
