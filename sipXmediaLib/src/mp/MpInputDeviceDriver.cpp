//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriver.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpInputDeviceDriver::MpInputDeviceDriver(const UtlString& name, 
                                         MpInputDeviceManager& deviceManager)
: UtlString(name)
, mpInputDeviceManager(&deviceManager)
, mIsEnabled(FALSE)
, mDeviceId(-1)
, mSamplesPerFrame(0)
, mSamplesPerSec(0)
, mCurrentFrameTime(0)
{
}

MpInputDeviceDriver::~MpInputDeviceDriver()
{
  // NOTE: This assert() will often need to be removed when running this under Valgrind:
    assert(!isEnabled());
    clearDeviceId();
    mpInputDeviceManager = NULL;
}

/* ============================ MANIPULATORS ============================== */

// I have no clue why this is necessary.  disableDevice is
// pure virtual and should not be implemented, but MSVS6 is bitching
// about it being undefined.
OsStatus MpInputDeviceDriver::disableDevice()
{
    return(OS_SUCCESS);
}

OsStatus MpInputDeviceDriver::setDeviceId(MpInputDeviceHandle deviceId) 
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

OsStatus MpInputDeviceDriver::clearDeviceId()
{
    OsStatus status = OS_BUSY;
    if (!isEnabled())
    {
        mDeviceId = -1;
        status = OS_SUCCESS;
    }
    return status;
}

/* ============================ ACCESSORS ================================= */

MpInputDeviceHandle MpInputDeviceDriver::getDeviceId() const
{
    return(mDeviceId);
}

/* ============================ INQUIRY =================================== */

UtlBoolean MpInputDeviceDriver::isEnabled()
{
    return(mIsEnabled);
};

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */
