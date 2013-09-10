//  
// Copyright (C) 2007-2013 SIPez LLC. All rights reserved.
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpOutputDeviceManager_h_
#define _MpOutputDeviceManager_h_

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
class MpOutputDeviceDriver;
class MpAudioOutputConnection;
class UtlString;
class MpBufPtr;
class OsNotification;

/**
*  @brief Container of output devices.
*
*  The MpOutputDeviceManager is a container of output device drivers, serving
*  as sink for MprToOuputDevice resources. MpOutputDeviceDriver instances are
*  added to the MpOutputDeviceManager and removed from it by external entities,
*  using addDevice() and removeDevice().
*  The specific device driver is accessed by the device ID (handle).
*  The MpOutputDeviceManager maintains a device ID to device name mapping. All
*  device IDs and device names are unique within the scope of this
*  MpOutputDeviceManager.  The device ID provides a level of indirection that
*  allows the MpOutputDeviceManager to protect against access of transient
*  devices.
*
*  MpOutputDeviceManager allows multiple MprToOutputDevice resources to
*  send a frame of media via the pushFrame() method. 
*  
*  The MpOutputDeviceManager uses MpAudioOutputConnection internally to
*  map the association between MpOutputDeviceDriver and device ID.  The
*  MpAudioInputConnection also maintains the mixer buffer.
*
*  @NOTE This class is intensionally NOT a singleton so that multiple
*        instances can be created in support of multiple instances of the media
*        subsystem being created.  Multiple instances may be useful for taking
*        advantage of multi-processors, testing and other interesting applications.
*/
class MpOutputDeviceManager
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor.
   MpOutputDeviceManager(unsigned defaultSamplesPerFrame,
                         unsigned defaultSamplesPerSecond,
                         MpFrameTime defaultMixerBufferLength);
     /**<
     *  @param[in] defaultSamplesPerFrame - the default number of samples in
     *         media frame. Will be used when enabling devices.
     *  @param[in] defaultSamplesPerSecond - default sample rate for media frame
     *         in samples per second. Will be used when enabling devices.
     *  @param[in] defaultMixerBufferLength - default length of mixer buffer in
     *         milliseconds. Will be used when enabling devices. Mixer buffer
     *         is used to mix multiple media streams. Stream sources (callers
     *         of pushFrame()) should produce data with difference in time less,
     *         then mixer buffer length, delayed stream data would be rejected.
     */


     /// Destructor.
   virtual
   ~MpOutputDeviceManager();
     /**<
     *  @NOTE This is NOT thread safe.  The invoker of this destructor
     *        MUST be sure that no connections or resources are referencing
     *        this device manager.
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Add a new input device for use.
   MpOutputDeviceHandle addDevice(MpOutputDeviceDriver  *newDevice);
     /**<
     *  Returns device ID which is unique within this device manager.
     *  This method locks the device manager for exclusive use.
     *
     *  @param newDevice[in] - A new input device to add to this manager.
     *                     The device should <b>only</b> be added to one manager.
     *
     *  @returns A handle to reference the device by in other manager calls.
     *
     *  Multi-thread safe.
     */


     /// Remove an existing input device.
   MpOutputDeviceDriver* removeDevice(MpOutputDeviceHandle deviceId);
     /**<
     *  This method locks the device manager for exclusive use.
     *
     *  @param deviceId[in] - The device to disable.
     *
     *  @returns A pointer to the input device driver that was removed.
     *  @returns NULL if the device could not be found or device is busy.
     *
     *  Multi-thread safe.
     */

   int removeAllDevices();
   /**<
    *  Remove all the device drivers, disable them if they are not already
    */


     /// Helper to enable device driver.
   OsStatus enableDevice(MpOutputDeviceHandle deviceId,
                         MpFrameTime mixerBufferLength = 0,
                         uint32_t samplesPerFrame = 0,
                         uint32_t samplesPerSec = 0);
     /**<
     *  This method enables the device driver indicated by the device id.
     *
     *  @param[in] deviceId - The device to enable.
     *  @param[in] mixerBufferLength (optional) - length of mixer buffer
     *         in milliseconds. Mixer buffer is used to mix multiple media
     *         streams coming to this device. Stream sources (callers of pushFrame())
     *         should produce data with difference in time less, then mixer
     *         buffer length, because delayed stream data would be rejected.
     *         Set \p mixerBufferLength to 0 to use manager's default.
     *  @param[in] samplesPerFrame (optional) - The samples per frame that this 
     *         device should operate with.  If not specified, the manager's 
     *         default will be used.
     *  @param[in] samplesPerSec (optional) - The sample rate that this device 
     *         should operate at.  If not specified, the manager's default will 
     *         be used.
     *
     *  @returns OS_NOT_FOUND if the device could not be found.
     *  @returns OS_INVALID_STATE if device already enabled.
     *  
     *  @NOTE This SHOULD NOT be used to mute/unmute a device. Disabling and
     *        enabling a device results in state and buffer queues being cleared.
     *
     *  Multi-thread safe.
     */


     /// Helper to disable device driver.
   OsStatus disableDevice(MpOutputDeviceHandle deviceId);
     /**<
     *  This method disables the device driver indicated by the device id.
     *
     *  @param deviceId[in] - The device to disable.
     *
     *  @returns OS_NOT_FOUND if the device could not be found.
     *  @returns OS_BUSY if the device is currently being removed or disabled.
     *
     *  @NOTE This SHOULD NOT be used to mute/unmute a device. Disabling and
     *        enabling a device results in state and buffer queues being cleared.
     *
     *  Multi-thread safe.
     */

   /// Helper to disable all device drivers except the optional list of devices
   OsStatus disableAllDevicesExcept(int exceptCount = 0, MpOutputDeviceHandle exceptIdArray[] = NULL);
   /**<
    *  This method diables all the devices except the given array of devices
    *
    *  @param exceptCount[in] number of device Ids in exceptIdArray
    *
    *  @param exceptIdArray[in] array of devices which are not to be disabled (i.e. exceptions)
    */


     /// Method for sending frame of data to output device.
   OsStatus pushFrameFirst(MpOutputDeviceHandle deviceId,
                           MpFrameTime &frameTime,
                           const MpBufPtr& frame,
                           UtlBoolean initFrameTime = FALSE);
     /**<
     *  This method is used to push a frame to the MpOutputDeviceManager to be
     *  buffered for a short window of time and mixed with data from other
     *  contributors.
     *
     *  If \p initFrameTime is TRUE, value of \p frameTime is ignored and
     *  frame is pushed to the very beginning of the mixer buffer. \p frameTime
     *  is set to correct frame time and should be used as a base for later
     *  pushes. If \p initFrameTime is FALSE, \p frameTime should point
     *  to correct frame time.
     *
     *  This method is typically invoked by MprToOutputDevice resources.
     *
     *  @param deviceId[in] - Device id to identify device to which this frame
     *         will be sent.
     *  @param frameTime[in,out] - Time in milliseconds for beginning of frame
     *         relative to the MpOutputDeviceManager reference time.
     *  @param frame[in] - Frame of media to be sent to output device.
     *  @param initFrameTime[in] - If TRUE, frame will be pushed to the
     *         beginning of mixer buffer and \p frameTime will be set to correct
     *         frame time. If FALSE, \p frameTime will be used to determine
     *         where to push the frame.
     *
     *  @returns OS_NOT_FOUND if the device could not be found.
     *  @returns OS_LIMIT_REACHED if mixer buffer is full, i.e. frame come too
     *           early.
     *  @returns OS_INVALID_STATE if frame come too late and was rejected.
     *           Frame considered late if whole frame does not fit into buffer,
     *           i.e. it would be rejected even if part of frame fit into buffer.
     *
     *  Multi-thread safe.
     */

     /// Method for sending frame of data to output device with initialized frame time.
   OsStatus pushFrame(MpOutputDeviceHandle deviceId,
                      MpFrameTime frameTime,
                      const MpBufPtr& frame);
     /**<
     *  This method may be used instead of pushFrameFirst() when frame time
     *  is initialized. Thus is does not take \p initFrameTime parameter and
     *  is not able to return frame time with \p frameTime parameter. In other
     *  aspects it is identical to pushFrameFirst().
     *
     *  @see pushFrameFirst()
     */

     /// Set device that will be used to send frame start message to media task.
   OsStatus setFlowgraphTickerSource(MpOutputDeviceHandle deviceId,
                                     OsNotification *pFlowgraphTicker);
     /**<
     *  Selected device will call MpMediaTask::signalFrameStart() method after
     *  every frame period to signal start of next frame processing interval.
     *
     *  @note Note, that ticker work only when device is enabled. So do not
     *        forget to enable device after call to this function.
     *
     *  @param[in] deviceId - Device id for device that will be used as ticker
     *         source. Pass MP_INVALID_OUTPUT_DEVICE_HANDLE to totally disable
     *         flowgraph ticker.
     *  @param[in] pFlowgraphTicker - Notification to signal. Value is ignored
     *         if \p deviceId equals to MP_INVALID_OUTPUT_DEVICE_HANDLE.
     *
     *  @returns OS_SUCCESS if device accepted set ticker request.
     *  @returns OS_NOT_FOUND if the device could not be found.
     *  @returns OS_FAILED if device rejected set ticker request.
     *  @returns OS_INVALID_STATE if current ticker source is no more
     *           available. New notification source is not set in this case.
     *  @returns OS_NOT_SUPPORTED if device could not provide ticker.
     *
     *  Multi-thread safe.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get the device driver name for the given device ID
   OsStatus getDeviceName(MpOutputDeviceHandle deviceId, UtlString& deviceName) const;
     /**<
     *  Get the name for the given deviceId.
     *
     *  @param deviceId[in] - The device to fetch the name of.
     *  @param deviceName[out] - A string that will hold the device name.
     *
     *  @returns OS_SUCCESS if device found and <tt>deviceName</tt> filled with
     *                      the name of the device.
     *  @returns OS_NOT_FOUND if specified device could not be found.
     *
     *  Multi-thread safe.
     */


     /// Get the device id for the given device driver name
   OsStatus getDeviceId(const UtlString& deviceName, MpOutputDeviceHandle &deviceId) const;
     /**<
     *  The MpOutputDeviceManager maintains a device ID to device name
     *  mapping.  All device IDs and device names are unique within the
     *  scope of this MpOutputDeviceManager.
     *
     *  @param deviceName[in] - The name of a device to get the ID of.
     *  @param deviceId[out] - A handle to reference the device by in other
     *         manager calls.
     *
     *  @returns OS_SUCCESS if device found and <tt>deviceId</tt> filled in.
     *  @returns OS_NOT_FOUND if specified device could not be found.
     *
     *  Multi-thread safe.
     */

     /// Get mixer buffer length (in milliseconds) for given device.
   OsStatus getMixerBufferLength(MpOutputDeviceHandle deviceId, MpFrameTime &length) const;
     /**<
     *  @returns OS_SUCCESS if device found and <tt>length</tt> filled in.
     *  @returns OS_NOT_FOUND if specified device could not be found.
     *
     *  Multi-thread safe.
     */

     /// Get current frame timestamp
   MpFrameTime getCurrentFrameTime(MpOutputDeviceHandle deviceId) const;
     /**<
     *  The timestamp is in milliseconds from the initial reference point
     *  in time for this device.
     *
     *  @NOTE This number will wrap roughly every 49.7 days.
     *
     *  Multi-thread safe.
     */

     /// @brief Get the sample rate that a particular device is running at.
   OsStatus getDeviceSamplesPerSec(MpOutputDeviceHandle deviceId, 
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
   OsStatus getDeviceSamplesPerFrame(MpOutputDeviceHandle deviceId,
                                     uint32_t& samplesPerFrame) const;
     /**<
     *  @param[in]  deviceId - The ID of the device to query.
     *  @param[out] samplesPerFrame - Filled with the indicated device's 
     *              number of samples per frame if OS_SUCCESS is returned.
     *  @retval OS_NOT_FOUND if device was not able to be found.
     *  @retval OS_SUCCESS if device was able to be found and number of 
     *          samples per frame was looked up.
     */



//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// Inquire if device is enabled (e.g. consuming media data).
   UtlBoolean isDeviceEnabled(MpOutputDeviceHandle deviceId) const;
     /**<
     *  Inquire if specified device is enabled (e.g. consuming media data).
     *
     *  @param deviceId[in] - The device to determine enabled status of.
     */

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpAudioOutputConnection* findConnectionBlocking(MpOutputDeviceHandle deviceId,
                                                   int tries=10) const;
   // Try 10 times (this is a blind guess).
   // Checking every 10ms over 100ms seems reasonable.


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

  mutable OsRWMutex mRwMutex;         ///< Mutex to serialize access to
                      ///< connections. It is mutable, as we lock it in const
                      ///< methods too.
  MpOutputDeviceHandle mLastDeviceId;
  unsigned mDefaultSamplesPerFrame;
  unsigned mDefaultSamplesPerSecond;
  MpFrameTime mDefaultBufferLength;
  UtlHashMap mConnectionsByDeviceName;
  UtlHashBag mConnectionsByDeviceId;
  OsTime mTimeZero;
  MpOutputDeviceHandle mCurrentTickerDevice;

    /// Copy constructor (not implemented for this class)
  MpOutputDeviceManager(const MpOutputDeviceManager& rMpOutputDeviceManager);

    /// Assignment operator (not implemented for this class)
  MpOutputDeviceManager& operator=(const MpOutputDeviceManager& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpOutputDeviceManager_h_
