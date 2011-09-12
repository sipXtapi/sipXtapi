//
// Copyright (C) 2006-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
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
#include "utl/UtlBool.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlBool::TYPE = "UtlBool" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an optional default value.
UtlBool::UtlBool(bool value)
{
    mValue = value ;
} 


// Copy constructor



// Destructor
UtlBool::~UtlBool()
{
}

UtlCopyableContainable* UtlBool::clone() const
{
    return(new UtlBool(*this));
}

/* ============================ MANIPULATORS ============================== */

void UtlBool::setValue(bool value)
{
    mValue = value ;
}

/* ============================ ACCESSORS ================================= */

bool UtlBool::getValue() const 
{
    return mValue ; 
}


unsigned UtlBool::hash() const
{
   return mValue ; 
}


UtlContainableType UtlBool::getContainableType() const
{
    return UtlBool::TYPE ;
}

/* ============================ INQUIRY =================================== */

int UtlBool::compareTo(UtlContainable const * inVal) const
{
   int result ; 
   
   if (inVal->isInstanceOf(UtlBool::TYPE))
    {
        UtlBool* temp = (UtlBool*)inVal ; 
        bool inBool = temp -> getValue() ; 
        if (inBool == mValue)
        {
            result = 0 ;
        }
        else
        {
            result = 1 ;
        }
    }
    else
    {
        result = 1 ; 
    }

    return result ;
}

UtlBoolean UtlBool::isInstanceOf(const UtlContainableType type) const
{
    // Check if it is my type and the defer parent type comparisons to parent
    return(areSameTypes(type, UtlBool::TYPE) ||
           UtlCopyableContainable::isInstanceOf(type));
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
