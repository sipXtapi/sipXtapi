//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: REFACTORING CODE

#ifndef _MpodOSS_h_
#define _MpodOSS_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>

#ifdef __linux__
#include <sys/types.h>
#include <sys/soundcard.h>
#endif // __linux__


// APPLICATION INCLUDES
#include "mp/MpOutputDeviceDriver.h"
#include "mp/MpOSSDeviceWrapper.h"

// DEFINES
#define DEFAULT_N_OUTPUT_BUFS 64
//#define NUM_SAMPLES 80

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
class MpodOSS : public MpOutputDeviceDriver
{
    friend class MpOSSDeviceWrapper;
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Default constructor
   MpodOSS(const UtlString& name,
                          unsigned nInputBuffers = DEFAULT_N_OUTPUT_BUFS);
     /**<
     *  @param name - unique device driver name (e.g. "/dev/dsp", 
     *         "YAMAHA AC-XG WDM Audio", etc.)
     *  @param deviceManager - MpOutputDeviceManager this device is to
     *         recive frames throught pushFrame method
     */

     /// @brief Destructor
   ~MpodOSS();

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
      *  @param samplesPerFrame - the number of samples in a frame of media
      *  @param samplesPerSec - sample rate for media frame in samples per second
      *  @param currentFrameTime - time in milliseconds for beginning of frame
      *         relative to the MpInputDeviceManager reference time
      */

      /// @brief Uninitialize OSS device driver
    OsStatus disableDevice();
      /**<
      *  This method disables the device driver and should release any
      *  platform device resources so that the device might be used else where.
      *
      *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
      *        enabling a device results in state and buffer queues being cleared.
      */
      
     /// @brief Send data to OSS output device.
    OsStatus pushFrame(unsigned int numSamples,
                      MpAudioSample* samples);
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
    unsigned mNumInBuffers;   ///< The number of buffers to supply to OSS
                              ///< for audio processing.
    char* mpWaveBuffers;      ///< Array of nNumInBuffers wave buffers.
    
    int mCurBuff; /// Number of buffer in which will be store pushed data
    int mLastReceived; /// Number of buffer will be send send to OSS device
    
    OsNotification *pNotificator;
        
    sem_t mPushPopSem;
    
    MpFrameTime mCurrentFrameTime; ///< TODO: Fill in mCurrentFrameTime description
    
    UtlBoolean mDirectWritePending; ///< Setting to TRUE when current operations is direct write
    UtlBoolean mConditionWrite;

protected:    
    OsStatus initBuffers();
    void freeBuffers();
    
     /// @brief Get data from buffer.
    MpAudioSample* popFrame(unsigned& size);
     /**< 
      *  If successed return value is pointer to data and in 
      *  <tt>size</tt> strored size of sample in bytes
      *  otherwise return value is NULL
      */
     
     OsStatus signalForNextFrame();
     OsStatus signalForNextFrameWithCond();
     //OsStatus signalForNextFrameAndDirectWrite();
     
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    MpOSSDeviceWrapper *pDevWrapper;

      /// @brief Copy constructor (not implemented for this class)
    MpodOSS(const MpodOSS& rMpOutputDeviceDriver);

      /// @brief Assignment operator (not implemented for this class)
    MpodOSS& operator=(const MpodOSS& rhs);


};

/* ============================ INLINE METHODS ============================ */

UtlBoolean MpodOSS::isDeviceValid()
{
   return ((pDevWrapper != NULL) && pDevWrapper->isDeviceValid()); 
}

UtlBoolean MpodOSS::isNotificationNeeded()
{
    return (isDeviceValid() && 
            /* isEnabled() && */
            pNotificator != NULL);
}

#endif  // _MpodOSS_h_
