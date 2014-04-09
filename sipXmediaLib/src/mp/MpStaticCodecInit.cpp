//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>


// SYSTEM INCLUDES
#include <mp/codecs/PlgDefsV1.h>
#include <mp/MpStaticCodecInit.h>
#include <utl/UtlDefs.h>

static UtlBoolean gStaticCodecInitialized = FALSE;

extern "C" {
static void initStaticCodec(void)
{
#ifdef CODEC_PCMA_PCMU_STATIC
   REGISTER_STATIC_PLG(sipXpcmapcmu);
#endif
#ifdef CODEC_TONES_STATIC
   REGISTER_STATIC_PLG(tones);
#endif
#ifdef CODEC_G722_STATIC
   REGISTER_STATIC_PLG(g722);
#endif
#ifdef CODEC_G726_STATIC
   REGISTER_STATIC_PLG(g726);
#endif
#ifdef CODEC_GSM_STATIC
   REGISTER_STATIC_PLG(libgsm);
#endif
#ifdef CODEC_SPEEX_STATIC
   REGISTER_STATIC_PLG(speex);
#endif
#ifdef CODEC_OPUS_STATIC
   REGISTER_STATIC_PLG(opus);
#endif
}
}

void mpStaticCodecInitializer(void)
{
   if (!gStaticCodecInitialized)
   {
      initStaticCodec();
      gStaticCodecInitialized = TRUE;
   }
}

void mpStaticCodecUninitializer(void)
{
   gStaticCodecInitialized = FALSE;
}
