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
#include "ps/PsTaoComponent.h"
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PsTaoComponent::PsTaoComponent() :
mMutex(OsMutex::Q_FIFO)
{
}

PsTaoComponent::PsTaoComponent(const UtlString& rComponentName, int componentType) :
mMutex(OsMutex::Q_FIFO),
mName(rComponentName),
mType(componentType)
{
}

// Copy constructor
PsTaoComponent::PsTaoComponent(const PsTaoComponent& rPsTaoComponent) :
mMutex(OsMutex::Q_FIFO)
{
}

// Destructor
PsTaoComponent::~PsTaoComponent()
{
        mName.remove(0);
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsTaoComponent&
PsTaoComponent::operator=(const PsTaoComponent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
void PsTaoComponent::getName(UtlString& rName)
{
        rName = mName;
}

int PsTaoComponent::getType(void)
{
        return mType;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
OsMutex* PsTaoComponent::getMutex(void)
{
        return &mMutex;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
