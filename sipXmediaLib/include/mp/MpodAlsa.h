//
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpodAlsa_h_
#define _MpodAlsa_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>

#ifdef __linux__
#include <sys/types.h>
#include <sys/soundcard.h>
#endif // __linux__


// APPLICATION INCLUDES
#include "mp/MpOutputDeviceDriver.h"
#include "mp/MpAlsa.h"


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAX_DEVICE_NAME_SIZE 256

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpOutputDeviceManager;
class UtlContainer;


/**
*  @brief Container for device specific input ALSA driver.
*/
class MpodAlsa : public MpOutputDeviceDriver
{
    friend class MpAlsa;
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Default constructor.
   MpodAlsa(const UtlString& name);
     /**<
     *  @param name - (in) unique device driver name (e.g. "/dev/dsp",
     *         "YAMAHA AC-XG WDM Audio", etc.).
     *  @param nInputBuffers - (in) Maximum number of frames in internal buffer.
     */

     /// @brief Destructor
   virtual ~MpodAlsa();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MpOutputDeviceDriver::enableDevice()
   OsStatus enableDevice(unsigned samplesPerFrame,
                         unsigned samplesPerSec,
                         MpFrameTime currentFrameTime,
                         OsCallback &frameTicker);

     /// @copydoc MpOutputDeviceDriver::disableDevice()
   OsStatus disableDevice();

     /// @copydoc MpOutputDeviceDriver::pushFrame()
   OsStatus pushFrame(unsigned int numSamples,
                      const MpAudioSample* samples,
                      MpFrameTime frameTime);

   OsStatus setNotificationMode(UtlBoolean bThreadNotification);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{
    /// @brief Get the name of the default output ALSA PCM audio device
    static const char* getDefaultDeviceName();

    /// @brief Get the list of names of the available output ALSA PCM audio device(s)
    static int getDeviceNames(UtlContainer& devicesNames);
    /**
     *  @returns the number of output devices found
     */
//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

   /// Check if this device can connect
   virtual OsStatus canEnable();
   /* @see MpOutputDeviceDriver::canEnable */

     /// @brief Inquire if the ALSA device is valid
   inline UtlBoolean isDeviceValid();

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   MpAudioSample* mAudioFrame;  ///< Wave buffer.
   MpFrameTime mCurrentFrameTime; ///< The current frame time for this device.
   UtlBoolean mNotificationThreadEn;

   MpAlsaContainer* mpCont; ///< Pointer to Wrapper container

     /// @brief Signaling for next frame if notificator used do nothing otherwise
   OsStatus signalForNextFrame();

     /// @brief Adding frame time to <tt>mCurrentFrameTime</tt>.
   void skipFrame();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   MpAlsa *pDevWrapper;
   static char spDefaultDeviceName[MAX_DEVICE_NAME_SIZE];

     /// @brief Copy constructor (not implemented for this class)
   MpodAlsa(const MpodAlsa& rMpOutputDeviceDriver);

     /// @brief Assignment operator (not implemented for this class)
   MpodAlsa& operator=(const MpodAlsa& rhs);

   int mNullTickers;

};

/* ============================ INLINE METHODS ============================ */

UtlBoolean MpodAlsa::isDeviceValid()
{
   //printf("MpRsOdAlsa::isDeviceValid()\n"); fflush(stdout);
   return ((pDevWrapper != NULL) && pDevWrapper->isDeviceValid());
}

#endif  // _MpodAlsa_h_
