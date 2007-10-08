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

// DEFINES
#if defined( WIN32 ) && !defined( WINCE )
#define strtoll _strtoi64
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlLongLongInt::TYPE = "UtlLongLongInt" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an optional default value.
UtlLongLongInt::UtlLongLongInt(int64_t value)
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
UtlLongLongInt& UtlLongLongInt::operator++()
{
    mValue++;
    return *this;
}

// Postfix increment operator
UtlLongLongInt UtlLongLongInt::operator++(int)
{
    UtlLongLongInt temp = *this;
    ++*this;
    return temp;
}

// Prefix decrement operator
UtlLongLongInt& UtlLongLongInt::operator--()
{
    mValue--;
    return *this;
}

// Postfix decrement operator
UtlLongLongInt UtlLongLongInt::operator--(int)
{
    UtlLongLongInt temp = *this;
    --*this;
    return temp;
}

/* ============================ MANIPULATORS ============================== */

int64_t UtlLongLongInt::setValue(int64_t iValue)
{
    int64_t iOldValue = mValue ;
    mValue = iValue ;

    return iOldValue ;
}

int64_t UtlLongLongInt::stringToLongLong(const char* longLongString)
{
#if defined(_VXWORKS)

    int numDigits = strlen(longLongString);
    int64_t sum = -1;

    if(numDigits <= 9)
    {
        sum = strtol(longLongString, 0, 0);
    }
    else if(numDigits > 9)
    {
        int64_t billions = 0;
        int64_t first9digits = strtol(&longLongString[numDigits - 9], 0, 0);
        char digitBuffer[10];

        if(numDigits <= 18)
        {
            // Billions digits
            memcpy(digitBuffer, longLongString, numDigits - 9);
            digitBuffer[numDigits - 9] = '\0';
            billions = strtol(digitBuffer, 0, 0);
            if(billions >= 0)
            {
                sum = billions * 1000000 + first9digits;
            }
            else
            {
                sum = billions * 1000000 + first9digits * -1;
            }
        }
        else //(numDigits > 18)
        {
            int64_t gazillions = 0;
            // Billions digits
            memcpy(digitBuffer, &longLongString[numDigits - 18], 9);
            digitBuffer[9] = '\0';
            billions = strtol(digitBuffer, 0, 0);

            // Gazillions digits (take a maximum of 5 digits and sign)
            int remainingDigits = numDigits > 23 ? 5 : numDigits - 18;
            memcpy(digitBuffer, longLongString, remainingDigits);
            digitBuffer[remainingDigits] = '\0';
            gazillions = strtol(digitBuffer, 0, 0);

            if(gazillions > 0)
            {
                sum = gazillions * 1000000000000 + billions * 1000000 + first9digits;
            }
            else
            {
                sum = gazillions * 1000000000000 + billions * -1000000 + first9digits * -1;
            }
        }
    }
    return sum;
#else
    // We could use "atoll" here but it is obsolete, "strtoll" is the recommended function
    // See http://www.delorie.com/gnu/docs/glibc/libc_423.html .
    return strtoll(longLongString, 0, 0);
#endif
}

/* ============================ ACCESSORS ================================= */

int64_t UtlLongLongInt::getValue() const
{
    return mValue ;
}


unsigned UtlLongLongInt::hash() const
{
   return (unsigned)mValue ;
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
        int64_t inIntll = temp -> getValue() ;
        if (mValue > inIntll)
        {
            result = 1 ;
        }
        else if (mValue == inIntll)
        {
            result = 0 ;
        }
        else
        {
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
    return (compareTo(inVal) == 0);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
