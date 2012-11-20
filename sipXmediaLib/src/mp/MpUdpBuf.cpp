//  
// Copyright (C) 2006 SIPez LLC.  All rights reserved.
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsIntTypes.h>
#include <mp/MpUdpBuf.h>

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

