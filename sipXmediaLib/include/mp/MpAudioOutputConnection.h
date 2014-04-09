//  
// Copyright (C) 2007-2013 SIPez LLC. All rights reserved.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpAudioOutputConnection_h_
#define _MpAudioOutputConnection_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpTypes.h>
#include <mp/MpOutputDeviceDriver.h>
#include <utl/UtlInt.h>
#include <os/OsMutex.h>
#include <os/OsCallback.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Private container class for MpOutputDeviceDriver pointer and
*         mixer buffer.
*
*  All public methods of MpOutputDeviceDriver class are synchronized with
*  internal mutex. This class also provide capability to implement safe delete.
*  All external entities, accessing this class should call increaseUseCount()
*  before accessing this class and call decreaseUseCount() after all access
*  operations are finished. Entity, owning this object, should check use counter
*  value before deleting with getUseCount() method and wait while it would not
*  be equal to zero. Note, that calls to increaseUseCount(), decreaseUseCount()
*  and getUseCount() should be synchronized by external entity, owning object
*  of this class.
*
*  @nosubgrouping
*/
class MpAudioOutputConnection : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MpAudioOutputConnection(MpOutputDeviceHandle deviceId,
                           MpOutputDeviceDriver *deviceDriver);
     /**<
     *  @param[in] deviceId - Device ID assigned to this connection.
     *  @param[in] deviceDriver - Device driver, assigned with this connection.
     */

     /// Destructor
   virtual
   ~MpAudioOutputConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Initialize device driver and mixer buffer.
   OsStatus enableDevice(unsigned samplesPerFrame, 
                         unsigned samplesPerSec,
                         MpFrameTime currentFrameTime,
                         MpFrameTime mixerBufferLength);
     /**<
     *  This method enables the device driver associated with this connection
     *  and create mixer buffer if requested.
     *
     *  @param[in] samplesPerFrame - the number of samples in a frame of media
     *  @param[in] samplesPerSec - sample rate for media frame in samples per second
     *  @param[in] currentFrameTime - time in milliseconds for beginning of frame
     *             relative to the MpOutputDeviceManager reference time
     *  @param[in] mixerBufferLength - length of mixer buffer in milliseconds. 
     *
     *  @returns OS_INVALID_STATE if device already enabled.
     *  @returns OS_SUCCESSS if device is successfully enabled.
     *  
     *  @see MpOutputDeviceDriver::enableDevice() for more information.
     *  @see MpOutputDeviceManager constructor description for more information
     *       on mixer buffer length parameter.
     */

     /// Uninitialize device driver.
   OsStatus disableDevice();
     /**<
     *  This method disables the device driver associated with this connection
     *  and free mixer buffer.
     *
     *  @see MpOutputDeviceDriver::disableDevice() for more information.
     */

     /// Use this device to signal frame processing interval start.
   OsStatus enableFlowgraphTicker(OsNotification *pFlowgraphTicker);
     /**<
     *  @param[in] pFlowgraphTicker - Ticker to signal for next frame
     *             processing interval.
     *
     *  @returns OS_SUCCESS if ticker registered successfully.
     *  @returns OS_FAILED if device failed to register ticker.
     */

     /// Stop using this device to signal frame processing interval start.
   OsStatus disableFlowgraphTicker();
     /**<
     *  @returns OS_SUCCESS if ticker registered successfully.
     *  @returns OS_FAILED if device failed to unregister ticker.
     */

     /// Send data frame to the beginning of mixer buffer and return frame time.
   OsStatus pushFrameBeginning(unsigned int numSamples,
                               const MpAudioSample* samples,
                               MpFrameTime &frameTime);
     /**<
     *  This method is usually used only once when first frame is pushed.
     *  It pushes frame to the very beginning of the mixer buffer and return
     *  frame time of that position. Client should use this time to initialize
     *  his own frame time counter to use usual pushFrame() method.
     *
     *  @see pushFrame() for more information.
     *
     *  @param[in] numSamples - Number of samples in \p samples array.
     *  @param[in] samples - Array of samples to push to device.
     *  @param[out] frameTime - Time in milliseconds for beginning of frame
     *             relative to the MpOutputDeviceManager reference time.
     */

     /// Send data frame to output device.
   OsStatus pushFrame(unsigned int numSamples,
                      const MpAudioSample* samples,
                      MpFrameTime frameTime);
     /**<
     *  This method is used to push a frame to the MpOutputDeviceManager to be
     *  buffered for a short window of time and mixed with data from other
     *  contributors.
     *
     *  This method is usually called from MpOutputDeviceManager::pushFrame().
     *
     *  @returns OS_LIMIT_REACHED if mixer buffer is full, i.e. frame come too
     *           early.
     *  @returns OS_INVALID_STATE if frame come too late and was rejected.
     *           Frame considered late if whole frame does not fit into buffer,
     *           i.e. it would be rejected even if part of frame fit into buffer.
     *  @returns OS_FAILED for other errors.
     *  @returns OS_SUCCESS if frame sent correctly.
     */

     /// Add one more reference to this connection.
   inline
   unsigned increaseUseCount();
     /**<
     *  Every external entity which refer to this connection should increment
     *  use counter to prevent sudden deletion of connection, while it is
     *  still in use. Do not forget to decrement use counter when no more needed
     *  or connection will never be deleted.
     *
     *  @returns Value of internal use counter after increment.
     *
     *  @NOTE This function is NOT thread-safe. All manipulations with use
     *  counter must be synchronized by external entities.
     *
     *  @see decreaseUseCount(), getUseCount()
     */


     /// Remove one reference to this connection.
   inline
   unsigned decreaseUseCount();
     /**<
     *  @returns Value of internal use counter after decrement.
     *
     *  @NOTE This function is NOT thread-safe. All manipulations with use
     *  counter must be synchronized by external entities.
     *
     *  @see increaseUseCount(), getUseCount()
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return device driver contained in this connection.
   inline
   MpOutputDeviceDriver* getDeviceDriver() const;

     /// Return length of mixer buffer in milliseconds.
   inline
   MpFrameTime getMixerBufferLength() const;

     /// Return number of external entities, referring to this connection.
   inline
   unsigned getUseCount() const;
     /**<
     *  @returns Value of internal use counter.
     *
     *  @NOTE This function is NOT thread-safe. All manipulations with use
     *  counter must be synchronized by external entities.
     *
     *  @see increaseUseCount(), decreaseUseCount()
     */

     /// Return number of samples per frame, this driver expect from flowgraph.
   inline
   unsigned getSamplesPerFrame() const;

   MpFrameTime getCurrentFrameTime() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

