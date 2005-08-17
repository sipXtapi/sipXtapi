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
#include "cp/CpMediaInterface.h"
#include "cp/CpMediaInterfaceFactoryImpl.h" 

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInterface::CpMediaInterface(CpMediaInterfaceFactoryImpl *pFactoryImpl)
{
    mpFactoryImpl = pFactoryImpl ; 
}

// Destructor
CpMediaInterface::~CpMediaInterface()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
