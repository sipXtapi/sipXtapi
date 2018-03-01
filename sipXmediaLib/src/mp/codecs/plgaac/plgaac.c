/*  
// Copyright (C) 2007-2017 SIPez LLC. All rights reserved.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////*/

/* Author: Daniel Petrie <dpetrie AT sipez DOT com> */

/* SYSTEM INCLUDES */
#include <stdlib.h>
#include <arpa/inet.h>

/* APPLICATION INCLUDES */
#include <mp/codecs/PlgDefsV1.h>

/* CODEC LIBRARY INCLUDES */
#include "avcodec.h"
#include "mpeg4audio.h"
#include <aac.h>

/* From FFmpeg */
#undef printf

/* EXTERNAL VARIABLES */
static int sSipxFfmpegCodecInitialized = 0;

/* CONSTANTS */
#define SDP_AUDIO_CONFIG_LINE_SIZE (5)

/* TODO: confirm this: */
#define SAMPLE_PER_AAC_LC_FRAME 1024

#define SIPX_AAC_ENCODER_BUFFER_SIZE (SAMPLE_PER_AAC_LC_FRAME * 2)
#define TMP_BUFFER_SIZE 4096

/* TYPEDEFS */
/* LOCAL DATA TYPES */
enum AacHeaderTypes
{
    NO_HEADER,
    AU_HEADER,
    ADTS_HEADER
};


/* EXTERNAL FUNCTIONS */

void* sipxAacCommonInit(const char* fmtp, int isDecoder,
                        struct MppCodecFmtpInfoV1_2* codecInfo, int sampleRate, int bitRate);

int sipxAacCommonDecode(void* opaqueCodecContext, const void* encodedData,
                                  unsigned encodedPacketSize, void* pcmAudioBuffer,
                                  unsigned pcmBufferSize, unsigned* decodedSamples,
                                  const struct RtpHeader* rtpHeader);

int sipxAacCommonEncode(void* opaqueCodecContext, enum AacHeaderTypes headerType, const void* pcmAudioBuffer,
                                     unsigned pcmAudioSamples, int* samplesConsumed,
                                     void* encodedData, unsigned maxCodedDataSize,
                                     int* encodedDataSize, unsigned* sendNow);

int sipxAacCommonFree(void* opaqueCodecContext, int isDecoder);


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

void sipxFfmpegCodecInit();

DECLARE_FUNCS_V1(mpeg4_generic_aac_lc_16000)
DECLARE_FUNCS_V1(mpeg4_generic_aac_lc_32000)
DECLARE_FUNCS_V1(mpeg4_generic_aac_lc_48000)
DECLARE_FUNCS_V1(aac_lc_32000)

