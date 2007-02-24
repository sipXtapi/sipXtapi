//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//  G.729 enabling controls.  Currently only on VxWorks and Windows
//////////////////////////////////////////////////////////////////////////////

#undef PLATFORM_SUPPORTS_G729

#ifdef HAVE_G729
#define PLATFORM_SUPPORTS_G729
#endif

//////////////////////////////////////////////////////////////////////////////

#include "assert.h"
#include "mp/MpCodecFactory.h"
#include "os/OsSysLog.h"

// all encoder child classes
#include "mp/MpePtAVT.h"

#ifdef HAVE_GIPS /* [ */
#include "mp/MpeGIPSPCMA.h"
#include "mp/MpeGIPSPCMU.h"
#include "mp/MpeGIPSiPCMA.h"
#include "mp/MpeGIPSiPCMU.h"
#include "mp/MpeGIPSiPCMWB.h"

#ifdef PLATFORM_SUPPORTS_G729 /* [ */
#include "mp/MpeGIPSG729ab.h"
#endif /* PLATFORM_SUPPORTS_G729 ] */

#ifdef WIN32 /* [ */
#include "mp/MpeGIPSiLBC.h"
#endif /* WIN32 ] */
#else /* HAVE_GIPS ] [ */
#include "mp/MpeSipxPcma.h"
#include "mp/MpeSipxPcmu.h"
#endif /* HAVE_GIPS ] */

#ifdef HAVE_SPEEX // [
#include "mp/MpeSipxSpeex.h"
#endif // HAVE_SPEEX ]

// All decoder child classes
#include "mp/MpdPtAVT.h"

#ifdef HAVE_GIPS /* [ */
#include "mp/MpdGIPSPCMA.h"
#include "mp/MpdGIPSPCMU.h"
#include "mp/MpdGIPSiPCMA.h"
#include "mp/MpdGIPSiPCMU.h"
#include "mp/MpdGIPSiPCMWB.h"

#ifdef PLATFORM_SUPPORTS_G729 /* [ */
#include "mp/MpdGIPSG729ab.h"
#endif /* PLATFORM_SUPPORTS_G729 ] */

#ifdef WIN32 /* [ */
#include "mp/MpdGIPSiLBC.h"
#endif /* WIN32 ] */
#else /* HAVE_GIPS ] [ */
#include "mp/MpdSipxPcma.h"
#include "mp/MpdSipxPcmu.h"
#endif /* HAVE_GIPS ] */

#ifdef HAVE_SPEEX // [
#include "mp/MpdSipxSpeex.h"
#endif // HAVE_SPEEX ]

#ifdef HAVE_GSM // [
#include "mp/MpdSipxGSM.h"
#include "mp/MpeSipxGSM.h"
#endif // HAVE_GSM ]

#ifdef HAVE_ILBC // [
#include "mp/MpdSipxILBC.h"
#include "mp/MpeSipxILBC.h"
#endif // HAVE_ILBC ]

MpCodecFactory* MpCodecFactory::spInstance = NULL;
OsBSem MpCodecFactory::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* ============================ CREATORS ================================== */

// Return a pointer to the MpCodecFactory singleton object, creating 
// it if necessary

MpCodecFactory* MpCodecFactory::getMpCodecFactory(void)
{
   // If the object already exists, then use it
   if (spInstance == NULL)
   {
      // If the object does not yet exist, then acquire
      // the lock to ensure that only one instance of the object is 
      // created
      sLock.acquire();
      if (spInstance == NULL)
         spInstance = new MpCodecFactory();
      sLock.release();
   }
   return spInstance;
}

MpCodecFactory::MpCodecFactory(void)
{
}

//:Destructor
MpCodecFactory::~MpCodecFactory()
{
}

/* ============================ MANIPULATORS ============================== */


