//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: 

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>
#include <speex/speex_preprocess.h>

struct speex_codec_data_decoder
{
   void *mpDecoderState;    ///< State of the decoder
   unsigned mNumSamplesPerFrame;
   SpeexBits mBits;         ///< Bits used by speex to store information
};

struct speex_codec_data_encoder
{
   SpeexBits mBits;         ///< Bits used by speex to store information
   void *mpEncoderState;    ///< State of the encoder   
   int mMode;
   /**< Mode used.
   * From the Speex documentation:
   *
   * <pre>
   * Mode  Bitrate  BitrateWB BitrateUWB   MFlops Quality (for narrow-band)
   *  0    250      3.95      5.75         N/A    No transmission (DTX)
   *  1    2.15     5.75      7.55         6      Vocoder (mostly for comfort noise)
   *  2    5.95     7.75      9.55         9      Very noticeable artifacts/noise, good intelligibility
   *  3    8        9.80      11.6         10     Artifacts/noise sometimes noticeable
   *  4    11       12.8      14.6         14     Artifacts usually noticeable only with headphones
   *  5    15       16.8      18.6         11     Need good headphones to tell the difference
   *  6    18.2     20.6      22.4         17.5   Hard to tell the difference even with good headphones
   *  7    24.6     23.8      25.6         14.5   Completely transparent for voice, good quality music
   *  8    3.95     27.8      29.6         10.5   Very noticeable artifacts/noise, good intelligibility
   *  9             34.2      36.0
   *  10            42.2      44.0
   * </pre>
   */
   int mDoVad;                ///< Set to 1 to enable voice activity detection
   int mDoDtx;                ///< Set to 1 to enable discontinuous transmission
   int mDoVbr;                ///< Set to 1 to enable variable bitrate mode
   spx_int16_t mpBuffer[160]; ///< Buffer used to store input samples
   int mBufferLoad;           ///< How much data there is in the buffer
   int mDoPreprocess;         ///< Should we do preprocess or not
   SpeexPreprocessState *mpPreprocessState; ///< Preprocessor state
   int mDoDenoise;            ///< Denoises the input
   int mDoAgc;                ///< Automatic Gain Control
};


/* Parsing {param}={value} */
static int analizeParamEqValue(const char *parsingString, const char* paramName, int* value)
{
   int tmp;
   char c;
   int eqFound = FALSE;
   int digitFound = FALSE;  
   const char* res;
   res = strstr(parsingString, paramName);
   if (!res) {
      return -1;
   }
   res += strlen(paramName); //Skip name of param world

   for (; (c=*res) != 0; res++ )
   {
      if (isspace(c)) {
         if (digitFound) 
            break;
         continue;
      }
      if (c == '=') {
         if (eqFound) 
            goto end_of_analize;
         eqFound = TRUE;
         continue;            
      }
      if (isdigit(c)) {
         if (!eqFound) 
            goto end_of_analize;
         tmp = (c - '0');
         for (res++; isdigit(c=*res); res++) {
            tmp = tmp * 10 + (c - '0');
         }
         res--;
         digitFound = TRUE;
         continue;
      }

      /* Unexpected character */
      goto end_of_analize;
   }
   if (digitFound) {
      *value = tmp;
      return 0;
   }

end_of_analize:
   return -1;
}

static int analizeDefRange(const char* str, const char* param, int defValue, int minValue, int maxValue)
{
   int value;
   int res = (!str) ? (-1) : analizeParamEqValue(str, param, &value);
   if ((res == 0) && (value >= minValue) && (value <= maxValue))
      return value;
   return defValue;
}

void* universal_speex_init(const char* fmt, int isDecoder, int samplerate,
                           struct MppCodecFmtpInfoV1_1* pCodecInfo)
{
   const SpeexMode *pSpeexMode;

   if (pCodecInfo == NULL)
   {
      return NULL;
   }

   /* Fill general codec information */
   pCodecInfo->signalingCodec = FALSE;
   /* It could do PLC, but wrapper should be fixed to support it. */
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
   /* It could do DTX+CNG, but wrapper should be fixed to support it. */
   pCodecInfo->vadCng = CODEC_CNG_NONE;

   /* Get Speex mode for given sample rate */
   switch (samplerate)
   {
   case 8000:
      pSpeexMode = speex_lib_get_mode(SPEEX_MODEID_NB);
      break;
   case 16000:
      pSpeexMode = speex_lib_get_mode(SPEEX_MODEID_WB);
      break;
   case 32000:
      pSpeexMode = speex_lib_get_mode(SPEEX_MODEID_UWB);
      break;
   default:
      assert(!"Wrong Speex sampling rate setting!");
   }

   if (isDecoder) 
   {
      int tmp;
      struct speex_codec_data_decoder *pSpeexDec;

      /* Preparing decoder */
      pSpeexDec = (struct speex_codec_data_decoder *)malloc(sizeof(struct speex_codec_data_decoder));
      if (!pSpeexDec)
      {
         return NULL;
      }
      pSpeexDec->mpDecoderState = NULL;
      pSpeexDec->mNumSamplesPerFrame = 0;

      /* Init decoder */
      pSpeexDec->mpDecoderState = speex_decoder_init(pSpeexMode);   

      /* It makes the decoded speech deviate further from the original,
      *  but it sounds subjectively better.*/
      tmp = 1;
      speex_decoder_ctl(pSpeexDec->mpDecoderState,SPEEX_SET_ENH,&tmp);

      /* Get number of samples in one frame */
      speex_decoder_ctl(pSpeexDec->mpDecoderState,SPEEX_GET_FRAME_SIZE,&pSpeexDec->mNumSamplesPerFrame);
      speex_bits_init(&pSpeexDec->mBits);

      /* Fill codec information, specific to concrete Speex settings */
      switch (samplerate)
      {
      case 8000:
         pCodecInfo->minBitrate = 3950;
         pCodecInfo->maxBitrate = 24600;
         pCodecInfo->numSamplesPerFrame = pSpeexDec->mNumSamplesPerFrame;
         pCodecInfo->minFrameBytes = (79/*bits/frame*/+7)/8;
         pCodecInfo->maxFrameBytes = (492/*bits/frame*/+7)/8;
         break;
      case 16000:
         pCodecInfo->minBitrate = 3950;
         pCodecInfo->maxBitrate = 42200;
         pCodecInfo->numSamplesPerFrame = pSpeexDec->mNumSamplesPerFrame;
         pCodecInfo->minFrameBytes = (79/*bits/frame*/+7)/8;
         pCodecInfo->maxFrameBytes = (844/*bits/frame*/+7)/8;
         break;
      case 32000:
         pCodecInfo->minBitrate = 5750;
         pCodecInfo->maxBitrate = 44000;
         pCodecInfo->numSamplesPerFrame = pSpeexDec->mNumSamplesPerFrame;
         pCodecInfo->minFrameBytes = (115/*bits/frame*/+7)/8;
         pCodecInfo->maxFrameBytes = (880/*bits/frame*/+7)/8;
         break;
      }

      return pSpeexDec;
   }
   else
   {
      struct speex_codec_data_encoder *pSpeexEnc;
      int mode = analizeDefRange(fmt, "mode", -1, 2, 8);

      pSpeexEnc = (struct speex_codec_data_encoder *)malloc(sizeof(struct speex_codec_data_encoder));
      if (!pSpeexEnc)
      {
         return NULL;
      }

      pSpeexEnc->mpEncoderState = NULL;

      pSpeexEnc->mDoVad = 0;
      pSpeexEnc->mDoDtx = 0;
      pSpeexEnc->mDoVbr = 0;
      pSpeexEnc->mBufferLoad = 0;
      pSpeexEnc->mDoPreprocess = FALSE;
      pSpeexEnc->mpPreprocessState = NULL;
      pSpeexEnc->mDoDenoise = 0;
      pSpeexEnc->mDoAgc = 0;

      pSpeexEnc->mBufferLoad = 0;

      pSpeexEnc->mMode = mode;
      if (samplerate == 8000 && pSpeexEnc->mMode == 2)
      {
         pSpeexEnc->mDoPreprocess = TRUE; 
      }

      /* Preparing encoder */
      pSpeexEnc->mpEncoderState = speex_encoder_init(pSpeexMode);
      /* Setting wanted mode */
      if (pSpeexEnc->mMode > -1)
      {
         speex_encoder_ctl(pSpeexEnc->mpEncoderState, SPEEX_SET_MODE,&pSpeexEnc->mMode);
      }

      // Enable wanted extensions.
      speex_encoder_ctl(pSpeexEnc->mpEncoderState, SPEEX_SET_VAD, &pSpeexEnc->mDoVad);
      speex_encoder_ctl(pSpeexEnc->mpEncoderState, SPEEX_SET_DTX, &pSpeexEnc->mDoDtx);
      speex_encoder_ctl(pSpeexEnc->mpEncoderState, SPEEX_SET_VBR, &pSpeexEnc->mDoVbr);

      if(pSpeexEnc->mDoPreprocess)
      {
         pSpeexEnc->mpPreprocessState = speex_preprocess_state_init(160, samplerate);
         speex_preprocess_ctl(pSpeexEnc->mpPreprocessState, SPEEX_PREPROCESS_SET_DENOISE,
                              &pSpeexEnc->mDoDenoise);
         speex_preprocess_ctl(pSpeexEnc->mpPreprocessState, SPEEX_PREPROCESS_SET_AGC,
                              &pSpeexEnc->mDoAgc);
      }

      speex_bits_init(&pSpeexEnc->mBits);

      /* Fill codec information, specific to concrete Speex settings */
      speex_encoder_ctl(pSpeexEnc->mpEncoderState, SPEEX_GET_FRAME_SIZE, &pCodecInfo->numSamplesPerFrame);
      if (mode > -1)
      {
         pCodecInfo->minFrameBytes = mode;
         pCodecInfo->maxFrameBytes = mode;
      } 
      else
      {
         switch (samplerate)
         {
         case 8000:
            pCodecInfo->minFrameBytes = 8;
            pCodecInfo->maxFrameBytes = 7;
//            pCodecInfo->minFrameBytes = (79/*bits/frame*/+7)/8;
//            pCodecInfo->maxFrameBytes = (492/*bits/frame*/+7)/8;
            break;
         case 16000:
         case 32000:
            pCodecInfo->minFrameBytes = 0;
            pCodecInfo->maxFrameBytes = 10;
//            pCodecInfo->minFrameBytes = (79/*bits/frame*/+7)/8;
//            pCodecInfo->maxFrameBytes = (844/*bits/frame*/+7)/8;
            break;
         }
      }
      speex_mode_query(pSpeexMode, SPEEX_SUBMODE_BITS_PER_FRAME, &pCodecInfo->minFrameBytes);
      speex_mode_query(pSpeexMode, SPEEX_SUBMODE_BITS_PER_FRAME, &pCodecInfo->maxFrameBytes);
      pCodecInfo->minFrameBytes = (pCodecInfo->minFrameBytes/*bits/frame*/+7)/8;
      pCodecInfo->maxFrameBytes = (pCodecInfo->maxFrameBytes/*bits/frame*/+7)/8;
      pCodecInfo->minBitrate = pCodecInfo->minFrameBytes*8/*bits/byte*/*50/*frames/sec*/;
      pCodecInfo->maxBitrate = pCodecInfo->maxFrameBytes*8/*bits/byte*/*50/*frames/sec*/;

      return pSpeexEnc;
   }
}

