//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpOutputDeviceDriver.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpOutputDeviceDriver::MpOutputDeviceDriver(const UtlString& name)
: UtlString(name)
, mpDeviceManager(NULL)
, mIsEnabled(FALSE)
//, mDeviceId(-1)
, mSamplesPerFrame(0)
, mSamplesPerSec(0)
{
}

MpOutputDeviceDriver::~MpOutputDeviceDriver()
{
    assert(!isEnabled());
//    clearDeviceId();
    mpDeviceManager = NULL;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/*
OsStatus MpOutputDeviceDriver::setDeviceId(MpOutputDeviceHandle deviceId) 
{ 
    assert(deviceId > 0);

    OsStatus status = OS_BUSY;
    if (!isEnabled())
    {
        mDeviceId = deviceId; 
        status = OS_SUCCESS;
    }
    return status;
}

OsStatus MpOutputDeviceDriver::clearDeviceId()
{
    OsStatus status = OS_BUSY;
    if (!isEnabled())
    {
        mDeviceId = -1;
        status = OS_SUCCESS;
    }
    return status;
}
*/

/* ============================ INQUIRY =================================== */

UtlBoolean MpOutputDeviceDriver::isEnabled()
{
    return mIsEnabled;
};

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */
