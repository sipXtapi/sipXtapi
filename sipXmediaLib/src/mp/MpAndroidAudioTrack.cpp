//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#define LOG_NDEBUG 0
#define LOG_TAG "MpAndroidAudioTrack"
#define MPID_ANDROID_CLEAN_EXIT
//#define ENABLE_FRAME_TIME_LOGGING
//#define ENABLE_FILE_LOGGING

// SIPX INCLUDES
#include <mp/MpAndroidAudioRecord.h>
#include <mp/MpAndroidAudioTrack.h>
#include <os/OsSharedLibMgr.h>

// SYSTEM INCLUDES
#include <utils/Log.h>
#include <media/AudioSystem.h>

using namespace android;

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

MpAndroidAudioTrack::MpAndroidAudioTrack()
{
    LOGV("MpAndroidAudioTrack default constructor\n");
}

MpAndroidAudioTrack::~MpAndroidAudioTrack()
{
    LOGV("MpAndroidAudioTrack destructor\n");
}

/* ============================ MANIPULATORS ============================== */

void MpAndroidAudioTrack::start()
{
    LOGE("MpAndroidAudioTrack unimplemented start method\n");
}

void MpAndroidAudioTrack::stop()
{
    LOGE("MpAndroidAudioTrack unimplemented stop method\n");
}

/* ============================ ACCESSORS ================================= */

int /*status_t*/ MpAndroidAudioTrack::initCheck() const
{
    LOGE("MpAndroidAudioTrack unimplemented initCheck method\n");
    //return(NO_INIT);
    // Temporarily return ok so we can test with stub
    return(NO_ERROR);
}

uint32_t MpAndroidAudioTrack::getSampleRate()
{
    LOGE("MpAndroidAudioTrack unimplemented getSampleRate method\n");
    return(NO_INIT);
}

int MpAndroidAudioTrack::frameSize() const
{
    LOGE("MpAndroidAudioTrack unimplemented frameSize method\n");
    return(NO_INIT);
}

uint32_t MpAndroidAudioTrack::frameCount() const
{
    LOGE("MpAndroidAudioTrack unimplemented frameCount method\n");
    return(NO_INIT);
}

uint32_t MpAndroidAudioTrack::latency() const
{
    LOGE("MpAndroidAudioTrack unimplemented latency method\n");
    return(NO_INIT);
}

void MpAndroidAudioTrack::setVolume(float left, float right)
{
    LOGE("MpAndroidAudioTrack unimplemented setVolume method\n");
}

void MpAndroidAudioTrack::dumpAudioTrack(const char* label)
{
    LOGE("MpAndroidAudioTrack unimplemented dumpAudioTrack method\n");
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
