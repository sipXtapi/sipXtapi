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
#include "ps/PsTaoSpeaker.h"
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PsTaoSpeaker::PsTaoSpeaker()
{
        mVolume = 0;
}

PsTaoSpeaker::PsTaoSpeaker(const UtlString& rComponentName, int componentType) :
PsTaoComponent(rComponentName, componentType)
{
        mVolume = 0;
}

// Copy constructor
PsTaoSpeaker::PsTaoSpeaker(const PsTaoSpeaker& rPsTaoSpeaker)
{
        mVolume = rPsTaoSpeaker.mVolume;
}

// Destructor
PsTaoSpeaker::~PsTaoSpeaker()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsTaoSpeaker&
PsTaoSpeaker::operator=(const PsTaoSpeaker& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsStatus PsTaoSpeaker::setVolume(int volume)
{
        if (0 <= volume && volume <= 100)
                mVolume = volume;

        osPrintf("===== PsTaoSpeaker::setVolume: mVolume = %d volume = %d\n", mVolume, volume);
        return OS_SUCCESS;
}


/* ============================ ACCESSORS ================================= */
OsStatus PsTaoSpeaker::getVolume(int& rVolume)
{
        rVolume = mVolume;
        osPrintf("===== PsTaoSpeaker::getVolume: mVolume = %d volume = %d\n", mVolume, rVolume);
        return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
