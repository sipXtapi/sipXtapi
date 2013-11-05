//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpAndroidX_XAudioRecord_h_
#define _MpAndroidX_XAudioRecord_h_

// Must be included before Android utils/log.h so we define LOG_TAG first

#ifdef ANDROID_2_0
#    define MP_ANDROID_AUDIO_RECORD MpAndroid2_0AudioRecord
#    define QUOTED_MP_ANDROID_AUDIO_RECORD "MpAndroid2_0AudioRecord"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_RECORD
#    endif
#elif ANDROID_2_3
#    define MP_ANDROID_AUDIO_RECORD MpAndroid2_3AudioRecord
#    define QUOTED_MP_ANDROID_AUDIO_RECORD "MpAndroid2_3AudioRecord"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_RECORD
#    endif
#elif ANDROID_2_3_4
#    define MP_ANDROID_AUDIO_RECORD MpAndroid2_3_4AudioRecord
#    define QUOTED_MP_ANDROID_AUDIO_RECORD "MpAndroid2_3_4AudioRecord"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_RECORD
#    endif
#elif ANDROID_4_0_1
#    define MP_ANDROID_AUDIO_RECORD MpAndroid4_0_1AudioRecord
#    define QUOTED_MP_ANDROID_AUDIO_RECORD "MpAndroid4_0_1AudioRecord"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_RECORD
#    endif
#elif ANDROID_4_1_1
#    define MP_ANDROID_AUDIO_RECORD MpAndroid4_1_1AudioRecord
#    define QUOTED_MP_ANDROID_AUDIO_RECORD "MpAndroid4_1_1AudioRecord"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_RECORD
#    endif
#elif ANDROID_4_2_1
#    define MP_ANDROID_AUDIO_RECORD MpAndroid4_2_1AudioRecord
#    define QUOTED_MP_ANDROID_AUDIO_RECORD "MpAndroid4_2_1AudioRecord"
#    ifndef LOG_TAG
#        define LOG_TAG QUOTED_MP_ANDROID_AUDIO_RECORD
#    endif
#else
#    error Unsupported version of Android AudioRecord
#endif

// SIPX INCLUDES
#include <mp/MpAndroidAudioRecord.h>

// SYSTEM INCLUDES
#include <media/AudioRecord.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
extern "C" MpAndroidAudioRecord* createAndroidAudioRecord();

using namespace android;

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
*  @brief Audio input driver for Android OS.
*
*  @see MpInputDeviceDriver
*  @nosubgrouping
*/
class MP_ANDROID_AUDIO_RECORD : public MpAndroidAudioRecord
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend MpAndroidAudioRecord* createAndroidAudioRecord();

    friend class MP_ANDROID_AUDIO_BINDING_INTERFACE;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

private:
      /// Disable direct invocation of constructor, use factory method
    MP_ANDROID_AUDIO_RECORD();

public:
      /// Destructor
    virtual ~MP_ANDROID_AUDIO_RECORD();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators

    virtual int /*status_t*/ start();

    virtual void stop();

    virtual int /*status_t*/ set(int inputSource,
                                 int sampleRate,
                                 sipXcallback_t audioCallback,
                                 void* user,
                                 int notificationFrames);
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    AudioRecord* mpAudioRecord;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Copy constructor (not implemented for this class)
    MP_ANDROID_AUDIO_RECORD(const MP_ANDROID_AUDIO_RECORD& rMpAndroidAudioRecord);

      /// Assignment operator (not implemented for this class)
    MP_ANDROID_AUDIO_RECORD& operator=(const MP_ANDROID_AUDIO_RECORD& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAndroidX_XAudioRecord_h_
