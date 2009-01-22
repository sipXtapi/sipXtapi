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
#include "mp/RtpHeader.h"
#ifdef WIN32
#  include <winsock2.h>
#else
#  include <netinet/in.h>
#endif

//#define RTL_ENABLED
#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#else
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#endif

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

/* ============================= MANIPULATORS ============================= */

OsStatus MpJbeFixed::init(int samplerate)
{
   mSamplerate = samplerate;
   // Set JB length to 1/3sec = 333ms
   mJbLength = mSamplerate/3;
   mDelay = 0;

   RTL_EVENT("JbUpdate_first_offset", 0);

   return OS_SUCCESS;
}

OsStatus MpJbeFixed::update(const RtpHeader *rtp,
                            uint32_t cur_rtp_timestamp,
                            uint32_t cur_playback_time,
                            int32_t *hint)
{
   // Calculate packet delay
   int32_t delay = cur_playback_time - ntohl(rtp->timestamp);

   // If packet falls either after or before JB, update JB position
   // by half of its size.
   if ((mDelay - delay > mJbLength) || (mDelay - delay < 0))
   {
      mDelay = delay + mJbLength/2;
   }

   // Return JB position
   *hint = mDelay;

   RTL_EVENT("JbUpdate_real_delay", delay);
   RTL_EVENT("JbUpdate_cur_time", cur_playback_time);
   RTL_EVENT("JbUpdate_delay", mDelay);
   RTL_EVENT("JbUpdate_variation", 0);
   RTL_EVENT("JbUpdate_recommended_delay", *hint);

   return OS_SUCCESS;
}

void MpJbeFixed::reset()
{
   // Set JB length to 1/3sec = 333ms
   mJbLength = mSamplerate/3;
   mDelay = 0;

   RTL_EVENT("JbUpdate_first_offset", 0);

}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */
