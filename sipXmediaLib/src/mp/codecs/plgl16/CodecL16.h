//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT sipez DOT com>

#ifndef _CodecL16_h_ // [
#define _CodecL16_h_

int sipxL16decode(const void* pCodedData, unsigned cbCodedPacketSize,
                  void* pAudioBuffer, unsigned cbBufferSize,
                  unsigned *pcbCodedSize);

int sipxL16encode(const void* pAudioBuffer, unsigned cbAudioSamples,
                  int* rSamplesConsumed, void* pCodedData,
                  unsigned cbMaxCodedData, int* pcbCodedSize);

#endif // _CodecL16_h_ ]
