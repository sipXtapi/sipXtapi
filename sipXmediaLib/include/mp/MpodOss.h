//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpodOss_h_
#define _MpodOss_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>

#ifdef __linux__
#include <sys/types.h>
#include <sys/soundcard.h>
#endif // __linux__


// APPLICATION INCLUDES
#include "mp/MpOutputDeviceDriver.h"
#include "mp/MpOss.h"


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpOutputDeviceManager;


/**
*  @brief Container for device specific input OSS driver.
*/
class MpodOss : public MpOutputDeviceDriver
{
    friend class MpOss;
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Default constructor.
   MpodOss(const UtlString& name);
     /**<
     *  @param name - (in) unique device driver name (e.g. "/dev/dsp",
     *         "YAMAHA AC-XG WDM Audio", etc.).
     *  @param nInputBuffers - (in) Maximum number of frames in internal buffer.
     */

     /// @brief Destructor
   ~MpodOss();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @brief  Initialize OSS device driver and state.
   OsStatus enableDevice(unsigned samplesPerFrame,
                         unsigned samplesPerSec,
                         MpFrameTime currentFrameTime);
     /**<
     *  This method enables the device driver.
     *
     *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
     *  enabling a device results in state and buffer queues being cleared.
     *
     *  @param samplesPerFrame - (in) the number of samples in a frame of media.
     *  @param samplesPerSec - (in) sample rate for media frame in samples per
     *         second.
     *  @param currentFrameTime - (in) time in milliseconds for beginning of frame
     *         relative to the MpInputDeviceManager reference time.
     */

     /// @brief Uninitialize OSS device driver
   OsStatus disableDevice();
     /**<
     *  This method disables the device driver and should release any
     *  platform device resources so that the device might be used else where.
     *
     *  @note this SHOULD NOT be used to mute/unmute a device. Disabling and
     *        enabling a device results in state and buffer queues being cleared.
     */

     /// @brief Send data to OSS output device.
   OsStatus pushFrame(unsigned int numSamples,
                      const MpAudioSample* samples,
                      MpFrameTime frameTime);
     /**<
     *  This method is usually called from MpAudioOutputConnection::pushFrame().
     *
     *  @param numSamples - (in) Number of samples in <tt>samples</tt> array.
     *  @param samples - (in) Array of samples to push to device.
     */

     /// Set frame ticker notification which this device should signal.
   OsStatus setTickerNotification(OsNotification *pFrameTicker);
     /**<
     *  @param pFrameTicker - (in) notification to signal when device become
     *         ready. Pass NULL to pFrameTicker if you do not want receive
     *         frame ticks, e.g. in direct write mode. In this case driver
     *         may stop its thread if it used one for for notifications.
     *
     *  @see See mpFrameTicker documentation for detailed discussion.
     *  @see See isFrameTickerSupported().
     *
     *  @returns OS_SUCCESS if frame ticker notification set successfully.
     *  @returns OS_NOT_SUPPORTED if this driver implementation does not support
     *           frame ticker notification.
     */

    OsStatus setNotificationMode(UtlBoolean bThreadNotification);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Is frame ticker notification support by this driver notification.
  UtlBoolean isFrameTickerSupported() const;
    /**<
    *  If this method returns TRUE, setTickerNotification() method may be
    *  used to set frame ticker notification. If this method returns FALSE,
    *  setTickerNotification() must return OS_NOT_SUPPORTED.
    */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// @brief Inquire if the OSS device is valid
   inline UtlBoolean isDeviceValid();

     /// @brief Inquire if the notification is needed
   inline UtlBoolean isNotificationNeeded();
//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   MpAudioSample* mAudioFrame;  ///< Wave buffer.
   OsNotification *pNotificator; ///< Notificator used for signaling next frame
   MpFrameTime mCurrentFrameTime; ///< The current frame time for this device.
   UtlBoolean mNotificationThreadEn;

   MpOssContainer* mpCont; ///< Pointer to Wrapper container

     /// @brief Signaling for next frame if notificator used do nothing otherwise
   OsStatus signalForNextFrame();

     /// @brief Adding frame time to <tt>mCurrentFrameTime</tt>.
   void skipFrame();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   MpOss *pDevWrapper;

     /// @brief Copy constructor (not implemented for this class)
   MpodOss(const MpodOss& rMpOutputDeviceDriver);

     /// @brief Assignment operator (not implemented for this class)
   MpodOss& operator=(const MpodOss& rhs);


};

/* ============================ INLINE METHODS ============================ */

UtlBoolean MpodOss::isDeviceValid()
{
   //printf("MpRsOdOss::isDeviceValid()\n"); fflush(stdout);
   return ((pDevWrapper != NULL) && pDevWrapper->isDeviceValid());
}

UtlBoolean MpodOss::isNotificationNeeded()
{
   return (isDeviceValid() && (pNotificator != NULL));
}

#endif  // _MpodOss_h_
