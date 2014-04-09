//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#define LOG_NDEBUG 0
//#define LOG_TAG defined in <mp/MpAndroidX_XAudioRecord.h>
//#define ENABLE_FRAME_TIME_LOGGING
//#define ENABLE_FILE_LOGGING

// SIPX INCLUDES
#if defined(ANDROID_2_3) || defined(ANDROID_2_3_4) || defined(ANDROID_4_0_1) || defined(ANDROID_4_1_1) || defined(ANDROID_4_2_1)
// Must include specific version of pthreads here before Android audio stuff for Android 2.3 so
// so that this can be compiled for Android 2.3 using NDK r3
#    include <development/ndk/platforms/android-9/include/pthread.h>
#endif
#include <mp/MpAndroidX_XAudioRecord.h>
#include <mp/MpAndroidX_XAudioTrack.h>

// SYSTEM INCLUDES
#include <utils/Log.h>
//#include <media/AudioSystem.h>

using namespace android;

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// DEFINES
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
    return(mpAudioRecord->set(
#if defined(ANDROID_4_1_1) || defined(ANDROID_4_2_1)
                              (audio_source_t)
#endif
                                inputSource,
                              sampleRate,
#if defined(ANDROID_4_0_1) || defined(ANDROID_4_1_1) || defined(ANDROID_4_2_1)
                              AUDIO_FORMAT_PCM_16_BIT, // format
                              AUDIO_CHANNEL_IN_MONO, // # channels
#else
                              AudioSystem::PCM_16_BIT,  // format
                              AudioSystem::CHANNEL_IN_MONO,  // # channels
#endif
                              0,  // frameCount
#if !defined(ANDROID_4_2_1)
#  if defined(ANDROID_4_1_1) || defined(ANDROID_4_2_1)
                              (AudioRecord::record_flags)
#  endif
                                (AudioRecord::RECORD_AGC_ENABLE | AudioRecord::RECORD_NS_ENABLE),  // flags
#endif
                              audioCallback,  // cbf
                              user,
                              notificationFrames,
                              false));  // threadCanCallJava
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
