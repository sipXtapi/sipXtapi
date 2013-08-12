//  
// Copyright (C) 2008-2013 SIPez LLC.  All rights reserved.
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
UtlString MpPlcBase::smDefaultAlgorithm;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpPlcBase *MpPlcBase::createPlc(const UtlString &name)
{
   const UtlString &algName = (name.length() == 0) ? smDefaultAlgorithm : name;

   if (algName == MpPlcSilence::name)
   {
      return new MpPlcSilence();
   } 
   else
   {
#ifdef EXTERNAL_PLC // [
       OsSysLog::add(FAC_MP, PRI_DEBUG, "MpPlcBase::createPlc(%s) using external PLC", name.data());

      return ::createPlc(algName.data());

#else // EXTERNAL_PLC ][
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpPlcBase::createPlc(): "
                    "Could not find PLC with name \"%s\"! Using default PLC instead.",
                    algName.data());
      // Default PLC for unknown names is silence substitution.
      return new MpPlcSilence();

#endif // EXTERNAL_PLC ]
   }
}

/* ============================ MANIPULATORS ============================== */

void MpPlcBase::setDefaultAlgorithm(const UtlString& name)
{
   smDefaultAlgorithm = name;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
