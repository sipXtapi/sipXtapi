//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
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
#include "mp/MpeSipxPcma.h"
#include "mp/MpeSipxPcmu.h"

#ifdef HAVE_SPEEX // [
#include "mp/MpeSipxSpeex.h"
#endif // HAVE_SPEEX ]

// All decoder child classes
#include "mp/MpdPtAVT.h"
#include "mp/MpdSipxPcma.h"
#include "mp/MpdSipxPcmu.h"

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
// param: connId - (in) The ID of the connection this decoder is associated with.
// param: pFlowGraph - (in) Pointer to the flowgraph that the decoder is part of. (used for notifications)
OsStatus MpCodecFactory::createDecoder(SdpCodec::SdpCodecTypes internalCodecId,
                                       int payloadType, MpDecoderBase*& rpDecoder,
                                       MpConnectionID connId, MpFlowGraphBase* pFlowGraph)
{
   rpDecoder=NULL;

   switch (internalCodecId) 
   {
   case (SdpCodec::SDP_CODEC_TONES):
      rpDecoder = new MpdPtAVT(payloadType, connId, pFlowGraph);
      break;

   case (SdpCodec::SDP_CODEC_PCMA):
      rpDecoder = new MpdSipxPcma(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_PCMU):
      rpDecoder = new MpdSipxPcmu(payloadType);
      break;

#ifdef HAVE_SPEEX // [

   case (SdpCodec::SDP_CODEC_SPEEX):
   case (SdpCodec::SDP_CODEC_SPEEX_5):
   case (SdpCodec::SDP_CODEC_SPEEX_15):
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

   case (SdpCodec::SDP_CODEC_PCMA):
      rpEncoder = new MpeSipxPcma(payloadType);
      break;

   case (SdpCodec::SDP_CODEC_PCMU):
      rpEncoder = new MpeSipxPcmu(payloadType);
      break;

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
