//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#define LOG_NDEBUG 0
//#define LOG_TAG defined in <mp/MpAndroidX_XAudioTrack.h>
//#define ENABLE_FRAME_TIME_LOGGING
//#define ENABLE_FILE_LOGGING

// SIPX INCLUDES
#if defined(ANDROID_2_3) || defined(ANDROID_2_3_4) || defined(ANDROID_4_0_1) ||  defined(ANDROID_4_1_1) || defined(ANDROID_4_2_1)
// Must include specific version of pthreads here before Android audio stuff for Android 2.3 so
// so that this can be compiled for Android 2.3 using NDK r3
#    include <development/ndk/platforms/android-9/include/pthread.h>
#endif
#include <mp/MpAndroidX_XAudioTrack.h>

// SYSTEM INCLUDES
#include <utils/Log.h>
//#include <media/AudioSystem.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// DEFINES
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

MpAndroidAudioTrack* createAndroidAudioTrack(int streamType,
                                          uint32_t sampleRate,
                                          int format,
                                          int channels,
                                          int frameCount,
                                          uint32_t flags,
                                          sipXcallback_t cbf,
                                          void* user,
                                          int notificationFrames)
{
    return(new MP_ANDROID_AUDIO_TRACK(streamType, sampleRate, format, channels, frameCount, flags, cbf, user, notificationFrames));
}

/* ============================ CREATORS ================================== */
// Default constructor
MP_ANDROID_AUDIO_TRACK::MP_ANDROID_AUDIO_TRACK(int streamType,
                  uint32_t sampleRate,
                  int format         ,
                  int channels       ,
                  int frameCount     ,
                  uint32_t flags     ,
                  sipXcallback_t cbf     ,
                  void* user         ,
                  int notificationFrames) :
mpAudioTrack(NULL)
{
    LOGV("%s constructor\n", QUOTED_MP_ANDROID_AUDIO_TRACK);
    mpAudioTrack = new SipxAudioTrack(streamType, sampleRate, format, channels, frameCount, flags, cbf, user, notificationFrames);
}

MP_ANDROID_AUDIO_TRACK::~MP_ANDROID_AUDIO_TRACK()
{
    LOGV("%s destructor\n", QUOTED_MP_ANDROID_AUDIO_TRACK);
    if(mpAudioTrack)
    {
        delete mpAudioTrack;
        mpAudioTrack = NULL;
    }
}

/* ============================ MANIPULATORS ============================== */

void MP_ANDROID_AUDIO_TRACK::start()
{
    LOGV("start");
    mpAudioTrack->start();
}

void MP_ANDROID_AUDIO_TRACK::stop()
{
    LOGV("stop");
    mpAudioTrack->stop();
}

/* ============================ ACCESSORS ================================= */

int /*status_t*/ MP_ANDROID_AUDIO_TRACK::initCheck() const
{
    LOGV("initCheck");
    return(mpAudioTrack->initCheck());
}

uint32_t MP_ANDROID_AUDIO_TRACK::getSampleRate()
{
    LOGV("getSampleRate");
    uint32_t rate = mpAudioTrack->getSampleRate();
    LOGV("getSampleRate exit");
    return(rate);
}

int MP_ANDROID_AUDIO_TRACK::frameSize() const
{
    LOGV("frameSize");
    return(mpAudioTrack->frameSize());
}

uint32_t MP_ANDROID_AUDIO_TRACK::frameCount() const
{
    LOGV("frameCount");
    return(mpAudioTrack->frameCount());
}

uint32_t MP_ANDROID_AUDIO_TRACK::latency() const
{
    LOGV("latency");
    return(mpAudioTrack->latency());
}

void MP_ANDROID_AUDIO_TRACK::setVolume(float left, float right)
{
    LOGV("setVolume");
    mpAudioTrack->setVolume(left, right);
}

void MP_ANDROID_AUDIO_TRACK::dumpAudioTrack(const char* label)
{
    LOGV("dumpAudioTrack");
    if(mpAudioTrack)
    {
        LOGV("sizeof AudioTrack: %d sizeof size_t: %d\n", sizeof(AudioTrack), sizeof(size_t));
        LOGV("%s AudioTrack[-2] = %p\n", label, *(((int*)(mpAudioTrack))-2));
        LOGV("%s AudioTrack[-1] = %p\n", label, *(((int*)(mpAudioTrack))-1));
#if 0
        LOGV("%s AudioTrack[%d] = %p\n", label, (sizeof(SipxAudioTrack)/4)+1, *(((int*)(mpAudioTrack))-(sizeof(SipxAudioTrack)/4)+1));
        LOGV("%s AudioTrack[%d] = %p\n", label, (sizeof(SipxAudioTrack)/4)+2, *(((int*)(mpAudioTrack))-(sizeof(SipxAudioTrack)/4)+2));
        LOGV("%s AudioTrack[%d] = %p\n", label, (sizeof(SipxAudioTrack)/4)+3, *(((int*)(mpAudioTrack))-(sizeof(SipxAudioTrack)/4)+3));
        LOGV("%s AudioTrack[%d] = %p\n", label, (sizeof(SipxAudioTrack)/4)+4, *(((int*)(mpAudioTrack))-(sizeof(SipxAudioTrack)/4)+4));
        int ptrIndex = 0;
        for(ptrIndex = 0; ptrIndex < ((sizeof(AudioTrack) / 4) + 12); ptrIndex ++)
        {
            LOGV("%s AudioTrack[%d] = %p\n", label, ptrIndex, *(((int*)mpAudioTrack)+ptrIndex));
        }
        LOGV("ptrIndex: %d max: %d", ptrIndex, ((sizeof(AudioTrack) / 4) + 4));
#endif
    }
    else
    {
        LOGV("%s dumpAudioTrack null mpAudioTrack", label);
    }
    LOGV("dumpAudioTrack exit");
}
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
