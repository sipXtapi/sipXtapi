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
#include <assert.h>

// APPLICATION INCLUDES
#include "ps/PsTaoHookswitch.h"
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PsTaoHookswitch::PsTaoHookswitch()
{
}

PsTaoHookswitch::PsTaoHookswitch(const UtlString& rComponentName, int componentType) :
PsTaoComponent(rComponentName, componentType)
{
}

// Copy constructor
PsTaoHookswitch::PsTaoHookswitch(const PsTaoHookswitch& rPsTaoHookswitch)
{
}

// Destructor
PsTaoHookswitch::~PsTaoHookswitch()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsTaoHookswitch& 
PsTaoHookswitch::operator=(const PsTaoHookswitch& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

