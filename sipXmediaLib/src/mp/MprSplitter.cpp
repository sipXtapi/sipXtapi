//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpBuf.h"
#include "mp/MprSplitter.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprSplitter::MprSplitter(const UtlString& rName, int numOutputs,
                         int samplesPerFrame, int samplesPerSec)
:  MpAudioResource(rName, 1, 1, 1, numOutputs, samplesPerFrame, samplesPerSec)
{
}

// Destructor
MprSplitter::~MprSplitter()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprSplitter::doProcessFrame(MpBufPtr inBufs[],
                                       MpBufPtr outBufs[],
                                       int inBufsSize,
                                       int outBufsSize,
                                       UtlBoolean isEnabled,
                                       int samplesPerFrame,
                                       int samplesPerSecond)
{
    MpBufPtr in;
    int      i;

    if (outBufsSize == 0)
        return FALSE;

    if (inBufsSize > 0) {
        in = inBufs[0];
    }

    if (isEnabled) {
        for (i=0; i<outBufsSize; i++) {
            if (isOutputConnected(i)) {
                outBufs[i] = in;
            }
        }
    } else {
#ifndef OLD_SPLITTER // [
        outBufs[0] = in;
#else // OLD_SPLITTER ][
        for (i=0; ((i<outBufsSize) && (in.isValid())); i++) {
            if (isOutputConnected(i)) {
                outBufs[i] = in;
            }
        }
#endif // OLD_SPLITTER ]
    }
    return TRUE;
}

/* ============================ FUNCTIONS ================================= */

