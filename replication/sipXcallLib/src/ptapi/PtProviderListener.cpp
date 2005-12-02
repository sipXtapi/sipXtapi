//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

// APPLICATION INCLUDES
#include "ptapi/PtProviderListener.h"
#include "ptapi/PtEventListener.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtProviderListener::PtProviderListener(PtEventMask* pMask)
{
}

PtProviderListener::PtProviderListener(const PtProviderListener& rPtProviderListener)
{
}

PtProviderListener::~PtProviderListener()
{
}

// Assignment operator
PtProviderListener& 
PtProviderListener::operator=(const PtProviderListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

