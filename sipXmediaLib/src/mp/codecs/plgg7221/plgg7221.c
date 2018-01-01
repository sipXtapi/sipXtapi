//  
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////
   
// Author: Dan Petrie < dpetrie AT sipez DOT com >

// SYSTEM INCLUDES
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// CODEC LIBRARY INCLUDES
#include <g722_1.h>

// EXTERNAL VARIABLES
// CONSTANTS
#define G7221_2FRAME_SIZE (48000 /* bit per second */ * 20 /* msec */ / 1000)
#define G7221_ENCODER_BUFFER_SIZE G7221_2FRAME_SIZE

// TYPEDEFS
// LOCAL DATA TYPES

struct plgg7221_codec_data
{
    int mSampleRate;
    int mBitRate;
    g722_1_encode_state_t* mpEncoderState;
    g722_1_decode_state_t* mpDecoderState;
    int mBufferSize;
    int mBufferedSampleCount;
    audio_sample_t mSampleBuffer[G7221_ENCODER_BUFFER_SIZE];
};

// EXTERNAL FUNCTIONS
// MACROS
// DEFINES
// STATIC VARIABLES INITIALIZATON
static const char* defaultFmtps[] = {
   "bitrate=24000",
   "bitrate=32000",
   "bitrate=48000"
};

// Exported codec information.
static const struct MppCodecInfoV1_1 sgCodecInfo_16000 =
{
///////////// Implementation and codec info /////////////
   "ITU",                  // codecManufacturer
   "G.722.1",              // codecName
   "05/2005 Release 2.1",  // codecVersion
   CODEC_TYPE_FRAME_BASED, // codecType

/////////////////////// SDP info ///////////////////////
   "g7221",                     // mimeSubtype
   2,                           // fmtpsNum
   defaultFmtps,                // fmtps
   16000,                       // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_NONE     // framePacking
};

static const struct MppCodecInfoV1_1 sgCodecInfo_32000 =
{
///////////// Implementation and codec info /////////////
   "ITU",                  // codecManufacturer
   "G.722.1",              // codecName
   "05/2005 Release 2.1",  // codecVersion
   CODEC_TYPE_FRAME_BASED, // codecType

/////////////////////// SDP info ///////////////////////
   "g7221",                     // mimeSubtype
   3,                           // fmtpsNum
   defaultFmtps,                // fmtps
   32000,                       // sampleRate
   1,                           // numChannels
   CODEC_FRAME_PACKING_NONE     // framePacking
};


/* PROTOTYPES */
DECLARE_FUNCS_V1(g7221_16000sps)
DECLARE_FUNCS_V1(g7221_32000sps)

/* ============================== FUNCTIONS MACRO ========================== */

#define DEFINE_G7221_FUNCS(SPS) \
 \
