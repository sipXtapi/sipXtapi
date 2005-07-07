// $Id$
//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <string.h>

// APPLICATION INCLUDES
#include "utl/UtlContainable.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType UtlContainable::TYPE = "UtlContainable" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor


// Copy constructor


// Destructor
UtlContainable::~UtlContainable()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// RTTI
UtlBoolean UtlContainable::isInstanceOf(const UtlContainableType type) const
{
    return (   (type != NULL)
            && (getContainableType() != NULL)
            && (type == getContainableType())
            );
}

// Test this object to another like-object for equality. 
UtlBoolean UtlContainable::isEqual(UtlContainable const * compareContainable) const 
{
   return ( compareTo(compareContainable) == 0 );
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


