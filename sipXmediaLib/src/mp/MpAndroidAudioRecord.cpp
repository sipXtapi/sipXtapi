//  
// Copyright (C) 2010-2013 SIPez LLC. 
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#define LOG_NDEBUG 0
#define LOG_TAG "MpAndroidAudioRecord"

// SIPX INCLUDES
#include <os/OsSharedLibMgr.h>
#include <mp/MpAndroidAudioRecord.h>

// SYSTEM INCLUDES
#include <utils/Log.h>
#include <utils/Errors.h>

using namespace android;

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// private constructor
MpAndroidAudioRecord::MpAndroidAudioRecord()
{
    LOGV("MpAndroidAudioRecord constructor\n");
}

MpAndroidAudioRecord::~MpAndroidAudioRecord()
{
    LOGV("MpAndroidAudioRecord destructor\n");
}

/* ============================ MANIPULATORS ============================== */

int /*status_t*/ MpAndroidAudioRecord::start()
{
    LOGE("MpAndroidAudioRecord unimplemented start method\n");
    return(NO_ERROR); /* NO_INIT if we want to fail if there is no implementation */
}

void MpAndroidAudioRecord::stop()
{
    LOGE("MpAndroidAudioRecord unimplemented stop method\n");
}

int /*status_t*/ MpAndroidAudioRecord::set(int inputSource,
                                           int sampleRate,
                                           sipXcallback_t audioCallback,
                                           void* user,
                                           int notificationFrames)
{
    LOGE("MpAndroidAudioRecord unimplemented set method\n");
    return(NO_ERROR); /* NO_INIT if we want to fail if there is no implementation */
}

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
