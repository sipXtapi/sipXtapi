//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MpInputDeviceDriverWnt_h_
#define _MpInputDeviceDriverWnt_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriver.h"

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
*/
class MpInputDeviceDriverWnt : public MpInputDeviceDriver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MpInputDeviceDriverWnt(const UtlString& name,
                          MpInputDeviceManager& deviceManager);
     /**<
     *  @param name - unique device driver name (e.g. "/dev/dsp", 
     *         "YAMAHA AC-XG WDM Audio", etc.)
     *  @param deviceManager - MpInputDeviceManager this device is to
     *         push frames to via pushFrame method
     */

     /// Destructor
   virtual
   ~MpInputDeviceDriverWnt();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

      /// Initialize device driver and state
    OsStatus enableDevice(unsigned samplesPerFrame, 
                          unsigned samplesPerSec,
                          unsigned currentFrameTime);
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
      *  @param deviceId - device Id used to identify this input device to 
      *         MpInputDeviceManager for pushFrame calls.
      */

      /// Uninitialize device driver
    OsStatus disableDevice();
      /**<
      *  This method disables the device driver and should release any
      *  platform device resources so that the device might be used else where.
      *
      *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
      *        enabling a device results in state and buffer queues being cleared.
      */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

      /// Inquire if the windows device is valid
    virtual UtlBoolean isDeviceValid() { return (mWntDeviceId > 0); };
//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
      /// The underlying windows Device ID (not the logical Mp device ID)
    int mWntDeviceId;


      /// Copy constructor (not implemented for this class)
    MpInputDeviceDriverWnt(const MpInputDeviceDriver& rMpInputDeviceDriver);

      /// Assignment operator (not implemented for this class)
    MpInputDeviceDriverWnt& operator=(const MpInputDeviceDriver& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpInputDeviceDriverWnt_h_
