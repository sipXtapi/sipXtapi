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
    if ((size < 0) || (size > getMaxDataSize())) {
       if (size < 0) OsSysLog::add(FAC_MP, PRI_ERR, "MpArrayBuf::setDataSize(%d) --  invalid:  size < 0", size);
       if (size > getMaxDataSize()) OsSysLog::add(FAC_MP, PRI_ERR, "MpArrayBuf::setDataSize(%d) --  size too large, max=%ld", size, getMaxDataSize());
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
