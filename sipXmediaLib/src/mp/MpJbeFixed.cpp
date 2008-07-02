//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpJbeFixed.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char *MpJbeFixed::name = "Fixed length JB";

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpJbeFixed::MpJbeFixed()
{
}

MpJbeFixed::~MpJbeFixed()
{
}

OsStatus MpJbeFixed::init(int samplerate)
{
   // Calculate desired delay.
   mSamplerate = samplerate;
   mDelay = mSamplerate/5;

   return OS_SUCCESS;
}

OsStatus MpJbeFixed::update(const RtpHeader *rtp,
                            uint32_t cur_rtp_timestamp,
                            uint32_t cur_playback_time,
                            int32_t *hint)
{
   *hint = mDelay;

   return OS_SUCCESS;
}

/* ============================= MANIPULATORS ============================= */

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */
