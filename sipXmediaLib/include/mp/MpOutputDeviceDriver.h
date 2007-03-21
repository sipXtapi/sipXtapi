//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpOutputDeviceDriver_h_
#define _MpOutputDeviceDriver_h_

// SYSTEM INCLUDES
//#include <utl/UtlDefs.h>
#include <os/OsStatus.h>
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
class MpOutputDeviceManager;

/**
*  @brief Container for device specific output driver.
*
*  The MpOutputDeviceDriver is the abstract base class for the implementations
*  of output media drivers.  An instance of MpOutputDeviceDriver is created for
*  every physical or logical input device (e.g. speaker).  A driver is
*  instantiated and then added to the MpOutputDeviceManager.  The driver must
*  be enabled via the MpOutputDeviceManager to begin consuming frames.
*
*  Each audio output driver could work in two modes: direct write mode and
*  non direct write (mixing) mode. In direct write mode data is pushed to device
*  as soon as it become available. In mixer mode data will be buffered and then
*  pulled by device itself. Direct write mode have less latency, but could be
*  fed by one source only. If two sources will try to push data, only one will
*  succeed. In opposite to direct write mode, mixer mode is supposed to accept
*  several sreams and mix them. In this mode device should pull data when needed,
*  as device manager do not have clocks and do not know the time to push next
*  frame.
*
*  In mixer mode device have simple circular buffer of samples associated with it.
*  This buffer is driven by respective AudioOutputConnetion, which is responsible
*  for mixing incoming data and handling of all exception situations. In this mode
*  device driver should provide timing for pulling data, so callback or thread
*  mode are appropriate. In direct write mode data passed to AudioOutputConnetion
*  is simply pushed to device, so device driver might work in any mode, but
*  asynchronous write mode looks like best choice.

*  MpOutputDeviceDriver has a text name which is defined upon construction.
*  This name will typically be the same as the OS defined name for the
*  input device.  The name of the MpOutputDeviceDriver is accessed via the
*  data() method inherited from UtlString.  This allows MpOutputDeviceDriver
*  to be contained and accessed by name.
*/
class MpOutputDeviceDriver : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

      /// Default constructor
    MpOutputDeviceDriver(const UtlString& name);
      /**<
      *  @param name - (in) unique device driver name (e.g. "/dev/dsp", 
      *         "YAMAHA AC-XG WDM Audio", etc.)
      */

      /// Destructor
    virtual
    ~MpOutputDeviceDriver();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

      /// Set device manager that is used to access to this device.
    virtual OsStatus setDeviceManager(MpOutputDeviceManager *deviceManager);
      /**<
      *  @param deviceManager - (in) device manager that is used to access
      *         to this device.
      */

      /// Initialize device driver and state
    virtual OsStatus enableDevice(unsigned samplesPerFrame, 
                                  unsigned samplesPerSec,
                                  MpFrameTime currentFrameTime,
                                  UtlBoolean enableDirectWriteMode) = 0;
      /**<
      *  This method enables the device driver.
      *
      *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
      *  enabling a device results in state and buffer queues being cleared.
      *
      *  @param samplesPerFrame - (in) the number of samples in a frame of media
      *  @param samplesPerSec - (in) sample rate for media frame in samples per second
      *  @param currentFrameTime - (in) time in milliseconds for beginning of frame
      *         relative to the MpOutputDeviceManager reference time
      *  @param enableDirectWriteMode - (in) pass TRUE to enable direct write
      *         mode. In this mode data is pushed to device as soon as it become
      *         available. In non direct (mixer) write mode data will be pulled
      *         by device itself.
      */

      /// Uninitialize device driver
    virtual OsStatus disableDevice() = 0;
      /**<
      *  This method disables the device driver and should release any
      *  platform device resources so that the device might be used else where.
      *
      *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
      *        enabling a device results in state and buffer queues being cleared.
      */

      /// Set device ID associated with this device in parent input device manager.
    virtual OsStatus setDeviceId(MpOutputDeviceHandle deviceId);

      /// Clear the device ID associated with this device.
    virtual OsStatus clearDeviceId();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

      /// Get device manager which is used to access to this device driver.
    inline MpOutputDeviceManager *getDeviceManager() const;

      /// Get device ID associated with this device in parent input device manager.
    inline MpOutputDeviceHandle getDeviceId() const;

      /// Calculate the number of milliseconds that a frame occupies in time.
    inline MpFrameTime getFramePeriod() const;

//@}

      /// Calculate the number of milliseconds that a frame occupies in time. 
    static inline MpFrameTime getFramePeriod(unsigned samplesPerFrame,
                                             unsigned samplesPerSec);

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

      /// Inquire if this driver is enabled
    virtual UtlBoolean isEnabled();

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    MpOutputDeviceManager* mpDeviceManager;  ///< The object that manages 
                    ///< this device driver.
    UtlBoolean mIsEnabled;         ///< Whether this device driver is enabled or not.
    MpOutputDeviceHandle mDeviceId; ///< The logical device ID that identifies 
                    ///< this device, as supplied by the InputDeviceManager.
    unsigned mSamplesPerFrame;     ///< Device produce audio frame with this
                    ///< number of samples.
    unsigned mSamplesPerSec;       ///< Device produce audio with this number
                    ///< of samples per second.
    unsigned mMixerBufferLength;   ///< Length of mixer buffer (in samples).
    MpAudioSample *mpMixerBuffer;  ///< Mixer circular buffer. Used to mix
                    ///< several media streams in non direct write mode.
    unsigned mMixerBufferBegin;    ///< Index of first available sample in mixer buffer.
    unsigned mMixerBufferEnd;      ///< Index of one sample behind last available
                    ///< media sample in mixer buffer.
    unsigned mMixerBufferSamples;  ///< Number of samples in mixer buffer.
    MpFrameTime mCurrentFrameTime; ///< TODO: Fill in mCurrentFrameTime description
    

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Copy constructor (not implemented for this class)
    MpOutputDeviceDriver(const MpOutputDeviceDriver& rMpOutputDeviceDriver);

      /// Assignment operator (not implemented for this class)
    MpOutputDeviceDriver& operator=(const MpOutputDeviceDriver& rhs);
};


/* ============================ INLINE METHODS ============================ */

MpOutputDeviceManager *MpOutputDeviceDriver::getDeviceManager() const
{
   return mpDeviceManager;
}

MpFrameTime MpOutputDeviceDriver::getFramePeriod() const
{
   return getFramePeriod(mSamplesPerFrame, mSamplesPerSec);
}

MpOutputDeviceHandle MpOutputDeviceDriver::getDeviceId() const
{
   return mDeviceId;
}

static MpFrameTime MpOutputDeviceDriver::getFramePeriod(unsigned samplesPerFrame,
                                                        unsigned samplesPerSec)
{
   return (1000*samplesPerFrame)/samplesPerSec;
}

#endif  // _MpOutputDeviceDriver_h_
