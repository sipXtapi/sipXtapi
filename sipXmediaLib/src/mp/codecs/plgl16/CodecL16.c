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

// SYSTEM INCLUDES
#include <memory.h>

#ifdef __pingtel_on_posix__ // [
#  include <netinet/in.h>
#elif defined(WIN32) //  ][ 
#  include <winsock2.h>
#endif

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// EXTERNAL FUNCTIONS
// DEFINES
// STATIC VARIABLE INITIALIZATIONS

int sipxL16decode(const void* pCodedData, unsigned cbCodedPacketSize,
                  void* pAudioBuffer, unsigned cbBufferSize,
                  unsigned *pcbCodedSize)
{
   int i;
   int samples;

   samples = min(cbCodedPacketSize*sizeof(audio_sample_t), cbBufferSize);
   for (i=0; i<samples; i++)
   {
      ((audio_sample_t*)pAudioBuffer)[i] = ntohs(((const audio_sample_t*)pCodedData)[i]);
   }
   *pcbCodedSize = samples;

   return RPLG_SUCCESS;
}

int sipxL16encode(const void* pAudioBuffer, unsigned cbAudioSamples,
                  int* rSamplesConsumed, void* pCodedData,
                  unsigned cbMaxCodedData, int* pcbCodedSize)
{
   unsigned i;

   for (i=0; i<cbAudioSamples; i++)
   {
      ((audio_sample_t*)pCodedData)[i] = htons(((const audio_sample_t*)pAudioBuffer)[i]);
   }
   *pcbCodedSize = cbAudioSamples*sizeof(audio_sample_t);

   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}