// Returns a new instance of a decoder of the indicated type
// param: internalCodecId - (in) codec type identifier
// param: payloadType - (in) RTP payload type associated with this decoder
// param: rpDecoder - (out) Reference to a pointer to the new decoder object
OsStatus MpCodecFactory::createDecoder(SdpCodec::SdpCodecTypes internalCodecId,
                        int payloadType, MpDecoderBase*& rpDecoder)
{
   rpDecoder=NULL;

   switch (internalCodecId) {

   case (SdpCodec::SDP_CODEC_TONES):
      rpDecoder = new MpdPtAVT(payloadType);
      break;

#ifdef HAVE_GIPS /* [ */

   case (SdpCodec::SDP_CODEC_GIPS_PCMA):
      rpDecoder = new MpdGIPSPCMA(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_GIPS_PCMU):
      rpDecoder = new MpdGIPSPCMU(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_GIPS_IPCMA):
      rpDecoder = new MpdGIPSiPCMA(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_GIPS_IPCMU):
      rpDecoder = new MpdGIPSiPCMU(payloadType);
      break;

#ifdef WIN32 /* [ */
   case (SdpCodec::SDP_CODEC_ILBC):
      rpDecoder = new MpdGIPSiLBC(payloadType);
      break;
#endif /* WIN32 ] */

   case (SdpCodec::SDP_CODEC_GIPS_IPCMWB):
      rpDecoder = new MpdGIPSiPCMWB(payloadType);
      break;

#ifdef PLATFORM_SUPPORTS_G729 /* [ */
   case (SdpCodec::SDP_CODEC_G729A):
      rpDecoder = new MpdGIPSG729ab(payloadType);
      break;
   case (SdpCodec::SDP_CODEC_G729AB):
      rpDecoder = new MpdGIPSG729ab(payloadType);
      break;
   case (SdpCodec::SDP_CODEC_G729ACISCO7960):
      rpDecoder = new MpdGIPSG729ab(payloadType);
      break;
#endif /* PLATFORM_SUPPORTS_G729 ] */

#endif /* HAVE_GIPS ] */

   case (SdpCodec::SDP_CODEC_GIPS_PCMA):
      rpDecoder = new MpdSipxPcma(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_GIPS_PCMU):
      rpDecoder = new MpdSipxPcmu(payloadType);
      break;

#ifdef HAVE_SPEEX // [

   case (SdpCodec::SDP_CODEC_SPEEX):
      rpDecoder = new MpdSipxSpeex(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_SPEEX_5):
      rpDecoder = new MpdSipxSpeex(payloadType);
      break;

    case (SdpCodec::SDP_CODEC_SPEEX_15):
      rpDecoder = new MpdSipxSpeex(payloadType);
      break;

    case (SdpCodec::SDP_CODEC_SPEEX_24):
      rpDecoder = new MpdSipxSpeex(payloadType);
      break;

#endif // HAVE_SPEEX ]

#ifdef HAVE_GSM // [
   case (SdpCodec::SDP_CODEC_GSM):
      rpDecoder = new MpdSipxGSM(payloadType);
      break;
#endif // HAVE_GSM ]

#ifdef HAVE_ILBC // [
   case (SdpCodec::SDP_CODEC_ILBC):
      rpDecoder = new MpdSipxILBC(payloadType);
      break;
#endif // HAVE_ILBC ]

   default:
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "MpCodecFactory::createDecoder unknown codec type "
                    "internalCodecId = (SdpCodec::SdpCodecTypes) %d, "
                    "payloadType = %d",
                    internalCodecId, payloadType);
      assert(FALSE);
      break;
   }

   if (NULL != rpDecoder) {
/*
      osPrintf("MpCodecFactory::createDecoder(i:%d, x:%d, 0x%X)\n",
         internalCodecId, payloadType, (int) rpDecoder);
*/
      return OS_SUCCESS;
   }

   return OS_INVALID_ARGUMENT;
}

// Returns a new instance of an encoder of the indicated type
// param: internalCodecId - (in) codec type identifier
// param: payloadType - (in) RTP payload type associated with this encoder
// param: rpEncoder - (out) Reference to a pointer to the new encoder object

OsStatus MpCodecFactory::createEncoder(SdpCodec::SdpCodecTypes internalCodecId,
                          int payloadType, MpEncoderBase*& rpEncoder)
{
   rpEncoder=NULL;
   switch (internalCodecId) {

   case (SdpCodec::SDP_CODEC_TONES):
      rpEncoder = new MpePtAVT(payloadType);
      break;

#ifdef HAVE_GIPS /* [ */
   case (SdpCodec::SDP_CODEC_GIPS_PCMA):
      rpEncoder = new MpeGIPSPCMA(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_GIPS_PCMU):
      rpEncoder = new MpeGIPSPCMU(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_GIPS_IPCMA):
      rpEncoder = new MpeGIPSiPCMA(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_GIPS_IPCMU):
      rpEncoder = new MpeGIPSiPCMU(payloadType);
      break;

#ifdef WIN32 /* [ */
   case (SdpCodec::SDP_CODEC_ILBC):
      rpEncoder = new MpeGIPSiLBC(payloadType);
      break;
#endif /* WIN32 ] */

   case (SdpCodec::SDP_CODEC_GIPS_IPCMWB):
      rpEncoder = new MpeGIPSiPCMWB(payloadType);
      break;

#ifdef PLATFORM_SUPPORTS_G729 /* [ */
   case (SdpCodec::SDP_CODEC_G729A):
      rpEncoder = new MpeGIPSG729ab(payloadType);
      ((MpeGIPSG729ab*) rpEncoder)->setVad(FALSE);
      break;
   case (SdpCodec::SDP_CODEC_G729AB):
      rpEncoder = new MpeGIPSG729ab(payloadType);
      ((MpeGIPSG729ab*) rpEncoder)->setVad(TRUE);
      break;
#endif /* PLATFORM_SUPPORTS_G729 ] */
#else /* HAVE_GIPS ] [ */
   case (SdpCodec::SDP_CODEC_GIPS_PCMA):
      rpEncoder = new MpeSipxPcma(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_GIPS_PCMU):
      rpEncoder = new MpeSipxPcmu(payloadType);
      break;
#endif /* HAVE_GIPS*/

#ifdef HAVE_SPEEX // [
   case (SdpCodec::SDP_CODEC_SPEEX):
      rpEncoder = new MpeSipxSpeex(payloadType);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_5):
      rpEncoder = new MpeSipxSpeex(payloadType, 2);
      break;
    case (SdpCodec::SDP_CODEC_SPEEX_15):
      rpEncoder = new MpeSipxSpeex(payloadType, 5);
      break;
    case (SdpCodec::SDP_CODEC_SPEEX_24):
      rpEncoder = new MpeSipxSpeex(payloadType, 7);
      break;
#endif // HAVE_SPEEX ]

#ifdef HAVE_GSM // [
   case (SdpCodec::SDP_CODEC_GSM):
      rpEncoder = new MpeSipxGSM(payloadType);
      break;
#endif // HAVE_GSM ]

#ifdef HAVE_ILBC // [
   case (SdpCodec::SDP_CODEC_ILBC):
      rpEncoder = new MpeSipxILBC(payloadType);
      break;
#endif // HAVE_ILBC ]

   default:
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "MpCodecFactory::createEncoder unknown codec type "
                    "internalCodecId = (SdpCodec::SdpCodecTypes) %d, "
                    "payloadType = %d",
                    internalCodecId, payloadType);
      assert(FALSE);
      break;
   }

   if (NULL != rpEncoder) 
   {
/*
      osPrintf("MpCodecFactory::createEncoder(i:%d, x:%d, 0x%X)\n",
         internalCodecId, payloadType, (int) rpEncoder);
*/
      return OS_SUCCESS;
   }

   return OS_INVALID_ARGUMENT;
}
