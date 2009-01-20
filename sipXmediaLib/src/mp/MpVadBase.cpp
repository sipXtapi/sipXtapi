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
UtlString MpVadBase::smDefaultAlgorithm;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpVadBase *MpVadBase::createVad(const UtlString &name)
{
   const UtlString &algName = (name.length() == 0) ? smDefaultAlgorithm : name;

   if (algName == MpVadSimple::name)
   {
      return new MpVadSimple();
   } 
   else
   {
#ifdef EXTERNAL_VAD // [
      return ::createVad(algName.data());

#else // EXTERNAL_VAD ][
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpVadBase::createVad(): "
                    "Could not find VAD with name \"%s\"! Using default VAD instead.",
                    algName.data());
      // Default VAD for unknown names is simple VAD.
      return new MpVadSimple();

#endif // EXTERNAL_VAD ]
   }
}

/* ============================ MANIPULATORS ============================== */

void MpVadBase::setDefaultAlgorithm(const UtlString& name)
{
   smDefaultAlgorithm = name;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
