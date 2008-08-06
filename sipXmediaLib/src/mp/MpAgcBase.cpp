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
#include "mp/MpAgcBase.h"
#include "mp/MpAgcSimple.h"
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
#ifdef EXTERNAL_AGC // [
   extern "C" MpAgcBase *createAgc(const char *name);
#endif // EXTERNAL_AGC ]

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpAgcBase *MpAgcBase::createAgc(const UtlString &name)
{
   if (name == MpAgcSimple::name)
   {
      return new MpAgcSimple();
   } 
   else
   {
#ifdef EXTERNAL_AGC // [
      return ::createAgc(name.data());

#else // EXTERNAL_AGC ][
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpAgcBase::createAgc(): "
                    "Could not find AGC with name \"%s\"! Using default AGC instead.",
                    name.data());
      // Default AGC for unknown names is simple AGC.
      return new MpAgcSimple();

#endif // EXTERNAL_AGC ]
   }
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
