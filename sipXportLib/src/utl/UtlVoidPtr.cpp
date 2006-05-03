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
#include <string.h>

// APPLICATION INCLUDES
#include "utl/UtlInt.h"
#include "utl/UtlVoidPtr.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType UtlVoidPtr::TYPE = "UtlVoidPtr" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlVoidPtr::UtlVoidPtr(void* pValue)
{
    mpValue = pValue ;
}


// Copy constructor


// Destructor
UtlVoidPtr::~UtlVoidPtr()
{
}

/* ============================ MANIPULATORS ============================== */

void* UtlVoidPtr::setValue(void* pValue)
{
    void* pOldValue = mpValue ;
    mpValue = pValue ;
    return pOldValue ;
}

/* ============================ ACCESSORS ================================= */

void* UtlVoidPtr::getValue() const 
{
    return mpValue ; 
}


unsigned UtlVoidPtr::hash() const
{
    return (unsigned) mpValue ; 
}


UtlContainableType UtlVoidPtr::getContainableType() const
{
    return UtlVoidPtr::TYPE ;
}

/* ============================ INQUIRY =================================== */

// Compare the this object to another like-object. 
int UtlVoidPtr::compareTo(UtlContainable const * inVal) const
{
   int result ; 
   
   if (inVal->isInstanceOf(UtlVoidPtr::TYPE))
   {
      result = ((unsigned) mpValue) - ((unsigned) ((UtlVoidPtr*) inVal)->mpValue);
   }
   else
   {
      result = -1; 
   }

   return result;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
