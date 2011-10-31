//
// Copyright (C) 2006-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "utl/UtlCopyableContainable.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlCopyableContainable::TYPE = "UtlCopyableContainable";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor 
UtlCopyableContainable::UtlCopyableContainable()
{
} 

// Destructor
UtlCopyableContainable::~UtlCopyableContainable()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

UtlContainableType UtlCopyableContainable::getContainableType() const
{
    return(UtlCopyableContainable::TYPE);
}

/* ============================ INQUIRY =================================== */

UtlBoolean UtlCopyableContainable::isInstanceOf(const UtlContainableType type) const
{
    // Generally classes should check their own type and then deligate to
    // their parent class implementation of isInstanceOf.  However this is
    // a slight optimization as UtlContainable will also compare with the base
    // class type again.
    return(areSameTypes(type, UtlCopyableContainable::TYPE) ||
           areSameTypes(type, UtlContainable::TYPE));
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

