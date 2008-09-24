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
#include <os/OsNotification.h>
#include <os/OsTimer.h>
#include <utl/UtlSListIterator.h>

// APPLICATION INCLUDES
#include "RingTonePlayer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
RingTonePlayer* RingTonePlayer::mInstance = NULL;
UtlString RingTonePlayer::mRingToneFile = NULL;
int RingTonePlayer::mState = RING_STATE_QUIET;

// MACROS


// Constructor

RingTonePlayer* RingTonePlayer::getRingTonePlayer()
{
    if (RingTonePlayer::mInstance == NULL)
    {
        RingTonePlayer::mInstance = new RingTonePlayer();
    }
    return RingTonePlayer::mInstance;
}

void RingTonePlayer::releaseRingTonePlayer()
{
    if (RingTonePlayer::mState == RING_STATE_QUIET && 
        RingTonePlayer::mInstance != NULL)
    {
        delete RingTonePlayer::mInstance;
        RingTonePlayer::mInstance = NULL;
    }
}

RingTonePlayer::RingTonePlayer() : 
                 mNotification(Callback), 
                 mTimer(mNotification)
{
}

RingTonePlayer::~RingTonePlayer()
{
}

void RingTonePlayer::Callback()
{
#ifdef _WIN32
    if (mRingToneFile != NULL)
    {
        PlaySound(mRingToneFile, NULL, SND_FILENAME);
    }
#endif
}

void RingTonePlayer::startRinging()
{
    RingTonePlayer::mState = RING_STATE_RINGING;
    mTimer.periodicEvery(OsTime(0,0), OsTime(2,0));
}

void RingTonePlayer::stopRinging()
{
    mTimer.stop();
    RingTonePlayer::mState = RING_STATE_QUIET;
}

void RingTonePlayer::setRingToneFile(char *sFileName)
{
    RingTonePlayer::mRingToneFile = sFileName;
}

/////////////////////////////////////////////////////////////////////////////////
RingToneNotification::RingToneNotification(void (*fn)())
{
    mCallback = fn;
}

RingToneNotification::~RingToneNotification()
{
}

OsStatus RingToneNotification::signal(const int eventdata)
{
    if (mCallback)
    {
        mCallback();
    }
    return OS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////
