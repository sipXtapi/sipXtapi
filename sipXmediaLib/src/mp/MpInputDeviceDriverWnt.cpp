//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <Windows.h>

// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriverWnt.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpInputDeviceDriverWnt::MpInputDeviceDriverWnt(
        const UtlString& name, 
        MpInputDeviceManager& deviceManager)
    : MpInputDeviceDriver(name, deviceManager)
    , mWntDeviceId(-1)
{
    WAVEINCAPS devCaps;
    // Grab the number of input devices that are available.
    int nInputDevs = waveInGetNumDevs();

    // Search through the input devices looking for the input device specified.
    MMRESULT wavResult = MMSYSERR_NOERROR;
    int i;
    for (i = 0; i < nInputDevs; i++)
    {
        MMRESULT res = waveInGetDevCaps(i, &devCaps, sizeof(devCaps));
        if (res != MMSYSERR_NOERROR)
        {
            wavResult = res;
        } 
        else if (strncmp(name, devCaps.szPname, MAXPNAMELEN) == 0)
        {
            mWntDeviceId = i;
        }
    }
}

// Destructor
MpInputDeviceDriverWnt::~MpInputDeviceDriverWnt() {}


/* ============================ MANIPULATORS ============================== */
OsStatus MpInputDeviceDriverWnt::enableDevice(unsigned samplesPerFrame, 
                                              unsigned samplesPerSec, 
                                              unsigned currentFrameTime)
{
    // If the device is not valid, let the user know it's bad.
    if (!isDeviceValid())
        return OS_INVALID_STATE;  // perhaps new OsState of OS_RESOURCE_INVALID?

    OsStatus status = 
        MpInputDeviceDriver::enableDevice(samplesPerFrame, samplesPerSec, 
                                          currentFrameTime);
    if (status == OS_SUCCESS)
    {
        // TODO: Do stuff to enable device.
    }

    return status;
}

OsStatus MpInputDeviceDriverWnt::disableDevice()
{
    OsStatus status = OS_SUCCESS;
    
    // TODO: Do stuff to disable device.

    if (status == OS_SUCCESS)
        status = MpInputDeviceDriver::disableDevice();
    return status;
}

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */
// Copy constructor (not implemented for this class)
//MpWntInputDeviceDriver::MpWntInputDeviceDriver(const MpInputDeviceDriver& rMpInputDeviceDriver) {}
// Copy constructor (not implemented for this class)
//MpWntInputDeviceDriver& MpWntInputDeviceDriver::operator =(const MpInputDeviceDriver &rhs) {}