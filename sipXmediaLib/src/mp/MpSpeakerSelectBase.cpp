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
UtlString MpSpeakerSelectBase::smDefaultAlgorithm;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpSpeakerSelectBase *MpSpeakerSelectBase::createInstance(const UtlString &name)
{
   const UtlString &algName = (name.length() == 0) ? smDefaultAlgorithm : name;

#ifdef EXTERNAL_SS // [
   return ::createSS(algName.data());

#else // EXTERNAL_SS ][
   OsSysLog::add(FAC_MP, PRI_WARNING,
                 "MpSpeakerSelectBase::createInstance(): "
                 "Could not find SS with name \"%s\"!",
                 algName.data());
   return NULL;
#endif // EXTERNAL_SS ]
}

/* ============================ MANIPULATORS ============================== */

void MpSpeakerSelectBase::setDefaultAlgorithm(const UtlString& name)
{
   smDefaultAlgorithm = name;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
