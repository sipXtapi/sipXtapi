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

#ifndef _plgg726_h_
#define _plgg726_h_

#include <mp/codecs/PlgDefsV1.h>

#include <spandsp/bitstream.h>
#include <spandsp/g726.h>

#ifndef G726_PACKING_NONE
#define G726_PACKING_NONE 0
#endif

#ifndef G726_ENCODING_LINEAR
#define G726_ENCODING_LINEAR	0
#endif

int internal_decode_g726(void* handle, const void* pCodedData, 
                         unsigned cbCodedPacketSize, void* pAudioBuffer, 
                         unsigned cbBufferSize, unsigned *pcbCodedSize, 
                         const struct RtpHeader* pRtpHeader);

int internal_encode_g726(void* handle, const void* pAudioBuffer, 
                         unsigned cbAudioSamples, int* rSamplesConsumed, 
                         void* pCodedData, unsigned cbMaxCodedData, 
                         int* pcbCodedSize, unsigned* pbSendNow);

#endif



