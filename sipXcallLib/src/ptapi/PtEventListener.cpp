//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "ptapi/PtEventListener.h"
#include "ptapi/PtDefs.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtEventListener::PtEventListener(PtEventMask* mask)
{
mpMask = mask;

}

// Copy constructor
PtEventListener::PtEventListener(const PtEventListener& rPtEventListener)
{
}

// Destructor
PtEventListener::~PtEventListener()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtEventListener& 
PtEventListener::operator=(const PtEventListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtEventListener, PT_NO_PARENT_CLASS)

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

