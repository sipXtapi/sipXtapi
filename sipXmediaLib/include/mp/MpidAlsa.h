//
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpidAlsa_h_
#define _MpidAlsa_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>


// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriver.h"
#include "mp/MpAlsa.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAX_DEVICE_NAME_SIZE 256

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpInputDeviceManager;
class UtlContainer;

/**
*  @brief Container for device specific input ALSA driver.
*/
class MpidAlsa : public MpInputDeviceDriver
{
   friend class MpAlsa;
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Default constructor.
   MpidAlsa(const UtlString& name,
           MpInputDeviceManager& deviceManager);
     /**<
     *  @param name - (in) unique device driver name (e.g. "/dev/dsp",
     *         "YAMAHA AC-XG WDM Audio", etc.).
     *  @param deviceManager - (in) MpInputDeviceManager this device is to
     *         push frames to via pushFrame method.
     */

     /// @brief Destructor
   virtual ~MpidAlsa();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @brief Initialize device driver and state.
   OsStatus enableDevice(unsigned samplesPerFrame,
                         unsigned samplesPerSec,
                         MpFrameTime currentFrameTime);
     /**<
     *  This method enables the device driver.
     *
     *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
     *  enabling a device results in state and buffer queues being cleared.
     *
     *  @param samplesPerFrame - (in) the number of samples in a frame of media
     *  @param samplesPerSec - (in) sample rate for media frame in samples per
     *         second
     *  @param currentFrameTime - (in) time in milliseconds for beginning of
     *         frame relative to the MpInputDeviceManager reference time
     */

     /// @brief Uninitialize device driver.
   OsStatus disableDevice();
     /**<
     *  This method disables the device driver and should release any
     *  platform device resources so that the device might be used else where.
     *
     *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
     *        enabling a device results in state and buffer queues being
     *        cleared.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{
    /// @brief Get the name of the default input ALSA PCM audio device
    static const char* getDefaultDeviceName();

    /// @brief Get the list of names of the available input ALSA PCM audio device(s)
    static int getDeviceNames(UtlContainer& devicesNames);
    /**
     *  @returns the number of input devices found
     */
//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// @brief Inquire if the ALSA device is valid.
   inline UtlBoolean isDeviceValid();

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   MpAudioSample* mAudioFrame;  ///< Wave buffer.
   MpAlsaContainer* mpCont; ///< Pointer to Wrapper container

     /// @brief Allocate internal ALSA buffers.
   OsStatus initBuffers();

     /// @brief Free internal ALSA buffers.
   void freeBuffers();

     /// @brief Get buffer from internal buffers.
   MpAudioSample* getBuffer();

     /// @brief Push audio frame to InputDeviceManager.
   void pushFrame();

     /// @brief Add frame time to <tt>mCurrentFrameTime</tt>.
   void skipFrame();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   MpAlsa *pDevWrapper;
   static char spDefaultDeviceName[MAX_DEVICE_NAME_SIZE];

     /// @brief Copy constructor (not implemented for this class).
   MpidAlsa(const MpidAlsa& rMpInputDeviceDriver);

     /// @brief Assignment operator (not implemented for this class).
   MpidAlsa& operator=(const MpidAlsa& rhs);

};

/* ============================ INLINE METHODS ============================ */

UtlBoolean MpidAlsa::isDeviceValid()
{
   //printf("MpRsIdAlsa::isDeviceValid()\n"); fflush(stdout);
   return ((pDevWrapper != NULL) && pDevWrapper->isDeviceValid());
}

#endif  // _MpidAlsa_h_
