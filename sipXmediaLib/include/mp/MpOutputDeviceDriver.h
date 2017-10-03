//  
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpOutputDeviceDriver_h_
#define _MpOutputDeviceDriver_h_

// SYSTEM INCLUDES
//#include <utl/UtlDefs.h>
#include <os/OsStatus.h>
#include <os/OsAtomics.h>
#include <utl/UtlString.h>

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
class OsCallback;

/**
*  @brief Container for device specific output driver.
*
*  The MpOutputDeviceDriver is the abstract base class for the implementations
*  of output media drivers.  An instance of MpOutputDeviceDriver is created for
*  every physical or logical input device (e.g. speaker).  A driver is
*  instantiated and then added to the MpOutputDeviceManager.  The driver must
*  be enabled via the MpOutputDeviceManager to begin consuming frames.
*
*  Each audio output driver should notify its MpAudioOutputConnection when
*  the device is ready to accept the next frame of data with provided
*  OsNotification. See enableDevice() for more information.
*
*  MpOutputDeviceDriver has a text name which is defined upon construction.
*  This name will typically be the same as the OS defined name for the
*  input device.  The name of the MpOutputDeviceDriver is accessed via the
*  data() method inherited from UtlString.  This allows MpOutputDeviceDriver
*  to be contained and accessed by name.
*
*  @nosubgrouping
*/
class MpOutputDeviceDriver : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor.
   explicit
   MpOutputDeviceDriver(const UtlString& name);
     /**<
     *  @param name - (in) unique device driver name (e.g. "/dev/dsp", 
     *         "YAMAHA AC-XG WDM Audio", etc.)
     */

     /// Destructor.
   virtual
   ~MpOutputDeviceDriver();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Initialize device driver and state.
   virtual OsStatus enableDevice(unsigned samplesPerFrame, 
                                 unsigned samplesPerSec,
                                 MpFrameTime currentFrameTime,
                                 OsCallback &frameTicker) = 0;
     /**<
     *  This method enables the device driver.
     *
     *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
     *  enabling a device results in state and buffer queues being cleared.
     *
     *  @param[in] samplesPerFrame - the number of samples in a frame of media
     *  @param[in] samplesPerSec - sample rate for media frame in samples per second
     *  @param[in] currentFrameTime - time in milliseconds for beginning of frame
     *         relative to the MpOutputDeviceManager reference time
     *  @param[in] frameTicker - notification to signal when device become ready. 
     *         Device driver MUST signal this notification as soon as it become
     *         ready to receive next portion of data to play back. Note, it is
     *         a callback which in turn calls pushFrame() method of this device
     *         driver. Also notification may be used to signal begin of  frame
     *         interval for one or several flowgraphs, so it should be as uniform
     *         as possible, i.e. it should not burst or hold over, driver should
     *         signal this notification after equal intervals of time.
     *
     *  @returns OS_INVALID_STATE if device already enabled.
     *
     *  @NOTE This method is supposed to be used from MpAudioOutputConnection only.
     *        If you want enable device, use MpOutputDeviceManager or
     *        MpAudioOutputConnection methods.
     */

     /// Uninitialize device driver.
   virtual OsStatus disableDevice() = 0;
     /**<
     *  This method disables the device driver and should release any
     *  platform device resources so that the device might be used else where.
     *
     *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
     *        enabling a device results in state and buffer queues being cleared.
     *
     *  @NOTE This method is supposed to be used from MpAudioOutputConnection only.
     *        If you want disable device, use MpOutputDeviceManager or
     *        MpAudioOutputConnection methods.
     */

     /// @brief Send data to output device.
   virtual
   OsStatus pushFrame(unsigned int numSamples,
                      const MpAudioSample* samples,
                      MpFrameTime frameTime) = 0;
     /**<
     *  This method is called from a callback which is called when we fire
     *  ticker notification. See enableDevice() for details.
     *
     *  @param[in] numSamples - Number of samples in \p samples array.
     *  @param[in] samples - Array of samples to push to device.
     *  @param[in] frameTime - Time of pushed frame. Device may consider
     *         does it want it or not internally. If frame come too late
     *         driver should return OS_SUCCESS and throw out it silently.
     *         Common problem is when MediaTask queue is jammed - no frames
     *         are processed for some time and then they are processed bursty.
     *         Using given frameTime driver may keep its internal queue short.
     *         It is guaranteed that frame time would increase monotonically.
     *
     *  @returns OS_LIMIT_REACHED if mixer buffer is full, i.e. frame come too
     *           early.
     *  @returns OS_FAILED for other errors.
     *  @returns OS_SUCCESS if frame sent correctly.
     *  @see enableDevice() for documentation when this is being called.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Calculate the number of milliseconds that a frame occupies in time.
   inline
   MpFrameTime getFramePeriod() const;

     /// Get number of samples in a frame.
   inline
   unsigned getSamplesPerFrame() const;

     /// Get number of samples per second.
   inline
   unsigned getSamplesPerSec() const;

     /// Calculate the number of milliseconds that a frame occupies in time. 
   static inline
   MpFrameTime getFramePeriod(unsigned samplesPerFrame,
                              unsigned samplesPerSec);

      /// Get device name.
    inline
    const UtlString &getDeviceName() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{
   /// Check if this device can connect
   virtual OsStatus canEnable() {return(OS_SUCCESS);};
   /**
     @returns -
         OS_SUCCESS if the device can be enabled
         OS_NOT_FOUND if the named device does not exist
         OS_NOT_OWNER if the user does not have permission to connect
         OS_BUSY if the device is already in use and cannot be shared
    */

     /// Inquire if this driver is enabled
   virtual UtlBoolean isEnabled() const;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsAtomicLightBool mIsEnabled;         ///< Whether this device driver is enabled or not.
   OsAtomicLightUInt mSamplesPerFrame;   ///< Device produce audio frame with this
                   ///< number of samples.
   OsAtomicLightUInt mSamplesPerSec;     ///< Device produce audio with this number
                   ///< of samples per second.
   OsCallback       *mpTickerNotification; ///< Callback to be called when device
                   ///< is ready to accept more data. See enableDevice() for details.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MpOutputDeviceDriver(const MpOutputDeviceDriver& rMpOutputDeviceDriver);

     /// Assignment operator (not implemented for this class)
   MpOutputDeviceDriver& operator=(const MpOutputDeviceDriver& rhs);
};


/* ============================ INLINE METHODS ============================ */

MpFrameTime MpOutputDeviceDriver::getFramePeriod() const
{
   return getFramePeriod(mSamplesPerFrame, mSamplesPerSec);
}

MpFrameTime MpOutputDeviceDriver::getFramePeriod(unsigned samplesPerFrame,
                                                 unsigned samplesPerSec)
{
    assert(samplesPerSec);
    return(samplesPerSec ? ((1000*samplesPerFrame)/samplesPerSec) : 0);
}

unsigned MpOutputDeviceDriver::getSamplesPerFrame() const
{
   return mSamplesPerFrame;
}

unsigned MpOutputDeviceDriver::getSamplesPerSec() const
{
   return mSamplesPerSec;
}

const UtlString &MpOutputDeviceDriver::getDeviceName() const
{
   return *this;
}

#endif  // _MpOutputDeviceDriver_h_
