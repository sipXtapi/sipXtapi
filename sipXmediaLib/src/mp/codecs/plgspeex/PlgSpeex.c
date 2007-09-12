//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
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

const char codecSpeexMIMEsubtype[] = "speex";

struct plgCodecInfoV1 codecSPEEX = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   /*SdpCodec::SDP_CODEC_SPEEX,        //codecSDPType*/
   codecSpeexMIMEsubtype,
   "speex",                        //codecName
   "Speex codec",                      //codecVersion
   8000,                            //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   38,                             //interleaveBlockSize
   15000,                           //bitRate
   1*8,                            //minPacketBits
   38*8,                            //avgPacketBits
   63*8,                            //maxPacketBits
   160,                             //numSamplesPerFrame
   5                                //preCodecJitterBufferSize
};


struct speex_coedec_data {
   void *mpDecoderState;
   SpeexBits mDecbits;
   unsigned mNumSamplesPerFrame;
   int mPreparedDec;

   int mPreparedEnc;
   SpeexBits mBits;         ///< Bits used by speex to store information
   void *mpEncoderState;    ///< State of the encoder   
   int mSampleRate;         ///< Sample rate
   int mMode;
   /**< Mode used.
   * From the Speex documentation:
   *
   * Mode  Bitrate  MFlops Quality
   *  0    250      N/A    No transmission (DTX)
   *  1    2,150    6      Vocoder (mostly for comfort noise)
   *  2    5,950    9      Very noticeable artifacts/noise, good intelligibility
   *  3    8,000    10     Artifacts/noise sometimes noticeable
   *  4    11,000   14     Artifacts usually noticeable only with headphones
   *  5    15,000   11     Need good headphones to tell the difference
   *  6    18,200   17.5   Hard to tell the difference even with good headphones
   *  7    24,600   14.5   Completely transparent for voice, good quality music
   *  8    3,950    10.5   Very noticeable artifacts/noise, good intelligibility
   */
   int mDoVad;              ///< Set to 1 to enable voice activity detection
   int mDoDtx;              ///< Set to 1 to enable discontinuous transmission
   int mDoVbr;              ///< Set to 1 to enable variable bitrate mode
   spx_int16_t mpBuffer[160]; ///< Buffer used to store input samples
   int mBufferLoad;          ///< How much data there is in the buffer
   int mDoPreprocess;         ///< Should we do preprocess or not
   SpeexPreprocessState *mpPreprocessState; ///< Preprocessor state
   int mDoDenoise;             ///< Denoises the input
   int mDoAgc;                 ///< Automatic Gain Control
};

const char* modesSPEEX[] = {
   "mode=2",
   "mode=3",
   "mode=4",
   "mode=5",
   "mode=6",
   "mode=7",
   "mode=8"
};

