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
#include <memory.h>

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
typedef audio_sample_t MpAudioSample;

// EXTERNAL FUNCTIONS
extern int G711U_Decoder(int numSamples, const uint8_t* inBuff, MpAudioSample* outBuf);
extern int G711U_Encoder(int numSamples, const MpAudioSample* inBuff, uint8_t* outBuf);

// DEFINES
#define DECODER_HANDLE     ((void*)1)
#define ENCODER_HANDLE     ((void*)2)

// STATIC VARIABLE INITIALIZATIONS
static const char codecMIMEsubtype[] = "pcmu";

static const struct plgCodecInfoV1 sipxCodecInfoPCMU = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   codecMIMEsubtype,                //mimeSubtype
   "sipxPcmu",                      //codecName
   "SIPfoundry 1.0",                //codecVersion
   8000,                            //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   160,                             //interleaveBlockSize
   64000,                           //bitRate
   1280,                            //minPacketBits
   1280,                            //avgPacketBits
   1280,                            //maxPacketBits
   160,                             //numSamplesPerFrame
   3                                //preCodecJitterBufferSize
};

/* ============================== FUNCTIONS =============================== */

CODEC_API int PLG_ENUM_V1(sipxPcmu)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
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

CODEC_API void *PLG_INIT_V1(sipxPcmu)(const char* fmtps, int isDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   if (pCodecInfo == NULL) {
      return NULL;
   }
   memcpy(pCodecInfo, &sipxCodecInfoPCMU, sizeof(struct plgCodecInfoV1));
   if (isDecoder)
      return DECODER_HANDLE;
   else
      return ENCODER_HANDLE;
}

CODEC_API int PLG_FREE_V1(sipxPcmu)(void* handle, int isDecoder)
{
   return 0;
}

CODEC_API int PLG_DECODE_V1(sipxPcmu)(void* handle, const void* pCodedData, 
                                      unsigned cbCodedPacketSize, void* pAudioBuffer, 
                                      unsigned cbBufferSize, unsigned *pcbCodedSize, 
                                      const struct RtpHeader* pRtpHeader)
{
   int samples;

   if (handle != DECODER_HANDLE)
      return RPLG_INVALID_SEQUENCE_CALL;

   // Assert that available buffer size is enough for the packet.
   if (cbCodedPacketSize > cbBufferSize)
   {
      return RPLG_INVALID_ARGUMENT;
   }

   if (cbBufferSize == 0)
      return RPLG_INVALID_ARGUMENT;

   samples = min(cbCodedPacketSize, cbBufferSize);
   G711U_Decoder(samples, (uint8_t*)pCodedData, (MpAudioSample *)pAudioBuffer);
   *pcbCodedSize = samples;

   return RPLG_SUCCESS;
}


CODEC_API int PLG_ENCODE_V1(sipxPcmu)(void* handle, const void* pAudioBuffer, 
                                      unsigned cbAudioSamples, int* rSamplesConsumed, 
                                      void* pCodedData, unsigned cbMaxCodedData, 
                                      int* pcbCodedSize, unsigned* pbSendNow)
{
   if (handle != ENCODER_HANDLE)
      return RPLG_INVALID_SEQUENCE_CALL;

   G711U_Encoder(cbAudioSamples, (MpAudioSample *)pAudioBuffer, (uint8_t*)pCodedData);
   *pcbCodedSize = cbAudioSamples;

   *pbSendNow = FALSE;
   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}
