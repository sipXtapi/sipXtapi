//  
// Copyright (C) 2010-2011 SIPez LLC. 
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#define LOG_NDEBUG 0
//#define LOG_TAG defined in <mp/MpAndroidX_XAudioTrack.h>
//#define ENABLE_FRAME_TIME_LOGGING
//#define ENABLE_FILE_LOGGING

// SIPX INCLUDES
#if ANDROID_2_3
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
    mpAudioTrack->start();
}

void MP_ANDROID_AUDIO_TRACK::stop()
{
    mpAudioTrack->stop();
}

/* ============================ ACCESSORS ================================= */

int /*status_t*/ MP_ANDROID_AUDIO_TRACK::initCheck() const
{
    return(mpAudioTrack->initCheck());
}

uint32_t MP_ANDROID_AUDIO_TRACK::getSampleRate()
{
    return(mpAudioTrack->getSampleRate());
}

int MP_ANDROID_AUDIO_TRACK::frameSize() const
{
    return(mpAudioTrack->frameSize());
}

uint32_t MP_ANDROID_AUDIO_TRACK::frameCount() const
{
    return(mpAudioTrack->frameCount());
}

uint32_t MP_ANDROID_AUDIO_TRACK::latency() const
{
    return(mpAudioTrack->latency());
}

void MP_ANDROID_AUDIO_TRACK::setVolume(float left, float right)
{
    mpAudioTrack->setVolume(left, right);
}

void MP_ANDROID_AUDIO_TRACK::dumpAudioTrack(const char* label)
{
    if(mpAudioTrack)
    {
        LOGV("sizeof AudioTrack: %d sizeof size_t: %d\n", sizeof(AudioTrack), sizeof(size_t));
        LOGV("%s AudioTrack[-2] = %p\n", label, *(((int*)(mpAudioTrack))-2));
        LOGV("%s AudioTrack[-1] = %p\n", label, *(((int*)(mpAudioTrack))-1));
        LOGV("%s AudioTrack[%d] = %p\n", label, (sizeof(SipxAudioTrack)/4)+2, *(((int*)(mpAudioTrack))-(sizeof(SipxAudioTrack)/4)+2));
        LOGV("%s AudioTrack[%d] = %p\n", label, (sizeof(SipxAudioTrack)/4)+1, *(((int*)(mpAudioTrack))-(sizeof(SipxAudioTrack)/4)+1));
#if 0
        for(int ptrIndex = 0; ptrIndex < (sizeof(SipxAudioTrack) / 4); ptrIndex ++)
        {
            LOGV("%s AudioTrack[%d] = %p\n", label, ptrIndex, *(((int*)(mpAudioTrack))+ptrIndex));
        }
#endif
    }
    else
    {
        LOGV("%s dumpAudioTrack null mpAudioTrack", label);
    }
}
/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
