// 
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// Author: Dan Petrie (dpetrie AT SIPez DOT com)

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mi/CpMediaInterface.h"
#include "mi/CpMediaInterfaceFactoryImpl.h" 

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
int CpMediaInterface::sInvalidConnectionId = -1;

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

OsStatus CpMediaInterface::setSrtpParams(SdpSrtpParameters& srtpParameters)
{
    if (srtpParameters.masterKey[0] != '\0') // only set the key if it comes from the caller
    {
        memcpy((void*)&mSrtpParams, (void*)&srtpParameters, sizeof(SdpSrtpParameters));
    }
    return OS_SUCCESS;
    
}

/* ============================ ACCESSORS ================================= */

int CpMediaInterface::getInvalidConnectionId()
{
    return(sInvalidConnectionId);
}

void CpMediaInterface::setConfiguredIpAddress(const UtlString& ipAddress)
{
    mConfiguredIpAddress = ipAddress;
}

/* ============================ INQUIRY =================================== */

UtlBoolean CpMediaInterface::isConnectionIdValid(int connectionId)
{
    return(connectionId > sInvalidConnectionId);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
