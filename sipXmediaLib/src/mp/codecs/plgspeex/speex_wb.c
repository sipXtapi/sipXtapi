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

#include "plgspeex.h"

static const char codecMIMEsubtype[] = "speex-wb";

static const struct plgCodecInfoV1 codecSPEEX_WB = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   codecMIMEsubtype,                //mimeSubtype
   "speex-wb",                      //codecName
   "SpeexWB codec",                 //codecVersion
   16000,                           //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   38,                              //interleaveBlockSize
   15000,                           //bitRate
   1*8,                             //minPacketBits
   38*8,                            //avgPacketBits
   63*8,                            //maxPacketBits
   160,                             //numSamplesPerFrame
   5                                //preCodecJitterBufferSize
};

static const char* defaultFmtps[] =
{
   "mode=2",
   "mode=3",
   "mode=4",
   "mode=5",
   "mode=6",
   "mode=7",
   "mode=8"
};

CODEC_API int PLG_ENUM_V1(speex_wb)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
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

CODEC_API  void *PLG_INIT_V1(speex_wb)(const char* fmt, int isDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   return universe_speex_init(fmt, isDecoder, pCodecInfo, 16000, &codecSPEEX_WB);
}


CODEC_API int PLG_FREE_V1(speex_wb)(void* handle, int isDecoder)
{
   return universe_speex_free(handle, isDecoder);
}


CODEC_API  int PLG_DECODE_V1(speex_wb)(void* handle, const void* pCodedData, 
                                    unsigned cbCodedPacketSize, void* pAudioBuffer, 
                                    unsigned cbBufferSize, unsigned *pcbDecodedSize, 
                                    const struct RtpHeader* pRtpHeader)
{
   return universe_speex_decode(handle, pCodedData, cbCodedPacketSize,
      pAudioBuffer, cbBufferSize, pcbDecodedSize, pRtpHeader);
}

CODEC_API int PLG_ENCODE_V1(speex_wb)(void* handle, const void* pAudioBuffer, 
                                   unsigned cbAudioSamples, int* rSamplesConsumed, 
                                   void* pCodedData, unsigned cbMaxCodedData, 
                                   int* pcbCodedSize, unsigned* pbSendNow)
{
   return universe_speex_encode(handle, pAudioBuffer, cbAudioSamples, 
      rSamplesConsumed, pCodedData, cbMaxCodedData, pcbCodedSize, pbSendNow);
}
