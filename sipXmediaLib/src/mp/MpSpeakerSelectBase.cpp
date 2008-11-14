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
// APPLICATION INCLUDES
#include "mp/MpSpeakerSelectBase.h"
//#include "mp/MpSpeakerSelectSimple.h"
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
#ifdef EXTERNAL_SS // [
   extern "C" MpSpeakerSelectBase *createSS(const char *name);
#endif // EXTERNAL_SS ]

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpSpeakerSelectBase *MpSpeakerSelectBase::createInstance(const UtlString &name)
{
   //if (name == MpSpeakerSelectSimple::name)
   if (true)
   {
      //return new MpSpeakerSelectSimple();
      return NULL;
   } 
   else
   {
#ifdef EXTERNAL_SS // [
      return ::createSS(name.data());

#else // EXTERNAL_SS ][
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpSpeakerSelectBase::createInstance(): "
                    "Could not find SS with name \"%s\"! Using default SS instead.",
                    name.data());
      // Default SS for unknown names is simple SS.
      //return new MpSpeakerSelectSimple();
      return NULL;
#endif // EXTERNAL_SS ]
   }
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
