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
#include "mp/MpPlcBase.h"
#include "mp/MpPlcSilence.h"
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
#ifdef EXTERNAL_PLC // [
   extern "C" MpPlcBase *createPlc(const char *name);
#endif // EXTERNAL_PLC ]

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpPlcBase *MpPlcBase::createPlc(const UtlString &plcName)
{
   if (plcName == MpPlcSilence::name)
   {
      return new MpPlcSilence();
   } 
   else
   {
#ifdef EXTERNAL_PLC // [
      return ::createPlc(plcName.data());

#else // EXTERNAL_PLC ][
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpPlcBase::createPlc(): "
                    "Could not find PLC with name \"%s\"! Using default PLC instead.",
                    plcName.data());
      // Default PLC for unknown names is silence substitution.
      return new MpPlcSilence();

#endif // EXTERNAL_PLC ]
   }
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
