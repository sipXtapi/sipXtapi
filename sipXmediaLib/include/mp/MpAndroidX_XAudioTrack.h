//  
// Copyright (C) 2010-2011 SIPez LLC. 
// Licensed under the LGPL license.
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
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_TRACK
#    endif
#else
#    error Unsupported version of Android AudioTrack
#endif

// SIPX INCLUDES
#include <mp/MpAndroidAudioTrack.h>

// SYSTEM INCLUDES
#include <media/AudioTrack.h>

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
      dummy1 = 11;
      dummy2 = 7;
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

    AudioTrack* mpAudioTrack;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAndroidX_XAudioTrack_h_
