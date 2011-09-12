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
#include <string.h>
#include <limits.h>

// APPLICATION INCLUDES
#include "utl/UtlIntPtr.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlIntPtr::TYPE = "UtlIntPtr";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an optional default value.
UtlIntPtr::UtlIntPtr(intptr_t value) : mValue(value)
{
} 


// Copy constructor
UtlIntPtr::UtlIntPtr(const UtlIntPtr& rhs) : mValue(rhs.mValue)
{
}

// Destructor
UtlIntPtr::~UtlIntPtr()
{
}

UtlCopyableContainable* UtlIntPtr::clone() const
{
   return new UtlIntPtr(*this); 
}

/* ============================ OPERATORS ============================== */

// Prefix increment operator
UtlIntPtr& UtlIntPtr::operator++() {
    mValue++;
    return *this;
}

// Postfix increment operator
UtlIntPtr UtlIntPtr::operator++(int) {
    UtlIntPtr temp = *this;
    ++*this;
    return temp;
}

// Prefix decrement operator
UtlIntPtr& UtlIntPtr::operator--() {
    mValue--;
    return *this;
}

// Postfix decrement operator
UtlIntPtr UtlIntPtr::operator--(int) {
    UtlIntPtr temp = *this;
    --*this;
    return temp;
}

UtlIntPtr& UtlIntPtr::operator=(const UtlIntPtr& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Assign values
   mValue = rhs.mValue;

   return *this;
}

/* ============================ MANIPULATORS ============================== */

intptr_t UtlIntPtr::setValue(intptr_t iValue)
{
    intptr_t iOldValue = mValue ;
    mValue = iValue ;

    return iOldValue ;
}

/* ============================ ACCESSORS ================================= */

intptr_t UtlIntPtr::getValue() const 
{
    return mValue ; 
}


unsigned UtlIntPtr::hash() const
{
   return (unsigned)mValue ; 
}


UtlContainableType UtlIntPtr::getContainableType() const
{
    return UtlIntPtr::TYPE ;
}

/* ============================ INQUIRY =================================== */

int UtlIntPtr::compareTo(UtlContainable const * inVal) const
{
   int result ; 
   
   if (inVal->isInstanceOf(UtlIntPtr::TYPE))
    {
        UtlIntPtr* temp = (UtlIntPtr*)inVal ; 
        intptr_t inInt = temp->getValue() ; 
        result = (int)(mValue - inInt); 
    }
    else
    {
        result = INT_MAX ; 
    }

    return result ;
}


UtlBoolean UtlIntPtr::isEqual(UtlContainable const * inVal) const
{
    return (compareTo(inVal) == 0) ; 
}

UtlBoolean UtlIntPtr::isInstanceOf(const UtlContainableType type) const
{
    // Check if it is my type and the defer parent type comparisons to parent
    return(areSameTypes(type, UtlIntPtr::TYPE) ||
           UtlCopyableContainable::isInstanceOf(type));
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
