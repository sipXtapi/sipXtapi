//
// Copyright (C) 2015 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <os/OsIntTypes.h>
#include <utl/CircularBufferPtr.h>
#include <os/OsSysLog.h>
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// DEFINES
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

CircularBufferPtr::CircularBufferPtr(unsigned long capacity)
    : CircularBuffer(capacity), 
      mRefCount(0),
      mMutex(OsMutexBase::Q_FIFO)
{
}

CircularBufferPtr::~CircularBufferPtr()
{
}

/* ============================ MANIPULATORS ============================== */

void CircularBufferPtr::addRef()
{
    OsLock lock(mMutex);
    mRefCount++;
}

void CircularBufferPtr::release()
{
    bool release = false;
    {
        OsLock lock(mMutex);
        mRefCount--;

        if (mRefCount == 0)
            release = true;
    }
    if (release)
        delete this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

