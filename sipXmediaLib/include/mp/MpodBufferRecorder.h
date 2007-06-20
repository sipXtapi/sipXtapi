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

#ifndef _MpodBufferRecorder_h_
#define _MpodBufferRecorder_h_

// SYSTEM INCLUDES
//#include <utl/UtlDefs.h>
#include <os/OsStatus.h>
#include <utl/UtlString.h>

// APPLICATION INCLUDES
#include "mp/MpOutputDeviceDriver.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsNotification;
class OsTimer;

/**
*  @brief Container for device specific output driver.
*
*/
class MpodBufferRecorder : public MpOutputDeviceDriver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor.
   explicit
   MpodBufferRecorder(const UtlString& name, MpFrameTime bufferLength);
     /**<
     *  @param name - (in) unique device driver name (e.g. "/dev/dsp", 
     *         "YAMAHA AC-XG WDM Audio", etc.)
    *  @param bufferLength - (in) length of buffer for recorded data (in milliseconds).
     */

     /// Destructor.
   virtual
   ~MpodBufferRecorder();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Initialize device driver and state.
   virtual OsStatus enableDevice(unsigned samplesPerFrame, 
                                 unsigned samplesPerSec,
                                 MpFrameTime currentFrameTime);
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
     *
     *  @returns OS_INVALID_STATE if device already enabled.
     *
     *  @NOTE This method is supposed to be used from MpAudioOutputConnection only.
     *        If you want enable device, use MpOutputDeviceManager or
     *        MpAudioOutputConnection methods.
     */

     /// Uninitialize device driver.
   virtual OsStatus disableDevice();
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
                      MpFrameTime frameTime);
     /**<
     *  This method is usually called from MpAudioOutputConnection::pushFrame().
     *
     *  @param numSamples - (in) Number of samples in <tt>samples</tt> array.
     *  @param samples - (in) Array of samples to push to device.
     *  @param frameTime - (in) This driver ignore frame time.
     *
     *  @returns OS_LIMIT_REACHED if mixer buffer is full, i.e. frame come too
     *           early.
     *  @returns OS_FAILED for other errors.
     *  @returns OS_SUCCESS if frame sent correctly.
     */

     /// Set frame ticker notification which this device should signal.
   virtual
   OsStatus setTickerNotification(OsNotification *pFrameTicker);
     /**<
     *  If pFrameTicker is not NULL, device driver MUST signal this
     *  notification as soon as it become ready to receive next frame of data.
     *  Note, that signaling this notification may block for some time, as it
     *  would normally be a callback which in turn calls pushFrame() method of
     *  this device driver. Also notification may be used to signal begin of
     *  frame interval for one or several flowgraphs, so it should be as
     *  uniform as possible, i.e. it should not burst or hold over, driver
     *  should signal this notification after equal intervals of time.
     *
     *  Pass NULL to pFrameTicker if you do not want receive frame ticks, e.g.
     *  in direct write mode. In this case driver may stop its thread if it used
     *  one for for notifications.
     *
     *  @param pFrameTicker - (in) notification to signal when device become ready. 
     *
     *  @see See isFrameTickerSupported().
     *
     *  @returns OS_SUCCESS if frame ticker notification set successfully.
     *  @returns OS_NOT_SUPPORTED if this driver implementation does not support
     *           frame ticker notification.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Is frame ticker notification support by this driver notification.
   virtual
   UtlBoolean isFrameTickerSupported() const;
     /**<
     *  If this method returns TRUE, setTickerNotification() method may be
     *  used to set frame ticker notification. If this method returns FALSE,
     *  setTickerNotification() must return OS_NOT_SUPPORTED.
     */

   inline
   const MpAudioSample *getBufferData() const;

   inline
   unsigned getBufferLength() const;

   inline
   unsigned getBufferEnd() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpFrameTime    mBufferLengthMS;
   unsigned       mBufferLength;
   MpAudioSample *mpBuffer;
   unsigned       mBufferEnd;

   OsTimer       *mpTickerTimer;
   OsNotification *mpTickerNotification;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MpodBufferRecorder(const MpodBufferRecorder& rMpodBufferRecorder);

     /// Assignment operator (not implemented for this class)
   MpodBufferRecorder& operator=(const MpodBufferRecorder& rhs);
};


/* ============================ INLINE METHODS ============================ */

const MpAudioSample *MpodBufferRecorder::getBufferData() const
{
   return mpBuffer;
}

unsigned MpodBufferRecorder::getBufferLength() const
{
   return mBufferLength;
}

unsigned MpodBufferRecorder::getBufferEnd() const
{
   return mBufferEnd;
}

#endif  // _MpodBufferRecorder_h_
