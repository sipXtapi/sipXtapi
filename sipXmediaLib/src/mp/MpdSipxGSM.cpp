//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_GSM /* [ */

// WIN32: Add libgsm to linker input.
#ifdef WIN32 // [
#   ifdef _DEBUG // [
#      pragma comment(lib, "gsmd.lib")
#   else // _DEBUG ][
#      pragma comment(lib, "gsm.lib")
#   endif // _DEBUG ]
#endif // WIN32 ]

// APPLICATION INCLUDES
#include "mp/MpdSipxGSM.h"
extern "C" {
#include <gsm.h>
}


const MpCodecInfo MpdSipxGSM::smCodecInfo(
         SdpCodec::SDP_CODEC_GSM,    // codecType
         "GSM 6.10",                 // codecVersion
         false,                      // usesNetEq
         8000,                       // samplingRate
         8,                          // numBitsPerSample (not used)
         1,                          // numChannels
         160,                        // interleaveBlockSize
         13200,                      // bitRate
         33 * 8,                     // minPacketBits
         33 * 8,                     // avgPacketBits
         33 * 8,                     // maxPacketBits
         160,                        // numSamplesPerFrame
         6);                         // preCodecJitterBufferSize (should be adjusted)

              

MpdSipxGSM::MpdSipxGSM(int payloadType)
: MpDecoderBase(payloadType, &smCodecInfo)
, mpGsmState(NULL)
{
}

MpdSipxGSM::~MpdSipxGSM()
{
}

OsStatus MpdSipxGSM::initDecode()
{
   if (mpGsmState == NULL) 
      mpGsmState = gsm_create();

   return OS_SUCCESS;
}

OsStatus MpdSipxGSM::freeDecode(void)
{
   if (mpGsmState != NULL) 
   {
      gsm_destroy(mpGsmState);
      mpGsmState = NULL;
   }

   return OS_SUCCESS;
}

int MpdSipxGSM::decode(const MpRtpBufPtr &pPacket, unsigned decodedBufferLength, MpAudioSample *samplesBuffer)
{
   // Assert that available buffer size is enough for the packet.
   assert(33 == pPacket->getPayloadSize());
   if (33 != pPacket->getPayloadSize())
      return 0;

   if (decodedBufferLength < 160)
   {
      osPrintf("MpdSipxGSM::decode: Jitter buffer overloaded. Glitch!\n");
      return 0;
   }

   gsm_decode(mpGsmState, (gsm_byte*)pPacket->getDataPtr(), (gsm_signal*)samplesBuffer);
   return 160;
}

#endif /* HAVE_GSM ] */