///@name Mixer buffer methods
//@{

     /// Allocate mixer buffer and initialize all related member variables.
   OsStatus initMixerBuffer(unsigned mixerBufferLength);
     /**<
     *  @param[in] mixerBufferLength - Number of samples in mixer buffer. This
     *         parameter must be greater then 0.
     *
     *  @NOTE Not thread-safe. This function is supposed to be used from
     *        other (synchronized) functions.
     */

     /// Free mixer buffer and set all related member variables to zero.
   OsStatus freeMixerBuffer();
     /**<
     *  @NOTE Not thread-safe. This function is supposed to be used from
     *        other (synchronized) functions.
     *
     *  @returns OS_FAILED if mixer buffer is not allocated.
     */

     /// Mix frame to mixer buffer.
   OsStatus mixFrame(unsigned frameOffset,
                     const MpAudioSample* samples,
                     unsigned numSamples);
     /**<
     *  @param[in] frameOffset - Number of samples between the beginning of
     *             mixer buffer and the start of pushed frame.
     *  @param[in] samples - Pointer to frame data.
     *  @param[in] numSamples - Number of samples in frame.
     *
     *  @returns OS_LIMIT_REACHED if mixer buffer is full.
     *
     *  @NOTE Not thread-safe. This function is supposed to be used from
     *        other (synchronized) functions.
     */

     /// Check whether it's too late to mix this frame in or not (does not check for too early).
   inline OsStatus isLateToMix(unsigned frameOffset,
                               unsigned numSamples) const;
     /**<
     *  @param[in] frameOffset - Number of samples between the beginning of
     *             mixer buffer and the start of pushed frame.
     *  @param[in] numSamples - Number of samples in frame.
     *
     *  @retval OS_LIMIT_REACHED if mixer buffer is full and it's too late
     *          mix this frame in.
     *  @retval OS_SUCCESS if it's not yet too late.
     *
     *  @NOTE Not thread-safe. This function is supposed to be used from
     *        other (synchronized) functions.
     */

     /// Pull frame from mixer buffer.
   OsStatus advanceMixerBuffer(unsigned numSamples);
     /**<
     *  Remove samples from mixer buffer begin, equal number of samples become
     *  available at the end of mixer buffer. This method is usually called
     *  after device driver read samples from the beginning of buffer. After
     *  this current frame time should be advanced by appropriate number of
     *  milliseconds to move time window forward.
     *
     *  @param[in] numSamples - How many samples to advance.
     */

     /// Call this when driver become ready for the next frame.
   static
   void readyForDataCallback(const intptr_t userData, const intptr_t eventData);
     /**<
     *  @param[in] userData - Contain pointer connection it is associated with.
     *  @param[in] eventData - contains 0 for now.
     */

