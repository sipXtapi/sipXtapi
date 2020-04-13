//  
// Copyright (C) 2007-2020 SIPez LLC.  All rights reserved.
//
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _PlgDefs_h_
#define _PlgDefs_h_

#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "mp/RtpHeader.h"

#ifndef CODEC_STATIC
#define CODEC_DYNAMIC
#endif

#ifdef CODEC_DYNAMIC
#  ifdef WIN32
#     define CODEC_API    __declspec(dllexport)
#  else
#     define CODEC_API
#  endif
#else
#  define CODEC_API
#endif

#ifndef FALSE
#define FALSE  (0!=0)
#endif

#ifndef TRUE
#define TRUE   (0==0)
#endif

///@name Constants for plgCodecStaticInfoV1::codecType values.
//@{
   /// Codec is frame based (like GSM, iLBC, etc).
#  define CODEC_TYPE_FRAME_BASED       0
   /// Codec is sample based (like G.711, G.726, etc).
#  define CODEC_TYPE_SAMPLE_BASED      1
//@}

///@name Constants for plgCodecStaticInfoV1::framePacking values.
//@{
   /// Frames could be simply concatenated (like GSM, iLBC, etc).
#  define CODEC_FRAME_PACKING_NONE     0
   /// Codec require special processing to pack several frames in one RTP packet
   /// (like AMR, Speex, etc).
#  define CODEC_FRAME_PACKING_SPECIAL  1
//@}

///@name Constants for plgCodecStaticInfoV1::packetLossConcealment values.
//@{
   /// Codec need external PLC.
#  define CODEC_PLC_NONE               0
   /// Codec have internal PLC.
#  define CODEC_PLC_INTERNAL           1
//@}

///@name Constants for plgCodecStaticInfoV1::vadCng values.
//@{
   /// Codec need external PLC.
#  define CODEC_CNG_NONE               0
   /// Codec have internal PLC.
#  define CODEC_CNG_INTERNAL           1
//@}

///@name Return codes for API functions
//@{
#  define RPLG_SUCCESS                    0
#  define RPLG_INVALID_ARGUMENT           (-1)
#  define RPLG_FAILED                     (-2)
#  define RPLG_NOT_SUPPORTED              (-3)
#  define RPLG_CORRUPTED_DATA             (-4) ///< Encoded data is corrupted
#  define RPLG_BAD_HANDLE                 (-5)
#  define RPLG_NO_MORE_DATA               (-6)
#  define RPLG_BUFFER_TOO_SMALL           (-7) ///< Encoding/decoding buffer
                                               ///< is too small for output data.
//@}

///@name Values for PLG_INIT_V1() and PLG_FREE_V1() isDecoder argument
//@{
#  define CODEC_DECODER            1
#  define CODEC_ENCODER            0
//@}

/* c wrapper for OsSysLog::add(FAC_MP, PRI_ERR, const char* format, ...); */
void mppLogError(const char* format, ...);

