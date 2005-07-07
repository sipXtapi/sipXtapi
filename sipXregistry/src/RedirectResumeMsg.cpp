// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "RedirectResumeMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

RedirectResumeMsg::RedirectResumeMsg(RequestSeqNo seqNo,
                                     int redirectorNo) :
   OsMsg(REDIRECT_RESTART, 0),
   mSeqNo(seqNo),
   mRedirectorNo(redirectorNo)
{
}

// Create a copy of this msg object
RedirectResumeMsg* RedirectResumeMsg::createCopy(void) const
{
   return new RedirectResumeMsg(mSeqNo, mRedirectorNo);
}
