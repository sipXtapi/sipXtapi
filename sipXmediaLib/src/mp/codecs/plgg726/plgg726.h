//  
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _plgg726_h_
#define _plgg726_h_
#ifdef __pingtel_on_posix__
#  include <mp/MpTypes.h>
#  include <config.h>
#  include <spandsp/telephony.h>
#else
   // On Windows we always could use latest SpanDSP, so we do not need HAVE_OLD_LIBSPANDSP
#  undef HAVE_OLD_LIBSPANDSP
#endif
#include <mp/codecs/PlgDefsV1.h>

#if HAVE_OLD_LIBSPANDSP
#  define G726_PACKING_NONE    0
#  define G726_PACKING_LEFT    1
#  define G726_ENCODING_LINEAR 0
#else
#  include <spandsp/bitstream.h>
#endif

#include <spandsp/g726.h>


int internal_decode_g726(void* handle, const void* pCodedData, 
                         unsigned cbCodedPacketSize, void* pAudioBuffer, 
                         unsigned cbBufferSize, unsigned *pcbCodedSize, 
                         const struct RtpHeader* pRtpHeader);

int internal_encode_g726(void* handle, const void* pAudioBuffer, 
                         unsigned cbAudioSamples, int* rSamplesConsumed, 
                         void* pCodedData, unsigned cbMaxCodedData, 
                         int* pcbCodedSize, unsigned* pbSendNow);

#endif



