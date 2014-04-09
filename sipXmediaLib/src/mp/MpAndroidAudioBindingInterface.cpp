//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#define LOG_NDEBUG 0
#define LOG_TAG "MpAndroidAudioBindingInterface"
#define MPID_ANDROID_CLEAN_EXIT
//#define ENABLE_FILE_LOGGING

// SIPX INCLUDES
#include <os/OsSharedLibMgr.h>
#include <utl/UtlString.h>
#include <mp/MpAndroidAudioRecord.h>
#include <mp/MpAndroidAudioTrack.h>
#include <mp/MpAndroidAudioBindingInterface.h>

// SYSTEM INCLUDES
#include <utils/Log.h>
#include <media/AudioSystem.h>

using namespace android;

// EXTERNAL FUNCTIONS
MpAndroidAudioBindingInterface* stubGetAndroidAudioBindingInterface();

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
MpAndroidAudioBindingInterface::MpAndroidAudioBindingInterfaceCreator MpAndroidAudioBindingInterface::spGetAndroidAudioBinding = stubGetAndroidAudioBindingInterface;
MpAndroidAudioBindingInterface* MpAndroidAudioBindingInterface::spMpAndroidAudioBindingInterface = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
MpAndroidAudioBindingInterface* stubGetAndroidAudioBindingInterface()
{
    if(MpAndroidAudioBindingInterface::spMpAndroidAudioBindingInterface == NULL)
    {
        UtlString versionSpecificLibrary;
        OsStatus status = MpAndroidAudioBindingInterface::getVersionSpecificLibraryName(versionSpecificLibrary);
        if(status == OS_SUCCESS)
        {
            // Find the audio system object creator symbol
            void* symbolAddress = NULL;
            const char* symbolName = CREATE_ANDROID_AUDIO_BINDING_SYMBOL_STRING;

            OsSharedLibMgrBase* sharedLibMgr = OsSharedLibMgr::getOsSharedLibMgr();
            status = sharedLibMgr->getSharedLibSymbol(versionSpecificLibrary, symbolName, symbolAddress);

            if(status == OS_SUCCESS && symbolAddress)
            {
                MpAndroidAudioBindingInterface::spGetAndroidAudioBinding = 
                    (MpAndroidAudioBindingInterface::MpAndroidAudioBindingInterfaceCreator)symbolAddress;
                LOGD("got symbol: \"%s\" funcPtr: %p res: %d", symbolName, symbolAddress, status);
                MpAndroidAudioBindingInterface::spMpAndroidAudioBindingInterface = 
                     MpAndroidAudioBindingInterface::spGetAndroidAudioBinding();
            }

            // If failed to load audio implementation from shared lib, construct default stub
            if( MpAndroidAudioBindingInterface::spMpAndroidAudioBindingInterface == NULL)
            {
                LOGE("stubGetAndroidAudioBindingInterface: Unimplemented Android AudioTrack");
                MpAndroidAudioBindingInterface::spMpAndroidAudioBindingInterface = new MpAndroidAudioBindingInterface();
            }
        }
        MpAndroidAudioBindingInterface::spMpAndroidAudioBindingInterface->mStatus = status;
    }

    // Previously initialized/created audio binding
    else
    {
        LOGV("stubGetAndroidAudioBindingInterface previously initialized, status: %d",
             MpAndroidAudioBindingInterface::spMpAndroidAudioBindingInterface->getStatus());
    }

    return(MpAndroidAudioBindingInterface::spMpAndroidAudioBindingInterface);
}

MpAndroidAudioBindingInterface::MpAndroidAudioBindingInterface() :
mStatus(OS_INVALID)
{
    LOGV("MpAndroidAudioBindingInterface default constructor\n");
}

MpAndroidAudioBindingInterface::~MpAndroidAudioBindingInterface()
{
    LOGV("MpAndroidAudioBindingInterface destructor\n");
}

/* ============================ MANIPULATORS ============================== */


OsStatus MpAndroidAudioBindingInterface::getVersionSpecificLibraryName(UtlString& sharedLibraryName)
{
    sharedLibraryName.remove(0);
    OsStatus res = OS_NOT_FOUND;
    OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();
    const char* audioDriverLibNames[] =
    {
        "libsipXandroid2_0.so",
        "libsipXandroid2_3.so",
        "libsipXandroid2_3_4.so",
	"libsipXandroid4_0_1.so",
        "libsipXandroid4_1_1.so",
        "libsipXandroid4_2_1.so"
    };

    const char* libName = NULL;
    for(int libIndex = 0; libIndex < sizeof(audioDriverLibNames)/sizeof(const char*); libIndex++)
    {
        libName = audioDriverLibNames[libIndex];
        res = pShrMgr->loadSharedLib(libName);
        LOGD("Trying libs [%d/%d] for platform specific audio driver, loadSharedLib(\"%s\") returned: %d", 
            libIndex, sizeof(audioDriverLibNames)/sizeof(const char*), libName, res);
        if(res == OS_SUCCESS)
        {
            sharedLibraryName = libName;
            break;
        }
    }
    return(res);
}

MpAndroidAudioTrack* MpAndroidAudioBindingInterface::createAudioTrack(int streamType,
                                                                      uint32_t sampleRate,
                                                                      int format,
                                                                      int channels,
                                                                      int frameCount,
                                                                      uint32_t flags,
                                                                      sipXcallback_t cbf,
                                                                      void* user,
                                                                      int notificationFrames) const
{
   return(new MpAndroidAudioTrack());
}

MpAndroidAudioRecord* MpAndroidAudioBindingInterface::createAudioRecord() const
{
    return(new MpAndroidAudioRecord());
}

/* ============================ ACCESSORS ================================= */

OsStatus MpAndroidAudioBindingInterface::getStatus() const
{
    return(mStatus);
}

int MpAndroidAudioBindingInterface::getOutputLatency(uint32_t& outputLatency, StreamType streamType) const
{
    LOGE("MpAndroidAudioBindingInterface unimplemented frameSize method\n");
    outputLatency = -1;
    return(NO_INIT);
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
