/*  
// Copyright (C) 2007-2013 SIPez LLC. All rights reserved.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////*/

/* Author: Daniel Petrie <dpetrie AT sipez DOT com> */

/* SYSTEM INCLUDES */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//#include <arpa/inet.h>

/* APPLICATION INCLUDES */
#include "config.h"
#include <mp/codecs/PlgDefsV1.h>

/* CODEC LIBRARY INCLUDES */
#include <opus.h>

/* EXTERNAL VARIABLES */
/* CONSTANTS */
/* TYPEDEFS */
/* LOCAL DATA TYPES */
/* EXTERNAL FUNCTIONS */
DECLARE_FUNCS_V1(opus_48000)

static const char* defaultFmtps[] =
{
   "",
   "stereo=0; sprop-stereo=0"
};

static const struct MppCodecInfoV1_1 sgCodecInfo_opus_48000 =
{
/*/////////// Implementation and codec info ////////////*/
   "Xiph.org",                 /* codecManufacturer */
   "Opus",                     /* codecName */
   PACKAGE_VERSION,            /* codecVersion */
   CODEC_TYPE_FRAME_BASED,     /* codecType */

/*///////////////////// SDP info /////////////////////*/
   "opus",                      /* mimeSubtype */
   sizeof(defaultFmtps)/sizeof(defaultFmtps[0]), /* fmtpsNum */
   defaultFmtps,                /* fmtps */
   48000,                       /* sampleRate */
   2,                           /* numChannels */
   CODEC_FRAME_PACKING_NONE     /* framePacking */
};

enum OpusCodecType
{
    ENCODE_DECODE_UNKNOWN = 0,
    CODEC_DECODE,
    CODEC_ENCODE
};

struct MpCodecOpusCodecState
{
    enum OpusCodecType mCodecType;
    char* mFmtp;
    OpusEncoder* mpEncoderContext;
    OpusDecoder* mpDecoderContext;
};

int OpusToPluginError(int opusError)
{
    int status;
    switch(opusError)
    {

    case OPUS_OK:
        status = RPLG_SUCCESS;
    break;

    case OPUS_BAD_ARG:
        status = RPLG_INVALID_ARGUMENT;
    break;

    case OPUS_BUFFER_TOO_SMALL:
        status = RPLG_BUFFER_TOO_SMALL;
    break;

    case OPUS_INTERNAL_ERROR:
        status = RPLG_FAILED;
    break;

    case OPUS_INVALID_PACKET:
        status = RPLG_CORRUPTED_DATA;
    break;

    case OPUS_UNIMPLEMENTED:
        status = RPLG_NOT_SUPPORTED;
    break;

    case OPUS_INVALID_STATE:
        status = RPLG_BAD_HANDLE;
    break;

    case OPUS_ALLOC_FAIL:

    default:
        status = RPLG_FAILED;
    break;
    }

    return(status);
}

CODEC_API int PLG_GET_INFO_V1_1(opus_48000)(const struct MppCodecInfoV1_1 **codecInfo) \
{
   if(codecInfo)
   {
      *codecInfo = &sgCodecInfo_opus_48000;
   }
   return RPLG_SUCCESS;
}

CODEC_API void* PLG_INIT_V1_2(opus_48000)(const char* fmtp, int isDecoder,
                                          struct MppCodecFmtpInfoV1_2* codecInfo)
{
    if (codecInfo == NULL)
    {
        return NULL;
    }

    /* Fill general codec information */
    codecInfo->signalingCodec = FALSE;
    /* It could do PLC, but wrapper should be fixed to support it. */
    codecInfo->packetLossConcealment = CODEC_PLC_INTERNAL;
    /* It could do DTX+CNG, but wrapper should be fixed to support it. */
    codecInfo->vadCng = CODEC_CNG_NONE;

    int createError = OPUS_UNIMPLEMENTED;
    struct MpCodecOpusCodecState* codecContext = 
        malloc(sizeof(struct MpCodecOpusCodecState));
    codecContext->mCodecType = ENCODE_DECODE_UNKNOWN;
    codecContext->mFmtp = malloc(strlen(fmtp) + 1);
    strcpy(codecContext->mFmtp, fmtp);
    codecContext->mpEncoderContext = NULL;
    codecContext->mpDecoderContext = NULL;

    // Init decoder
    if(isDecoder)
    {
        codecContext->mCodecType = CODEC_DECODE;
        /* note: we can optimize this by using flowgraph sample, however the 
           MprDecoder and MprEncoder need an enhansement to allow RTP timestamp
           to run at 48000 samples/second and the actual samples at a different
           rate (e.g. 16000 samples/sec). */
        codecContext->mpDecoderContext = opus_decoder_create(48000, /* Output sample rate */
                                                             1, /* output num channels */
                                                             &createError);

        switch(createError)
        {
        case OPUS_OK:
        break;

        default:
            mppLogError("opus_48000 init(fmpt: \"%s\" decoder returned error: %d", fmtp, createError);
        break;
        }
    }

    // Init encoder
    else
    {
        codecContext->mCodecType = CODEC_ENCODE;
        /* note: we can optimize this by using flowgraph sample, however the 
           MprDecoder and MprEncoder need an enhansement to allow RTP timestamp
           to run at 48000 samples/second and the actual samples at a different
           rate (e.g. 16000 samples/sec). */
        codecContext->mpEncoderContext = opus_encoder_create(48000, /* Output sample rate */
                                                             1, /* output num channels */
                                                             OPUS_APPLICATION_VOIP,
                                                             &createError);

        switch(createError)
        {
        case OPUS_OK:
        break;

        default:
            mppLogError("opus_48000 init(fmpt: \"%s\" encoder returned error: %d", fmtp, createError);
            codecContext = NULL;
        break;
        }

    }

