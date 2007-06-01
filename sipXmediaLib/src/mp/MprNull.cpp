//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpBuf.h>
#include <mp/MprNull.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprNull::MprNull(const UtlString& rName, 
                 int maxInOutputs,
                 int samplesPerFrame, 
                 int samplesPerSec)
:  MpAudioResource(rName, 1, maxInOutputs, 1, maxInOutputs,
                   samplesPerFrame, samplesPerSec)
{
}

// Destructor
MprNull::~MprNull()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprNull::doProcessFrame(MpBufPtr inBufs[],
                                   MpBufPtr outBufs[],
                                   int inBufsSize,
                                   int outBufsSize,
                                   UtlBoolean isEnabled,
                                   int samplesPerFrame,
                                   int samplesPerSecond)
{
   if (!isEnabled)
   {
      return TRUE;
   }

   for (int inIndex = 0; inIndex < inBufsSize; inIndex++)
   {
      inBufs[inIndex].release();
   }

   return TRUE;
}

/* ============================ FUNCTIONS ================================= */