CODEC_API int PLG_GET_INFO_V1_1(g7221_ ## SPS ## sps)(const struct MppCodecInfoV1_1 **codecInfo) \
{ \
   if (codecInfo) \
   { \
      *codecInfo = &sgCodecInfo_ ## SPS ; \
   } \
   return RPLG_SUCCESS; \
} \
 \
CODEC_API void* PLG_INIT_V1_2(g7221_ ## SPS ## sps)(const char* fmtp,  \
                                                    int isDecoder, \
                                                    struct MppCodecFmtpInfoV1_2* pCodecInfo) \
{ \
   int sampleRate = SPS; \
   int bitRate = getFmtpValueRange(fmtp, "bitrate", -1, 24000, 48000); \
   if (pCodecInfo == NULL || (bitRate != 24000 && bitRate != 32000 && bitRate != 48000)) \
   { \
      mppLogError("plgg7221 failed to init %s fmtp: %s sample rate: %d bit rate: %d", \
                  isDecoder ? "decoder" : "encoder", \
                  fmtp, \
                  sampleRate, \
                  bitRate); \
      return NULL; \
   } \
 \
   struct plgg7221_codec_data* codecData = malloc(sizeof(struct plgg7221_codec_data)); \
   codecData->mSampleRate = sampleRate; \
   codecData->mBitRate = bitRate; \
   codecData->mpEncoderState = NULL; \
   codecData->mpDecoderState = NULL; \
   codecData->mBufferSize = G7221_ENCODER_BUFFER_SIZE; \
   codecData->mBufferedSampleCount = 0; \
 \
   if(isDecoder) \
   { \
      codecData->mpDecoderState = g722_1_decode_init(codecData->mpDecoderState, \
                                                     bitRate, \
                                                     sampleRate); \
      if(codecData->mpDecoderState == NULL) \
      { \
          mppLogError("plgg7221 failed to init decoder sample rate: %d bit rate: %d", \
                      sampleRate, \
                      bitRate); \
 \
          free(codecData); \
          codecData = NULL; \
          return(NULL); \
      } \
   } \
   else \
   { \
      codecData->mpEncoderState = g722_1_encode_init(codecData->mpEncoderState, \
                                                     bitRate, \
                                                     sampleRate); \
      if(codecData->mpEncoderState == NULL) \
      { \
          mppLogError("plgg7221 failed to init encoder sample rate: %d bit rate: %d", \
                      sampleRate, \
                      bitRate); \
 \
          free(codecData); \
          codecData = NULL; \
          return(NULL); \
      } \
   } \
 \
   pCodecInfo->signalingCodec = FALSE; \
   pCodecInfo->minFrameBytes = bitRate * 20 / 8 / 1000; \
   pCodecInfo->maxFrameBytes = (bitRate * 20 / 8 / 1000) + 1; \
   pCodecInfo->minBitrate = bitRate; \
   pCodecInfo->maxBitrate = bitRate; \
   pCodecInfo->numSamplesPerFrame = sampleRate * 20 / 1000; \
   if(isDecoder) \
   { \
      pCodecInfo->algorithmicDelay = 0; \
   } \
   else \
   { \
      pCodecInfo->algorithmicDelay = 0; /* expresses as num samples */ \
   } \
   pCodecInfo->packetLossConcealment = CODEC_PLC_NONE; \
   pCodecInfo->vadCng = CODEC_CNG_NONE; \
 \
   return(codecData); \
} \
 \
CODEC_API int PLG_FREE_V1(g7221_ ## SPS ## sps)(void* handle, int isDecoder) \
{ \
   struct plgg7221_codec_data* codecStateData = (struct plgg7221_codec_data*) handle; \
 \
   if(codecStateData) \
   { \
       if(isDecoder && codecStateData->mpDecoderState) \
       { \
           g722_1_decode_release(codecStateData->mpDecoderState); \
           codecStateData->mpDecoderState = NULL; \
       } \
       else if(!isDecoder && codecStateData->mpEncoderState) \
       { \
           g722_1_encode_release(codecStateData->mpEncoderState); \
           codecStateData->mpEncoderState = NULL; \
       } \
       if(codecStateData->mpDecoderState) \
       { \
          mppLogError("plgg7221 PLG_FREE_V1 isDecoder: %s decoderState: %p", \
                      isDecoder ? "TRUE" : "FALSE", \
                      codecStateData->mpDecoderState); \
       } \
       if(codecStateData->mpEncoderState) \
       { \
          mppLogError("plgg7221 PLG_FREE_V1 isDecoder: %s encoderState: %p", \
                      isDecoder ? "TRUE" : "FALSE", \
                      codecStateData->mpEncoderState); \
       } \
 \
       free(codecStateData); \
       codecStateData = NULL; \
   } \
   else \
   { \
       mppLogError("plgg7221 PLG_FREE_V1 isDecoder: %s NULL handle", \
                   isDecoder ? "TRUE" : "FALSE"); \
   } \
 \
   return(0); \
} \
 \
CODEC_API int PLG_DECODE_V1(g7221_ ## SPS ## sps)(void* handle,  \
                                                  const void* pCodedData,  \
                                                  unsigned cbCodedPacketSize,  \
                                                  void* pAudioBuffer,  \
                                                  unsigned cbBufferSize,  \
                                                  unsigned *pcbDecodedSize,  \
                                                  const struct RtpHeader* pRtpHeader) \
{ \
   struct plgg7221_codec_data* codecStateData = (struct plgg7221_codec_data*) handle; \
   int returnCode = RPLG_INVALID_ARGUMENT; \
   *pcbDecodedSize = 0; \
 \
   if(codecStateData) \
   { \
       if(codecStateData->mpDecoderState) \
       { \
          int bufferSizeRequired = 20 /* ms */ * codecStateData->mSampleRate / 1000; \
          int expectedDataSize = 20 /* ms */ * codecStateData->mBitRate / 1000 / 8 ; \
          if(cbCodedPacketSize != expectedDataSize) \
          { \
             *pcbDecodedSize = 0; \
             mppLogError("plgg7221 PLG_DECODE_V1 expected packet size: %d actual: %d", \
                         expectedDataSize, \
                         cbCodedPacketSize); \
          } \
          else if(bufferSizeRequired <= cbBufferSize) \
          { \
             *pcbDecodedSize = g722_1_decode(codecStateData->mpDecoderState, \
                                             pAudioBuffer, \
                                             pCodedData, \
                                             cbCodedPacketSize); \
             returnCode = RPLG_SUCCESS; \
             if(*pcbDecodedSize > cbBufferSize) \
             { \
                mppLogError("plgg7221 PLG_DECODE_V1 resulting audio samples too big, may have overwrite.  " \
                            "samples generated: %d  buffer size: %d", \
                            *pcbDecodedSize, \
                            cbBufferSize); \
             } \
          } \
          else \
          { \
             mppLogError("plgg7221 PLG_DECODE_V1 audio buffer too small: %d need: %d", \
                         cbBufferSize, \
                         bufferSizeRequired); \
          } \
       } \
       else \
       { \
          mppLogError("plgg7221 PLG_DECODE_V1 NULL decoder state codec data: %p", \
                      codecStateData); \
       } \
   } \
   else \
   { \
      mppLogError("plgg7221 PLG_DECODE_V1 NULL codec data handle"); \
   } \
 \
   return(returnCode); \
} \
 \
CODEC_API int PLG_ENCODE_V1(g7221_ ## SPS ## sps)(void* handle,  \
                                                  const void* pAudioBuffer,  \
                                                  unsigned cbAudioSamples,  \
                                                  int* rSamplesConsumed,  \
                                                  void* pCodedData, \
                                                  unsigned cbMaxCodedData,  \
                                                  int* pcbCodedSize,  \
                                                  unsigned* pbSendNow) \
{ \
   struct plgg7221_codec_data* codecStateData = ( struct plgg7221_codec_data*) handle; \
   int returnCode = RPLG_INVALID_ARGUMENT; \
   int sampleRate = SPS; \
   assert(sampleRate == codecStateData->mSampleRate); \
 \
   if(codecStateData) \
   { \
       if(codecStateData->mpEncoderState) \
       { \
          int samplesPerFrame = sampleRate * 20 /* ms */ / 1000; \
          unsigned samplesToEncode = 0; \
          int remainingSamples = 0; \
          const audio_sample_t* samplesPtr = NULL; \
          *pbSendNow = FALSE; \
          *pcbCodedSize = 0; \
          *rSamplesConsumed = cbAudioSamples; /* all is consumed whether buffered or encoded */ \
          /* If nothing is buffered and we have whole frames input: */ \
          if(codecStateData->mBufferedSampleCount == 0 && \
             cbAudioSamples % samplesPerFrame == 0) \
          { /* encode directly from input rather than copiing to buffer */ \
              samplesToEncode = cbAudioSamples; \
              samplesPtr = pAudioBuffer; \
          } \
          else \
          { \
              /* need to copy to buffer and encode when we have enough for a frame */ \
              assert(codecStateData->mBufferedSampleCount + cbAudioSamples <= codecStateData->mBufferSize); \
              memcpy(&codecStateData->mSampleBuffer[codecStateData->mBufferedSampleCount], \
                     pAudioBuffer, cbAudioSamples * sizeof(audio_sample_t)); \
              codecStateData->mBufferedSampleCount += cbAudioSamples; \
              samplesToEncode = (codecStateData->mBufferedSampleCount / samplesPerFrame) * samplesPerFrame; \
              samplesPtr = codecStateData->mSampleBuffer; \
              remainingSamples = codecStateData->mBufferedSampleCount % samplesPerFrame; \
          } \
          if(0) \
          { \
              mppLogError("plgg7221 frame size: %d samples buffered: %d samples to encode: %d", \
                          samplesPerFrame, \
                          codecStateData->mBufferedSampleCount, \
                          samplesToEncode); \
          } \
          if(samplesToEncode >= samplesPerFrame) \
          { \
              *pcbCodedSize = g722_1_encode(codecStateData->mpEncoderState, \
                                            pCodedData, \
                                            samplesPtr, \
                                            samplesToEncode); \
 \
              if(remainingSamples) \
              { \
                  memcpy(codecStateData->mSampleBuffer, \
                         &codecStateData->mSampleBuffer[samplesToEncode], \
                         remainingSamples * sizeof(audio_sample_t)); /* shift leftovers to front of buffer */ \
              } \
              if(codecStateData->mBufferedSampleCount) \
              { \
                  codecStateData->mBufferedSampleCount -= samplesToEncode; \
              } \
              if(*pcbCodedSize > cbMaxCodedData) \
              { \
                 mppLogError("plgg7221 PLG_ENCODE_V1 resulting data too big, may have overwrite.  " \
                             "bytes generated: %d  buffer size: %d", \
                             *pcbCodedSize, \
                             cbMaxCodedData); \
              } \
              if(*pcbCodedSize > 0) \
              { \
                 returnCode = RPLG_SUCCESS; \
                 *pbSendNow = TRUE; \
              } \
              if(0) \
              { \
                  mppLogError("plgg7221 %d sps %d bps g722_1_encode(%p, %p, %d) returned: %d send now: %s buffered samples: %d", \
                              codecStateData->mSampleRate, \
                              codecStateData->mBitRate, \
                              pCodedData, \
                              pAudioBuffer, \
                              (int)cbAudioSamples, \
                              *pcbCodedSize, \
                              *pbSendNow ? "TRUE" : "FALSE", \
                              codecStateData->mBufferedSampleCount); \
               } \
           } \
           else /* nothing to encode, just buffering */ \
           { \
                 returnCode = RPLG_SUCCESS; \
           } \
       } \
       else \
       { \
          mppLogError("plgg7221 PLG_ENCODE_V1 NULL encoder state codec data: %p", \
                      codecStateData); \
       } \
   } \
   else \
   { \
      mppLogError("plgg7221 PLG_ENCODE_V1 NULL codec data handle"); \
   } \
 \
   return(returnCode); \
} 

/* ============================== FUNCTIONS EXPANSIONS ======================= */

DEFINE_G7221_FUNCS(16000) 
DEFINE_G7221_FUNCS(32000) 


// Register the codecs for this plugin
PLG_ENUM_CODEC_START(g7221)
  PLG_ENUM_CODEC(g7221_16000sps)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(g7221_16000sps)
  PLG_ENUM_CODEC_NO_SIGNALING(g7221_16000sps)

  PLG_ENUM_CODEC(g7221_32000sps)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(g7221_32000sps)
  PLG_ENUM_CODEC_NO_SIGNALING(g7221_32000sps)
PLG_ENUM_CODEC_END

