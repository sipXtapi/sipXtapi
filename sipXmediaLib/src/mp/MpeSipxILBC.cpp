//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_ILBC // [

// SYSTEM INCLUDES
#include "assert.h"

// APPLICATION INCLUDES
#include "mp/MpeSipxILBC.h"
#include "mp/JB/JB_API.h"
#include "mp/NetInTask.h"
#include <limits.h>  
extern "C" {
#include <iLBC_define.h>
#include <iLBC_encode.h>
}

const MpCodecInfo MpeSipxILBC::smCodecInfo(
    SdpCodec::SDP_CODEC_ILBC,   // codecType
    "iLBC",                     // codecVersion
    false,                      // usesNetEq
    8000,                       // samplingRate
    8,                          // numBitsPerSample
    1,                          // numChannels
    240,                        // interleaveBlockSize
    13334,                      // bitRate. It doesn't matter right now.
    NO_OF_BYTES_30MS*8,         // minPacketBits
    NO_OF_BYTES_30MS*8,         // avgPacketBits
    NO_OF_BYTES_30MS*8,         // maxPacketBits
    240);                       // numSamplesPerFrame


MpeSipxILBC::MpeSipxILBC(int payloadType)
: MpEncoderBase(payloadType, &smCodecInfo)
, mpState(NULL)
, mBufferLoad(0)
{
}

MpeSipxILBC::~MpeSipxILBC()
{
   freeEncode();
}

OsStatus MpeSipxILBC::initEncode(void)
{
   assert(NULL == mpState);
   mpState = new iLBC_Enc_Inst_t();
   memset(mpState, 0, sizeof(*mpState));
   ::initEncode(mpState, 30);

   return OS_SUCCESS;
}

OsStatus MpeSipxILBC::freeEncode(void)
{
   delete mpState;
   mpState = NULL;

   return OS_SUCCESS;
}

OsStatus MpeSipxILBC::encode(const MpAudioSample* pAudioSamples,
                              const int numSamples,
                              int& rSamplesConsumed,
                              unsigned char* pCodeBuf,
                              const int bytesLeft,
                              int& rSizeInBytes,
                              UtlBoolean& sendNow,
                              MpAudioBuf::SpeechType& rAudioCategory)
{
   memcpy(&mpBuffer[mBufferLoad], pAudioSamples, sizeof(MpAudioSample)*numSamples);
   mBufferLoad += numSamples;
   assert(mBufferLoad <= 240);

   if (mBufferLoad == 240)
   {
      float buffer[240];
      for (int i = 0; i < 240; ++i)
         buffer[i] =  float(mpBuffer[i]);

      iLBC_encode((unsigned char*)pCodeBuf, buffer, mpState);

      mBufferLoad = 0;
      rSizeInBytes = NO_OF_BYTES_30MS;
      sendNow = true;
   } 
   else 
   {
      rSizeInBytes = 0;
      sendNow = false;
   }

   rSamplesConsumed = numSamples;
   rAudioCategory = MpAudioBuf::MP_SPEECH_UNKNOWN;
   return OS_SUCCESS;
}

#endif // HAVE_ILBC ]