    return(codecContext);
}

CODEC_API int PLG_DECODE_V1(opus_48000)(void* opaqueCodecContext, const void* encodedData,
                                        unsigned encodedPacketSize, void* pcmAudioBuffer,
                                        unsigned pcmBufferSize, unsigned* decodedSamples,
                                        const struct RtpHeader* rtpHeader)
{
    int status = RPLG_INVALID_ARGUMENT;
    assert(opaqueCodecContext);

    struct MpCodecOpusCodecState* decoderContext = (struct MpCodecOpusCodecState*) opaqueCodecContext;
    assert(decoderContext->mCodecType == CODEC_DECODE);
    assert(decoderContext->mpDecoderContext);

    int useFec = 1; /* 1=true, 0=false */
    int numSamples = opus_decode(decoderContext->mpDecoderContext, encodedData, encodedPacketSize,
                                pcmAudioBuffer, pcmBufferSize, useFec);
    if(numSamples < 0)
    {
        *decodedSamples = 0;
        status = OpusToPluginError(numSamples);
        mppLogError("opus_decode(decoderContext: %p, packet: %p, packetSize: %d, audioBuf: %p, bufSize: %d, useFec: %d)",
                                  decoderContext->mpDecoderContext, encodedData, encodedPacketSize, pcmAudioBuffer, pcmBufferSize, useFec);
    }

    // Successful decode
    else
    {
        *decodedSamples = numSamples;
        status = RPLG_SUCCESS;
    }

    return(status);
}

CODEC_API int PLG_ENCODE_V1(opus_48000)(void* opaqueCodecContext, const void* pcmAudioBuffer,
                                        unsigned pcmAudioSamples, int* samplesConsumed,
                                        void* encodedData, unsigned maxCodedDataSize,
                                        int* encodedDataSize, unsigned* sendNow)
{
    int status = RPLG_INVALID_ARGUMENT;
    assert(opaqueCodecContext);
    struct MpCodecOpusCodecState* encoderContext = (struct MpCodecOpusCodecState*) opaqueCodecContext;
    assert(encoderContext->mCodecType == CODEC_ENCODE);
    assert(encoderContext->mpEncoderContext);
    if(encoderContext->mpEncoderContext)
    {

        int encodedBytes = opus_encode(encoderContext->mpEncoderContext, 
                                       pcmAudioBuffer,
                                       pcmAudioSamples,
                                       encodedData,
                                       maxCodedDataSize);
        if(encodedBytes >= 0)
        {
            status = RPLG_SUCCESS;
            *encodedDataSize = encodedBytes;
            *samplesConsumed = pcmAudioSamples;
            *sendNow = TRUE;
        }
        else
        {
            status = OpusToPluginError(encodedBytes);
            *encodedDataSize = 0;
            *samplesConsumed = 0;
            *sendNow = FALSE;
            mppLogError("opus_encode buffer: %p, samples: %d, opus error: %d status: %d",
                        pcmAudioBuffer, pcmAudioSamples, encodedBytes, status);
        }
    }

    else
    {
        mppLogError("opus_48000 encode called with NULL context");
    }

    return(status);
}

CODEC_API int PLG_FREE_V1(opus_48000)(void* opaqueCodecContext, int isDecoder) 
{
    int status = RPLG_INVALID_ARGUMENT;
    assert(opaqueCodecContext);
    if(opaqueCodecContext)
    {
        struct MpCodecOpusCodecState* codecContext = (struct MpCodecOpusCodecState*) opaqueCodecContext;
        if(isDecoder)
        {
            assert(codecContext->mCodecType == CODEC_DECODE);
            assert(codecContext->mpDecoderContext);

            if(codecContext->mpDecoderContext)
            {
                opus_encoder_destroy(codecContext->mpDecoderContext);
                status = RPLG_BAD_HANDLE;
                codecContext->mpDecoderContext = NULL;
            }

            else
            {
                status = RPLG_BAD_HANDLE;
            }

        }

        else
        {
            assert(codecContext->mCodecType == CODEC_ENCODE);
            assert(codecContext->mpEncoderContext);

            if(codecContext->mpEncoderContext)
            {
                opus_decoder_destroy(codecContext->mpEncoderContext);
                status = RPLG_BAD_HANDLE;
                codecContext->mpEncoderContext = NULL;
            }

            else
            {
                status = RPLG_BAD_HANDLE;
            }

        }

        if(codecContext->mFmtp)
        {
             free(codecContext->mFmtp);
             codecContext->mFmtp = NULL;
        }

        free(codecContext);
        codecContext = NULL;
        opaqueCodecContext = NULL;
    }

    return(status);
}

PLG_ENUM_CODEC_START(opus)
    PLG_ENUM_CODEC(opus_48000)
    PLG_ENUM_CODEC_NO_SPECIAL_PACKING(opus_48000)
    PLG_ENUM_CODEC_NO_SIGNALING(opus_48000)
PLG_ENUM_CODEC_END

/* ============================== FUNCTIONS =============================== */


