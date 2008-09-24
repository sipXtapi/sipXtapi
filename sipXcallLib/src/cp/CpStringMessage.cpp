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

#ifdef TEST
#include <assert.h>
#include "utl/UtlMemCheck.h"
#endif //TEST

// APPLICATION INCLUDES
#include <cp/CpStringMessage.h>
#include <cp/CallManager.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpStringMessage::CpStringMessage(unsigned char messageSubtype, const char* str) :
OsMsg(OsMsg::PHONE_APP, messageSubtype)
{
        stringData.append(str);
}

// Copy constructor
CpStringMessage::CpStringMessage(const CpStringMessage& rCpStringMessage):
OsMsg(OsMsg::PHONE_APP, rCpStringMessage.getMsgType())
{
        stringData = rCpStringMessage.stringData;
}

// Destructor
CpStringMessage::~CpStringMessage()
{

}

OsMsg* CpStringMessage::createCopy(void) const
{
        return(new CpStringMessage(getMsgSubType(), stringData.data()));
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
CpStringMessage&
CpStringMessage::operator=(const CpStringMessage& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);
        stringData = rhs.stringData;

   return *this;
}

/* ============================ ACCESSORS ================================= */

void CpStringMessage::getStringData(UtlString& str) const
{
        str = stringData;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
