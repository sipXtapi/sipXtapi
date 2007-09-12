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

#ifndef _PlgDefs_h_
#define _PlgDefs_h_

#ifdef cplusplus
extern "C" {
#endif

#include "mp/RtpHeader.h"

#ifndef CODEC_STATIC
#define CODEC_DYNAMIC
#endif

#ifndef FALSE
#define FALSE  (0!=0)
#endif

#ifndef TRUE
#define TRUE   (0==0)
#endif

struct plgCodecInfoV1
{
   unsigned    cbSize;					///< Size of srtuct for computability with future version
//   unsigned	   codecSDPType;		   ///< codec type 
   const char* mimeSubtype;         ///< codec type 
   const char* codecName;				///< string identifying the codec name
   const char* codecVersion;			///< string identifying the codec version
   unsigned    samplingRate;			///< sampling rate for the PCM data expected by the codec
//	unsigned    numBitsPerSample,
   unsigned    fmtAndBitsPerSample;    ///< Format of data and bits per sample count
                                       ///< e.g. AFMT_U16LE, AFMT_S16LE, AFMT_S16BE, AFMT_U16BE
   unsigned    numChannels;			   ///< number of channels supported by the codec
   unsigned    interleaveBlockSize; 	///< size of the interleave block (in samples)
   unsigned    bitRate;				      ///< bit rate for this codec (in bits per second)
   unsigned    minPacketBits;			   ///< minimum number of bits in an encoded frame
   unsigned    avgPacketBits;			   ///< average number of bits in an encoded frame
   unsigned    maxPacketBits;			   ///< maximum number of bits in an encoded frame
   unsigned    numSamplesPerFrame;		///< number of PCM samples per input frame
										         ///< for this codec
   unsigned    preCodecJitterBufferSize; ///< requested length of jitter
										///< buffer that the flowgraph should apply to
										///< the stream prior to getting packets for
										///< the codec. If set to 0, then there is NO
										///< jitter buffer, which implies that the
										///< codec itself is doing the JB function.

   unsigned    codecSupportPLC; 
   unsigned    codecSupportNotification;
};

#define DECLARE_FUNCS_V1(x) \
   void* PLG_INIT_V1(x)(const char* fmt, int bDecoder, struct plgCodecInfoV1* pCodecInfo); \
   int   PLG_ENUM_V1(x)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes); \
   int   PLG_FREE_V1(x)(void* handle); \
   int   PLG_DECODE_V1(x)(void* handle, const void* pCodedData, unsigned cbCodedPacketSize, void* pAudioBuffer, unsigned cbBufferSize, unsigned *pcbDecodedSize, const struct RtpHeader* pRtpHeader); \
   int   PLG_ENCODE_V1(x)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData, unsigned cbMaCodedData, int* pcbCodedSize, unsigned* pbSendNow);  \

#define CPP_DECLARE_FUNCS_V1(x)  \
extern "C"  DECLARE_FUNCS_V1(x)

#define PLG_GET_CODEC_NAME             get_codecs_v1
#define PLG_INIT_V1(x)		            x##_init_v1
#define PLG_DECODE_V1(x)	            x##_decode_v1
#define PLG_ENCODE_V1(x)	            x##_encode_v1
#define PLG_FREE_V1(x)		            x##_free_v1
#define PLG_ENUM_V1(x)                 x##_enumsdp_modes_v1
#define PLG_SIGNALING_V1(x)            x##_signaling_v1

#define MSK_INIT_V1		   	         "_init_v1"
#define MSK_DECODE_V1	   	         "_decode_v1"
#define MSK_ENCODE_V1	   	         "_encode_v1"
#define MSK_FREE_V1		   	         "_free_v1"
#define MSK_ENUM_V1                    "_enumsdp_modes_v1"
#define MSK_SIGNALING_V1               "_signaling_v1"

#define MSK_GET_CODEC_NAME_V1    	   "get_codecs_v1"

