//  
// Copyright (C) 2010-2011 SIPez LLC. 
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
MpAndroidAudioRecord::MpAndroidAudioRecordCreator MpAndroidAudioRecord::spAudioRecordCreate = MpAndroidAudioRecord::stubAndroidAudioRecordCreator;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
MpAndroidAudioRecord* MpAndroidAudioRecord::stubAndroidAudioRecordCreator()
{
    LOGE("stubAndroidAudioRecordCreator: Unimplemented Android AudioRecord");

    return(new MpAndroidAudioRecord());
}

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

OsStatus MpAndroidAudioRecord::setAudioRecordCreator(const char* libName)
{
    void* symbolAddress = NULL;
    const char* symbolName = "createAndroidAudioRecord";
    OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();
    OsStatus result = pShrMgr->getSharedLibSymbol(libName, symbolName, symbolAddress);

    if(result == OS_SUCCESS && symbolAddress)
    {
        spAudioRecordCreate = (MpAndroidAudioRecordCreator)symbolAddress;
        LOGD("got symbol: \"%s\" funcPtr: %p result: %d", symbolName, symbolAddress, result);
    }
    else
    {
        result = OS_PLATFORM_NOT_SUPPORTED;
        LOGE("get symbol: %s failed: %d", symbolName, result);
    }

    return(result);
}

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
