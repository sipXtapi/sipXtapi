//  
// Copyright (C) 2010-2011 SIPez LLC. 
// Licensed by SIPfoundry under the LGPL license.
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
MpAndroidAudioTrack::MpAndroidAudioTrackCreator MpAndroidAudioTrack::spAudioTrackCreate = stubAndroidAudioTrackCreator;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
MpAndroidAudioTrack* stubAndroidAudioTrackCreator(int streamType,
                                          uint32_t sampleRate,
                                          int format,
                                          int channels,
                                          int frameCount,
                                          uint32_t flags,
                                          sipXcallback_t cbf,
                                          void* user,
                                          int notificationFrames)
{
    LOGE("stubAndroidAudioTrackCreator: Unimplemented Android AudioTrack");

    return(new MpAndroidAudioTrack());
}

MpAndroidAudioTrack::MpAndroidAudioTrack()
{
    LOGV("MpAndroidAudioTrack default constructor\n");
}

MpAndroidAudioTrack::~MpAndroidAudioTrack()
{
    LOGV("MpAndroidAudioTrack destructor\n");
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpAndroidAudioTrack::setAudioTrackCreator()
{
    OsStatus res;
    OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();
    const char* libName = "libsipXandroid2_0.so";
    res = pShrMgr->loadSharedLib(libName);
    LOGD("loadShardLib(\"%s\") returned: %d", libName, res);

    // Try loading the Android 2.3 drivers if 2.0 failed
    if(res != OS_SUCCESS)
    {
        libName = "libsipXandroid2_3.so";
        res = pShrMgr->loadSharedLib(libName);
        LOGD("loadShardLib(\"%s\") returned: %d", libName, res);
    }

    if(res == OS_SUCCESS)
    {
        void* symbolAddress = NULL;
        const char* symbolName = "createAndroidAudioTrack";

        res = pShrMgr->getSharedLibSymbol(libName, symbolName, symbolAddress);

        if(res == OS_SUCCESS && symbolAddress)
        {
            spAudioTrackCreate = (MpAndroidAudioTrackCreator)symbolAddress;
            LOGD("got symbol: \"%s\" funcPtr: %p res: %d", symbolName, symbolAddress, res);

            // Get the record creator too
            res = MpAndroidAudioRecord::setAudioRecordCreator(libName);
        }
        else
        {
            res = OS_PLATFORM_NOT_SUPPORTED;
            LOGE("get symbol: %s failed: %d", symbolName, res);
        }
    }

    return(res);
}

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