#define RPLG_SUCCESS                    0
#define RPLG_INVALID_ARGUMENT           (-1)
#define RPLG_FAILED                     (-2)
#define RPLG_NOT_SUPPORTED              (-3)
#define RPLG_INVALID_SEQUENCE_CALL      (-4)
#define RPLG_BAD_HANDLE                 (-5)

struct sdpModeInfo {
/*   unsigned int sdpType; */
   const char* modeString;
};

#define PREPARE_DECODER    1
#define PREPARE_ENCODER    0

#define SIGNALING_DEFAULT_TYPE   0

typedef int   (*dlGetCodecsV1)(int iNum, const char** pCodecModuleName);

typedef int   (*dlPlgEnumSDPAndModesV1)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes);
typedef void* (*dlPlgInitV1)(const char* fmt, int bDecoder, struct plgCodecInfoV1* pCodecInfo);
typedef int   (*dlPlgGetSignalingDataV1)(void* handle, int dataId, uint32_t* outEvent, uint32_t* outDuration, uint32_t* startStatus, uint32_t *stopStatus);
typedef int   (*dlPlgDecodeV1)(void* handle, const void* pCodedData, unsigned cbCodedPacketSize, void* pAudioBuffer, unsigned cbBufferSize, unsigned *pcbDecodedSize,
                               const struct RtpHeader* pRtpHeader);
typedef int   (*dlPlgEncodeV1)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData,
								unsigned cbMaxCodedData, int* pcbCodedSize, unsigned* pbSendNow);
typedef int   (*dlPlgFreeV1)(void* handle);


#ifdef CODEC_DYNAMIC
# ifdef WIN32
#  define CODEC_API    __declspec(dllexport)
# else
#  define CODEC_API
# endif
#else
# define CODEC_API
#endif

#define IPLG_ENUM_CODEC_NAME       plugin_emum_codec
#define IPLG_ENUM_CODEC_START      static const char* IPLG_ENUM_CODEC_NAME [] = {
#define IPLG_ENUM_CODEC(x)         #x ,
#define IPLG_ENUM_CODEC_END        NULL};
#define IPLG_ENUM_CODEC_FUNC      \
   CODEC_API int PLG_GET_CODEC_NAME (int iNum, const char** pCodecModuleName)  {     \
   int i = (sizeof ( IPLG_ENUM_CODEC_NAME ) / sizeof ( IPLG_ENUM_CODEC_NAME[0] )) - 1; \
   if ((iNum < 0) || (iNum > i)) return RPLG_FAILED;                                 \
   *pCodecModuleName = IPLG_ENUM_CODEC_NAME [iNum]; return RPLG_SUCCESS; }


#ifdef CODEC_DYNAMIC
#define PLG_ENUM_CODEC_START        IPLG_ENUM_CODEC_START
#define PLG_ENUM_CODEC(x)           IPLG_ENUM_CODEC(x) 
#define PLG_ENUM_CODEC_END          IPLG_ENUM_CODEC_END  \
                                    IPLG_ENUM_CODEC_FUNC
#else
#define PLG_ENUM_CODEC_START        
#define PLG_ENUM_CODEC(x)           
#define PLG_ENUM_CODEC_END          
#endif


#define PLG_SINGLE_CODEC(x)         \
   PLG_ENUM_CODEC_START             \
   PLG_ENUM_CODEC(x)                \
   PLG_ENUM_CODEC_END   

#define PLG_DOUBLE_CODECS(x,y)      \
   PLG_ENUM_CODEC_START             \
   PLG_ENUM_CODEC(x)                \
   PLG_ENUM_CODEC(y)                \
   PLG_ENUM_CODEC_END   

/* ABI format declarations */
/* Currently only signed, 16 bit, little-endian format supported */
#define SIZE_OF_SAMPLE          2
#define SIZE_OF_SAMPLE_BITS     (SIZE_OF_SAMPLE * 8)

typedef int16_t audio_sample_t;


#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef cplusplus
};
#endif


#endif //_PlgDefs_h_
