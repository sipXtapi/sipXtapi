//  
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>
#include <speex/speex_preprocess.h>
#include <bits_extensions.h>

struct speex_codec_data_decoder
{
   void *mpDecoderState;    ///< State of the decoder
   unsigned mNumSamplesPerFrame; ///< Number of samples in one frame
};

struct speex_codec_data_encoder
{
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
   unsigned mNumSamplesPerFrame; ///< Number of samples in one frame
   int mDoVad;                ///< Set to 1 to enable voice activity detection
   int mDoDtx;                ///< Set to 1 to enable discontinuous transmission
   int mDoVbr;                ///< Set to 1 to enable variable bitrate mode
   spx_int16_t mpBuffer[640]; ///< Buffer used to store input samples
   unsigned mBufferLoad;      ///< How much data there is in the buffer
   int mDoPreprocess;         ///< Should we do preprocess or not
   SpeexPreprocessState *mpPreprocessState; ///< Preprocessor state
   int mDoDenoise;            ///< Denoises the input
   int mDoAgc;                ///< Automatic Gain Control
};

/* Prototypes */
void* universal_speex_init(const char* fmt, int isDecoder, int samplerate,
                           struct MppCodecFmtpInfoV1_2* pCodecInfo);
int universal_speex_free(void* handle, int isDecoder);
int universal_speex_get_packet_samples(void          *handle,
                                       const uint8_t *pPacketData,
                                       unsigned       packetSize,
                                       unsigned      *pNumSamples,
                                       const struct RtpHeader* pRtpHeader);
int universal_speex_decode(void* handle, const void* pCodedData, 
                          unsigned cbCodedPacketSize, void* pAudioBuffer, 
                          unsigned cbBufferSize, unsigned *pcbDecodedSize, 
                          const struct RtpHeader* pRtpHeader);
int universal_speex_encode(void* handle, const void* pAudioBuffer, 
                          unsigned cbAudioSamples, int* rSamplesConsumed, 
                          void* pCodedData, unsigned cbMaxCodedData, 
                          int* pcbCodedSize, unsigned* pbSendNow);




void* universal_speex_init(const char* fmt, int isDecoder, int samplerate,
                           struct MppCodecFmtpInfoV1_2* pCodecInfo)
{
   const SpeexMode *pSpeexMode;

   if (pCodecInfo == NULL)
   {
      return NULL;
   }

   /* Fill general codec information */
   pCodecInfo->signalingCodec = FALSE;
   /* It could do PLC, but wrapper should be fixed to support it. */
   pCodecInfo->packetLossConcealment = CODEC_PLC_INTERNAL;
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

      /* Get decoder lookahead */
      speex_decoder_ctl(pSpeexDec->mpDecoderState,SPEEX_GET_LOOKAHEAD,&pCodecInfo->algorithmicDelay);

      /* Fill codec information, specific to concrete Speex settings */
      pCodecInfo->numSamplesPerFrame = pSpeexDec->mNumSamplesPerFrame;
      switch (samplerate)
      {
      case 8000:
         pCodecInfo->minBitrate = 3950;
         pCodecInfo->maxBitrate = 24600;
         pCodecInfo->minFrameBytes = (79/*bits/frame*/+7)/8;
         pCodecInfo->maxFrameBytes = (492/*bits/frame*/+7)/8;
         break;
      case 16000:
         pCodecInfo->minBitrate = 3950;
         pCodecInfo->maxBitrate = 42200;
         pCodecInfo->minFrameBytes = (79/*bits/frame*/+7)/8;
         pCodecInfo->maxFrameBytes = (844/*bits/frame*/+7)/8;
         break;
      case 32000:
         pCodecInfo->minBitrate = 5750;
         pCodecInfo->maxBitrate = 44000;
         pCodecInfo->minFrameBytes = (115/*bits/frame*/+7)/8;
         pCodecInfo->maxFrameBytes = (880/*bits/frame*/+7)/8;
         break;
      }

      return pSpeexDec;
   }
   else
   {
      struct speex_codec_data_encoder *pSpeexEnc;
      int mode = getFmtpValueRange(fmt, "mode", -1, 2, 8);

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

      /* Get codec frame size */
      speex_encoder_ctl(pSpeexEnc->mpEncoderState, SPEEX_GET_FRAME_SIZE, &pSpeexEnc->mNumSamplesPerFrame);

      /* Get encoder lookahead */
      speex_encoder_ctl(pSpeexEnc->mpEncoderState,SPEEX_GET_LOOKAHEAD,&pCodecInfo->algorithmicDelay);

      /* Fill codec information, specific to concrete Speex settings */
      pCodecInfo->numSamplesPerFrame = pSpeexEnc->mNumSamplesPerFrame;
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
            speex_mode_query(pSpeexMode, SPEEX_SUBMODE_BITS_PER_FRAME, &pCodecInfo->minFrameBytes);
            speex_mode_query(pSpeexMode, SPEEX_SUBMODE_BITS_PER_FRAME, &pCodecInfo->maxFrameBytes);
            pCodecInfo->minFrameBytes = (pCodecInfo->minFrameBytes/*bits/frame*/+7)/8;
            pCodecInfo->maxFrameBytes = (pCodecInfo->maxFrameBytes/*bits/frame*/+7)/8;
            pCodecInfo->minBitrate = pCodecInfo->minFrameBytes*8/*bits/byte*/*50/*frames/sec*/;
            pCodecInfo->maxBitrate = pCodecInfo->maxFrameBytes*8/*bits/byte*/*50/*frames/sec*/;
            break;
/*         case 16000:
         case 32000:
            pCodecInfo->minFrameBytes = 0;
            pCodecInfo->maxFrameBytes = 10;
            break;*/
         case 16000:
            pCodecInfo->minBitrate = 3950;
            pCodecInfo->maxBitrate = 42200;
            //pCodecInfo->numSamplesPerFrame = 16000;
            pCodecInfo->minFrameBytes = (79/*bits/frame*/+7)/8;
            pCodecInfo->maxFrameBytes = (844/*bits/frame*/+7)/8;
            break;
         case 32000:
            pCodecInfo->minBitrate = 5750;
            pCodecInfo->maxBitrate = 44000;
            //pCodecInfo->numSamplesPerFrame = 32000;
            pCodecInfo->minFrameBytes = (115/*bits/frame*/+7)/8;
            pCodecInfo->maxFrameBytes = (880/*bits/frame*/+7)/8;
            break;
         }
      }

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
      } else {
         struct speex_codec_data_encoder *mpSpeexEnc =
            (struct speex_codec_data_encoder *)handle;
         speex_encoder_destroy(mpSpeexEnc->mpEncoderState);
      }      
      free(handle);
   }
   return 0;
}

int universal_speex_get_packet_samples(void          *handle,
                                       const uint8_t *pPacketData,
                                       unsigned       packetSize,
                                       unsigned      *pNumSamples,
                                       const struct RtpHeader* pRtpHeader)
{
   SpeexBits bits;
   int num_frames;
   struct speex_codec_data_decoder *pSpeexDec = 
      (struct speex_codec_data_decoder *)handle;
   assert(handle != NULL);

   /* Wrap data to speex_bits struct */
   speex_bits_set_bit_buffer(&bits, (void*)pPacketData, packetSize);

   /* Get number of frames */
   num_frames = speex_bits_get_num_frames(&bits);
   if (num_frames < 0)
   {
      return RPLG_CORRUPTED_DATA;
   }

   /* Return number of samples */
   *pNumSamples = num_frames * pSpeexDec->mNumSamplesPerFrame;
   return RPLG_SUCCESS;
}

