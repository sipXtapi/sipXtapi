//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


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
    OsTime now ;
    OsDateTime::getCurTime(now) ;
    srand((int) (now.cvtToMsecs() % INT_MAX)) ;
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


