//  
// Copyright (C) 2010-2011 SIPez LLC. 
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#define LOG_NDEBUG 0
//#define LOG_TAG defined in <mp/MpAndroidX_XAudioRecord.h>
//#define ENABLE_FRAME_TIME_LOGGING
//#define ENABLE_FILE_LOGGING

// SIPX INCLUDES
#if ANDROID_2_3
// Must include specific version of pthreads here before Android audio stuff for Android 2.3 so
// so that this can be compiled for Android 2.3 using NDK r3
#    include <development/ndk/platforms/android-9/include/pthread.h>
#endif
#include <mp/MpAndroidX_XAudioRecord.h>

// SYSTEM INCLUDES
#include <utils/Log.h>
//#include <media/AudioSystem.h>

using namespace android;

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

MpAndroidAudioRecord* createAndroidAudioRecord()
{
    return(new MP_ANDROID_AUDIO_RECORD());
}

/* ============================ CREATORS ================================== */
// Default constructor
MP_ANDROID_AUDIO_RECORD::MP_ANDROID_AUDIO_RECORD() :
mpAudioRecord(NULL)
{
    LOGV("%s constructor\n", QUOTED_MP_ANDROID_AUDIO_RECORD);
    mpAudioRecord = new AudioRecord();
}

MP_ANDROID_AUDIO_RECORD::~MP_ANDROID_AUDIO_RECORD()
{
    LOGV("%s destructor\n", QUOTED_MP_ANDROID_AUDIO_RECORD);
    if(mpAudioRecord)
    {
        delete mpAudioRecord;
        mpAudioRecord = NULL;
    }
}

/* ============================ MANIPULATORS ============================== */

int /*status_t*/ MP_ANDROID_AUDIO_RECORD::start()
{
    mpAudioRecord->start();
}

void MP_ANDROID_AUDIO_RECORD::stop()
{
    mpAudioRecord->stop();
}

int /*status_t*/ MP_ANDROID_AUDIO_RECORD::set(int inputSource,
                                              int sampleRate,
                                              sipXcallback_t audioCallback,
                                              void* user,
                                              int notificationFrames)
{
    return(mpAudioRecord->set(inputSource,
                              sampleRate,
                              AudioSystem::PCM_16_BIT,  // format
                              AudioSystem::CHANNEL_IN_MONO,  // channels
                              0,  // frameCount
                              AudioRecord::RECORD_AGC_ENABLE | AudioRecord::RECORD_NS_ENABLE,  // flags
                              audioCallback,  // cbf
                              user,
                              notificationFrames,
                              false));  // threadCanCallJava
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