int universal_speex_decode(void* handle, const void* pCodedData, 
                          unsigned cbCodedPacketSize, void* pAudioBuffer, 
                          unsigned cbBufferSize, unsigned *pcbDecodedSize, 
                          const struct RtpHeader* pRtpHeader)
{
   SpeexBits bits;
   struct speex_codec_data_decoder *mpSpeexDec = 
      (struct speex_codec_data_decoder *)handle;
   assert(handle != NULL);

   if (mpSpeexDec->mNumSamplesPerFrame > cbBufferSize)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   /* Reset number of decoded samples */
   *pcbDecodedSize = 0;

   /* Decode incoming packet if present. Do PLC if no packet. */
   if (pCodedData)
   {
      /* Prepare data for Speex decoder */
      speex_bits_set_bit_buffer(&bits,(char*)pCodedData, cbCodedPacketSize);

      /* Decode while there are something to decode and enough space
      * for decoded data. */
      while (cbBufferSize >= mpSpeexDec->mNumSamplesPerFrame &&
             (speex_bits_remaining(&bits) > 0))
      {
         int res;

         /* Decode frame */
         res = speex_decode_int(mpSpeexDec->mpDecoderState, &bits,
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
   }
   else
   {
      int res;

      /* Do PLC */
      res = speex_decode_int(mpSpeexDec->mpDecoderState, NULL,
                             ((spx_int16_t*)pAudioBuffer)+(*pcbDecodedSize));
      if (res == 0)
      {
         /* Update number of decoded and available samples on success */
         *pcbDecodedSize += mpSpeexDec->mNumSamplesPerFrame;
      }
      else
      {
         /* This mustn't happen for lost packet, but who knows... */
         return RPLG_FAILED;
      }
   }

   return RPLG_SUCCESS;
}

int universal_speex_encode(void* handle, const void* pAudioBuffer, 
                          unsigned cbAudioSamples, int* rSamplesConsumed, 
                          void* pCodedData, unsigned cbMaxCodedData, 
                          int* pcbCodedSize, unsigned* pbSendNow)
{
   struct speex_codec_data_encoder *mpSpeexEnc = 
      (struct speex_codec_data_encoder *)handle;
   assert(handle != NULL);

   memcpy(&mpSpeexEnc->mpBuffer[mpSpeexEnc->mBufferLoad], pAudioBuffer,
          SIZE_OF_SAMPLE * cbAudioSamples);
   mpSpeexEnc->mBufferLoad = mpSpeexEnc->mBufferLoad+cbAudioSamples;
   assert(mpSpeexEnc->mBufferLoad <= mpSpeexEnc->mNumSamplesPerFrame);

   // Check for necessary number of samples
   if(mpSpeexEnc->mBufferLoad == mpSpeexEnc->mNumSamplesPerFrame)
   {
      SpeexBits bits;

      // Wrap our buffer to speex bits structure
      speex_bits_init_buffer(&bits, pCodedData, cbMaxCodedData);

      // Preprocess data if requested
      if(mpSpeexEnc->mDoPreprocess)
         speex_preprocess(mpSpeexEnc->mpPreprocessState, mpSpeexEnc->mpBuffer, NULL);
      // Encode frame and append terminator
      speex_encode_int(mpSpeexEnc->mpEncoderState, mpSpeexEnc->mpBuffer, &bits);
      speex_bits_insert_terminator(&bits);

      // Tell that we've produced packet
      *pbSendNow = TRUE;
      *pcbCodedSize = speex_bits_nbytes(&bits);

      // Reset the buffer count.
      mpSpeexEnc->mBufferLoad = 0;
   }
   else
   {
      *pbSendNow = FALSE;
      *pcbCodedSize = 0;
   }

   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}

DECLARE_FUNCS_V1(speex)
DECLARE_FUNCS_V1(speex_wb)
DECLARE_FUNCS_V1(speex_uwb)

PLG_ENUM_CODEC_START(speex)
   PLG_ENUM_CODEC(speex)
   PLG_ENUM_CODEC_SPECIAL_PACKING(speex)
   PLG_ENUM_CODEC_NO_SIGNALING(speex)

   PLG_ENUM_CODEC(speex_wb)
   PLG_ENUM_CODEC_SPECIAL_PACKING(speex)
   PLG_ENUM_CODEC_NO_SIGNALING(speex_wb)

   PLG_ENUM_CODEC(speex_uwb)
   PLG_ENUM_CODEC_SPECIAL_PACKING(speex)
   PLG_ENUM_CODEC_NO_SIGNALING(speex_uwb)
PLG_ENUM_CODEC_END 
