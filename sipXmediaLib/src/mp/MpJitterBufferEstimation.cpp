//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander.Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpJitterBufferEstimation.h"
#include "mp/MpJbeFixed.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
#ifdef EXTERNAL_JB_ESTIMATION // [
   extern "C" MpJitterBufferEstimation *createJbe(const char *name);
#endif // !EXTERNAL_JB_ESTIMATION ]

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// LOCAL DATA TYPES

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpJitterBufferEstimation *MpJitterBufferEstimation::createJbe(const UtlString &algName)
{
   if (algName == MpJbeFixed::name)
   {
      return new MpJbeFixed();
   } 
   else
   {
#ifdef EXTERNAL_JB_ESTIMATION // [
      return ::createJbe(algName.data());

#else // EXTERNAL_VAD ][
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpJitterBufferEstimation::createJbe(): "
                    "Could not find JBE algorithm with name \"%s\"!"
                    " Using default JBE algorithm instead.",
                    algName.data());
      // Default VAD for unknown names is simple VAD.
      return new MpJbeFixed();

#endif // EXTERNAL_VAD ]
   }

}

/* ============================= MANIPULATORS ============================= */

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */

