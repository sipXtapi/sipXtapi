//  
// Copyright (C) 2008-2017 SIPez LLC.  All rights reserved.
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
#include <CodecL16.h>

// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// EXTERNAL FUNCTIONS
// DEFINES
// STATIC VARIABLE INITIALIZATIONS

int sipxL16decode(const void* pCodedData, unsigned cbCodedPacketSize,
                  void* pAudioBuffer, unsigned cbBufferSize,
                  unsigned *pcbCodedSize);

/* ============================== FUNCTIONS =============================== */

int sipxL16decode(const void* pCodedData, unsigned cbCodedPacketSize,
                  void* pAudioBuffer, unsigned cbBufferSize,
                  unsigned *pcbCodedSize)
{
   int i;
   int samples;

   samples = PLG_MIN(cbCodedPacketSize/sizeof(audio_sample_t), cbBufferSize);
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

   /* A frame can be larger than the MTU.  Don't put more than will fit. */
   if(cbAudioSamples * sizeof(audio_sample_t) > cbMaxCodedData)
   {
      cbAudioSamples = cbMaxCodedData / sizeof(audio_sample_t);
   }

   for (i=0; i<cbAudioSamples; i++)
   {
      ((audio_sample_t*)pCodedData)[i] = htons(((const audio_sample_t*)pAudioBuffer)[i]);
   }
   *pcbCodedSize = cbAudioSamples*sizeof(audio_sample_t);

   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}
