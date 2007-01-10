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
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

int compare(RtpTimestamp timestamp1, RtpTimestamp timestamp2)
{
   if (timestamp1 == timestamp2)
      return 0;
   else if (timestamp1 >= RTP_HALF_TIMESTAMP)
      return ((timestamp1-RTP_HALF_TIMESTAMP < timestamp2) && (timestamp2 < timestamp1)) ? 1 : -1 ;
   else
      return ((timestamp1+RTP_HALF_TIMESTAMP < timestamp2) || (timestamp2 < timestamp1)) ? 1 : -1 ;
}

int compare(RtpSeq seq1, RtpSeq seq2)
{
   if (seq1 == seq2)
      return 0;
   else if (seq1 >= RTP_HALF_SEQ)
      return ((seq1-RTP_HALF_SEQ < seq2) && (seq2 < seq1)) ? 1 : -1 ;
   else
      return ((seq1+RTP_HALF_SEQ < seq2) || (seq2 < seq1)) ? 1 : -1 ;
}
