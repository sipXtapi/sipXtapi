//  
// Copyright (C) 2007-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
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
#define MP_INVALID_INPUT_DEVICE_HANDLE -1

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
class MpBufPool;

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

     /// @brief Default constructor
   MpInputDeviceManager(unsigned defaultSamplesPerFrame, 
                        unsigned defaultSamplesPerSecond,
                        unsigned defaultNumBufferedFrames,
                        MpBufPool& bufferPool);
     /**<
     *  @param defaultSamplesPerFrame - (in) the default number of samples in
     *         a frame of media to be used when enabling devices.
     *  @param defaultSamplesPerSecond - (in) default sample rate for media frame
     *         in samples per second to be used when enabling devices.
     *  @param defaultNumBufferedFrames - (in) default number of frames to
     *         buffer for a device before releasing the frame.  Consumers of
     *         frames (callers of getFrame) have the window of time to retrieve
     *         any of the frames buffered.
     *  @param bufferPool - pool from which buffers are obtained for media
     *         data space
     */


     /// @brief Destructor
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

     /// @brief Add a new input device for use
   MpInputDeviceHandle addDevice(MpInputDeviceDriver& newDevice);
     /**<
     *  Returns device ID which is unique within this device manager.
     *  This method locks the device manager for exclusive use.
     *
     *  @param newDevice - (in) A new input device to add to this manager.
     *                     The device should <b>only</b> be added to one manager.
     *  @returns A handle to reference the device by in other manager calls.
     *
     *  Multi-thread safe.
     */


     /// @brief Remove an existing input device
   MpInputDeviceDriver* removeDevice(MpInputDeviceHandle deviceId);
     /**<
     *  This method locks the device manager for exclusive use.
     *
     *  @param deviceId - (in) The device to disable.
     *  @returns A pointer to the input device driver that was removed.
     *  @returns NULL if the device could not be found or device is busy.
     *
     *  Multi-thread safe.
     */

   int removeAllDevices();
   /**<
    *  Remove all the device drivers, disable them if they are not already
    */


     /// @brief Set device parameters before enabling it.
   OsStatus setupDevice(MpInputDeviceHandle deviceId,
                        uint32_t samplesPerFrame = 0,
                        uint32_t samplesPerSec = 0);
     /**<
     *  Use this method if you want to set parameters different from the manager
     *  defaults. If you're happy with the manager defaults, you may omit this.
     *
     *  @param[in] deviceId - (in) The device to setup.
     *  @param[in] samplesPerFrame (optional) - The samples per frame that this 
     *         device should operate with.  If not specified, the manager's 
     *         default will be used.
     *  @param[in] samplesPerSec (optional) - The sample rate that this device 
     *         should operate at.  If not specified, the manager's default will 
     *         be used.
     *  @returns OS_NOT_FOUND if the device could not be found.
     *  
     *  @NOTE This SHOULD NOT be used to mute/unmute a device. Disabling and
     *        enabling a device results in state and buffer queues being cleared.
     *
     *  Multi-thread safe.
     */


     /// @brief Helper to enable device driver
   OsStatus enableDevice(MpInputDeviceHandle deviceId);
     /**<
     *  This method enables the device driver indicated by the device id.
     *
     *  @param[in] deviceId - The device to enable.
     *  @returns OS_NOT_FOUND if the device could not be found.
     *  
     *  @NOTE This SHOULD NOT be used to mute/unmute a device. Disabling and
     *        enabling a device results in state and buffer queues being cleared.
     *
     *  Multi-thread safe.
     */


     /// @brief Helper to disable device driver
   OsStatus disableDevice(MpInputDeviceHandle deviceId);
     /**<
     *  This method disables the device driver indicated by the device id.
     *
     *  @param[in] deviceId - The device to disable.
     *  @returns OS_NOT_FOUND if the device could not be found.
     *  @returns OS_BUSY if the device is currently being removed or disabled.
     *
     *  @NOTE This SHOULD NOT be used to mute/unmute a device. Disabling and
     *        enabling a device results in state and buffer queues being cleared.
     *
     *  Multi-thread safe.
     */

   /// Helper to disable all device drivers except the optional list of devices
   OsStatus disableAllDevicesExcept(int exceptCount = 0, MpInputDeviceHandle exceptIdArray[] = NULL);
   /**<
    *  This method diables all the devices except the given array of devices
    *
    *  @param exceptCount[in] number of device Ids in exceptIdArray
    *
    *  @param exceptIdArray[in] array of devices which are not to be disabled (i.e. exceptions)
    */

     /// @brief Method for MpInputDeviceDriver to push a frame of media for a given time
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
     *  @returns OS_NOT_FOUND if the device could not be found.
     *
     *  Multi-thread safe.
     */


     /// @brief Method for obtaining the buffer for a given frame and device ID
   OsStatus getFrame(MpInputDeviceHandle deviceId,
                     MpFrameTime &frameTime,
                     MpBufPtr& buffer,
                     unsigned& numFramesBefore,
                     unsigned& numFramesAfter);
     /**<
     *  Method for obtaining the buffer for a given frame and device ID
     *  This method is typically invoked by MprFromInputDevice resources.
     *
     *  @param deviceId - (in) device id to identify from which device a frame
     *         is to be retrieved.
     *  @param frameTime - (in/out) time in milliseconds for beginning of frame
     *         relative to the MpInputDeviceManager reference time. If frame
     *         successfully pulled, this param would be set to frame time of
     *         this frame. You may use this value to synchronize to device
     *         driver.
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

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @brief Get the device driver name for the given device ID
   OsStatus getDeviceName(MpInputDeviceHandle deviceId, UtlString& deviceName) const;
     /**<
     *  Get the name for the given deviceId.
     *
     *  @param deviceId - (in) The device to fetch the name of.
     *  @param deviceName - (out) a UtlString that will hold the deviceName
     *  @returns OS_SUCCESS and <tt>deviceName</tt> filled with the
     *           name of the device.
     *  @returns OS_NOT_FOUND if the device could not be found.
     *
     *  Multi-thread safe.
     */


     /// @brief Get the device id for the given device driver name
   OsStatus getDeviceId(const UtlString& deviceName,
                        MpInputDeviceHandle& deviceId) const;
     /**<
     *  The MpInputDeviceManager maintains a device ID to device name
     *  mapping.  All device IDs and device names are unique within the
     *  scope of this MpInputDeviceManager.
     *
     *  @param deviceName - (in) The name of a device to get the ID of.
     *  @param deviceId   - (out) A place to store the ID of the device.
     *  @returns OS_SUCCESS and \c deviceId set with the ID of the 
     *           device, if the device was found.
     *  @returns OS_NOT_FOUND and \c deviceId set with MP_INVALID_INPUT_DEVICE_HANDLE if the 
     *           device could not be found.
     *
     *  Multi-thread safe.
     */


     /// @brief Get current frame timestamp
   MpFrameTime getCurrentFrameTime(MpInputDeviceHandle deviceId) const;
     /**<
     *  The timestamp is in milliseconds from the initial reference point
     *  in time for this device.
     *
     *  @param[in] deviceId - A place to store the ID of the device.
     *
     *  @NOTE This number will wrap roughly every 49.7 days.
     *
     *  Multi-thread safe.
     */


     /// @brief Get the sample rate that a particular device is running at.
   OsStatus getDeviceSamplesPerSec(MpInputDeviceHandle deviceId, 
                                   uint32_t& samplesPerSec) const;
     /**<
     *  @param[in]  deviceId - The ID of the device to query.
     *  @param[out] samplesPerSec - Filled with the indicated device's 
     *              sample rate if OS_SUCCESS is returned.
     *  @retval OS_NOT_FOUND if device was not able to be found.
     *  @retval OS_SUCCESS if device was able to be found and sample rate was 
     *          looked up.
     */


     /// @brief Get the number of samples per frame that a particular device is running at.
   OsStatus getDeviceSamplesPerFrame(MpInputDeviceHandle deviceId,
                                     uint32_t& samplesPerFrame) const;
     /**<
     *  @param[in]  deviceId - The ID of the device to query.
     *  @param[out] samplesPerFrame - Filled with the indicated device's 
     *              number of samples per frame if OS_SUCCESS is returned.
     *  @retval OS_NOT_FOUND if device was not able to be found.
     *  @retval OS_SUCCESS if device was able to be found and number of 
     *          samples per frame was looked up.
     */


     /// @brief Method for obtaining the time derivatives for sequential frames 
     /// @brief as relates to reference time.
   OsStatus getTimeDerivatives(MpInputDeviceHandle deviceId,
                               unsigned& nDerivatives,
                               double*& derivativeBuf) const;
     /**<
     *  Calculates the derivative: 
     *  <tt>(t2-t1)/(reference frame period) for sequential t1,t2</tt>
     *  for each set of times buffered, starting from the most recent
     *  in the buffer.
     *
     *  @param deviceId - (in) device to get statistics for.
     *  @param nDerivatives - (in/out) number of derivatives to attempt to obtain.
     *  @param derivativeBuf - (out) an allocated buffer of at least size 
     *         \cnDerivatives which will be filled with the derivatives.
     *
     *  @returns \cnDerivatives contains the number of derivatives calculated 
     *           and returned.  If there are less than \cnDerivatives frames 
     *           buffered, then the number of derivatives calculated and 
     *           returned is the number of frames buffered.
     *  @returns OS_INVALID_ARGUMENT if the device id does not exist.
     * 
     *  Multi-thread safe.
     */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// @brief Inquire if device is enabled (e.g. generating media data).
   UtlBoolean isDeviceEnabled(MpInputDeviceHandle deviceId) const;
     /**<
     *  Inquire if device is enabled (e.g. generating media data).
     *
     *  @param deviceId - (in) The device to determine enabled status of.
     */

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   mutable OsRWMutex mRwMutex;
   MpInputDeviceHandle mLastDeviceId;
   unsigned mDefaultSamplesPerFrame;
   unsigned mDefaultSamplesPerSecond;
   unsigned mDefaultNumBufferedFrames;
   MpBufPool* mpBufferPool;
   UtlHashMap mConnectionsByDeviceName;
   UtlHashBag mConnectionsByDeviceId;
   OsTime mTimeZero;

    /// Copy constructor (not implemented for this class)
  MpInputDeviceManager(const MpInputDeviceManager& rMpInputDeviceManager);

    /// Assignment operator (not implemented for this class)
  MpInputDeviceManager& operator=(const MpInputDeviceManager& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpInputDeviceManager_h_
