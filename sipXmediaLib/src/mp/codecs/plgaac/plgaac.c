//  
// Copyright (C) 2007-2011 SIPez LLC. All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Daniel Petrie <dpetrie AT sipez DOT com>

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// CODEC LIBRARY INCLUDES
#include "avcodec.h"
#include "mpeg4audio.h"
#include <aac.h>

// From FFmpeg
#undef printf

// EXTERNAL VARIABLES
static int sSipxFfmpegCodecInitialized = 0;

// CONSTANTS
#define SDP_AUDIO_CONFIG_LINE_SIZE (5)

/* TODO: confirm this: */
#define SAMPLE_PER_AAC_LC_FRAME 1024

#define SIPX_AAC_ENCODER_BUFFER_SIZE (SAMPLE_PER_AAC_LC_FRAME * 2)

// TYPEDEFS
// LOCAL DATA TYPES
// EXTERNAL FUNCTIONS
void sipxFfmpegCodecInit();

void* sipxAacCommonInit(const char* fmtp, int isDecoder,
                        struct MppCodecFmtpInfoV1_2* codecInfo, int sampleRate);

int sipxAacCommonDecode(void* opaqueCodecContext, const void* encodedData,
                                  unsigned encodedPacketSize, void* pcmAudioBuffer,
                                  unsigned pcmBufferSize, unsigned* decodedSamples,
                                  const struct RtpHeader* rtpHeader);

int sipxAacCommonEncode(void* opaqueCodecContext, const void* pcmAudioBuffer,
                                     unsigned pcmAudioSamples, int* samplesConsumed,
                                     void* encodedData, unsigned maxCodedDataSize,
                                     int* encodedDataSize, unsigned* sendNow);

/* Set config bits  
   15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 
                                           p  p  p -> PADS 
                               c  c  c  c ----------> Channel Config 
                   s  s  s  s ----------------------> Sample Rate Index 
    t  t  t  t  t ----------------------------------> Audio Object Type 

    0  0  0  1  0  1  0  0  0  0  0  0  1  0  0  0 == 0x1408 (AAC-LC audio object, 16000 samples/sec, mono) 
    0  0  0  1  0  0  1  0  1  0  0  0  1  0  0  0 == 0x1288 (AAC-LC audio object, 32000 samples/sec, mono) 
    0  0  0  1  0  0  0  1  1  0  0  0  1  0  0  0 == 0x1188 (AAC-LC audio object, 48000 samples/sec, mono) 
*/

