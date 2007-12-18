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

static const char codecMIMEsubtype[] = "speex-uwb";

static const struct plgCodecInfoV1 codecSPEEX_UWB = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   codecMIMEsubtype,                //mimeSubtype
   "speex-uwb",                     //codecName
   "SpeexUWB codec",                //codecVersion
   32000,                           //samplingRate
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
   "",
   "mode=2",
   "mode=3",
   "mode=4",
   "mode=5",
   "mode=6",
   "mode=7",
   "mode=8"
};

CODEC_API int PLG_ENUM_V1(speex_uwb)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
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

CODEC_API  void *PLG_INIT_V1(speex_uwb)(const char* fmt, int isDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   return universal_speex_init(fmt, isDecoder, pCodecInfo, 32000, &codecSPEEX_UWB);
}


CODEC_API int PLG_FREE_V1(speex_uwb)(void* handle, int isDecoder)
{
   return universal_speex_free(handle, isDecoder);
}


CODEC_API  int PLG_DECODE_V1(speex_uwb)(void* handle, const void* pCodedData, 
                                    unsigned cbCodedPacketSize, void* pAudioBuffer, 
                                    unsigned cbBufferSize, unsigned *pcbDecodedSize, 
                                    const struct RtpHeader* pRtpHeader)
{
   return universal_speex_decode(handle, pCodedData, cbCodedPacketSize,
      pAudioBuffer, cbBufferSize, pcbDecodedSize, pRtpHeader);
}

CODEC_API int PLG_ENCODE_V1(speex_uwb)(void* handle, const void* pAudioBuffer, 
                                   unsigned cbAudioSamples, int* rSamplesConsumed, 
                                   void* pCodedData, unsigned cbMaxCodedData, 
                                   int* pcbCodedSize, unsigned* pbSendNow)
{
   return universal_speex_encode(handle, pAudioBuffer, cbAudioSamples, 
      rSamplesConsumed, pCodedData, cbMaxCodedData, pcbCodedSize, pbSendNow);
}
