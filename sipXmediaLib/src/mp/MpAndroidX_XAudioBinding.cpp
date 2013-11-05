//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#define LOG_NDEBUG 0
//#define LOG_TAG defined in <mp/MpAndroidX_XAudioBinding.h>
//#define ENABLE_FRAME_TIME_LOGGING
//#define ENABLE_FILE_LOGGING

// SIPX INCLUDES
#if defined(ANDROID_2_3) || defined(ANDROID_2_3_4) || defined(ANDROID_4_0_1) ||  defined(ANDROID_4_1_1) || defined(ANDROID_4_2_1)
// Must include specific version of pthreads here before Android audio stuff for Android 2.3 so
// so that this can be compiled for Android 2.3 using NDK r3
#    include <development/ndk/platforms/android-9/include/pthread.h>
#endif
#include <mp/MpAndroidX_XAudioBinding.h>
#include <mp/MpAndroidX_XAudioTrack.h>
#include <mp/MpAndroidX_XAudioRecord.h>

// SYSTEM INCLUDES
#include <utils/Log.h>
#include <media/AudioSystem.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// DEFINES
#define QUOTED_MACRO(M) #M

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

MpAndroidAudioBindingInterface* CREATE_ANDROID_AUDIO_BINDING_SYMBOL()
{
    return(MpAndroidAudioBindingInterface::spMpAndroidAudioBindingInterface ? 
       MpAndroidAudioBindingInterface::spMpAndroidAudioBindingInterface :
       new MP_ANDROID_AUDIO_BINDING_INTERFACE());
}

/* ============================ CREATORS ================================== */
// Default constructor
MP_ANDROID_AUDIO_BINDING_INTERFACE::MP_ANDROID_AUDIO_BINDING_INTERFACE()
{
    LOGV("%s constructor\n", QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE);
}

MP_ANDROID_AUDIO_BINDING_INTERFACE::~MP_ANDROID_AUDIO_BINDING_INTERFACE()
{
    LOGV("%s destructor\n", QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

MpAndroidAudioTrack* MP_ANDROID_AUDIO_BINDING_INTERFACE::createAudioTrack(int streamType,
                                                                          uint32_t sampleRate,
                                                                          int format,
                                                                          int channels,
                                                                          int frameCount,
                                                                          uint32_t flags,
                                                                          sipXcallback_t cbf,
                                                                          void* user,
                                                                          int notificationFrames) const
{
    return(new MP_ANDROID_AUDIO_TRACK(streamType,
                                      sampleRate,
                                      format,
                                      channels,
                                      frameCount,
                                      flags,
                                      cbf,
                                      user,
                                      notificationFrames));
}

MpAndroidAudioRecord* MP_ANDROID_AUDIO_BINDING_INTERFACE::createAudioRecord() const
{
    return(new MP_ANDROID_AUDIO_RECORD());
}

int MP_ANDROID_AUDIO_BINDING_INTERFACE::getOutputLatency(uint32_t& outputLatency, StreamType streamType) const
{
    int result = AudioSystem::getOutputLatency(&outputLatency, 
#if defined(ANDROID_2_0) || defined(ANDROID_2_3) || defined(ANDROID_2_3_4) || defined(ANDROID_4_0_1)
                                         (int)
#else
                                         (audio_stream_type_t)
#endif
                                            streamType);
    LOGV(QUOTED_MACRO(MP_ANDROID_AUDIO_BINDING_INTERFACE) "::getOutputLatency returned: %d", result);
    return(result);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
