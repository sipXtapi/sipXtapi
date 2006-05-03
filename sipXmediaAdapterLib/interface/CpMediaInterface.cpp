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
#include <assert.h>

// APPLICATION INCLUDES
#include "mi/CpMediaInterface.h"
#include "mi/CpMediaInterfaceFactoryImpl.h" 

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

OsStatus CpMediaInterface::setSrtpParams(SdpSrtpParameters& srtpParameters)
{
    if (srtpParameters.masterKey[0] != '\0') // only set the key if it comes from the caller
    {
        memcpy((void*)&mSrtpParams, (void*)&srtpParameters, sizeof(SdpSrtpParameters));
    }
    return OS_SUCCESS;
    
}


/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
