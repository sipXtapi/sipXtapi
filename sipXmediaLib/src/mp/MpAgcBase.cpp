//  
// Copyright (C) 2008-2013 SIPez LLC. All rights reserved.
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
UtlString MpAgcBase::smDefaultAlgorithm;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpAgcBase *MpAgcBase::createAgc(const UtlString &name)
{
   const UtlString &algName = (name.length() == 0) ? smDefaultAlgorithm : name;

   if (algName == MpAgcSimple::name)
   {
      return new MpAgcSimple();
   } 
   else
   {
#ifdef EXTERNAL_AGC // [
       OsSysLog::add(FAC_MP, PRI_DEBUG, "MpAgcBase::createAgc(%s) using external AGC", name.data());

      return ::createAgc(algName.data());

#else // EXTERNAL_AGC ][
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpAgcBase::createAgc(): "
                    "Could not find AGC with name \"%s\"! Using default AGC instead.",
                    algName.data());
      // Default AGC for unknown names is simple AGC.
      return new MpAgcSimple();

#endif // EXTERNAL_AGC ]
   }
}

/* ============================ MANIPULATORS ============================== */

void MpAgcBase::setDefaultAlgorithm(const UtlString& name)
{
   smDefaultAlgorithm = name;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
