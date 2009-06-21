//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpInputDeviceDriver_h_
#define _MpInputDeviceDriver_h_

// SYSTEM INCLUDES
#include <utl/UtlDefs.h>
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
class MpInputDeviceManager;

/**
*  @brief Container for device specific input driver.
*
*  The MpInputDeviceDriver is the abstract container for the implementation
*  of input media drivers.  An instance of MpInputDeviceDriver is created for
*  every physical and logical input device (e.g. microphone).  A driver is
*  instantiated and then added to the MpInputDeviceManager.  The driver must
*  be enabled via the MpInputDeviceManager before it yields input frames.
*
*  The MpInputDeviceDriver obtains frames of audio from hardware and  
*  provides them to the MpInputDeviceManager which makes the frames available to 
*  MprFromInputDevice resources in flowgraphs.  The MpInputDeviceDriver may be 
*  an OsTask which runs and gets frames for the OS and hardware specific device
*  or MpInputDeviceDriver may be driven via callbacks from the OS/hardware
*  input device driver.  This is an implementation choice.
*
*  MpInputDeviceDriver has a text name which is defined upon construction.
*  This name will typically be the same as the OS defined name for the
*  input device.  The name of the MpInputDeviceDriver is accessed via the
*  data() method inherited from UtlString.  This allows MpInputDeviceDriver
*  to be contained and accessed by name.
*
*  @nosubgrouping
*/
class MpInputDeviceDriver : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

      /// Default constructor
    MpInputDeviceDriver(const UtlString& name,
                        MpInputDeviceManager& deviceManager);
      /**<
      *  @param name - unique device driver name (e.g. "/dev/dsp", 
      *         "YAMAHA AC-XG WDM Audio", etc.)
      *  @param deviceManager - MpInputDeviceManager this device is to
      *         push frames to via pushFrame method
      */

      /// Destructor
    virtual
    ~MpInputDeviceDriver();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
      /// Initialize device driver and state
    virtual OsStatus enableDevice(unsigned samplesPerFrame, 
                                  unsigned samplesPerSec,
                                  MpFrameTime currentFrameTime=0
                                  ) = 0;
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
    virtual OsStatus disableDevice() = 0;
      /**<
      *  This method disables the device driver and should release any
      *  platform device resources so that the device might be used else where.
      *
      *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
      *        enabling a device results in state and buffer queues being cleared.
      */

      /// Set device ID associated with this device in parent input device manager.
    virtual OsStatus setDeviceId(MpInputDeviceHandle deviceId);

      /// Clear the device ID associated with this device.
    virtual OsStatus clearDeviceId();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

      /// Get device ID associated with this device in parent input device manager.
    virtual MpInputDeviceHandle getDeviceId() const;

      /// Get the sample rate of this device driver, if enabled.
    inline uint32_t getSamplesPerSec() const;

      /// Get the samples per frame of this device driver, if enabled.
    inline uint32_t getSamplesPerFrame() const;

      /// Calculate the number of milliseconds that a frame occupies in time.
    inline MpFrameTime getFramePeriod();

      /// Calculate the number of milliseconds that a frame occupies in time. 
    static inline MpFrameTime getFramePeriod(unsigned samplesPerFrame,
                                             unsigned samplesPerSec);

      /// Get timestamp for the next frame to be pushed to input device manager.
    inline MpFrameTime getCurrentFrameTime() const;

      /// Get device name.
    inline const UtlString &getDeviceName() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

      /// Inquire if this driver is enabled
    virtual UtlBoolean isEnabled();

      /// @brief Inquire if the device is valid
    virtual UtlBoolean isDeviceValid() = 0;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    MpInputDeviceManager* mpInputDeviceManager;  ///< The object that manages 
                    ///< this device driver.
    UtlBoolean mIsEnabled;         ///< Whether this device driver is enabled or not.
    MpInputDeviceHandle mDeviceId; ///< The logical device ID that identifies 
                    ///< this device, as supplied by the InputDeviceManager.
    unsigned mSamplesPerFrame;     ///< Device produce audio frame with this
                    ///< number of samples.
    unsigned mSamplesPerSec;       ///< Device produce audio with this number
                    ///< of samples per second.
    MpFrameTime mCurrentFrameTime; ///< Timestamp for the next frame to be
                    ///< pushed to input device manager.
    

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Copy constructor (not implemented for this class)
    MpInputDeviceDriver(const MpInputDeviceDriver& rMpInputDeviceDriver);

      /// Assignment operator (not implemented for this class)
    MpInputDeviceDriver& operator=(const MpInputDeviceDriver& rhs);
};

/* ============================ INLINE METHODS ============================ */

uint32_t MpInputDeviceDriver::getSamplesPerSec() const
{
   return mSamplesPerSec;
}

uint32_t MpInputDeviceDriver::getSamplesPerFrame() const
{
   return mSamplesPerFrame;
}

MpFrameTime MpInputDeviceDriver::getFramePeriod()
{
   return getFramePeriod(mSamplesPerFrame, mSamplesPerSec);
}

MpFrameTime MpInputDeviceDriver::getFramePeriod(unsigned int samplesPerFrame,
                                                unsigned int samplesPerSec)
{
   return (1000*samplesPerFrame)/samplesPerSec;
}

MpFrameTime MpInputDeviceDriver::getCurrentFrameTime() const
{
   return mCurrentFrameTime;
}

const UtlString &MpInputDeviceDriver::getDeviceName() const
{
   return *this;
}

#endif  // _MpInputDeviceDriver_h_
