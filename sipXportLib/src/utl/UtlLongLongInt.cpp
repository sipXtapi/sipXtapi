// 
// 
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004-2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// APPLICATION INCLUDES
#include "utl/UtlLongLongInt.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlLongLongInt::TYPE = "UtlLongLongInt" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an optional default value.
UtlLongLongInt::UtlLongLongInt(INT64 value)
{
    mValue = value ;
} 


// Copy constructor



// Destructor
UtlLongLongInt::~UtlLongLongInt()
{
}

/* ============================ OPERATORS ============================== */

// Prefix increment operator
UtlLongLongInt& UtlLongLongInt::operator++() {
    mValue++;
    return *this;
}

// Postfix increment operator
UtlLongLongInt UtlLongLongInt::operator++(int) {
    UtlLongLongInt temp = *this;
    ++*this;
    return temp;
}

// Prefix decrement operator
UtlLongLongInt& UtlLongLongInt::operator--() {
    mValue--;
    return *this;
}

// Postfix decrement operator
UtlLongLongInt UtlLongLongInt::operator--(int) {
    UtlLongLongInt temp = *this;
    --*this;
    return temp;
}

/* ============================ MANIPULATORS ============================== */

INT64 UtlLongLongInt::setValue(INT64 iValue)
{
    INT64 iOldValue = mValue ;
    mValue = iValue ;

    return iOldValue ;
}

INT64 UtlLongLongInt::stringToLongLong(const char* longLongString)
{
#ifdef WIN32
    return(_atoi64(longLongString));
#else
    // We could use "atoll" here but it is obsolete, "strtoll" is the recommended function
    // See http://www.delorie.com/gnu/docs/glibc/libc_423.html .
    return(strtoll(longLongString, 0, 0));
#endif
}

/* ============================ ACCESSORS ================================= */

INT64 UtlLongLongInt::getValue() const 
{
    return mValue ; 
}


unsigned UtlLongLongInt::hash() const
{
   return mValue ; 
}


UtlContainableType UtlLongLongInt::getContainableType() const
{
    return UtlLongLongInt::TYPE ;
}

/* ============================ INQUIRY =================================== */

int UtlLongLongInt::compareTo(UtlContainable const * inVal) const
{
   int result ; 
   
   if (inVal->isInstanceOf(UtlLongLongInt::TYPE))
    {
        UtlLongLongInt* temp = (UtlLongLongInt*)inVal ; 
        INT64 inIntll = temp -> getValue() ;
        if (mValue > inIntll) {
        	result = 1 ;
        }
        else if (mValue == inIntll) {
        	result = 0 ;
        }
        else {
        	// mValue < inIntll
        	result = -1 ;
        }
    }
    else
    {
    	// The result for a non-like object is undefined except that we must
    	// declare that the two objects are not equal
    	result = INT_MAX ; 
    }

    return result ;
}


UtlBoolean UtlLongLongInt::isEqual(UtlContainable const * inVal) const
{
    return (compareTo(inVal) == 0) ; 
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
