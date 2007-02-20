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

#ifndef _MpInputDeviceManager_h_
#define _MpInputDeviceManager_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsRWMutex.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashBag.h>
#include "mp/MpTypes.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpInputDeviceDriver;
class UtlString;
class MpBufPtr;

/**
*  @brief Container of input devices for input drivers and resources.
*
*  The MpInputDeviceManager is a container of input device drivers and provides 
*  the media frame collector and accessor for MprFromInputDevice resources.
*  MpInputDeviceDriver instances are added to the MpInputDeviceManager and
*  removed from it by external entities, using addDevice() and removeDevice().
*  Enabled MpInputDeviceDrivers push frames of media on to the connection
*  indicated by the device ID.  The MpInputDeviceManager queues the media
*  frames to be accessible by MprFromInputDevice resources via the getFrame
*  method.  The specific device driver is accessed by the device ID or handle.  
*  The MpInputDeviceManager maintains a device ID to device name
*  mapping.  All device IDs and device names are unique within the
*  scope of this MpInputDeviceManager.  The device ID provides a level of
*  indirection that allows the MpInputDeviceManager to protect against access of
*  transient devices.
*
*  MpInputDeviceManager allows multiple MprFromInputDevice resources to
*  read a frame of media via the getFrame method.  Each resource gets its 
*  own copy of an MpBuf.
*
*  MpInputDeviceDrivers invoke the pushFrame method to provide frames of
*  media for a specific device ID.  The MpInputDeviceManager maintains
*  a simple queue of frames for a short window of time.  It is the 
*  MprFromInputDevice resource's (or other consumer invoking getFrame)
*  responsibility to decide how far behind (buffer latency) the current frame
*  to maintain within the configured window of frames buffered.
*  
*  The MpInputDeviceManager uses MpAudioInputConnection internally to
*  map the association between MpInputDeviceDrivers and device ID.  The
*  MpAudioInputConnection also maintains the buffered media frames for a
*  device for a configured window of time.
*
*  @NOTE This class is intensionally NOT a singleton so that multiple
*        instances can be created in support of multiple instances of the media
*        subsystem being created.  Multiple instances may be useful for taking
*        advantage of multi-processors, testing and other interesting applications.
*/
class MpInputDeviceManager
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

      /// Default constructor
    MpInputDeviceManager(unsigned defaultSamplesPerFrame, 
                         unsigned defaultSamplesPerSec,
                         unsigned defaultNumBufferedFrames);
      /**<
      *  @param defaultSamplesPerFrame - (in) the default number of samples in
      *         a frame of media to be used when enabling devices.
      *  @param defaultSamplesPerSec - (in) default sample rate for media frame
      *         in samples per second to be used when enabling devices.
      *  @param defaultNumBufferedFrames - (in) default number of frames to
      *         buffer for a device before releasing the frame.  Consumers of
      *         frames (callers of getFrame) have the window of time to retrieve
      *         any of the frames buffered.
      */

      /// Destructor
    virtual
    ~MpInputDeviceManager();
      /**<
      *  @NOTE This is NOT thread safe.  The invoker of this destructor
      *        MUST be sure that no device drivers or resources are referencing
      *        this device manager.
      */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

      /// Add a new input device for use
    MpInputDeviceHandle addDevice(MpInputDeviceDriver& newDevice);
      /**<
      *  Returns device ID which is unique within this device manager.
      *  This method locks the device manager for exclusive use.
      *
      *  Multi-thread safe.
      */

      /// Remove an existing input device
    MpInputDeviceDriver* removeDevice(MpInputDeviceHandle deviceId);
      /**<
      *  This method locks the device manager for exclusive use.
      *
      *  Multi-thread safe.
      */

      /// Helper to enable device driver
    OsStatus enableDevice(MpInputDeviceHandle deviceId);
      /**<
      *  This method enables the device driver indicated by the device id.
      *
      *  @NOTE This SHOULD NOT be used to mute/unmute a device. Disabling and
      *        enabling a device results in state and buffer queues being cleared.
      *
      *  Multi-thread safe.
      */

      /// Helper to disable device driver
    OsStatus disableDevice(MpInputDeviceHandle deviceId);
      /**<
      *  This method disables the device driver indicated by the device id.
      *
      *  @NOTE This SHOULD NOT be used to mute/unmute a device. Disabling and
      *        enabling a device results in state and buffer queues being cleared.
      *
      *  Multi-thread safe.
      */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

      /// Get the device driver name for the given device ID
    UtlString& getDeviceName(MpInputDeviceHandle deviceId) const;
      /**<
      *  Multi-thread safe.
      */

      /// Get the device id for the given device driver name
    MpInputDeviceHandle getDeviceId(const char* deviceName) const;
      /**<
      *  The MpInputDeviceManager maintains a device ID to device name
      *  mapping.  All device IDs and device names are unique within the
      *  scope of this MpInputDeviceManager.
      *
      *  Multi-thread safe.
      */

      /// Get current frame timestamp
    MpFrameTime getCurrentFrameTime() const;
      /**<
      *  The timestamp is in milliseconds from the initial reference point
      *  in time for this device manager
      *
      *  @NOTE This number will wrap roughly every 11.5 days.
      *
      *  Multi-thread safe.
      */


      /// Method for MpInputDeviceDriver to push a frame of media for a given time
    OsStatus pushFrame(MpInputDeviceHandle deviceId,
                       unsigned numSamples,
                       MpAudioSample *samples,
                       MpFrameTime frameTime);
      /**<
      *  This method is used to push a frame to the MpInputDeviceManager to be
      *  buffered for a short window of time during which consumers such as
      *  MpFromInputDevice can retrieve the frame via the getFrame method.
      *
      *  @param deviceId - (in) device id to identify from which device the
      *         frame is from.
      *  @param numSamples - (in) number of samples in the frame of media from
      *         the device.  This must be the frame size number of samples that
      *         the device is configure to provide.
      *  @param samples - (in) the actual media for the frame.
      *  @param frameTime - (in) time in milliseconds for beginning of frame
      *         relative to the MpInputDeviceManager reference time.
      *
      *  Multi-thread safe.
      */

      /// Method for obtaining the buffer for a given frame and device ID
    OsStatus getFrame(MpInputDeviceHandle deviceId,
                      unsigned frameTime,
                      MpBufPtr& buffer,
                      unsigned& numFramesBefore,
                      unsigned& numFramesAfter);
      /**<
      *  This method is typically invoked by MprFromInputDevice resources.
      *
      *  @param deviceId - (in) device id to identify from which device a frame
      *         is to be retrieved.
      *  @param frameTime - (in) time in milliseconds for beginning of frame
      *         relative to the MpInputDeviceManager reference time.
      *  @param buffer - (out) frame of media to be retrieved (copied for
      *         callers context).
      *  @param numFramesBefore - (out) number of frames buffered which are
      *         newer than the frame for the requested time.
      *  @param numFramesAfter - (out) number of frames buffered which are older
      *         than the frame for the requested time.
      *
      *  @returns OS_INVALID_ARGUMENT if the device id does not exist.
      *  @returns OS_INVALID_STATE if the frame for the requested time is not
      *                            available yet and the device is not enabled.
      *  @returns OS_NOT_FOUND if the frame for the requested time is not
      *                        available yet.
      *
      *  Multi-thread safe.
      */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    OsRWMutex mRwMutex;
    MpInputDeviceHandle mLastDeviceId;
    unsigned mDefaultSamplesPerFrame;
    unsigned mDefaultSamplesPerSecond;
    unsigned mDefaultNumBufferedFrames;
    UtlHashMap mConnectionsByDeviceName;
    UtlHashBag mConnectionsByDeviceId;

     /// Copy constructor (not implemented for this class)
   MpInputDeviceManager(const MpInputDeviceManager& rMpInputDeviceManager);

     /// Assignment operator (not implemented for this class)
   MpInputDeviceManager& operator=(const MpInputDeviceManager& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpInputDeviceManager_h_