//@}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   mutable OsMutex mMutex;        ///< Mutex to synchronize access to connection.
   int mUseCount;                 ///< Use counter, used to implement safe delete.

   MpOutputDeviceDriver* mpDeviceDriver; ///< Device driver associated with this
                   ///< connection.
   MpFrameTime mCurrentFrameTime; ///< Frame time of the mixer buffer begin.

   unsigned mMixerBufferLength;   ///< Length of mixer buffer (in samples).
   MpAudioSample *mpMixerBuffer;  ///< Mixer circular buffer. Used to mix
                   ///< several media streams.
   unsigned mMixerBufferBegin;    ///< Index of first available sample in mixer buffer.

   OsCallback readyForDataCallbackNotf;  ///< This callback is used in mixer mode
                   ///< to notify connection that device is ready for
                   ///< the next frame.
   OsNotification *mpFlowgraphTicker; ///< Notification to call to notify graph
                   ///< when it should process next frame.


     /// Copy constructor (not implemented for this class)
   MpAudioOutputConnection(const MpAudioOutputConnection& rMpAudioOutputConnection);

     /// Assignment operator (not implemented for this class)
   MpAudioOutputConnection& operator=(const MpAudioOutputConnection& rhs);

};

/* ============================ INLINE METHODS ============================ */

unsigned MpAudioOutputConnection::increaseUseCount()
{
   return ++mUseCount;
}

unsigned MpAudioOutputConnection::decreaseUseCount()
{
   return --mUseCount;
}

MpOutputDeviceDriver* MpAudioOutputConnection::getDeviceDriver() const
{
   return mpDeviceDriver;
};

MpFrameTime MpAudioOutputConnection::getMixerBufferLength() const
{
   return mMixerBufferLength*1000/getDeviceDriver()->getSamplesPerSec();
}

unsigned MpAudioOutputConnection::getUseCount() const
{
   return mUseCount;
}

unsigned MpAudioOutputConnection::getSamplesPerFrame() const
{
   return getDeviceDriver()->getSamplesPerFrame();
}

OsStatus MpAudioOutputConnection::isLateToMix(unsigned frameOffset,
                                              unsigned numSamples) const
{

   // Whole frame should fit into buffer to be accepted.
   if (frameOffset+numSamples > mMixerBufferLength)
   {
      return OS_LIMIT_REACHED;
   }
   return OS_SUCCESS;
}

#endif  // _MpAudioOutputConnection_h_
