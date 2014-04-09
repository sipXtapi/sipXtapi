//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpAndroidX_XAudioBinding_h_
#define _MpAndroidX_XAudioBinding_h_

// Must be included before Android utils/log.h so we define LOG_TAG first

#ifdef ANDROID_2_0
#    define MP_ANDROID_AUDIO_BINDING_INTERFACE MpAndroid2_0AudioBindingInterface
#    define QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE "MpAndroid2_0AudioBindingInterface"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE
#    endif
#elif ANDROID_2_3
#    define MP_ANDROID_AUDIO_BINDING_INTERFACE MpAndroid2_3AudioBindingInterface
#    define QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE "MpAndroid2_3AudioBindingInterface"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE
#    endif
     extern "C" int setpriority(int, int, int);
#    define PRIO_PROCESS 0
#elif ANDROID_2_3_4
#    define MP_ANDROID_AUDIO_BINDING_INTERFACE MpAndroid2_3_4AudioBindingInterface
#    define QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE "MpAndroid2_3_4AudioBindingInterface"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE
#    endif
     extern "C" int setpriority(int, int, int);
#    define PRIO_PROCESS 0
#elif ANDROID_4_0_1
#    define MP_ANDROID_AUDIO_BINDING_INTERFACE MpAndroid4_0_1AudioBindingInterface
#    define QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE "MpAndroid4_0_1AudioBindingInterface"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE
#    endif
     extern "C" int setpriority(int, int, int);
#    define PRIO_PROCESS 0
#elif ANDROID_4_1_1
#    define MP_ANDROID_AUDIO_BINDING_INTERFACE MpAndroid4_1_1AudioBindingInterface
#    define QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE "MpAndroid4_1_1AudioBindingInterface"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE
#    endif
     extern "C" int setpriority(int, int, int);
#    define PRIO_PROCESS 0
#    define LOGV ALOGV
#    define LOGD ALOGD
#elif ANDROID_4_2_1
#    define MP_ANDROID_AUDIO_BINDING_INTERFACE MpAndroid4_2_1AudioBindingInterface
#    define QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE "MpAndroid4_2_1AudioBindingInterface"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_BINDING_INTERFACE
#    endif
     extern "C" int setpriority(int, int, int);
#    define PRIO_PROCESS 0
#    define LOGV ALOGV
#    define LOGD ALOGD
#else
#    error Unsupported version of Android AudioBindingInterface
#endif

// SIPX INCLUDES
#include <mp/MpAndroidAudioBindingInterface.h>
#include <mp/MpAndroidX_XAudioTrack.h>
#include <mp/MpAndroidX_XAudioRecord.h>

// SYSTEM INCLUDES
#include <media/AudioSystem.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
extern "C" MpAndroidAudioBindingInterface* CREATE_ANDROID_AUDIO_BINDING_SYMBOL();

using namespace android;

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
*  @brief Audio interface for Android OS version specific differences
*
*  @nosubgrouping
*/
class MP_ANDROID_AUDIO_BINDING_INTERFACE : public MpAndroidAudioBindingInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend MpAndroidAudioBindingInterface* CREATE_ANDROID_AUDIO_BINDING_SYMBOL();

/* ============================ CREATORS ================================== */
///@name Creators
//@{

private:
      /// Disable direct invocation of constructor, use factory method
    MP_ANDROID_AUDIO_BINDING_INTERFACE();

public:
      /// Destructor
    virtual ~MP_ANDROID_AUDIO_BINDING_INTERFACE();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    MpAndroidAudioTrack* createAudioTrack(int streamType,
                                          uint32_t sampleRate,
                                          int format,
                                          int channels,
                                          int frameCount,
                                          uint32_t flags,
                                          sipXcallback_t cbf,
                                          void* user,
                                          int notificationFrames) const;

    MpAndroidAudioRecord* createAudioRecord() const;

    int getOutputLatency(uint32_t& outputLatency, StreamType streamType) const;
    
//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Copy constructor (not implemented for this class)
    MP_ANDROID_AUDIO_BINDING_INTERFACE(const MP_ANDROID_AUDIO_BINDING_INTERFACE& rMpAndroidAudioBindingInterface);

      /// Assignment operator (not implemented for this class)
    MP_ANDROID_AUDIO_BINDING_INTERFACE& operator=(const MP_ANDROID_AUDIO_BINDING_INTERFACE& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAndroidX_XAudioBinding_h_
