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
#include "ps/PsTaoMicrophone.h"
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PsTaoMicrophone::PsTaoMicrophone()
{
   mGain = 0;
}

PsTaoMicrophone::PsTaoMicrophone(const UtlString& rComponentName, int componentType) :
PsTaoComponent(rComponentName, componentType)
{
   mGain = 0;
}

// Copy constructor
PsTaoMicrophone::PsTaoMicrophone(const PsTaoMicrophone& rPsTaoMicrophone)
{
        mGain = rPsTaoMicrophone.mGain;
}

// Destructor
PsTaoMicrophone::~PsTaoMicrophone()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsTaoMicrophone&
PsTaoMicrophone::operator=(const PsTaoMicrophone& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsStatus PsTaoMicrophone::setGain(int gain)
{
        if (0 <= gain && gain <= 100)
                mGain = gain;
        return OS_SUCCESS;
}


/* ============================ ACCESSORS ================================= */
OsStatus PsTaoMicrophone::getGain(int& rGain)
{
        rGain = mGain;
        return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
