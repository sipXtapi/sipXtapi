//  
// Copyright (C) 2006-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsIntTypes.h>
#include "mp/MpRtpBuf.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
MpBufPool *MpRtpBuf::smpDefaultPool = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpRtpBuf::init()
{
#ifdef MPBUF_DEBUG
    osPrintf(">>> MpRtpBuf::init()\n");
#endif
   mInternalCodecId = SdpCodec::SDP_CODEC_UNKNOWN;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
