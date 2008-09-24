//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpTypes.h"

/* ============================ FUNCTIONS ================================= */

MpSpeechType mixSpeechTypes(MpSpeechType src1,
                            MpSpeechType src2)
{
   // Here we ignore MP_SPEECH_MUTED type, because it should not be mixed.
   if (src1 == MP_SPEECH_MUTED ||
       src2 == MP_SPEECH_MUTED)
   {
      assert(!"Muted audio must not be mixed!");
      return MP_SPEECH_UNKNOWN;
   }

   // If one of sources is tone, result is tone (though I'm not sure this
   // is fully correct)
   if (src1 == MP_SPEECH_TONE ||
       src2 == MP_SPEECH_TONE)
   {
      return MP_SPEECH_TONE;
   }

   // If active speech is mixed with active, silent or comfort noise
   // frame, result is active speech.
   if (src1 == MP_SPEECH_ACTIVE ||
       src2 == MP_SPEECH_ACTIVE)
   {
      return MP_SPEECH_ACTIVE;
   }

   // If one of speech types is unknown, result is unknown.
   if (src1 == MP_SPEECH_UNKNOWN ||
       src2 == MP_SPEECH_UNKNOWN)
   {
      return MP_SPEECH_UNKNOWN;
   }

   // If comfort noise is mixed with silent or comfort noise,
   // result is comfort noise.
   if (src1 == MP_SPEECH_COMFORT_NOISE ||
       src2 == MP_SPEECH_COMFORT_NOISE)
   {
      return MP_SPEECH_COMFORT_NOISE;
   }

   // At this point only remaining case is that both frames are silent.
   if (src1 != MP_SPEECH_SILENT ||
       src2 != MP_SPEECH_SILENT)
   {
      assert(!"Unknown frame speech type!");
      return MP_SPEECH_UNKNOWN;
   }

   return MP_SPEECH_SILENT;
}
