//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>
#include <string.h>

// APPLICATION INCLUDES
#include "cp/CpGatewayManager.h"
#include "os/OsStatus.h"
#include "ptapi/PtPhoneTerminal.h"
#include "ptapi/PtDefs.h"
#include "ptapi/PtProvider.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default Constructor
PtPhoneTerminal::PtPhoneTerminal()
    : PtTerminal()
{
}

PtPhoneTerminal::PtPhoneTerminal(const char* name)
  : PtTerminal(name)
{
}

// Copy constructor
PtPhoneTerminal::PtPhoneTerminal(const PtPhoneTerminal& rPtPhoneTerminal)
    : PtTerminal(rPtPhoneTerminal)
{
}

// Destructor
PtPhoneTerminal::~PtPhoneTerminal()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtPhoneTerminal&
PtPhoneTerminal::operator=(const PtPhoneTerminal& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   PtTerminal::operator=(rhs);

   return *this;
}

/* ============================ ACCESSORS ================================= */

PtStatus PtPhoneTerminal:: getComponentGroups(PtComponentGroup* pComponentGroup[],
                                                                                         int size,
                                                                                         int& nItems)
{
        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
