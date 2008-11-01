//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mi/MiIntNotf.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message notification class used to communicate DTMF signaling.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MiIntNotf::MiIntNotf(NotfType msgType,
                     const UtlString& sourceId,
                     int value,
                     int connId,
                     int streamId)
: MiNotification(msgType, sourceId, connId, streamId)
, mValue(value)
{
}

MiIntNotf::MiIntNotf(const MiIntNotf& rMsg)
: MiNotification(rMsg)
, mValue(rMsg.mValue)
{
}

OsMsg* MiIntNotf::createCopy(void) const
{
   return new MiIntNotf(*this);
}

MiIntNotf::~MiIntNotf()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

MiIntNotf& 
MiIntNotf::operator=(const MiIntNotf& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MiNotification::operator=(rhs);       // assign fields for parent class

   mValue = rhs.mValue;

   return *this;
}

void MiIntNotf::setValue(int value)
{
   mValue = value;
}

/* ============================ ACCESSORS ================================= */

int MiIntNotf::getValue() const
{
   return mValue;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
