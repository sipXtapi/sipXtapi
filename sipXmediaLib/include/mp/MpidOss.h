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

#ifndef _MpidOss_h_
#define _MpidOss_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>


// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriver.h"
#include "mp/MpOss.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpInputDeviceManager;

/**
*  @brief Container for device specific input OSS driver.
*/
class MpidOss : public MpInputDeviceDriver
{
   friend class MpOss;
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Default constructor.
   MpidOss(const UtlString& name,
           MpInputDeviceManager& deviceManager);
     /**<
     *  @param name - (in) unique device driver name (e.g. "/dev/dsp",
     *         "YAMAHA AC-XG WDM Audio", etc.).
     *  @param deviceManager - (in) MpInputDeviceManager this device is to
     *         push frames to via pushFrame method.
     */

     /// @brief Destructor
   ~MpidOss();

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

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// @brief Inquire if the OSS device is valid.
   inline UtlBoolean isDeviceValid();

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   MpAudioSample* mAudioFrame;  ///< Wave buffer.
   MpOssContainer* mpCont; ///< Pointer to Wrapper container

     /// @brief Allocate internal OSS buffers.
   OsStatus initBuffers();

     /// @brief Free internal OSS buffers.
   void freeBuffers();

     /// @brief Get buffer from internal buffers.
   MpAudioSample* getBuffer();

     /// @brief Push audio frame to InputDeviceManager.
   void pushFrame();

     /// @brief Add frame time to <tt>mCurrentFrameTime</tt>.
   void skipFrame();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   MpOss *pDevWrapper;

     /// @brief Copy constructor (not implemented for this class).
   MpidOss(const MpidOss& rMpInputDeviceDriver);

     /// @brief Assignment operator (not implemented for this class).
   MpidOss& operator=(const MpidOss& rhs);

};

/* ============================ INLINE METHODS ============================ */

UtlBoolean MpidOss::isDeviceValid()
{
   //printf("MpRsIdOss::isDeviceValid()\n"); fflush(stdout);
   return ((pDevWrapper != NULL) && pDevWrapper->isDeviceValid());
}

#endif  // _MpidOss_h_
