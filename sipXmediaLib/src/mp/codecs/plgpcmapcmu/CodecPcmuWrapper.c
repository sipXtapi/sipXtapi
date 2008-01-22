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
/// Exported codec information.
static const struct MppCodecInfoV1_1 sgCodecInfo = 
{
///////////// Implementation and codec info /////////////
   "SIPFoundry",                // codecManufacturer
   "G.711u",                    // codecName
   "1.0",                       // codecVersion
   CODEC_TYPE_SAMPLE_BASED,     // codecType

/////////////////////// SDP info ///////////////////////
   "PCMU",                      // mimeSubtype
   0,                           // fmtpsNum
   NULL,                        // fmtps
   8000,                        // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_NONE     // framePacking
};

/* ============================== FUNCTIONS =============================== */

CODEC_API int PLG_GET_INFO_V1_1(sipxPcmu)(const struct MppCodecInfoV1_1 **codecInfo)
{
   if (codecInfo)
   {
      *codecInfo = &sgCodecInfo;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1_1(sipxPcmu)(const char* fmtp, int isDecoder,
                                        struct MppCodecFmtpInfoV1_1* pCodecInfo)
{
   if (pCodecInfo == NULL)
   {
      return NULL;
   }
   pCodecInfo->signalingCodec = FALSE;
   pCodecInfo->minBitrate = 64000;
   pCodecInfo->maxBitrate = 64000;
   pCodecInfo->numSamplesPerFrame = 1;
   pCodecInfo->minFrameBytes = 1;
   pCodecInfo->maxFrameBytes = 1;
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE;
   pCodecInfo->vadCng = CODEC_CNG_NONE;

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
