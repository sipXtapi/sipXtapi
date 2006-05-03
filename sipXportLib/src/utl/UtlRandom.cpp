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

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlRandom::UtlRandom()
{
    static int count = 0 ;

    OsTime now ;
    OsDateTime::getCurTime(now) ;
    srand((int) (now.cvtToMsecs() % INT_MAX) ^ ++count) ;
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


