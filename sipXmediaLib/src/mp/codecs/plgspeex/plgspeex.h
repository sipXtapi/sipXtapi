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

#ifndef _plgspeex_h_
#define _plgspeex_h_

#include <mp/codecs/PlgDefsV1.h>

void* universal_speex_init(const char* fmt, int isDecoder, 
                          struct plgCodecInfoV1* pCodecInfo, 
                          int samplerate, 
                          const struct plgCodecInfoV1* cdi);

int universal_speex_free(void* handle, int isDecoder);

int universal_speex_decode(void* handle, const void* pCodedData, 
                          unsigned cbCodedPacketSize, void* pAudioBuffer, 
                          unsigned cbBufferSize, unsigned *pcbDecodedSize, 
                          const struct RtpHeader* pRtpHeader);

int universal_speex_encode(void* handle, const void* pAudioBuffer, 
                          unsigned cbAudioSamples, int* rSamplesConsumed, 
                          void* pCodedData, unsigned cbMaxCodedData, 
                          int* pcbCodedSize, unsigned* pbSendNow);

#endif



