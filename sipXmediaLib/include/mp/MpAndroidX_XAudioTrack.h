//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpAndroidX_XAudioTrack_h_
#define _MpAndroidX_XAudioTrack_h_

// Must be included before Android utils/log.h so we define LOG_TAG first

#ifdef ANDROID_2_0
#    define MP_ANDROID_AUDIO_TRACK MpAndroid2_0AudioTrack
#    define QUOTED_MP_ANDROID_AUDIO_TRACK "MpAndroid2_0AudioTrack"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_TRACK
#    endif
#elif ANDROID_2_3
#    define MP_ANDROID_AUDIO_TRACK MpAndroid2_3AudioTrack
#    define QUOTED_MP_ANDROID_AUDIO_TRACK "MpAndroid2_3AudioTrack"
#    define CREATE_TRACK_METHOD createTrack
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_TRACK
#    endif
     extern "C" int setpriority(int, int, int);
#    define PRIO_PROCESS 0
#elif ANDROID_2_3_4
#    define MP_ANDROID_AUDIO_TRACK MpAndroid2_3_4AudioTrack
#    define QUOTED_MP_ANDROID_AUDIO_TRACK "MpAndroid2_3_4AudioTrack"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_TRACK
#    endif
#    define CREATE_TRACK_METHOD createTrack_l
     extern "C" int setpriority(int, int, int);
#    define PRIO_PROCESS 0
#elif ANDROID_4_0_1
#    define MP_ANDROID_AUDIO_TRACK MpAndroid4_0_1AudioTrack
#    define QUOTED_MP_ANDROID_AUDIO_TRACK "MpAndroid4_0_1AudioTrack"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_TRACK
#    endif
#    define CREATE_TRACK_METHOD createTrack_l
     extern "C" int setpriority(int, int, int);
#    define PRIO_PROCESS 0
#elif ANDROID_4_1_1
#    define MP_ANDROID_AUDIO_TRACK MpAndroid4_1_1AudioTrack
#    define QUOTED_MP_ANDROID_AUDIO_TRACK "MpAndroid4_1_1AudioTrack"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_TRACK
#    endif
#    define CREATE_TRACK_METHOD createTrack_l
     extern "C" int setpriority(int, int, int);
#    define PRIO_PROCESS 0
#    define LOGV ALOGV
#    define LOGD ALOGD
#elif ANDROID_4_2_1
#    define MP_ANDROID_AUDIO_TRACK MpAndroid4_2_1AudioTrack
#    define QUOTED_MP_ANDROID_AUDIO_TRACK "MpAndroid4_2_1AudioTrack"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_TRACK
#    endif
#    define CREATE_TRACK_METHOD createTrack_l
     extern "C" int setpriority(int, int, int);
#    define PRIO_PROCESS 0
#    define LOGV ALOGV
#    define LOGD ALOGD
#else
#    error Unsupported version of Android AudioTrack
#endif

// SIPX INCLUDES
#include <mp/MpAndroidX_XAudioBinding.h>
#include <mp/MpAndroidAudioTrack.h>

// SYSTEM INCLUDES
#include <media/AudioTrack.h>
#include <private/media/AudioTrackShared.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
extern "C" MpAndroidAudioTrack* createAndroidAudioTrack(int streamType,
                                          uint32_t sampleRate,
                                          int format,
                                          int channels,
                                          int frameCount,
                                          uint32_t flags,
                                          sipXcallback_t cbf,
                                          void* user,
                                          int notificationFrames);

using namespace android;

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// This wraper class is used to create some extra memory padding
// at the end of the AudioTrack class.  on Droid X an assert was
// firing because memory was getting trashed in the footer of the
// malloc memory chunk for the AudioTrack.  It is not clear why
// this was happening.  However when we got lucky and used a chunk
// with an extra 8 bytes at the end, everything was happy.  Hense
// this wrapper class with 2 int of bufer space at the end.
class SipxAudioTrack : public AudioTrack
{
public:
   SipxAudioTrack(int streamType,
                  uint32_t sampleRate,
                  int format,
                  int channels,
                  int frameCount,
                  uint32_t flags,
                  callback_t cbf,
                  void* user,
                  int notificationFrames) :
   AudioTrack(streamType, sampleRate, format, channels, frameCount, flags, cbf, user, notificationFrames)
   {
      //dummy1 = 11;
      //dummy2 = 7;
   };