/* MACROS */
#define DEF_AAC_CODEC(SAMPLE_RATE, BIT_RATE, HEADER_TYPE) \
/* Exported codec information. */ \
static const struct MppCodecInfoV1_1 sgCodecInfo_mpeg4_generic_##SAMPLE_RATE = \
{ \
/*/////////// Implementation and codec info ////////////*/ \
   "FFmpeg",                   /* codecManufacturer */ \
   "MPEG4 AAC_LC/" #SAMPLE_RATE, /* codecName */ \
   "0.8",                      /* codecVersion */ \
   CODEC_TYPE_FRAME_BASED,     /* codecType */ \
\
/*///////////////////// SDP info /////////////////////*/ \
   "mpeg4-generic",             /* mimeSubtype */ \
   0,                           /* fmtpsNum */ \
   NULL,                        /* fmtps */ \
   SAMPLE_RATE,                 /* sampleRate */ \
   1,                           /* numChannels */ \
   CODEC_FRAME_PACKING_NONE     /* framePacking */ \
}; \
\
CODEC_API int PLG_GET_INFO_V1_1(mpeg4_generic_aac_lc_##SAMPLE_RATE)(const struct MppCodecInfoV1_1 **codecInfo) \
{ \
   if (codecInfo) \
   { \
      *codecInfo = &sgCodecInfo_mpeg4_generic_##SAMPLE_RATE; \
   } \
   return RPLG_SUCCESS; \
} \
\
CODEC_API void* PLG_INIT_V1_2(mpeg4_generic_aac_lc_##SAMPLE_RATE)(const char* fmtp, int isDecoder, \
                                            struct MppCodecFmtpInfoV1_2* codecInfo) \
{ \
    return(sipxAacCommonInit(fmtp, isDecoder, codecInfo, SAMPLE_RATE, BIT_RATE)); \
} \
 \
CODEC_API int PLG_DECODE_V1(mpeg4_generic_aac_lc_##SAMPLE_RATE)(void* opaqueCodecContext, const void* encodedData, \
                                     unsigned encodedPacketSize, void* pcmAudioBuffer, \
                                     unsigned pcmBufferSize, unsigned* decodedSamples, \
                                     const struct RtpHeader* rtpHeader) \
{ \
    return(sipxAacCommonDecode(opaqueCodecContext, encodedData, encodedPacketSize, pcmAudioBuffer, \
                               pcmBufferSize, decodedSamples, rtpHeader)); \
} \
 \
CODEC_API int PLG_ENCODE_V1(mpeg4_generic_aac_lc_##SAMPLE_RATE)(void* opaqueCodecContext, const void* pcmAudioBuffer, \
                                     unsigned pcmAudioSamples, int* samplesConsumed, \
                                     void* encodedData, unsigned maxCodedDataSize, \
                                     int* encodedDataSize, unsigned* sendNow) \
{ \
    return(sipxAacCommonEncode(opaqueCodecContext, HEADER_TYPE, pcmAudioBuffer, pcmAudioSamples, samplesConsumed, \
                               encodedData, maxCodedDataSize, encodedDataSize, sendNow)); \
} \
 \
CODEC_API int PLG_FREE_V1(mpeg4_generic_aac_lc_##SAMPLE_RATE)(void* opaqueCodecContext, int isDecoder) \
{ \
    return(sipxAacCommonFree(opaqueCodecContext, isDecoder)); \
}

/*     sample rate, bit rate, header type */
DEF_AAC_CODEC(16000, 16000, AU_HEADER)
DEF_AAC_CODEC(32000, 32000, AU_HEADER)
DEF_AAC_CODEC(48000, 64000, AU_HEADER)

static const struct MppCodecInfoV1_1 sgCodecInfo_aac_lc_32000 =
{
/*/////////// Implementation and codec info ////////////*/
   "FFmpeg",                   /* codecManufacturer */
   "AAC_LC/32000",             /* codecName */
   "0.8",                      /* codecVersion */
   CODEC_TYPE_FRAME_BASED,     /* codecType */

/*///////////////////// SDP info /////////////////////*/
   "aac_lc",                    /* mimeSubtype */
   0,                           /* fmtpsNum */
   NULL,                        /* fmtps */
   32000,                       /* sampleRate */
   1,                           /* numChannels */
   CODEC_FRAME_PACKING_NONE     /* framePacking */
};


CODEC_API int PLG_GET_INFO_V1_1(aac_lc_32000)(const struct MppCodecInfoV1_1 **codecInfo) \
{
   if(codecInfo)
   {
      *codecInfo = &sgCodecInfo_aac_lc_32000;
   }
   return RPLG_SUCCESS;
}

CODEC_API void* PLG_INIT_V1_2(aac_lc_32000)(const char* fmtp, int isDecoder,
                                            struct MppCodecFmtpInfoV1_2* codecInfo)
{
    return(sipxAacCommonInit(fmtp, isDecoder, codecInfo, 32000, /* sample rate */
                             64000 /* bit rate */));
}

CODEC_API int PLG_DECODE_V1(aac_lc_32000)(void* opaqueCodecContext, const void* encodedData,
                                         unsigned encodedPacketSize, void* pcmAudioBuffer,
                                         unsigned pcmBufferSize, unsigned* decodedSamples,
                                         const struct RtpHeader* rtpHeader)
{
    return(sipxAacCommonDecode(opaqueCodecContext, encodedData, encodedPacketSize, pcmAudioBuffer,
                               pcmBufferSize, decodedSamples, rtpHeader));
}

CODEC_API int PLG_ENCODE_V1(aac_lc_32000)(void* opaqueCodecContext, const void* pcmAudioBuffer,
                                     unsigned pcmAudioSamples, int* samplesConsumed,
                                     void* encodedData, unsigned maxCodedDataSize,
                                     int* encodedDataSize, unsigned* sendNow)
{
    return(sipxAacCommonEncode(opaqueCodecContext, ADTS_HEADER, pcmAudioBuffer, pcmAudioSamples, samplesConsumed,
                               encodedData, maxCodedDataSize, encodedDataSize, sendNow));
}

CODEC_API int PLG_FREE_V1(aac_lc_32000)(void* opaqueCodecContext, int isDecoder) 
{
    return(sipxAacCommonFree(opaqueCodecContext, isDecoder));
}

PLG_ENUM_CODEC_START(aac_lc)
    PLG_ENUM_CODEC(mpeg4_generic_aac_lc_16000)
    PLG_ENUM_CODEC_NO_SPECIAL_PACKING(mpeg4_generic_aac_lc_16000)
    PLG_ENUM_CODEC_NO_SIGNALING(mpeg4_generic_aac_lc_16000)

    PLG_ENUM_CODEC(mpeg4_generic_aac_lc_32000)
    PLG_ENUM_CODEC_NO_SPECIAL_PACKING(mpeg4_generic_aac_lc_32000)
    PLG_ENUM_CODEC_NO_SIGNALING(mpeg4_generic_aac_lc_32000)

    PLG_ENUM_CODEC(mpeg4_generic_aac_lc_48000)
    PLG_ENUM_CODEC_NO_SPECIAL_PACKING(mpeg4_generic_aac_lc_48000)
    PLG_ENUM_CODEC_NO_SIGNALING(mpeg4_generic_aac_lc_48000)

    PLG_ENUM_CODEC(aac_lc_32000)
    PLG_ENUM_CODEC_NO_SPECIAL_PACKING(aac_lc_32000)
    PLG_ENUM_CODEC_NO_SIGNALING(aac_lc_32000)
PLG_ENUM_CODEC_END

/* ============================== FUNCTIONS =============================== */

void sipxFfmpegCodecInit()
{
    if(sSipxFfmpegCodecInitialized != TRUE)
    {
        sSipxFfmpegCodecInitialized = TRUE;

       mppLogError("sSipxFfmpegCodecInitialized one time set.\n");
       /* Global FFmpeg codec init */
       avcodec_init();
       avcodec_register_all();
    }
}

void* sipxAacCommonInit(const char* fmtp, int isDecoder,
                        struct MppCodecFmtpInfoV1_2* codecInfo, int sampleRate, int bitRate)
{
    AVCodecContext* codecContext;
    codecContext = NULL; 
    sipxFfmpegCodecInit(); 
 
 
    if (codecInfo != NULL) 
    { 
       /* TODO: what is bit rate for AAC_LC???? */ 
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
        /* AAC consumes 2 frames before encoding the first one. We do not count packaging delay in alg. delay. */
        codecInfo->algorithmicDelay = frameSize; /* (SAMPLE_PER_AAC_LC_FRAME samples) */
        codecInfo->mSetMarker = TRUE;

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
                int openDecoderError;

                /* To make valgrind happy */
                memset(&configBits, 0, SDP_AUDIO_CONFIG_LINE_SIZE);

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

                /* to make vlagrind happy */
                memset(codecContext->extradata, 0, SDP_AUDIO_CONFIG_LINE_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);

                memcpy(codecContext->extradata, configBits, SDP_AUDIO_CONFIG_LINE_SIZE);

                openDecoderError = avcodec_open(codecContext, aacDecoder);
                if(openDecoderError == 0)
                {
                }
                else
                {
                    mppLogError("avcodec_open for AAC decoder returned: %d in %s\n", openDecoderError, __FILE__);

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
                mppLogError("avcodec_find_decoder returned NULL decoder for CODEC_ID_AAC in %s\n", __FILE__);
                /* Clean up */
                avcodec_close(codecContext);
                av_free(codecContext->extradata);
                codecContext->extradata = 0;
                av_free(codecContext);
                codecContext = NULL;
                assert(aacDecoder);
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
                    // Defaults to 64000
                    codecContext->bit_rate = bitRate;

                    mppLogError("AAC encoder initialized, init automatically set: frame_size=%d, profile=%d (FF_PROFILE_AAC_LOW=%d, FF_PROFILE_UNKNOWN=%d), bit_rate=%d extradata=%d %d\n",
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
                    mppLogError("avcodec_open for encoder returned: %d in %s\n", codecOpenError, __FILE__);

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
                mppLogError("avcodec_find_encoder returned NULL encoder for CODEC_ID_AAC in %s\n", __FILE__);
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

    mppLogError("sipxAacCommonInit returning codecContext: %p\n", codecContext);
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
        AVPacket ffmpegPacket;
        int decodedSampleBytes;
        /* HACK: FFMpeg wants tmpBuffer to be AVCODEC_MAX_AUDIO_FRAME_SIZE, but that blows our thread stack, so we lie */
        int16_t tmpBuffer[TMP_BUFFER_SIZE + 1]; 
        int decodedBytesConsumed;
        /* If there is an AU header the first 16 bits is the AU header length which defines the total length of the AU headers, 
           one for each frame (there could be more than one frame).  Then the minimal AU header is 16 bits the first 13 bits are the
           AU payload size and the last 3 bits are the ddddddddddd index or delta (if there is more than one frame/AU header.
        */
        const short* auHeaderLengthNetwork = (const short*) encodedData;
        short auHeaderLengthInBits = ntohs(*auHeaderLengthNetwork); /* Generally expect this to be small number (e.g. 16) */
        const short* auHeaderNetwork = (auHeaderLengthNetwork) + 1;
        short auDataLength = (ntohs(*auHeaderNetwork) >> 3);
        int auHeaderLengthInBytes = auHeaderLengthInBits / 8; // length of AU header after first short
        int seqNum = ntohl(rtpHeader->ssrc);

        /* AU header length makes sense and AU header specifies data size which calculates out right */
        int auHeaderLengthLength = 2; // short for header length

        returnValue = RPLG_SUCCESS;

        av_init_packet(&ffmpegPacket);

        if(encodedPacketSize - auHeaderLengthInBytes - auHeaderLengthLength == auDataLength && 
           encodedPacketSize - auHeaderLengthInBytes > 0)
        {
            ffmpegPacket.data = ((const uint8_t*) encodedData) + auHeaderLengthInBytes + auHeaderLengthLength;
            ffmpegPacket.size = encodedPacketSize - auHeaderLengthInBytes - auHeaderLengthLength;
            /* mppLogError("AAC decode AU header found in ssrc: %d\n", seqNum);
            */
        }
        /* ADTS header begins with 12 set bits */
        else if(auHeaderLengthInBits & 0xfff0 == 0xfff0)
        {
            /* validate length of payload in ADTF header */
            int adtsPayloadSize;
            adtsPayloadSize = /* 13 bits accross octets 4-6 */
                 ((*(((const uint8_t*) encodedData) + 3)) & 0x3) * 2048 + /* bottom 2 bits of 4th octet */
                 (*(((const uint8_t*) encodedData) + 4)) * 8 + /* 8 bits of 5th octet */
                 ((*(((const uint8_t*) encodedData) + 5)) & 0xe0) / 32; /* top 3 bits of 6th octet */
            if(adtsPayloadSize == encodedPacketSize)
            {
                /* mppLogError("AAC decode ADTS header found with correct payload length: %d in ssrc: %d\n", 
                       adtsPayloadSize, seqNum);
                */
            }
            else
            {
                mppLogError("AAC decode ADTS header found with incorrect payload length rtp: %d adts: %d in ssrc: %d\n", 
                       encodedPacketSize, adtsPayloadSize, seqNum);
            }

            if(auHeaderLengthInBits & 0x1)
            {
                /* No CRC header is 7 octets long */
                ffmpegPacket.data = ((const uint8_t*) encodedData) + 7;
                ffmpegPacket.size = encodedPacketSize - 7;
            }
            else
            {
                /* CRC included, header is 9 octets long */
                ffmpegPacket.data = ((const uint8_t*) encodedData) + 9;
                ffmpegPacket.size = encodedPacketSize - 9;
            }
            /*mppLogError("AAC decode ADTS header found %s CRC in ssrc: %d\n",
                   (auHeaderLengthInBits & 0x1) ? "without" : "with", seqNum);
           */
        }
        /*  Else assume no header and use whole payload for AAC frame */
        else
        {
            ffmpegPacket.data = (const uint8_t *) encodedData;
            ffmpegPacket.size = encodedPacketSize;
            mppLogError("AAC decode no header found in ssrc: %d first octets: %x %x\n", 
                   seqNum, (int) *(((const uint8_t*) encodedData) + 0), (int) *(((const uint8_t*) encodedData) + 1));
        
            //mppLogError("encodedPacketSize: %d auHeaderLengthInBytes: %d auDataLength: %d\n",    
            //    encodedPacketSize, auHeaderLengthInBytes, auDataLength);
        }
        decodedSampleBytes = AVCODEC_MAX_AUDIO_FRAME_SIZE; /* pcmBufferSize * 2;  max. size of out buffer in bytes,
        gets over written by actual bytes used. */
        /* ffmpeg decoder framework requires huge buffer so that it can be stupid about
           memory allocation and output buffer size estimation.  I would have rather decoded
           directly into pcmAudioBuffer rather than have to use a temp buffer and copy. */

        tmpBuffer[TMP_BUFFER_SIZE] = 0xab;
        decodedBytesConsumed = avcodec_decode_audio3(codecContext, tmpBuffer,
                                                         &decodedSampleBytes, &ffmpegPacket);
        if(tmpBuffer[TMP_BUFFER_SIZE] != 0xab)
        {
            mppLogError("avcodec_decode_audio3 clobbered end of tmpBuffer size: %d\n",
                    TMP_BUFFER_SIZE);
        }
        if(decodedBytesConsumed < 0)
        {
            mppLogError("avcodec_decode_audio3 returned: %d sipX buffer size: %d (bytes) AVCODEC_MAX_AUDIO_FRAME_SIZE: %d encodedPacketSize: %d ffmpegPacket.size: %d\n", 
                   decodedBytesConsumed, (int) (pcmBufferSize * sizeof(int16_t)), AVCODEC_MAX_AUDIO_FRAME_SIZE, encodedPacketSize, ffmpegPacket.size);
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


int sipxAacCommonEncode(void* opaqueCodecContext, enum AacHeaderTypes headerType, const void* pcmAudioBuffer,
                                     unsigned pcmAudioSamples, int* samplesConsumed,
                                     void* encodedData, unsigned maxCodedDataSize,
                                     int* encodedDataSize, unsigned* sendNow)
{
    int returnCode = RPLG_FAILED;
    AVCodecContext* codecContext;
    *sendNow = FALSE;
    codecContext = (AVCodecContext*) opaqueCodecContext;
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
        int consumedInputBytes;
        memcpy(&(codecContext->pkt->data[codecContext->pkt->pos]), pcmAudioBuffer, pcmAudioSamples * sizeof(audio_sample_t));
        codecContext->pkt->pos += pcmAudioSamples * sizeof(audio_sample_t);
        *samplesConsumed = pcmAudioSamples;

        /* size in bytes of PCM samples for frame of AAC_LC */
        consumedInputBytes = SAMPLE_PER_AAC_LC_FRAME * sizeof(audio_sample_t);

        if((codecContext->pkt->pos / sizeof(audio_sample_t)) >= SAMPLE_PER_AAC_LC_FRAME)
        {
            int headerSize = 0;

            switch(headerType)
            {
            case NO_HEADER:
            break;

            case AU_HEADER:
                headerSize = 4;
            break;

            case ADTS_HEADER:
                headerSize = 7;
            break;

            default:
                assert(0);
                break;
            }
            /* Leave space for header in packet */
            *encodedDataSize = avcodec_encode_audio(codecContext, 
                                                    (void*) (((uint8_t*)encodedData) + headerSize), maxCodedDataSize - headerSize,
                                                    (const short*) codecContext->pkt->data);
            if(*encodedDataSize < 0)
            {
                mppLogError("avcodec_encode_audio returned: %d\n", *encodedDataSize);
                *encodedDataSize = 0;
                returnCode = RPLG_FAILED;
            }
            else
            {
                int frequencyIndex;
                assert(codecContext);
                switch(codecContext->sample_rate)
                {
                case 16000:
                    frequencyIndex = 8;
                break;

                case 32000:
                    frequencyIndex = 5;
                break;
                case 48000:
                    frequencyIndex = 3;
                break;

                default:
                    assert(0);
                    frequencyIndex = 5;
                break;
                }

                /* Add header only if we have payload data */
                /* A guess is that the encoder does not generate payload for the first frame passed in due */
                /* to the builtin latency and forward error correction of AAC. */
                switch(*encodedDataSize > 0 ? headerType : NO_HEADER)
                {
                case NO_HEADER:
                break;

                case AU_HEADER:
                {
                    short* auHeaderLength = encodedData;
                    short* auHeader = auHeaderLength + 1;
                    *auHeaderLength = htons(16); /* We create a header that is always 16 bits long */
                    *auHeader = htons(*encodedDataSize << 3); /* First 13 bits of AU header is payload size */
                    *encodedDataSize += 4;
                }
                break;

                case ADTS_HEADER:
                    /* good reference: http://wiki.multimedia.cx/index.php?title=ADTS */
                    *encodedDataSize += 7;
                    /* 1st octet always 0xff */
                    *((uint8_t*)encodedData) = 0xff;
                    /* 2nd octet: sync always 0xf0, MPEG-4, no CRC */
                    *(((uint8_t*)encodedData) + 1) = 0xf0 + 0x1; 
                    /* 3rd octet: (MPEG3 audio object type AAC_LC = 2, minus 1), sample freqency index */
                    *(((uint8_t*)encodedData) + 2) = 0x1 * 64 + (frequencyIndex * 4);
                    /* 4th octet: lower 2 bits of channel configuration, top 2 bits of 13 bit payload length including ADTS header */
                    *(((uint8_t*)encodedData) + 3) = (0x1 & 0x3) * 64 + (((*encodedDataSize) / 2048) & 0x3);
                    /* 5th octet: bits 4-11 of payload length including ADTS header */
                    *(((uint8_t*)encodedData) + 4) = (((*encodedDataSize) / 8) & 0xff);
                    /* 6th octet: top 3 bits are the 3 least signficant bits of the 13 bit payload length, buffer fullness all 1s */
                    *(((uint8_t*)encodedData) + 5) = ((*encodedDataSize) & 0x7) * 32 + 0x1f;
                    /* 7th octet: upper 6 bits: buffer fullness all 1s*/
                    *(((uint8_t*)encodedData) + 6) = 0xfc;
                    /* We do not include 8th and 9th octet as we do not include CRC */

                break;

                default:
                    assert(0);
                break;
            }

                /* Left over samples, shift them to to front */
                if(consumedInputBytes < codecContext->pkt->pos)
                {
                    memcpy(&(codecContext->pkt->data[0]), &(codecContext->pkt->data[consumedInputBytes]), 
                           codecContext->pkt->pos - consumedInputBytes);
                }

                codecContext->pkt->pos -= consumedInputBytes;
                returnCode = RPLG_SUCCESS;
                *sendNow = TRUE;
            }
        }
        else
        {
            *encodedDataSize = 0;
            returnCode = RPLG_SUCCESS;
        }

    }


    return(returnCode);
}


int sipxAacCommonFree(void* opaqueCodecContext, int isDecoder)
{
    AVCodecContext* codecContext = (AVCodecContext*) opaqueCodecContext;
    if(!isDecoder)
    {
        /* we use pkt to buffer up enough sipX frames to encode an AAC frame */
        av_free(codecContext->pkt->data);
        codecContext->pkt->data = NULL;
        av_free(codecContext->pkt);
        codecContext->pkt = NULL;
    }
    avcodec_close(codecContext);
    av_free(codecContext->extradata);
    codecContext->extradata =  NULL;
    av_free(codecContext);
    codecContext = NULL;

   return(RPLG_SUCCESS);
}

