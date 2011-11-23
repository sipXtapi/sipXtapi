//
// Copyright (C) 2007-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Daniel Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mi/MiStringNotf.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message notification class used to communicate DTMF signaling.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MiStringNotf::MiStringNotf(NotfType msgType,
                           const UtlString& sourceId,
                           const UtlString& value,
                           int connId,
                           int streamId)
: MiNotification(msgType, sourceId, connId, streamId)
, mValue(value)
{
}

MiStringNotf::MiStringNotf(const MiStringNotf& rMsg)
: MiNotification(rMsg)
, mValue(rMsg.mValue)
{
}

OsMsg* MiStringNotf::createCopy(void) const
{
    return(new MiStringNotf(*this));
}

MiStringNotf::~MiStringNotf()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

MiStringNotf& 
MiStringNotf::operator=(const MiStringNotf& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MiNotification::operator=(rhs);       // assign fields for parent class

   mValue = rhs.mValue;

   return *this;
}

void MiStringNotf::setValue(const UtlString& value)
{
   mValue = value;
}

/* ============================ ACCESSORS ================================= */

void MiStringNotf::getValue(UtlString& value) const
{
    value = mValue;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
