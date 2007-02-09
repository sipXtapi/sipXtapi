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
*  MpInputDeviceDriver instances are added to the MpInputDeviceManager.
*  Enabled MpInputDeviceDrivers push frames of media on to the connection
*  indicated by the device ID.  The MpInputDeviceManager queues the media
*  frames to be accessable by MprFromInputDevice resources via the getFrame
*  method.  The specific device driver is accessed by the device ID or handle.  
*  The MpInputDeviceManager maintains a device ID to device name
*  mapping.  All device IDs and device names are unique within the
*  scope of this MpInputDeviceManager.  The device ID
*  provides a level of indirection that allows the MpInputDeviceManager
*  to protect against access of transient devices.
*
*  MpInputDeviceManager allows multiple MprFromInputDevice resources to
*  read a frame of media via the getFrame method.  Each resource gets its 
*  own copy of an MpBuf.
*
*  MpInputDeviceDrivers invoke the pushFrame method to provide frames of
*  media for a specific device ID.  The MpInputDeviceManager maintains
*  a simple queue of frames for a short window of time.  It is the 
*  MprFromInputDevice resource's (or consumer invoking getFrame) responsiblity
*  to decide how far behind (buffer latency) the current frame to maintain
*  within the configured window of frames buffered.
*  
*  The MpInputDeviceManager uses MpAudioInputConnection internally to
*  map the association between MpInputDeviceDrivers and device ID.  The
*  MpAudioInputConnection also maintains the buffered media frames for a
*  device for a configured window of time.
*
*  Note: this class is intensionally NOT a singleton so that multiple
*  instances can be created in support of multiple instances of the media
*  subsystem being created.  Multiple instances may be useful for taking
*  advantage of multi-processors, testing and other interesting applications.
*/
class MpInputDeviceManager
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Default constructor
    /*
     *  @param defaultSamplesPerFrame - the default number of samples in a frame 
     *         of media to be used when enabling devices.
     *  @param defaultSamplesPerSec - default sample rate for media frame in  
     *         samplesper second to be used when enabling devices.
     *  @param defaultNumBufferedFrames - default number of frames to buffer
     *         for a device before releasing the frame.  Consumers of frames
     *         (callers of getFrame) have the window of time to retrieve any 
     *         of the frames buffered.
     */
    MpInputDeviceManager(int defaultSamplesPerFrame, 
                         int defaultSamplesPerSec,
                         int defaultNumBufferedFrames);

    /// Destructor
    /* NOTE: THIS IS NOT thread safe.  The invoker of this destructor
     * MUST be sure that no device drivers or resources are referencing this
     * device manager.
     */
    virtual
    ~MpInputDeviceManager();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Add a new input device for use
    /*  returns device ID which is unique within this device manager.
     *  This method locks the device manager for exclusive use.
     *  Multi-thread safe
     */
    int addDevice(MpInputDeviceDriver& newDevice);

    /// Remove an existing input device
    /*
     *  This method locks the device manager for exclusive use.
     *  Multi-thread safe
     */
    MpInputDeviceDriver* removeDevice(int deviceId);

    /// Helper to enable device driver
    /*  This method enables the device driver indicated by the device id.
     *  NOTE: this SHOULD NOT be used to mute/unmute a device. Disabling and
     *  enabling a device results in state and buffer queues being cleared.
     *  Multi-thread safe
     */
    OsStatus enableDevice(int deviceId);

    /// Helper to disable device driver
    /*  This method disables the device driver indicated by the device id.
     *  NOTE: this SHOULD NOT be used to mute/unmute a device. Disabling and
     *  enabling a device results in state and buffer queues being cleared.
     *  Multi-thread safe
     */
    OsStatus disableDevice(int deviceId);
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Get the device driver name for the given device ID
    /*
     *  Multi-thread safe
     */
    UtlString& getDeviceName(int deviceId) const;

    /// Get the device id for the given device driver name
    /*  The MpInputDeviceManager maintains a device ID to device name
     *  mapping.  All device IDs and device names are unique within the
     *  scope of this MpInputDeviceManager.
     *  Multi-thread safe
     */
    int getDeviceId(const char* deviceName) const;

    /// Get current frame time stamp
    /*  The time stamp is in milli-seconds from the initial reference point
     *  in time for this device manager
     *  Note: this number will wrap roughly every 11.5 days
     *  Multi-thread safe
     */
    int getCurrentFrameTime() const;


    /// Method for MpInputDeviceDriver to push a frame of media for a given time
    /*  This method is used to push a frame to the MpInputDeviceManager to be
     *  buffered for a short window of time during which consumers such as
     *  MpFromInputDevice can retrieve the frame via the getFrame method.
     *
     *  @param deviceId - device id to indentify from which device the frame is
     *         from.
     *  @param numSamples - number of samples in the frame of media from the
     *         device.  This must be the frame size number of samples that
     *         the device is configure to provide.
     *  @param samples - the actual media for the frame.
     *  @param frameTime - time in milliseconds for begining of frame
     *         relative to the MpInputDeviceManager reference time
     *
     *  Multi-thread safe
     */
    OsStatus pushFrame(int deviceId,
                       int numSamples,
                       short samples,
                       int frameTime);

    /// Method for obtaining the buffer for a given frame and device ID
    /* This method is typically invoked by MprFromInputDevice resources.
     *
     *  @param deviceId - device id to indentify from which device a frame is
     *         to be retrieved.
     *  @param buffer - frame of media to be retrieved (copied for callers context)
     *  @param frameTime - time in milliseconds for begining of frame
     *         relative to the MpInputDeviceManager reference time
     *  @param numFramesBefore - number of frames buffered which are newer than
     *         the frame for the requested time
     *  @param numFramesAfter - number of frames buffered which are older than
     *         the frame for the requested time
     *  @returns OS_INVALID_ARGUMENT if the device id does not exist, 
     *           OS_INVALID_STATE if the frame for the requested time is not available
     *                            yet and the device is not enabled,
     *           OS_NOT_FOUND if the frame for the requested time is not available yet.
     *
     *  Multi-thread safe
     */
    OsStatus getFrame(int deviceId,
                     MpBufPtr& buffer,
                     int frameTime,
                     int& numFramesBefore,
                     int& numFramesAfter);
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
    int mLastDeviceId;
    int mDefaultSamplesPerFrame;
    int mDefaultSamplesPerSecond;
    int mDefaultNumBufferedFrames;
    UtlHashMap mConnectionsByDeviceName;
    UtlHashBag mConnectionsByDeviceId;

     /// Copy constructor (not implemented for this class)
   MpInputDeviceManager(const MpInputDeviceManager& rMpInputDeviceManager);

     /// Assignment operator (not implemented for this class)
   MpInputDeviceManager& operator=(const MpInputDeviceManager& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpInputDeviceManager_h_
