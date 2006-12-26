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
#include "mp/MpBuf.h"
#include "mp/MpBufPool.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

void MpBuf::attach()
{
    mRefCounter++;
#ifdef MPBUF_DEBUG
    osPrintf( "Buffer %d from pool %x have %d references now (++)\n"
            , mpPool->getBufferNumber(this)
            , mpPool
            , mRefCounter);
#endif
}

void MpBuf::detach()
{
#ifdef MPBUF_DEBUG
    osPrintf( "Buffer %d from pool %x have %d references now (--)\n"
            , mpPool->getBufferNumber(this)
            , mpPool
            , mRefCounter-1);
#endif

    mRefCounter--;
    // If other thread decremented mRefCounter in this execution point then
    // buffer will be freed twice. We try to fix this in the releaseBuffer()
    // code checking is buffer already free or not.
    if (mRefCounter == 0) {
        if (mpDestroy != NULL) {
            mpDestroy(this);
        }
        mpPool->releaseBuffer(this);
    }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpBuf::sInitClone(MpBuf *pBuffer)
{
   pBuffer->mRefCounter = 1;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
