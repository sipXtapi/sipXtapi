//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpodAndroid_h_
#define _MpodAndroid_h_

// SIPX INCLUDES
#include <os/OsIntTypes.h>
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include "mp/MpTypes.h"
#include "mp/MpOutputDeviceDriver.h"

// SYSTEM INCLUDES
#include <utils/threads.h>
#include <mp/MpAndroidAudioTrack.h>

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsNotification;
using namespace android;

/**
*  @brief Audio output driver for Android OS.
*
*  @see MpOutputDeviceDriver
*  @nosubgrouping
*/
class MpodAndroid : public MpOutputDeviceDriver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor.
   explicit
   MpodAndroid(MpAndroidAudioBindingInterface::StreamType streamType);
     /**<
     *  @note Device name is not supported under Android for now.
     */

     /// Destructor.
   virtual
   ~MpodAndroid();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MpOutputDeviceDriver::enableDevice(unsigned, unsigned, MpFrameTime)
   virtual OsStatus enableDevice(unsigned samplesPerFrame, 
                                 unsigned samplesPerSec,
                                 MpFrameTime currentFrameTime,
                                 OsCallback &frameTicker);

     /// @copydoc MpOutputDeviceDriver::disableDevice()
   virtual OsStatus disableDevice();

     /// @copydoc MpOutputDeviceDriver::pushFrame(unsigned int, const MpAudioSample*, MpFrameTime)
   virtual OsStatus pushFrame(unsigned int numSamples,
                              const MpAudioSample* samples,
                              MpFrameTime frameTime);

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

   enum State {
      DRIVER_IDLE,     ///< MpodAndroid is being initialized or initialization failed
      DRIVER_INIT,     ///< MpodAndroid has been successfully initialized and is not playing
      DRIVER_STARTING, ///< MpodAndroid is starting playing
      DRIVER_PLAYING,  ///< MpodAndroid is playing
      DRIVER_STOPPING, ///< MpodAndroid is stopping
      DRIVER_STOPPED   ///< MpodAndroid is stopped: the AudioTrack will be stopped
   };

   MpAndroidAudioBindingInterface::StreamType mStreamType;   ///< Android type of the output stream
   OsAtomicLightUInt mState; ///< Internal class state
   MpAndroidAudioTrack *mpAudioTrack; ///< Pointer to audio track used for playback
   Mutex mLock;              ///< Mutex to control concurrent access to this object
                             ///<  from audio callback and application API
   Condition mWaitCbkCond;   ///< condition enabling interface to wait for audio callback
                             ///<  completion after a change is requested
   MpFrameTime mCurFrameTime;  ///< The current frame time for this device.
   OsNotification* mpNotifier; ///< Event signaled when windows is ready to receive a new buffer.

   ///@name Variables to push audio data to callback.
   //@{
   MpAudioSample *mpSampleBuffer; ///< Buffer which pushBuffer should copy to.
   int mSampleBufferIndex;     ///< Index of the first sample not yet pushed to device.
   //@}

     /// Allocates and configures AudioTrack used for PCM output.
   UtlBoolean initAudioTrack();

     /// Callback function called by Android to request more data.
   static void audioCallback(int event, void* user, void *info);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MpodAndroid(const MpodAndroid& rMpodAndroid);

     /// Assignment operator (not implemented for this class)
   MpodAndroid& operator=(const MpodAndroid& rhs);
};


/* ============================ INLINE METHODS ============================ */

#endif  // _MpodAndroid_h_
