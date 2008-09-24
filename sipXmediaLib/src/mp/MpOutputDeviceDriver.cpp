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
, mIsEnabled(FALSE)
, mSamplesPerFrame(0)
, mSamplesPerSec(0)
{
}

MpOutputDeviceDriver::~MpOutputDeviceDriver()
{
   assert(!isEnabled());
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean MpOutputDeviceDriver::isEnabled() const
{
   return mIsEnabled;
};

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */
