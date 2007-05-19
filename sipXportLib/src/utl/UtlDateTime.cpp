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


// APPLICATION INCLUDES
#include "utl/UtlDateTime.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlDateTime::TYPE = "UtlDateTime" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an optional default value.
UtlDateTime::UtlDateTime(OsDateTime value)
{
    mTime = value ;
} 


// Copy constructor



// Destructor
UtlDateTime::~UtlDateTime()
{
}

/* ============================ MANIPULATORS ============================== */

void UtlDateTime::setTime(const OsDateTime& value)
{
    mTime = value ;
}

/* ============================ ACCESSORS ================================= */

void UtlDateTime::getTime(OsDateTime& value) const 
{
    value = mTime ; 
}


unsigned UtlDateTime::hash() const
{
   return mTime.getSecsSinceEpoch() ; 
}


UtlContainableType UtlDateTime::getContainableType() const
{
    return UtlDateTime::TYPE ;
}

/* ============================ INQUIRY =================================== */

int UtlDateTime::compareTo(UtlContainable const * inVal) const
{
   int result ; 
   
   if (inVal->isInstanceOf(UtlDateTime::TYPE))
    {
        UtlDateTime* temp = (UtlDateTime*)inVal ;
        OsDateTime inTime;
        temp->getTime(inTime);
        int tempValue = inTime.getSecsSinceEpoch() ;
        result = tempValue - mTime.getSecsSinceEpoch() ;
    }
    else
    {
        result = 1 ; 
    }

    return result ;
}



/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