// MACROS
#define DEF_AAC_CODEC(SAMPLE_RATE) \
/* Exported codec information. */ \
static const struct MppCodecInfoV1_1 sgCodecInfo_##SAMPLE_RATE = \
{ \
/*/////////// Implementation and codec info ////////////*/ \
   "FFmpeg",                   /* codecManufacturer */ \
   "AAC_LC/" #SAMPLE_RATE,     /* codecName */ \
   "0.8",                      /* codecVersion */ \
   CODEC_TYPE_FRAME_BASED,     /* codecType */ \
\
/*///////////////////// SDP info /////////////////////*/ \
   "AAC_LC",                    /* mimeSubtype */ \
   0,                           /* fmtpsNum */ \
   NULL,                        /* fmtps */ \
   SAMPLE_RATE,                 /* sampleRate */ \
   1,                           /* numChannels */ \
   CODEC_FRAME_PACKING_NONE     /* framePacking */ \
}; \
\
CODEC_API int PLG_GET_INFO_V1_1(aac_lc_##SAMPLE_RATE)(const struct MppCodecInfoV1_1 **codecInfo) \
{ \
   if (codecInfo) \
   { \
      *codecInfo = &sgCodecInfo_##SAMPLE_RATE; \
   } \
   return RPLG_SUCCESS; \
} \
\
CODEC_API void* PLG_INIT_V1_2(aac_lc_##SAMPLE_RATE)(const char* fmtp, int isDecoder, \
                                            struct MppCodecFmtpInfoV1_2* codecInfo) \
{ \
    return(sipxAacCommonInit(fmtp, isDecoder, codecInfo, SAMPLE_RATE)); \
} \
 \
CODEC_API int PLG_DECODE_V1(aac_lc_##SAMPLE_RATE)(void* opaqueCodecContext, const void* encodedData, \
                                     unsigned encodedPacketSize, void* pcmAudioBuffer, \
                                     unsigned pcmBufferSize, unsigned* decodedSamples, \
                                     const struct RtpHeader* rtpHeader) \
{ \
    return(sipxAacCommonDecode(opaqueCodecContext, encodedData, encodedPacketSize, pcmAudioBuffer, \
                               pcmBufferSize, decodedSamples, rtpHeader)); \
} \
 \
CODEC_API int PLG_ENCODE_V1(aac_lc_##SAMPLE_RATE)(void* opaqueCodecContext, const void* pcmAudioBuffer, \
                                     unsigned pcmAudioSamples, int* samplesConsumed, \
                                     void* encodedData, unsigned maxCodedDataSize, \
                                     int* encodedDataSize, unsigned* sendNow) \
{ \
    return(sipxAacCommonEncode(opaqueCodecContext, pcmAudioBuffer, pcmAudioSamples, samplesConsumed, \
                               encodedData, maxCodedDataSize, encodedDataSize, sendNow)); \
} \
 \
CODEC_API int PLG_FREE_V1(aac_lc_##SAMPLE_RATE)(void* opaqueCodecContext, int isDecoder) \
{ \
    AVCodecContext* codecContext = (AVCodecContext*) opaqueCodecContext; \
    if(!isDecoder) \
    { \
        /* we use pkt to buffer up enough sipX frames to encode an AAC frame */ \
        av_free(codecContext->pkt->data); \
        codecContext->pkt->data = NULL; \
        av_free(codecContext->pkt); \
        codecContext->pkt = NULL; \
    } \
    avcodec_close(codecContext); \
    av_free(codecContext->extradata); \
    codecContext->extradata =  NULL; \
    av_free(codecContext); \
    codecContext = NULL; \
 \
   return(RPLG_SUCCESS); \
} 



DECLARE_FUNCS_V1(aac_lc_16000)
DECLARE_FUNCS_V1(aac_lc_32000)
DECLARE_FUNCS_V1(aac_lc_48000)

PLG_ENUM_CODEC_START(aac_lc)
    PLG_ENUM_CODEC(aac_lc_16000)
    PLG_ENUM_CODEC_NO_SPECIAL_PACKING(aac_lc_16000)
    PLG_ENUM_CODEC_NO_SIGNALING(aac_lc_16000)

    PLG_ENUM_CODEC(aac_lc_32000)
    PLG_ENUM_CODEC_NO_SPECIAL_PACKING(aac_lc_32000)
    PLG_ENUM_CODEC_NO_SIGNALING(aac_lc_32000)

    PLG_ENUM_CODEC(aac_lc_48000)
    PLG_ENUM_CODEC_NO_SPECIAL_PACKING(aac_lc_48000)
    PLG_ENUM_CODEC_NO_SIGNALING(aac_lc_48000)
PLG_ENUM_CODEC_END

DEF_AAC_CODEC(16000)
DEF_AAC_CODEC(32000)
DEF_AAC_CODEC(48000)

/* ============================== FUNCTIONS =============================== */

void sipxFfmpegCodecInit()
{
    if(!sSipxFfmpegCodecInitialized)
    {
        sSipxFfmpegCodecInitialized = 1;

       printf("DDDDDDDD sSipxFfmpegCodecInitialized one time set.\n");
       // Global FFmpeg codec init
       avcodec_init();
       avcodec_register_all();
    }
}

void* sipxAacCommonInit(const char* fmtp, int isDecoder,
                        struct MppCodecFmtpInfoV1_2* codecInfo, int sampleRate)
{
    sipxFfmpegCodecInit(); 
 
    AVCodecContext* codecContext = NULL; 
 
    if (codecInfo != NULL) 
    { 
       /* TODO: what is bit rate for AAC_LC???? */ 
       int bitRate =  64000; 
       int frameSize = SAMPLE_PER_AAC_LC_FRAME; /* what is AAC_LC framesize???? */ 
       int numChannels = 1; /* as shown in SDP */ 
 
        codecInfo->signalingCodec = FALSE; 
        codecInfo->minBitrate = bitRate; 
        codecInfo->maxBitrate = bitRate; 
        codecInfo->numSamplesPerFrame = frameSize;
        /* TODO:  what are frame bytes size??? */
        codecInfo->minFrameBytes = 2;
        codecInfo->maxFrameBytes = 2;
        codecInfo->packetLossConcealment = CODEC_PLC_NONE;
        codecInfo->vadCng = CODEC_CNG_NONE;
        codecInfo->algorithmicDelay = frameSize * 1000 / (sampleRate / 1000); /* Microseconds of audio/frame (SAMPLE_PER_AAC_LC_FRAME samples) */

        codecContext = avcodec_alloc_context();
        codecContext->sample_rate = sampleRate;
        codecContext->channels = numChannels;
        codecContext->sample_fmt = AV_SAMPLE_FMT_S16;
        codecContext->profile = FF_PROFILE_AAC_LOW; /* AAC_LC */

        /* Decoder */
        if(isDecoder)
        {
            AVCodec *aacDecoder = avcodec_find_decoder(CODEC_ID_AAC);
            if(aacDecoder)
            {
                char configBits[SDP_AUDIO_CONFIG_LINE_SIZE];
                switch(sampleRate)
                {
                    case 16000:
                        configBits[0] = 0x14; /* see bit layout above */
                        configBits[1] = 0x08;
                    break;
                    case 32000:
                        configBits[0] = 0x12; /* see bit layout above */
                        configBits[1] = 0x88;
                    break;
                    case 48000:
                        configBits[0] = 0x11; /* see bit layout above */
                        configBits[1] = 0x88;
                    break;
                    default:
                        assert(sampleRate == 16000 || sampleRate == 32000 || sampleRate == 48000);
                        configBits[0] = 0x0;
                        configBits[1] = 0x0;
                    break;
                }
                codecContext->extradata_size = SDP_AUDIO_CONFIG_LINE_SIZE;
                codecContext->extradata = av_malloc(SDP_AUDIO_CONFIG_LINE_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
                /* to make vlagrind hapy */
                memset(codecContext->extradata, 0, SDP_AUDIO_CONFIG_LINE_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
                memcpy(codecContext->extradata, configBits, SDP_AUDIO_CONFIG_LINE_SIZE);

                int openDecoderError = avcodec_open(codecContext, aacDecoder);
                if(openDecoderError == 0)
                {
                }
                else
                {
                    printf("avcodec_open for AAC decoder returned: %d in %s\n", openDecoderError, __FILE__);

                    /* Clean up */
                    avcodec_close(codecContext);
                    av_free(codecContext->extradata);
                    codecContext->extradata = NULL;
                    av_free(codecContext);
                    codecContext = NULL;
                    assert(openDecoderError == 0);
                }
            }
            else
            {
                printf("avcodec_find_decoder returned NULL decoder for CODEC_ID_AAC in %s\n", __FILE__);
                /* Clean up */
                avcodec_close(codecContext);
                av_free(codecContext->extradata);
                codecContext->extradata = 0;
                av_free(codecContext);
                codecContext = 0;
                assert(aacEncoder);
            }
        }
        /* Encoder: */
        else
        {
            AVCodec* aacEncoder = avcodec_find_encoder(CODEC_ID_AAC);
            if(aacEncoder)
            {
                int codecOpenError = codecOpenError = avcodec_open(codecContext, aacEncoder);
                if(codecOpenError == 0)
                {
                    printf("AAC encoder initialized, init automatically set: frame_size=%d, profile=%d (FF_PROFILE_AAC_LOW=%d, FF_PROFILE_UNKNOWN=%d), bit_rate=%d extradata=%d %d\n",
                       codecContext->frame_size, codecContext->profile, FF_PROFILE_AAC_LOW, FF_PROFILE_UNKNOWN, 
                       codecContext->bit_rate, (int)codecContext->extradata[0], (int)codecContext->extradata[1]);
                       /* we use pkt to buffer up enough sipX frames to encode an AAC frame */
                       codecContext->pkt = av_malloc(sizeof(AVPacket));
                       av_init_packet(codecContext->pkt);
                       codecContext->pkt->data = av_malloc(sizeof(audio_sample_t) * SIPX_AAC_ENCODER_BUFFER_SIZE);
                       codecContext->pkt->size = SIPX_AAC_ENCODER_BUFFER_SIZE;
                       codecContext->pkt->pos = 0;
                }
                else
                {
                    printf("avcodec_open for encoder returned: %d in %s\n", codecOpenError, __FILE__);

                    /* Clean up */
                    avcodec_close(codecContext);
                    av_free(codecContext->extradata);
                    codecContext->extradata = 0;
                    av_free(codecContext);
                    codecContext = 0;
                    assert(codecOpenError == 0);
                }
            }
            else
            {
                printf("avcodec_find_encoder returned NULL encoder for CODEC_ID_AAC in %s\n", __FILE__);
                /* Clean up */
                avcodec_close(codecContext);
                av_free(codecContext->extradata);
                codecContext->extradata = NULL;
                av_free(codecContext);
                codecContext = NULL;
                assert(aacEncoder);
            }
        }
    }

    printf("sipxAacCommonInit returning codecContext: %p\n", codecContext);
    return(codecContext);
}


int sipxAacCommonDecode(void* opaqueCodecContext, const void* encodedData,
                                  unsigned encodedPacketSize, void* pcmAudioBuffer,
                                  unsigned pcmBufferSize, unsigned* decodedSamples,
                                  const struct RtpHeader* rtpHeader)

{
    AVCodecContext* codecContext = (AVCodecContext*) opaqueCodecContext;
    int returnValue = RPLG_BUFFER_TOO_SMALL;
    assert(pcmBufferSize >= SAMPLE_PER_AAC_LC_FRAME);
    if(pcmBufferSize >= SAMPLE_PER_AAC_LC_FRAME)
    {
        returnValue = RPLG_SUCCESS;

        AVPacket ffmpegPacket;
        av_init_packet(&ffmpegPacket);
        ffmpegPacket.data = (uint8_t *) encodedData;
        ffmpegPacket.size = encodedPacketSize;
        int decodedSampleBytes = AVCODEC_MAX_AUDIO_FRAME_SIZE; /* pcmBufferSize * 2;  max. size of out buffer in bytes,
        gets over written by actual bytes used. */
        /* ffmpeg decoder framework requires huge buffer so that it can be stupid about
           memory allocation and output buffer size estimation.  I would have rather decoded
           directly into pcmAudioBuffer rather than have to use a temp buffer and copy. */

        // HACK: FFMpeg wants tmpBuffer to be AVCODEC_MAX_AUDIO_FRAME_SIZE, but that blows our thread stack, so we lie
        int16_t tmpBuffer[4096]; 
        int decodedBytesConsumed = avcodec_decode_audio3(codecContext, tmpBuffer,
                                                         &decodedSampleBytes, &ffmpegPacket);
        if(decodedBytesConsumed < 0)
        {
            printf("avcodec_decode_audio3 returned: %d sipX buffer size: %d (bytes) AVCODEC_MAX_AUDIO_FRAME_SIZE: %d encodedPacketSize: %d\n", 
                   decodedBytesConsumed, (int) (pcmBufferSize * sizeof(int16_t)), AVCODEC_MAX_AUDIO_FRAME_SIZE, encodedPacketSize);
            returnValue = RPLG_FAILED;
            *decodedSamples = 0;
        }
        else
        {
            *decodedSamples = decodedSampleBytes / sizeof(int16_t);
            assert(decodedSampleBytes <= (pcmBufferSize * sizeof(int16_t)));
            memcpy(pcmAudioBuffer, tmpBuffer, decodedSampleBytes <= pcmBufferSize * sizeof(int16_t) ? decodedSampleBytes : pcmBufferSize * sizeof(int16_t)); 
        }
        assert(decodedBytesConsumed == encodedPacketSize);
    }

    return(returnValue);
}


int sipxAacCommonEncode(void* opaqueCodecContext, const void* pcmAudioBuffer,
                                     unsigned pcmAudioSamples, int* samplesConsumed,
                                     void* encodedData, unsigned maxCodedDataSize,
                                     int* encodedDataSize, unsigned* sendNow)
{
    int returnCode = RPLG_FAILED;
    AVCodecContext* codecContext = (AVCodecContext*) opaqueCodecContext;
    assert(codecContext->pkt);
    assert(codecContext->pkt->data);
    assert((codecContext->pkt->pos/sizeof(audio_sample_t)) + pcmAudioSamples <= codecContext->pkt->size);
    if(codecContext->pkt == NULL || codecContext->pkt->data == NULL)
    {
        returnCode = RPLG_CORRUPTED_DATA;
    }
    else if((codecContext->pkt->pos/sizeof(audio_sample_t)) + pcmAudioSamples > codecContext->pkt->size)
    {
        returnCode = RPLG_BUFFER_TOO_SMALL;
    }
    else
    {
        memcpy(&(codecContext->pkt->data[codecContext->pkt->pos]), pcmAudioBuffer, pcmAudioSamples * sizeof(audio_sample_t));
        codecContext->pkt->pos += pcmAudioSamples * sizeof(audio_sample_t);
        *samplesConsumed = pcmAudioSamples;

        /* size in bytes of PCM samples for frame of AAC_LC */
        int consumedInputBytes = SAMPLE_PER_AAC_LC_FRAME * sizeof(audio_sample_t);

        if((codecContext->pkt->pos / sizeof(audio_sample_t)) >= SAMPLE_PER_AAC_LC_FRAME)
        {
            *encodedDataSize = avcodec_encode_audio(codecContext, 
                                                    encodedData, maxCodedDataSize,
                                                    (const short*) codecContext->pkt->data);
            if(*encodedDataSize < 0)
            {
                printf("avcodec_encode_audio returned: %d\n", *encodedDataSize);
                *encodedDataSize = 0;
                returnCode = RPLG_FAILED;
            }
            else
            {
                /* Left over samples, shift them to to front */
                if(consumedInputBytes < codecContext->pkt->pos)
                {
                    memcpy(&(codecContext->pkt->data[0]), &(codecContext->pkt->data[consumedInputBytes]), 
                           codecContext->pkt->pos - consumedInputBytes);
                }

                codecContext->pkt->pos -= consumedInputBytes;
                returnCode = RPLG_SUCCESS;
            }
        }
        else
        {
            *encodedDataSize = 0;
            returnCode = RPLG_SUCCESS;
        }

    }

    *sendNow = FALSE;

    return(returnCode);
}

