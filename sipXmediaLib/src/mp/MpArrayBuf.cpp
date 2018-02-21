//  
// Copyright (C) 2006-2018 SIPez LLC.  All rights reserved.
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpArrayBuf.h"
#include "mp/MpBufPool.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

bool MpArrayBuf::setDataSize(int size)
{
    if ((size < 0) || (size > (int) getMaxDataSize())) {
        if (size < 0) OsSysLog::add(FAC_MP, PRI_ERR, "MpArrayBuf::setDataSize(%d) --  invalid:  size < 0", size);
        if (size > (int) getMaxDataSize())
        {
            OsSysLog::add(FAC_MP, PRI_ERR, 
                          "MpArrayBuf::setDataSize(%d) --  size too large, max=%d for %s pool", 
                          size, 
                          getMaxDataSize(),
                          getBufferPool()->getName().data());
        }
        mDataSize = 0;
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
