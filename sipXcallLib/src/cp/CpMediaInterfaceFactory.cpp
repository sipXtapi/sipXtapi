// $Id$
//
// Copyright (C) 2005 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "cp/CpMediaInterfaceFactory.h"
#include "cp/CpMediaInterfaceFactoryInterface.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInterfaceFactory::CpMediaInterfaceFactory()
    : mpFactoryInterface(NULL)
{
}

// Destructor
CpMediaInterfaceFactory::~CpMediaInterfaceFactory()
{
    setFactoryImplementation(NULL) ;
}

/* ============================ MANIPULATORS ============================== */

// Set the actual factory implementation
void CpMediaInterfaceFactory::setFactoryImplementation(CpMediaInterfaceFactoryInterface* pFactoryInterface) 
{
    // Only bother if the pointers are different
    if (pFactoryInterface != mpFactoryInterface)
    {
        // Delete old version
        if (mpFactoryInterface)
        {
            delete mpFactoryInterface ;
            mpFactoryInterface = NULL ;
        }

        // Set new version
        mpFactoryInterface = pFactoryInterface ;
    }
}


// Create a media interface via the specified factory
CpMediaInterface* CpMediaInterfaceFactory::createMediaInterface(int startRtpPort, 
                                                                int lastRtpPort,
                                                                const char* publicAddress,
                                                                const char* localAddress,
                                                                int numCodecs,
                                                                SdpCodec* sdpCodecArray[],
                                                                const char* locale,
                                                                int expeditedIpTos,
                                                                const char* szStunServer,
                                                                int iStunKeepAlivePeriodSecs) 
{
    CpMediaInterface* pInterface = NULL ;

    if (mpFactoryInterface) 
    {
        pInterface = mpFactoryInterface->createMediaInterface(startRtpPort, 
                lastRtpPort, publicAddress, localAddress, numCodecs, 
                sdpCodecArray, locale, expeditedIpTos, szStunServer, 
                iStunKeepAlivePeriodSecs) ;
    }

    return pInterface ;
}

/* ============================ ACCESSORS ================================= */

CpMediaInterfaceFactoryInterface* 
CpMediaInterfaceFactory::getFactoryImplementation()
{
    return mpFactoryInterface ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