   void start()
   {
       LOGD("SipxAudioTrack::start() entered");
#if defined(ANDROID_2_3) || defined(ANDROID_2_3_4)
    // Code from Android 2.3 AudioTrack::start() added here as it hangs on Android 3.0
    // BEGIN ANDROID CODE
    //////////////////////////////////////////////////////////////////////////////////
/* //device/extlibs/pv/android/AudioTrack.cpp
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
    LOGD("%s line: %d", __FILE__, __LINE__);
    LOGD("mAudioTrackThread: %p", &mAudioTrackThread);
    LOGD("exit pending: %s", mAudioTrackThread->exitPending() ? "TRUE" : "FALSE");
    LOGD("ref count: %d", mAudioTrackThread->getStrongCount());
 
    LOGD("%s line: %d", __FILE__, __LINE__);
    sp<AudioTrackThread> t = mAudioTrackThread;
    LOGD("%s line: %d", __FILE__, __LINE__);
    status_t status;

    LOGV("start %p", this);
    if (t != 0) {
        if (t->exitPending()) {
            LOGD("SipxAudioTrack::start() exitPending");

            if (t->requestExitAndWait() == WOULD_BLOCK) {
                LOGE("AudioTrack::start called from thread");
                return;
            }
        }
        else
        {
            LOGD("SipxAudioTrack::start() NOT exitPending");
        }
        t->mLock.lock();
     }
     LOGD("SipxAudioTrack::start() after t->mLock.lock");

    if (android_atomic_or(1, &mActive) == 0) {
        mNewPosition = mCblk->server + mUpdatePeriod;
        mCblk->bufferTimeoutMs = MAX_STARTUP_TIMEOUT_MS;
        mCblk->waitTimeMs = 0;
        mCblk->flags &= ~CBLK_DISABLED_ON;
        if (t != 0) {
           LOGD("SipxAudioTrack::start() about to t->run");
           t->run("AudioTrackThread", THREAD_PRIORITY_AUDIO_CLIENT);
        } else {
           LOGD("SipxAudioTrack::start() about to setpriority");
            setpriority(PRIO_PROCESS, 0, THREAD_PRIORITY_AUDIO_CLIENT);
        }
        LOGD("SipxAudioTrack::start() after set priority");

        if (mCblk->flags & CBLK_INVALID_MSK) {
            LOGW("start() track %p invalidated, creating a new one", this);
            // no need to clear the invalid flag as this cblk will not be used anymore
            // force new track creation
            status = DEAD_OBJECT;
        } else {
            status = mAudioTrack->start();
        }
        LOGD("SipxAudioTrack::start() after mAudioTrack->start()");
        if (status == DEAD_OBJECT) {
            LOGV("start() dead IAudioTrack: creating a new one");
            status = CREATE_TRACK_METHOD(mStreamType, mCblk->sampleRate, mFormat, mChannelCount,
                                 mFrameCount, mFlags, mSharedBuffer, getOutput(), false);
            if (status == NO_ERROR) {
                status = mAudioTrack->start();
                if (status == NO_ERROR) {
                    mNewPosition = mCblk->server + mUpdatePeriod;
                }
            }
        }
        if (status != NO_ERROR) {
            LOGV("start() failed");
            android_atomic_and(~1, &mActive);
            if (t != 0) {
                t->requestExit();
            } else {
                setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_NORMAL);
            }
        }
    }

    LOGD("SipxAudioTrack::start() end unlock");
    if (t != 0) {
        t->mLock.unlock();
    }
    //////////////////////////////////////////////////////////////////////////
    // END ANDROID CODE
    LOGD("SipxAudioTrack::start() exit");
#else
       AudioTrack::start();
    LOGD("SipxAudioTrack::start() exited");
#endif
   };

private:
   SipxAudioTrack(const SipxAudioTrack&); // no copy
   SipxAudioTrack& operator=(const SipxAudioTrack&); // no copy

   int dummy1;  // Padding to prevent overwrite on Droid X
   int dummy2;
};



/**
*  @brief Audio input driver for Android OS.
*
*  @see MpInputDeviceDriver
*  @nosubgrouping
*/
class MP_ANDROID_AUDIO_TRACK : public MpAndroidAudioTrack
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend MpAndroidAudioTrack* createAndroidAudioTrack(int streamType,
                                          uint32_t sampleRate,
                                          int format,
                                          int channels,
                                          int frameCount,
                                          uint32_t flags,
                                          sipXcallback_t cbf,
                                          void* user,
                                          int notificationFrames);

    friend class MP_ANDROID_AUDIO_BINDING_INTERFACE;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

private:
      /// Disable direct invocation of constructor, use factory method
    MP_ANDROID_AUDIO_TRACK(int streamType,
                  uint32_t sampleRate0,
                  int format,
                  int channels,
                  int frameCount,
                  uint32_t flags,
                  sipXcallback_t cbf,
                  void* user,
                  int notificationFrames);

public:
      /// Destructor
    virtual ~MP_ANDROID_AUDIO_TRACK();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    virtual void start();

    virtual void stop();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    virtual int /*status_t*/ initCheck() const;

    virtual uint32_t getSampleRate();

    virtual int frameSize() const;

    virtual uint32_t frameCount() const;

    virtual uint32_t latency() const;

    virtual void setVolume(float left, float right);

    virtual void dumpAudioTrack(const char* label);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Copy constructor (not implemented for this class)
    MP_ANDROID_AUDIO_TRACK(const MP_ANDROID_AUDIO_TRACK& rMpAndroidAudioTrack);

      /// Assignment operator (not implemented for this class)
    MP_ANDROID_AUDIO_TRACK& operator=(const MP_ANDROID_AUDIO_TRACK& rhs);

    SipxAudioTrack* mpAudioTrack;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAndroidX_XAudioTrack_h_
