//
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
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
#include <limits.h>

// APPLICATION INCLUDES
#include "utl/UtlRandom.h"
#include "os/OsTime.h"
#include "os/OsDateTime.h"
#include "os/OsTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlRandom::UtlRandom()
{
    static int siCounter = 0 ;

    OsTaskId_t iTaskId = 0 ;
    OsTime now ;

    OsTask::getCurrentTaskId(iTaskId) ;      
    OsDateTime::getCurTime(now) ;

    mSeed = (now.cvtToMsecs() ^ (now.usecs() + (now.usecs() << 16)) ^ 
            iTaskId) + siCounter++;

#ifdef __pingtel_on_posix__
    // using rand_r which takes seed/state at every invocation
#else
    srand(mSeed);
#endif
}

// Constructor
UtlRandom::UtlRandom(int seed) :
    mSeed(seed)
{
#ifdef __pingtel_on_posix__
    // using rand_r which takes seed/state at every invocation
#else
    srand(mSeed);
#endif
}

// Destructor
UtlRandom::~UtlRandom()
{
}


/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

int UtlRandom::rand()
{
#ifdef __pingtel_on_posix__
    return ::rand_r(&mSeed);
#else
    return ::rand();
#endif
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


