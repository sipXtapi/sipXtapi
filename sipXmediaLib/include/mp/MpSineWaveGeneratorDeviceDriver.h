//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpSineWaveGeneratorDeviceDriver_h_
#define _MpSineWaveGeneratorDeviceDriver_h_

// SYSTEM INCLUDES
#include <utl/UtlDefs.h>
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include <mp/MpInputDeviceDriver.h>

// APPLICATION INCLUDES
#include "mp/MpTypes.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpInputDeviceManager;
class OsServerTask;

/**
*  @brief Sine wave generator Input Device Driver.
*
*  Implements a MpSineWaveGeneratorDeviceDriver that generates a sine wave
*  of the given magnitude and period (in samples).  This device is
*  primarily intended for testing the input device framework as it
*  produces a predictable signal that can be calculated and predetermined,
*  a useful characteristic for unit testing.
*/
class MpSineWaveGeneratorDeviceDriver : public MpInputDeviceDriver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

      /// Default constructor
    MpSineWaveGeneratorDeviceDriver(const UtlString& name,
                                    MpInputDeviceManager& deviceManager,
                                    unsigned int magnitude,
                                    unsigned int periodInMicroseconds,
                                    int relativeSpeed);
      /**<
      *  @param name - unique for this device driver 
      *  @param deviceManager - MpInputDeviceManager this device is to
      *         push frames to via pushFrame method
      *  @param magnitude - maximum value of sine wave 
      *  @param periodInMicroseconds - the length of a complete 360 degree
      *         sine wave cycle in micro-seconds.  T=zero as determined
      *         by the device manager.
      *  @param relativeSpeed - produce frames faster or slower than CPU clock
      *         by the given number of microseconds.  This is the amount of 
      *         time sooner (negative) or later (positive) that a frame of 
      *         samples is produced.  The mRelativeSpeed is effectively zero if 
      *         its magnitude is not greater than the OsTimer granularity or 
      *         accuracy.
      */

      /// Destructor
    virtual
    ~MpSineWaveGeneratorDeviceDriver();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
      /// Initialize device driver and state
    virtual OsStatus enableDevice(unsigned samplesPerFrame, 
                                  unsigned samplesPerSec,
                                  MpFrameTime currentFrameTime);
      /**<
      *  This method enables the device driver.
      *
      *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
      *  enabling a device results in state and buffer queues being cleared.
      *
      *  @param samplesPerFrame - the number of samples in a frame of media
      *  @param samplesPerSec - sample rate for media frame in samples per second
      *  @param currentFrameTime - time in milliseconds for beginning of frame
      *         relative to the MpInputDeviceManager reference time
      */

      /// Uninitialize device driver
    virtual OsStatus disableDevice();
      /**<
      *  This method disables the device driver and should release any
      *  platform device resources so that the device might be used else where.
      *
      *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
      *        enabling a device results in state and buffer queues being cleared.
      */

      /// Set a new tone to be played.
    virtual OsStatus setTone(unsigned int periodInMicroseconds);

     /// Set a new magnitude to be played.
    virtual OsStatus setMagnitude(unsigned int magnitude);
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{


      /// Calculate a sample value for the specified sample in a frame
    static MpAudioSample calculateSample(MpFrameTime frameStartTime,
                                         unsigned int magnitude,
                                         unsigned int periodInMicroseconds,
                                         unsigned int frameSampleIndex,
                                         unsigned int samplesPerFrame, 
                                         unsigned int samplesPerSecond);
      /**<
      *  @param frameStartTime - time in milliseconds for the first sample
      *         in the frame from the device manager reference time.
      *  @param magnitude - value of sine amplitude.
      *  @param periodInMicroseconds - sine period in micro-seconds.
      *  @param frameSampleIndex - index to the requested sample of the 
      *         given frame.
      *  @param samplesPerFrame - number of samples in a frame
      *  @param samplesPerSecond - sample rate in samples per second
      *
      *  @returns a sample value for the requested sample and frame
      */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

      /// @copydoc MpInputDeviceDriver::isDeviceValid()
    inline UtlBoolean isDeviceValid();


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Copy constructor (not implemented for this class)
    MpSineWaveGeneratorDeviceDriver(const MpSineWaveGeneratorDeviceDriver& rMpSineWaveGeneratorDeviceDriver);

      /// Assignment operator (not implemented for this class)
    MpSineWaveGeneratorDeviceDriver& operator=(const MpSineWaveGeneratorDeviceDriver& rhs);

    unsigned int mMagnitude;
    unsigned int mPeriodInMicroseconds;
    int mRelativeSpeed;
    OsServerTask* mpReaderTask;
};

/* ============================ INLINE METHODS ============================ */

UtlBoolean MpSineWaveGeneratorDeviceDriver::isDeviceValid()
{
   // This test driver is always valid.
   return TRUE;
}

#endif  // _MpSineWaveGeneratorDeviceDriver_h_
