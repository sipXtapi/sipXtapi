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

#ifndef _plgspeex_h_
#define _plgspeex_h_

#include <mp/codecs/PlgDefsV1.h>

void* universal_speex_init(const char* fmt, int isDecoder, int samplerate,
                           struct MppCodecFmtpInfoV1_2* pCodecInfo);

int universal_speex_free(void* handle, int isDecoder);

int universal_speex_get_packet_samples(void          *handle,
                                       const uint8_t *pPacketData,
                                       unsigned       packetSize,
                                       unsigned      *pNumSamples,
                                       const struct RtpHeader* pRtpHeader);

int universal_speex_decode(void* handle, const void* pCodedData, 
                          unsigned cbCodedPacketSize, void* pAudioBuffer, 
                          unsigned cbBufferSize, unsigned *pcbDecodedSize, 
                          const struct RtpHeader* pRtpHeader);

int universal_speex_encode(void* handle, const void* pAudioBuffer, 
                          unsigned cbAudioSamples, int* rSamplesConsumed, 
                          void* pCodedData, unsigned cbMaxCodedData, 
                          int* pcbCodedSize, unsigned* pbSendNow);

#endif



