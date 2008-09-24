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
#include "mp/MpVadBase.h"
#include "mp/MpVadSimple.h"
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
#ifdef EXTERNAL_VAD // [
   extern "C" MpVadBase *createVad(const char *name);
#endif // EXTERNAL_VAD ]

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpVadBase *MpVadBase::createVad(const UtlString &vadName)
{
   if (vadName == MpVadSimple::name)
   {
      return new MpVadSimple();
   } 
   else
   {
#ifdef EXTERNAL_VAD // [
      return ::createVad(vadName.data());

#else // EXTERNAL_VAD ][
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpVadBase::createVad(): "
                    "Could not find VAD with name \"%s\"! Using default VAD instead.",
                    vadName.data());
      // Default VAD for unknown names is simple VAD.
      return new MpVadSimple();

#endif // EXTERNAL_VAD ]
   }
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