int universal_speex_free(void* handle, int isDecoder)
{
   if (NULL != handle)
   {
      if (isDecoder) {
         /* UnPreparing decoder */         
         struct speex_codec_data_decoder *mpSpeexDec = 
            (struct speex_codec_data_decoder *)handle;
         speex_decoder_destroy(mpSpeexDec->mpDecoderState);
         speex_bits_destroy(&mpSpeexDec->mBits);
      } else {
         struct speex_codec_data_encoder *mpSpeexEnc =
            (struct speex_codec_data_encoder *)handle;
         speex_encoder_destroy(mpSpeexEnc->mpEncoderState);
         speex_bits_destroy(&mpSpeexEnc->mBits);
      }      
      free(handle);
   }
   return 0;
}

int universal_speex_decode(void* handle, const void* pCodedData, 
                          unsigned cbCodedPacketSize, void* pAudioBuffer, 
                          unsigned cbBufferSize, unsigned *pcbDecodedSize, 
                          const struct RtpHeader* pRtpHeader)
{
   struct speex_codec_data_decoder *mpSpeexDec = 
      (struct speex_codec_data_decoder *)handle;
   assert(handle != NULL);

   if (mpSpeexDec->mNumSamplesPerFrame > cbBufferSize)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   /* Prepare data for Speex decoder */
   speex_bits_read_from(&mpSpeexDec->mBits,(char*)pCodedData,cbCodedPacketSize);

   /* Reset number of decoded samples */
   *pcbDecodedSize = 0;

   /* Decode while there are something to decode and enough space
    * for decoded data. */
   while (cbBufferSize >= mpSpeexDec->mNumSamplesPerFrame &&
          (speex_bits_remaining(&mpSpeexDec->mBits) > 0))
   {
      int res;

      /* Decode frame */
      res = speex_decode_int(mpSpeexDec->mpDecoderState, &mpSpeexDec->mBits,
                             ((spx_int16_t*)pAudioBuffer)+(*pcbDecodedSize));
      if (res == 0)
      {
         /* Update number of decoded and available samples on success */
         *pcbDecodedSize += mpSpeexDec->mNumSamplesPerFrame;
         cbBufferSize -= mpSpeexDec->mNumSamplesPerFrame;
      }
      else
      {
         /* If it's the end of the stream or corrupted stream just return */
         break;
      }
   }

   return RPLG_SUCCESS;
}

