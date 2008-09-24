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
#include "mp/MpUdpBuf.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
MpBufPool *MpUdpBuf::smpDefaultPool = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpUdpBuf::init()
{
   mTimecode = 0;
   mIP.s_addr = INADDR_LOOPBACK;
   mPort = 0;
#ifdef MPBUF_DEBUG
    osPrintf(">>> MpUdpBuf::init()\n");
#endif
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