CODEC_API int PLG_ENUM_V1(speex)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
{
   if (mimeSubtype) {
      *mimeSubtype = codecSpeexMIMEsubtype;
   }
   if (pModesCount) {
      *pModesCount = (sizeof(modesSPEEX)/sizeof(modesSPEEX[0]));
   }
   if (modes) {
      *modes = modesSPEEX;
   }
   return RPLG_SUCCESS;
}
/* Parsing {param}={value} */
static int analizeParamEqValue(const char *parsingString, const char* paramName, int* value)
{
 //  int mode = defMode; /* Initialized to default value */;
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

CODEC_API  void *PLG_INIT_V1(speex)(const char* fmt, int bDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   int mode;
   struct speex_coedec_data *mpSPEEX;
   if (pCodecInfo == NULL) {
      return NULL;
   }

   mode = analizeDefRange(fmt, "mode", 3, 2, 8);

   memcpy(pCodecInfo, &codecSPEEX, sizeof(struct plgCodecInfoV1));
   mpSPEEX = (struct speex_coedec_data *)malloc(sizeof(struct speex_coedec_data));
   if (!mpSPEEX) {
      return NULL;
   }

   mpSPEEX->mpEncoderState = NULL;
   mpSPEEX->mSampleRate = 8000;        // Sample rate of 8000Hz. We'll stick with NB for now.

   mpSPEEX->mDoVad = 0;
   mpSPEEX->mDoDtx = 0;
   mpSPEEX->mDoVbr = 0;
   mpSPEEX->mBufferLoad = 0;
   mpSPEEX->mDoPreprocess = FALSE;
   mpSPEEX->mpPreprocessState = NULL;
   mpSPEEX->mDoDenoise = 0;
   mpSPEEX->mDoAgc = 0;

   mpSPEEX->mBufferLoad = 0;

   mpSPEEX->mpDecoderState = NULL;
   mpSPEEX->mNumSamplesPerFrame = 0;

   mpSPEEX->mPreparedDec = FALSE;
   mpSPEEX->mPreparedEnc = FALSE;

   mpSPEEX->mMode = mode;
   if (mpSPEEX->mMode == 2) {
      mpSPEEX->mDoPreprocess = TRUE; 
   }

   if (bDecoder) {
      /* Preparing decoder */
      int tmp;

      // Init decoder
      mpSPEEX->mpDecoderState = speex_decoder_init(speex_lib_get_mode(SPEEX_MODEID_NB));   

      // It makes the decoded speech deviate further from the original,
      // but it sounds subjectively better.
      tmp = 1;
      speex_decoder_ctl(mpSPEEX->mpDecoderState,SPEEX_SET_ENH,&tmp);

      // Get number of samples in one frame
      speex_decoder_ctl(mpSPEEX->mpDecoderState,SPEEX_GET_FRAME_SIZE,&mpSPEEX->mNumSamplesPerFrame);

      speex_bits_init(&mpSPEEX->mDecbits);

      mpSPEEX->mPreparedDec = TRUE;
   } else {
      /* Preparing encoder */
      mpSPEEX->mpEncoderState = speex_encoder_init(speex_lib_get_mode(SPEEX_MODEID_NB));
      speex_encoder_ctl(mpSPEEX->mpEncoderState, SPEEX_SET_MODE,&mpSPEEX->mMode);
      speex_encoder_ctl(mpSPEEX->mpEncoderState, SPEEX_SET_SAMPLING_RATE, &mpSPEEX->mSampleRate);

      // Enable wanted extensions.
      speex_encoder_ctl(mpSPEEX->mpEncoderState, SPEEX_SET_VAD, &mpSPEEX->mDoVad);
      speex_encoder_ctl(mpSPEEX->mpEncoderState, SPEEX_SET_DTX, &mpSPEEX->mDoDtx);
      speex_encoder_ctl(mpSPEEX->mpEncoderState, SPEEX_SET_VBR, &mpSPEEX->mDoVbr);

      speex_bits_init(&mpSPEEX->mBits);

      if(mpSPEEX->mDoPreprocess)
      {
         mpSPEEX->mpPreprocessState = speex_preprocess_state_init(160, mpSPEEX->mSampleRate);
         speex_preprocess_ctl(mpSPEEX->mpPreprocessState, SPEEX_PREPROCESS_SET_DENOISE,
            &mpSPEEX->mDoDenoise);
         speex_preprocess_ctl(mpSPEEX->mpPreprocessState, SPEEX_PREPROCESS_SET_AGC, &mpSPEEX->mDoAgc);
      }

      mpSPEEX->mPreparedEnc = TRUE;
   }

   return mpSPEEX;
}


CODEC_API int PLG_FREE_V1(speex)(void* handle)
{
   int bDecoder;
   struct speex_coedec_data *mpSPEEX = (struct speex_coedec_data *)handle;

   if (NULL != handle)
   {
      //assert((mpSPEEX->mPreparedDec != FALSE) && (mpSPEEX->mPreparedEnc != FALSE));

      bDecoder = mpSPEEX->mPreparedDec;
      if (bDecoder) {
         /* UnPreparing decoder */
         speex_decoder_destroy(mpSPEEX->mpDecoderState);
         mpSPEEX->mpDecoderState = NULL;

         speex_bits_destroy(&mpSPEEX->mDecbits);
         mpSPEEX->mPreparedDec = FALSE;
      } else {
         speex_encoder_destroy(mpSPEEX->mpEncoderState);
         mpSPEEX->mpEncoderState = NULL;

         speex_bits_destroy(&mpSPEEX->mBits);

         mpSPEEX->mPreparedEnc = FALSE;
      }
      free(handle);
   }
   return 0;
}


CODEC_API  int PLG_DECODE_V1(speex)(void* handle, const void* pCodedData, unsigned cbCodedPacketSize, void* pAudioBuffer, unsigned cbBufferSize, unsigned *pcbDecodedSize, const struct RtpHeader* pRtpHeader)
{
   struct speex_coedec_data *mpSPEEX = (struct speex_coedec_data *)handle;
   assert(handle != NULL);
   if (!mpSPEEX->mPreparedDec)
   {
      return RPLG_INVALID_SEQUENCE_CALL;
   }

   if (mpSPEEX->mNumSamplesPerFrame > cbBufferSize)
   {
      //osPrintf("MpdSipxSpeex::decode: Jitter buffer overloaded. Glitch!\n");
      return RPLG_INVALID_ARGUMENT;
   }

   /* Prepare data for Speex decoder */
   speex_bits_read_from(&mpSPEEX->mDecbits,(char*)pCodedData,cbCodedPacketSize);

   /* Decode frame */
   speex_decode_int(mpSPEEX->mpDecoderState,&mpSPEEX->mDecbits,(spx_int16_t*)pAudioBuffer);   

   *pcbDecodedSize = mpSPEEX->mNumSamplesPerFrame;
   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(speex)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData,
                          unsigned cbMaxCodedData, int* pcbCodedSize, unsigned* pbSendNow)
{
   int size = 0;   
   struct speex_coedec_data *mpSPEEX = (struct speex_coedec_data *)handle;
   assert(handle != NULL);
   if (!mpSPEEX->mPreparedEnc)
   {
      return RPLG_INVALID_SEQUENCE_CALL;
   }

   memcpy(&mpSPEEX->mpBuffer[mpSPEEX->mBufferLoad], pAudioBuffer, SIZE_OF_SAMPLE * cbAudioSamples);
   mpSPEEX->mBufferLoad = mpSPEEX->mBufferLoad+cbAudioSamples;
   assert(mpSPEEX->mBufferLoad <= 160);

   // Check for necessary number of samples
   if(mpSPEEX->mBufferLoad == 160)
   {
      speex_bits_reset(&mpSPEEX->mBits);

      // We don't have echo data, but it should be possible to use the
      // Speex echo canceler in sipxtapi.
      if(mpSPEEX->mDoPreprocess)
         speex_preprocess(mpSPEEX->mpPreprocessState, mpSPEEX->mpBuffer, NULL);
      speex_encode_int(mpSPEEX->mpEncoderState, mpSPEEX->mpBuffer, &mpSPEEX->mBits);

      // Copy to the byte buffer
      //size = speex_bits_write(&mpSPEEX->mBits,(char*)pCodedData,200);      
      size = speex_bits_write(&mpSPEEX->mBits,(char*)pCodedData,cbMaxCodedData);      

      // Reset the buffer count.
      mpSPEEX->mBufferLoad = 0;

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

PLG_SINGLE_CODEC(speex);

#ifdef STATIC_CODEC
#include <mp/MpPlgStaffV1.h>
DECLARE_MP_STATIC_PLUGIN_CODEC_V1(speex);
const char* stub_function_to_do3() { return (const char*)&codecSPEEX; }
#endif
