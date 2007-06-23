//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_GSM /* [ */

#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpeSipxGSM.h"
#include "mp/JB/JB_API.h"
#include "mp/NetInTask.h"  
extern "C" {
#include <gsm.h>
}


const MpCodecInfo MpeSipxGSM::smCodecInfo(
         SdpCodec::SDP_CODEC_GSM,    // codecType
         "GSM 6.10",                 // codecVersion
         false,                      // usesNetEq
         8000,                       // samplingRate
         8,                          // numBitsPerSample
         1,                          // numChannels
         160,                        // interleaveBlockSize
         13200,                      // bitRate. It doesn't matter right now.
         33*8,                       // minPacketBits
         33*8,                       // avgPacketBits
         33*8,                       // maxPacketBits
         160);                       // numSamplesPerFrame


MpeSipxGSM::MpeSipxGSM(int payloadType)
: MpEncoderBase(payloadType, &smCodecInfo)
, mpGsmState(NULL)
, mBufferLoad(0)
{
   assert(CODEC_TYPE_GSM == payloadType);
}

MpeSipxGSM::~MpeSipxGSM()
{
   freeEncode();
}

OsStatus MpeSipxGSM::initEncode(void)
{
   assert(NULL == mpGsmState);
   mpGsmState = gsm_create();

   return OS_SUCCESS;
}

OsStatus MpeSipxGSM::freeEncode(void)
{
   if (NULL != mpGsmState)
   {
      gsm_destroy(mpGsmState);
      mpGsmState = NULL;
   }
   return OS_SUCCESS;
}

OsStatus MpeSipxGSM::encode(const MpAudioSample* pAudioSamples,
                            const int numSamples,
                            int& rSamplesConsumed,
                            unsigned char* pCodeBuf,
                            const int bytesLeft,
                            int& rSizeInBytes,
                            UtlBoolean& sendNow,
                            MpAudioBuf::SpeechType& rAudioCategory)
{
   int size = 0;   
   
   memcpy(&mpBuffer[mBufferLoad], pAudioSamples, sizeof(MpAudioSample)*numSamples);
   mBufferLoad = mBufferLoad+numSamples;
   assert(mBufferLoad <= 160);

   // Check for necessary number of samples
   if (mBufferLoad == 160)
   {
      size = 33;
      gsm_encode(mpGsmState, (gsm_signal*)mpBuffer, (gsm_byte*)pCodeBuf);
      mBufferLoad = 0;
      sendNow = true;
   } else {
      sendNow = false;
   }

   rSamplesConsumed = numSamples;
   rSizeInBytes = size;
   rAudioCategory = MpAudioBuf::MP_SPEECH_UNKNOWN;
   return OS_SUCCESS;
}

#endif /* HAVE_GSM ] */
