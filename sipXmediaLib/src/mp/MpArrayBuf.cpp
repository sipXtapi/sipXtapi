//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpArrayBuf.h"
#include "mp/MpBufPool.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

bool MpArrayBuf::setDataSize(unsigned size)
{
    if (size > getMaxDataSize()) {
        mDataSize = getMaxDataSize();
        return false;
    } else {
        mDataSize = size;
        return true;
    }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpArrayBuf::init()
{
    mDataSize = getMaxDataSize();
#ifdef MPBUF_DEBUG
    osPrintf(">>> MpArrayBuf::init()\n");
#endif
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