/* Parsing {param}={value} */
static inline int analizeParamEqValue(const char* parsingString, const char* paramName, int* value)
{
    int tmp;
    char c;
    int eqFound = FALSE;
    int digitFound = FALSE;
    const char* res;
    res = 
#if defined(WIN32)
        (const char*)
#endif
        strstr(parsingString, paramName);
    if (!res) {
        return -1;
    }
    res += strlen(paramName); //Skip name of param world

    for (; (c = *res) != 0; res++)
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
            for (res++; isdigit(c = *res); res++) {
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

/* fmtp parameter parser */
static inline int getFmtpValueRange(const char* fmtp, const char* paramName, int defaultValue, int minValue, int maxValue)
{
    int value;
    int res = (!fmtp) ? (-1) : analizeParamEqValue(fmtp, paramName, &value);
    if ((res == 0) && (value >= minValue) && (value <= maxValue))
        return value;
    return defaultValue;
}



/**
*  @brief Generic information about codec.
*/
struct MppCodecInfoV1_1
{
//   unsigned     cbSize;                 ///< Size of struct for compatibility with future version

///@name Implementation and codec info
//@{
//   const char*  codecId;                ///< Codec unique ID string. Should consist of alphanumeric
                                        ///<   characters and underscores.
   const char*  codecManufacturer;      ///< Codec manufacturer (human readable).
   const char*  codecName;              ///< Codec name (human readable).
   const char*  codecVersion;           ///< Codec version (human readable).
   unsigned     codecType;              ///< See CODEC_TYPE_* defines above.
//@}

///@name SDP info
//@{
   const char*  mimeSubtype;            ///< MIME subtype.
   unsigned     fmtpsNum;               ///< Number of elements in fmtps array.
                                        ///<  Set to 0 if codec does not need fmtp.
   const char** fmtps;                  ///< Array or recommended (supported) fmtp strings.
                                        ///<  Set to NULL if codec does not need fmtp.
   unsigned     sampleRate;             ///< Sample rate for the PCM data expected by the codec
                                        ///<  in samples per second. Only 8kHz, 16kHz and 32kHz
                                        ///<  sample rates are supported.
   unsigned     numChannels;            ///< Number of channels supported by the codec.
                                        ///<  Only single-channel codecs are supported.
   unsigned     framePacking;           ///< See CODEC_FRAME_PACKING_* for details.
//@}
};

/**
*  @brief Information about concrete codec instance (i.e. with selected fmtp).
*/
struct MppCodecFmtpInfoV1_2
{
//   unsigned     cbSize;                ///< Size of struct for compatability with future version.

   unsigned     signalingCodec;         ///< TRUE if codec could carry DTMF tones, FALSE otherwise.

   unsigned     minBitrate;             ///< Minimum bit rate for this codec (in bits per second).
   unsigned     maxBitrate;             ///< Maximum bit rate for this codec (in bits per second).
   unsigned     numSamplesPerFrame;     ///< Number of PCM samples per input frame for frame-based
                                        ///<  codecs. For sample-based codecs should be equal to
                                        ///<  number of samples to generate integer number of bytes
                                        ///<  of encoded data (e.g. 1 for G.711, 2 for G.726-32,
                                        ///<  4 for G.726-16, etc).
   unsigned     minFrameBytes;          ///< Minimum number of bytes produced after encoding
                                        ///<  numSamplesPerFrame samples of PCM data.
   unsigned     maxFrameBytes;          ///< Maximum number of bytes produced after encoding
                                        ///<  numSamplesPerFrame samples of PCM data.
   unsigned     packetLossConcealment;  ///< See CODEC_PLC_* for details.
   unsigned     vadCng;                 ///< See CODEC_CNG_* for details.
   unsigned     algorithmicDelay;       ///< Algorithmic delay (samples) of this encoder/decoder. Also known
                                        ///<  as codec lookahead. This should not include packaging
                                        ///<  delay (which equals to codec packet size). It will be
                                        ///<  taken into account independently. Counted in samples.
   int          mSetMarker;             ///< Whether encoder should set marker bit upon send of complete
                                        ///< frame (currently assumes only whole frames are sent)
};

#define DECLARE_FUNCS_V1(x)                                                         \
 CODEC_API int   PLG_GET_INFO_V1_1(x)(const struct MppCodecInfoV1_1 **codecInfo);   \
 CODEC_API void* PLG_INIT_V1_2(x)(const char* fmtp, int isDecoder,                  \
                                  struct MppCodecFmtpInfoV1_2* pCodecFmtpInfo);     \
 CODEC_API int   PLG_FREE_V1(x)(void* handle, int isDecoder);                       \
 CODEC_API int   PLG_GET_PACKET_SAMPLES_V1_2(x)(void* handle,                       \
                                                const uint8_t* pPacketData,         \
                                                unsigned packetSize,                \
                                                unsigned *pNumSamples,              \
                                                const struct RtpHeader* pRtpHeader);\
 CODEC_API int   PLG_DECODE_V1(x)(void* handle, const void* pCodedData,             \
                                  unsigned cbCodedPacketSize, void* pAudioBuffer,   \
                                  unsigned cbBufferSize, unsigned *pcbDecodedSize,  \
                                  const struct RtpHeader* pRtpHeader);              \
 CODEC_API int   PLG_ENCODE_V1(x)(void* handle, const void* pAudioBuffer,           \
                                  unsigned cbAudioSamples, int* rSamplesConsumed,   \
                                  void* pCodedData, unsigned cbMaxCodedData,        \
                                  int* pcbCodedSize, unsigned* pbSendNow);  

#define CPP_DECLARE_FUNCS_V1(x)  \
extern "C"  DECLARE_FUNCS_V1(x)

#define PLG_GET_CODEC_NAME             get_codecs_v1
#define PLG_GET_INFO_V1_1(x)           x##_get_info_v1_1
#define PLG_INIT_V1_2(x)               x##_init_v1_2
#define PLG_GET_PACKET_SAMPLES_V1_2(x) x##_get_packet_samples_v1_2
#define PLG_DECODE_V1(x)               x##_decode_v1
#define PLG_ENCODE_V1(x)               x##_encode_v1
#define PLG_FREE_V1(x)                 x##_free_v1
#define PLG_SIGNALING_V1(x)            x##_signaling_v1

#define MSK_GET_CODEC_NAME_V1          "get_codecs_v1"
#define MSK_GET_INFO_V1_1              "_get_info_v1_1"
#define MSK_INIT_V1_2                  "_init_v1_2"
#define MSK_GET_PACKET_SAMPLES_V1_2    "_get_packet_samples_v1_2"
#define MSK_DECODE_V1                  "_decode_v1"
#define MSK_ENCODE_V1                  "_encode_v1"
#define MSK_FREE_V1                    "_free_v1"
#define MSK_SIGNALING_V1               "_signaling_v1"

typedef int   (*dlGetCodecsV1)(int iNum, const char** pCodecModuleName);

typedef int   (*dlPlgGetInfoV1_1)(const struct MppCodecInfoV1_1 **codecInfo);
typedef void* (*dlPlgInitV1_2)(const char* fmtp, int isDecoder, struct MppCodecFmtpInfoV1_2* pCodecFmtpInfo);
typedef int   (*dlPlgGetSignalingDataV1)(void* handle, uint32_t* outEvent, uint32_t* outDuration,
                                         uint32_t* startStatus, uint32_t *stopStatus);
typedef int   (*dlPlgGetPacketSamplesV1_2)(void* handle,
                                           const uint8_t* pPacketData,
                                           unsigned packetSize,
                                           unsigned *pNumSamples,
                                           const struct RtpHeader* pRtpHeader);
typedef int   (*dlPlgDecodeV1)(void* handle, const void* pCodedData, unsigned cbCodedPacketSize, 
                               void* pAudioBuffer, unsigned cbBufferSize, unsigned *pcbDecodedSize,
                               const struct RtpHeader* pRtpHeader);
typedef int   (*dlPlgEncodeV1)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples,
                               int* rSamplesConsumed, void* pCodedData, unsigned cbMaxCodedData, 
                               int* pcbCodedSize, unsigned* pbSendNow);
typedef int   (*dlPlgFreeV1)(void* handle, int isDecoder);


#define IPLG_ENUM_CODEC_NAME       plugin_enum_codec
#define IPLG_ENUM_CODEC_START      static const char* IPLG_ENUM_CODEC_NAME [] = {
#define IPLG_ENUM_CODEC(x)         #x ,
#define IPLG_ENUM_CODEC_END        NULL};
#define IPLG_ENUM_CODEC_FUNC      \
   CODEC_API int PLG_GET_CODEC_NAME (int iNum, const char** pCodecModuleName); /* prototype */ \
   CODEC_API int PLG_GET_CODEC_NAME (int iNum, const char** pCodecModuleName)  /* implementation */ \
   { \
       int i = (sizeof ( IPLG_ENUM_CODEC_NAME ) / sizeof ( IPLG_ENUM_CODEC_NAME[0] )) - 1; \
       if ((iNum < 0) || (iNum > i)) return RPLG_FAILED;                                   \
       *pCodecModuleName = IPLG_ENUM_CODEC_NAME [iNum]; return RPLG_SUCCESS; \
   }

#define DEFINE_STATIC_REGISTRATOR                                             \
   void callbackRegisterStaticCodec(const char* moduleName,                   \
                                    const char* codecModuleName,              \
                                    dlPlgInitV1_2 plgInit,                    \
                                    dlPlgGetInfoV1_1 plgGetInfo,              \
                                    dlPlgDecodeV1 plgDecode,                  \
                                    dlPlgEncodeV1 plgEncode,                  \
                                    dlPlgFreeV1 plgFree,                      \
                                    dlPlgGetPacketSamplesV1_2 plgGetPacketSamples, \
                                    dlPlgGetSignalingDataV1 plgSignaling);
 
#define REG_STATIC_NAME(y)          registerStatic_##y

#define REGISTER_STATIC_PLG(x)      void REG_STATIC_NAME(x)(void); \
                                    REG_STATIC_NAME(x)();

#define SPLG_ENUM_CODEC_START(y)    \
   void REG_STATIC_NAME(y) (void) {
#define SPLG_ENUM_CODEC(x)                                      \
   callbackRegisterStaticCodec(__FILE__, #x,                    \
                               PLG_INIT_V1_2(x),                \
                               PLG_GET_INFO_V1_1(x),            \
                               PLG_DECODE_V1(x),                \
                               PLG_ENCODE_V1(x),                \
                               PLG_FREE_V1(x),
#define SPLG_ENUM_CODEC_SPECIAL_PACKING(x)                      \
                               PLG_GET_PACKET_SAMPLES_V1_2(x),
#define SPLG_ENUM_CODEC_NO_SPECIAL_PACKING(x)                   \
                               NULL,
#define SPLG_ENUM_CODEC_SIGNALING(x)                            \
                               PLG_SIGNALING_V1(x));
#define SPLG_ENUM_CODEC_NO_SIGNALING(x)                         \
                               NULL);
#define SPLG_ENUM_CODEC_END  }

#ifdef CODEC_DYNAMIC
#  define PLG_ENUM_CODEC_START(y)              IPLG_ENUM_CODEC_START
#  define PLG_ENUM_CODEC(x)                    IPLG_ENUM_CODEC(x) 
#  define PLG_ENUM_CODEC_SPECIAL_PACKING(x)
#  define PLG_ENUM_CODEC_NO_SPECIAL_PACKING(x)
#  define PLG_ENUM_CODEC_SIGNALING(x)
#  define PLG_ENUM_CODEC_NO_SIGNALING(x)
#  define PLG_ENUM_CODEC_END                   IPLG_ENUM_CODEC_END  \
                                               IPLG_ENUM_CODEC_FUNC
#else
#  define PLG_ENUM_CODEC_START(y)              DEFINE_STATIC_REGISTRATOR \
                                               SPLG_ENUM_CODEC_START(y)
#  define PLG_ENUM_CODEC(x)                    SPLG_ENUM_CODEC(x)
#  define PLG_ENUM_CODEC_SPECIAL_PACKING(x)    SPLG_ENUM_CODEC_SPECIAL_PACKING(x)
#  define PLG_ENUM_CODEC_NO_SPECIAL_PACKING(x) SPLG_ENUM_CODEC_NO_SPECIAL_PACKING(x)
#  define PLG_ENUM_CODEC_SIGNALING(x)          SPLG_ENUM_CODEC_SIGNALING(x)
#  define PLG_ENUM_CODEC_NO_SIGNALING(x)       SPLG_ENUM_CODEC_NO_SIGNALING(x)
#  define PLG_ENUM_CODEC_END                   SPLG_ENUM_CODEC_END
#endif


#define PLG_SINGLE_CODEC_SIG(x)     \
   PLG_ENUM_CODEC_START(x)          \
   PLG_ENUM_CODEC_SIG(x)            \
   PLG_ENUM_CODEC_END

#define PLG_SINGLE_CODEC(x)         \
   PLG_ENUM_CODEC_START(x)          \
   PLG_ENUM_CODEC(x)                \
   PLG_ENUM_CODEC_END

#define PLG_DOUBLE_CODECS(x,y)      \
   PLG_ENUM_CODEC_START(x##y)       \
   PLG_ENUM_CODEC(x)                \
   PLG_ENUM_CODEC(y)                \
   PLG_ENUM_CODEC_END

/* ABI format declarations */
/* Currently only signed, 16 bit, little-endian format supported */
#define SIZE_OF_SAMPLE          2
#define SIZE_OF_SAMPLE_BITS     (SIZE_OF_SAMPLE * 8)

typedef int16_t audio_sample_t;


#ifndef PLG_MAX
#define PLG_MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef PLG_MIN
#define PLG_MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef __cplusplus
};
#endif


#endif //_PlgDefs_h_