int universal_speex_encode(void* handle, const void* pAudioBuffer, 
                          unsigned cbAudioSamples, int* rSamplesConsumed, 
                          void* pCodedData, unsigned cbMaxCodedData, 
                          int* pcbCodedSize, unsigned* pbSendNow)
{
   int size = 0;   
   struct speex_codec_data_encoder *mpSpeexEnc = 
      (struct speex_codec_data_encoder *)handle;
   assert(handle != NULL);

   memcpy(&mpSpeexEnc->mpBuffer[mpSpeexEnc->mBufferLoad], pAudioBuffer, SIZE_OF_SAMPLE * cbAudioSamples);
   mpSpeexEnc->mBufferLoad = mpSpeexEnc->mBufferLoad+cbAudioSamples;
   assert(mpSpeexEnc->mBufferLoad <= 160);

   // Check for necessary number of samples
   if(mpSpeexEnc->mBufferLoad == 160)
   {
      speex_bits_reset(&mpSpeexEnc->mBits);

      // We don't have echo data, but it should be possible to use the
      // Speex echo canceler in sipxtapi.
      if(mpSpeexEnc->mDoPreprocess)
         speex_preprocess(mpSpeexEnc->mpPreprocessState, mpSpeexEnc->mpBuffer, NULL);
      speex_encode_int(mpSpeexEnc->mpEncoderState, mpSpeexEnc->mpBuffer, &mpSpeexEnc->mBits);

      // Copy to the byte buffer   
      size = speex_bits_write(&mpSpeexEnc->mBits,(char*)pCodedData,cbMaxCodedData);      

      // Reset the buffer count.
      mpSpeexEnc->mBufferLoad = 0;

      //if (size>0) {
      *pbSendNow = TRUE;
      //}
   }
   else
   {
      *pbSendNow = FALSE;
   }

   *rSamplesConsumed = cbAudioSamples;
   *pcbCodedSize = size;

   return RPLG_SUCCESS;
}

DECLARE_FUNCS_V1(speex)
DECLARE_FUNCS_V1(speex_wb)
DECLARE_FUNCS_V1(speex_uwb)

PLG_ENUM_CODEC_START(speex)
   PLG_ENUM_CODEC(speex)
   PLG_ENUM_CODEC(speex_wb)
   PLG_ENUM_CODEC(speex_uwb)
PLG_ENUM_CODEC_END 
