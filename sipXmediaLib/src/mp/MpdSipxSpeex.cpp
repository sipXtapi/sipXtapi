//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 Hector Izquierdo Seliva. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_SPEEX /* [ */


// APPLICATION INCLUDES
#include "mp/MpConnection.h"
#include "mp/MpdSipxSpeex.h"
#include "mp/JB/JB_API.h"
#include "mp/MprDejitter.h"
#include "os/OsSysLog.h"
#include "os/OsSysLogFacilities.h"


void* MpdSipxSpeex::smpDecoderState = NULL;
SpeexBits MpdSipxSpeex::decbits;
spx_int16_t MpdSipxSpeex::decbuffer[160];

const MpCodecInfo MpdSipxSpeex::smCodecInfo(
         SdpCodec::SDP_CODEC_SPEEX,    // codecType
         "Speex codec",                // codecVersion
         false,                        // usesNetEq
         8000,                         // samplingRate
         80,                           // numBitsPerSample (not used)
         1,                            // numChannels
         38,                           // interleaveBlockSize
         15000,                        // bitRate
         1*8,                          // minPacketBits
         38*8,                         // avgPacketBits
         63*8,                         // maxPacketBits
         160);                         // numSamplesPerFrame

              

MpdSipxSpeex::MpdSipxSpeex(int payloadType)
: MpDecoderBase(payloadType, &smCodecInfo)
, pJBState(NULL)
{   
   int tmp = 1;

   if (smpDecoderState == NULL) {
      smpDecoderState = speex_decoder_init(&speex_nb_mode);   

      // It makes the decoded speech deviate further from the original,
      // but it sounds subjectively better.
      speex_decoder_ctl(smpDecoderState,SPEEX_SET_ENH,&tmp);
      speex_bits_init(&decbits);
   }
}

MpdSipxSpeex::~MpdSipxSpeex()
{
   freeDecode();
}

OsStatus MpdSipxSpeex::initDecode(MpConnection* pConnection)
{
   pJBState = pConnection->getJBinst();

   JB_initCodepoint(pJBState, "SPEEX", 8000, getPayloadType());
   
   return OS_SUCCESS;
}

OsStatus MpdSipxSpeex::freeDecode(void)
{
   if (smpDecoderState != NULL) {
      speex_decoder_destroy(smpDecoderState);
      smpDecoderState = NULL;

      speex_bits_destroy(&decbits);
   }

   return OS_SUCCESS;
}

int MpdSipxSpeex::decode(int numSamples, JB_uchar *encoded, MpAudioSample *decoded) 
{
   speex_bits_read_from(&decbits,(char*)encoded,numSamples);
   speex_decode_int(smpDecoderState,&decbits,decbuffer);   

   // Copy decoded data to provided buffer.
   memcpy(decoded, decbuffer, numSamples*sizeof(MpAudioSample));

   return 0;
}

#endif /* HAVE_SPEEX ] */
