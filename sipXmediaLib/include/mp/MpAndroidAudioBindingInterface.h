//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpAndroidAudioBindingInterface_h_
#define _MpAndroidAudioBindingInterface_h_

// SIPX INCLUDES
#include <os/OsStatus.h>

// SYSTEM INCLUDES

// DEFINES
#define CREATE_ANDROID_AUDIO_BINDING_SYMBOL createAndroidAudioBinding
#define CREATE_ANDROID_AUDIO_BINDING_SYMBOL_STRING "createAndroidAudioBinding"

// MACROS
// EXTERNAL FUNCTIONS

class MpAndroidAudioBindingInterface;

extern "C" MpAndroidAudioBindingInterface* stubAndroidAudioBindingInterfaceCreator();

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*sipXcallback_t)(int event, void* user, void *info);

// FORWARD DECLARATIONS
class UtlString;
class MpAndroidAudioTrack;
class MpAndroidAudioRecord;

/**
*  @brief Audio wrapper interface driver for Android OS version spesific differences.
*
*  @see MpInputDeviceDriver
*  @nosubgrouping
*/
class MpAndroidAudioBindingInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   // Look at AudioSystem.h in Android sources for description.
   enum StreamType
   {
      DEFAULT          =-1,
      VOICE_CALL       = 0,
      SYSTEM           = 1,
      RING             = 2,
      MUSIC            = 3,
      ALARM            = 4,
      NOTIFICATION     = 5,
      BLUETOOTH_SCO    = 6,
      ENFORCED_AUDIBLE = 7, // Sounds that cannot be muted by user and must be routed to speaker
      DTMF             = 8,
      TTS              = 9,
      NUM_STREAM_TYPES
   };

    typedef MpAndroidAudioBindingInterface* (*MpAndroidAudioBindingInterfaceCreator) ();

    friend MpAndroidAudioBindingInterface* stubGetAndroidAudioBindingInterface();

    static MpAndroidAudioBindingInterfaceCreator spGetAndroidAudioBinding;

    //static OsStatus setAudioTrackCreator();

/* ============================ CREATORS ================================== */
///@name Creators
//@{

      /// Destructor
    virtual ~MpAndroidAudioBindingInterface();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Get the status of the construction and loading of the Android OS version specific libs
    OsStatus getStatus() const;

    static OsStatus getVersionSpecificLibraryName(UtlString& sharedLibraryName);

    /// Abstract/stub method to get Android OS version specific implementation of AudioTrack
    virtual MpAndroidAudioTrack* createAudioTrack(int streamType, 
                                                  uint32_t sampleRate,
                                                  int format,
                                                  int channels,
                                                  int frameCount,
                                                  uint32_t flags,
                                                  sipXcallback_t cbf,
                                                  void* user,
                                                  int notificationFrames) const;

    /// Abstract/stub method to get Android OS version specific implementation of AudioRecord
    virtual MpAndroidAudioRecord* createAudioRecord() const;

    /// Abstract/stub method to get Android OS version specific implementation to get output latency
    virtual int getOutputLatency(uint32_t& outputLatency, StreamType streamType) const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static MpAndroidAudioBindingInterface* spMpAndroidAudioBindingInterface;

    /// Disallow default constructor
    MpAndroidAudioBindingInterface();

    OsStatus mStatus;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Copy constructor (not implemented for this class)
    MpAndroidAudioBindingInterface(const MpAndroidAudioBindingInterface& rMpAndroidAudioBindingInterface);

      /// Assignment operator (not implemented for this class)
    MpAndroidAudioBindingInterface& operator=(const MpAndroidAudioBindingInterface& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAndroidAudioBindingInterface_h_
