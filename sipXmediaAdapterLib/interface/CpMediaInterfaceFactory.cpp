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
#include <stdlib.h>

// APPLICATION INCLUDES
#include "mi/CpMediaInterfaceFactory.h"
#include "mi/CpMediaInterfaceFactoryImpl.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInterfaceFactory::CpMediaInterfaceFactory()
    : mpFactoryImpl(NULL)
{
}

// Destructor
CpMediaInterfaceFactory::~CpMediaInterfaceFactory()
{
    setFactoryImplementation(NULL) ;
}

/* ============================ MANIPULATORS ============================== */

// Set the actual factory implementation
void CpMediaInterfaceFactory::setFactoryImplementation(CpMediaInterfaceFactoryImpl* pFactoryImpl) 
{
    // Only bother if the pointers are different
    if (pFactoryImpl != mpFactoryImpl)
    {
        // Delete old version
        if (mpFactoryImpl)
        {
            mpFactoryImpl->release() ;
            mpFactoryImpl = NULL ;
        }

        // Set new version
        mpFactoryImpl = pFactoryImpl;
    }
}


// Create a media interface via the specified factory
CpMediaInterface* CpMediaInterfaceFactory::createMediaInterface(const char* publicAddress,
                                                                const char* localAddress,
                                                                int numCodecs,
                                                                SdpCodec* sdpCodecArray[],
                                                                const char* locale,
                                                                int expeditedIpTos,
                                                                const char* szStunServer,
                                                                int iStunPort,
                                                                int iStunKeepAlivePeriodSecs,
                                                                const char* szTurnSever,
                                                                int iTurnPort,
                                                                const char* szTurnUsername,
                                                                const char* szTurnPassword,
                                                                int iTurnKeepAlivePeriodSecs,
                                                                bool bEnableICE) 
{
    CpMediaInterface* pInterface = NULL ;

    if (mpFactoryImpl) 
    {
        pInterface = mpFactoryImpl->createMediaInterface(publicAddress, 
                localAddress, numCodecs, sdpCodecArray, locale, 
                expeditedIpTos, szStunServer, iStunPort, iStunKeepAlivePeriodSecs,
                szTurnSever, iTurnPort, szTurnUsername, szTurnPassword,
                iTurnKeepAlivePeriodSecs, bEnableICE) ;
    }

    return pInterface ;
}

/* ============================ ACCESSORS ================================= */

CpMediaInterfaceFactoryImpl* 
CpMediaInterfaceFactory::getFactoryImplementation()
{
    return mpFactoryImpl ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


