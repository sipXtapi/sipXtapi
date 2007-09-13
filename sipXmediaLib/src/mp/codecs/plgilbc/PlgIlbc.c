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

#include <iLBC_define.h>
#include <iLBC_decode.h>
#include <iLBC_encode.h>

static const char codecMIMEsubtype[] = "ilbc";

struct plgCodecInfoV1 codeciLBC = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   codecMIMEsubtype,                //mimeSubtype
   "ilbc",                          //codecName
   "iLBC",                          //codecVersion
   8000,                            //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   240,                             //interleaveBlockSize
   13334,                           //bitRate
   NO_OF_BYTES_30MS * 8,            //minPacketBits
   NO_OF_BYTES_30MS * 8,            //avgPacketBits
   NO_OF_BYTES_30MS * 8,            //maxPacketBits
   240,                             //numSamplesPerFrame
   6,                               //preCodecJitterBufferSize
   1                                //codecSupportPLC
};

struct iLBC_codec_data {
   int mPreparedDec;
   int mPreparedEnc;

   int mMode;

   struct iLBC_Dec_Inst_t_ *mpStateDec;   ///< Internal iLBC decoder state.
   audio_sample_t mpBuffer[240];   ///< Buffer used to store input samples
   int mBufferLoad;                ///< How much data there is in the buffer
   struct iLBC_Enc_Inst_t_* mpStateEnc;   ///< Internal iLBC decoder state.
};

static const char* defaultFmtps[] = {
   "mode=30",
   "mode=20"
};

CODEC_API int PLG_ENUM_V1(ilbc)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
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

CODEC_API void *PLG_INIT_V1(ilbc)(const char* fmt, int bDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   int mode;
   struct iLBC_codec_data *mpiLBC;
   if (pCodecInfo == NULL) {
      return NULL;
   }

   mode = analizeDefRange(fmt, "mode", 30, 20, 30);
   if (mode != 20)
      mode = 30;


   memcpy(pCodecInfo, &codeciLBC, sizeof(struct plgCodecInfoV1));
   mpiLBC = (struct iLBC_codec_data *)malloc(sizeof(struct iLBC_codec_data));
   if (!mpiLBC) {
      return NULL;
   }

   mpiLBC->mpStateDec = NULL;
   mpiLBC->mpStateEnc = NULL;

   mpiLBC->mBufferLoad = 0;

   mpiLBC->mPreparedDec = FALSE;
   mpiLBC->mPreparedEnc = FALSE;

   mpiLBC->mMode = mode;

   if (bDecoder) {
      /* Preparing decoder */
      mpiLBC->mpStateDec = (struct iLBC_Dec_Inst_t_*)malloc(sizeof(struct iLBC_Dec_Inst_t_));
      if (mpiLBC->mpStateDec == NULL) {
         free(mpiLBC);
         return NULL;
      }
      memset(mpiLBC->mpStateDec, 0, sizeof(*mpiLBC->mpStateDec));
      initDecode(mpiLBC->mpStateDec, mpiLBC->mMode /*30*/, 1);

      mpiLBC->mPreparedDec = TRUE;
   } else {
      /* Preparing encoder */
      mpiLBC->mpStateEnc = (struct  iLBC_Enc_Inst_t_*)malloc(sizeof(struct iLBC_Enc_Inst_t_));
      if (mpiLBC->mpStateEnc == NULL) {
         free(mpiLBC);
         return NULL;
      }
      memset(mpiLBC->mpStateEnc, 0, sizeof(*mpiLBC->mpStateEnc));
      initEncode(mpiLBC->mpStateEnc, mpiLBC->mMode /*30*/);

      mpiLBC->mPreparedEnc = TRUE;
   }

   return mpiLBC;
}


CODEC_API int PLG_FREE_V1(ilbc)(void* handle)
{
   int bDecoder;
   struct iLBC_codec_data *mpiLBC = (struct iLBC_codec_data *)handle;

   if (NULL != handle)
   {
      bDecoder = mpiLBC->mPreparedDec;
      //assert((mpiLBC->mPreparedDec == FALSE) && (mpiLBC->mPreparedEnc == FALSE));

      if (bDecoder) {
         /* UnPreparing decoder */
         free(mpiLBC->mpStateDec);
         mpiLBC->mPreparedDec = FALSE;
      } else {
         free(mpiLBC->mpStateEnc);
         mpiLBC->mPreparedEnc = FALSE;
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
   if (!mpiLBC->mPreparedDec)
   {
      return RPLG_INVALID_SEQUENCE_CALL;
   }

   // Check if available buffer size is enough for the packet.
   if (cbBufferSize < (unsigned)mpiLBC->mMode * 8)
   {
      //osPrintf("MpdSipxILBC::decode: Jitter buffer overloaded. Glitch!\n");
      return RPLG_FAILED;
   }

   // Decode incoming packet to temp buffer. If no packet - do PLC.
   //if (pPacket.isValid())
   //{
   
   if (pCodedData) {
      if (((NO_OF_BYTES_30MS != cbCodedPacketSize) && (mpiLBC->mMode == 30)) ||
         ((NO_OF_BYTES_20MS != cbCodedPacketSize) && (mpiLBC->mMode == 20)))
      {
         //osPrintf("MpdSipxILBC::decode: Payload size: %d!\n", pPacket->getPayloadSize());
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

   for (i = 0; i < /*240*/ mpiLBC->mMode * 8; ++i)
   {
      float tmp = buffer[i];
      if (tmp > SHRT_MAX)
         tmp = SHRT_MAX;
      if (tmp < SHRT_MIN)
         tmp = SHRT_MIN;

      samplesBuffer[i] = (audio_sample_t)(tmp + 0.5f);
   }

   *pcbDecodedSize = mpiLBC->mMode * 8;
   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(ilbc)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData,
                         unsigned cbMaxCodedData, int* pcbCodedSize, unsigned* pbSendNow)
{
   struct iLBC_codec_data *mpiLBC = (struct iLBC_codec_data *)handle;
   assert(handle != NULL);
   if (!mpiLBC->mPreparedEnc)
   {
      return RPLG_INVALID_SEQUENCE_CALL;
   }

   memcpy(&mpiLBC->mpBuffer[mpiLBC->mBufferLoad], pAudioBuffer, SIZE_OF_SAMPLE*cbAudioSamples);
   mpiLBC->mBufferLoad += cbAudioSamples;
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

   *rSamplesConsumed = cbAudioSamples;
   return RPLG_SUCCESS;
}

PLG_SINGLE_CODEC(ilbc);

#ifdef STATIC_CODEC
#include <mp/MpPlgStaffV1.h>
DECLARE_MP_STATIC_PLUGIN_CODEC_V1(ilbc);
const char* stub_function_to_do4() { return __FILE__; }
#endif


