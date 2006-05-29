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
    int iTaskId = 0 ;
    OsTime now ;
    unsigned int seed ;

    OsTask::getCurrentTaskId(iTaskId) ;      
    OsDateTime::getCurTime(now) ;

    seed = (now.cvtToMsecs() ^ (now.usecs() + now.usecs() << 16) ^ iTaskId) ;

    srand(seed) ;
}

// Constructor
UtlRandom::UtlRandom(int seed)
{
    srand(seed) ;
}

// Destructor
UtlRandom::~UtlRandom()
{
}


/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

int UtlRandom::rand()
{
    return ::rand() ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


