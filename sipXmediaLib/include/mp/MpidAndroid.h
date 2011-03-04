//  
// Copyright (C) 2010 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2010 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpidAndroid_h_
#define _MpidAndroid_h_

// SIPX INCLUDES
#include <mp/MpInputDeviceDriver.h>
#include <mp/MpAndroidAudioRecord.h>

// SYSTEM INCLUDES
#include <utils/threads.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpResamplerBase;
using namespace android;

/**
*  @brief Audio input driver for Android OS.
*
*  @see MpInputDeviceDriver
*  @nosubgrouping
*/
class MpidAndroid : public MpInputDeviceDriver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

   /// Audio input source. Refer to Android OS sources.
   enum audio_source {
      AUDIO_SOURCE_DEFAULT = 0,
      AUDIO_SOURCE_MIC = 1,
      AUDIO_SOURCE_VOICE_UPLINK = 2,      // Don't work on Droid/Milestone
      AUDIO_SOURCE_VOICE_DOWNLINK = 3,    // Don't work on Droid/Milestone
      AUDIO_SOURCE_VOICE_CALL = 4,        // Don't work on Droid/Milestone
      AUDIO_SOURCE_CAMCORDER = 5,         // Seems equal to MIC on Droid/Milestone
      AUDIO_SOURCE_VOICE_RECOGNITION = 6, // Seems equal to MIC on Droid/Milestone
      AUDIO_SOURCE_MAX = AUDIO_SOURCE_VOICE_RECOGNITION,

      AUDIO_SOURCE_LIST_END  // must be last - used to validate audio source type
   };

      /// Default constructor
    MpidAndroid(audio_source source,
                MpInputDeviceManager& deviceManager);
      /**<
      *  @param[in] source - what audio source to record.
      *  @param[in] deviceManager - MpInputDeviceManager this device is to
      *         push frames to via pushFrame method
      */

      /// Destructor
    virtual
    ~MpidAndroid();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

      /// @copydoc MpInputDeviceDriver::enableDevice()
    virtual OsStatus enableDevice(unsigned samplesPerFrame, 
                                  unsigned samplesPerSec,
                                  MpFrameTime currentFrameTime=0);

      /// @copydoc MpInputDeviceDriver::disableDevice()
    virtual OsStatus disableDevice();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

      /// @copydoc MpInputDeviceDriver::isDeviceValid()
    virtual UtlBoolean isDeviceValid();

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   enum DriverState {
      DRIVER_IDLE,      ///< Driver is being initialized or initialization failed
      DRIVER_INIT,      ///< Driver has been successfully initialized and is not playing
      DRIVER_STARTING,  ///< Driver is starting playing
      DRIVER_RECORDING, ///< Driver is playing
      DRIVER_STOPPING,  ///< Driver is stopping
      DRIVER_STOPPED,   ///< Driver is stopped: the AudioRecord will be stopped
   };

   audio_source mStreamSource; ///< Source of a stream we're recording.
   DriverState mState;         ///< Driver internal state

   MpAndroidAudioRecord *mpAudioRecord; ///< Pointer to audio track used for record
   int mSamplesPerSecInternal; ///< Sample rate of AudioRecord.
   int mSamplesPerFrameInternal; ///< Number of sample per frame we use with AudioRecord.
   MpAudioSample *mpBufInternal; ///< Temporary audio buffer to save data if don't receive
                               ///<  full buffer. Have mSamplesPerFrameInternal size.
   int mBufInternalSamples;    ///< Number of samples currently stored in mpBufInternal.
   Mutex mLock;                ///< Mutex to control concurrent access to driver object
                               ///<  from audio callback and application API
   Condition mWaitCbkCond;     ///< condition enabling interface to wait for audio
                               ///<  callback completion after a change is requested
   MpResamplerBase *mpResampler; ///< Resampler used to resample from hardware samplerate
                               ///<  to output samplerate.
   MpAudioSample *mpResampleBuf; ///< Buffer of size mSamplesPerFrameInternal used by
                               ///<  resampler.

   static const int mpSampleRatesList[]; ///< List of sampling rates to probe during
                               ///<  AudioRecord initialization.
   static const int mSampleRatesListLen; ///< Number of entries in mpSampleRatesList

     /// Prepare AudioRecord stream for recording
   bool initAudioRecord();

     /// Call initAudioRecord() with frequencies listed until we find working one.
   bool probeSampleRate(int targetRate, int targetFrameSize,
                        const int *ratesList, int ratesListLen,
                        bool probeDefault=true);

     /// Callback function to be called by OS
   static void audioCallback(int event, void* user, void *info);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Copy constructor (not implemented for this class)
    MpidAndroid(const MpidAndroid& rMpidAndroid);

      /// Assignment operator (not implemented for this class)
    MpidAndroid& operator=(const MpidAndroid& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpidAndroid_h_
