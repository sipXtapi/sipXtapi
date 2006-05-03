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
#include "ps/PsTaoLamp.h"
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PsTaoLamp::PsTaoLamp() 
{
}

PsTaoLamp::PsTaoLamp(const UtlString& rComponentName, int componentType) :
PsTaoComponent(rComponentName, componentType)
{
}

// Copy constructor
PsTaoLamp::PsTaoLamp(const PsTaoLamp& rPsTaoLamp)
{
}

// Destructor
PsTaoLamp::~PsTaoLamp()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsTaoLamp& 
PsTaoLamp::operator=(const PsTaoLamp& rhs)
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

