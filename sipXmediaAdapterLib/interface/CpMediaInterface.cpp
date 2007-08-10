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
   : mpMediaNotfDispatcher(NULL)
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

OsMsgDispatcher* 
CpMediaInterface::setMediaNotificationDispatcher(OsMsgDispatcher* pNoteDisper)
{
   OsMsgDispatcher* ret = mpMediaNotfDispatcher;
   mpMediaNotfDispatcher = pNoteDisper;
   return ret;
}

OsStatus
CpMediaInterface::setMediaNotificationsEnabled(bool enabled, 
                                               const UtlString& resourceName)
{
   return OS_NOT_YET_IMPLEMENTED;
}

OsStatus CpMediaInterface::enableRtpReadNotification(int connectionId,
                                                     UtlBoolean bEnable) 
{
   return OS_NOT_SUPPORTED;
};

OsStatus CpMediaInterface::recordMic(UtlString* pAudioBuf) 
{ 
   return OS_NOT_SUPPORTED; 
};

/* ============================ ACCESSORS ================================= */

int CpMediaInterface::getInvalidConnectionId()
{
    return(sInvalidConnectionId);
}

OsMsgDispatcher* CpMediaInterface::getMediaNotificationDispatcher()
{
   return mpMediaNotfDispatcher;
}

/* ============================ INQUIRY =================================== */

UtlBoolean CpMediaInterface::isConnectionIdValid(int connectionId)
{
    return(connectionId > sInvalidConnectionId);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
