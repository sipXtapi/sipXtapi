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

#ifndef _MpidOSS_h_
#define _MpidOSS_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>

#ifdef __linux__
#include <sys/types.h>
#include <sys/soundcard.h>
#endif // __linux__


// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriver.h"
#include "mp/MpOSSDeviceWrapper.h"

// DEFINES
#define DEFAULT_N_INPUT_BUFS 32
//#define NUM_SAMPLES 80

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
class MpidOSS : public MpInputDeviceDriver
{
    friend class MpOSSDeviceWrapper;
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Default constructor
   MpidOSS(const UtlString& name,
                          MpInputDeviceManager& deviceManager,
                          unsigned nInputBuffers = DEFAULT_N_INPUT_BUFS);
     /**<
     *  @param name - unique device driver name (e.g. "/dev/dsp", 
     *         "YAMAHA AC-XG WDM Audio", etc.)
     *  @param deviceManager - MpInputDeviceManager this device is to
     *         push frames to via pushFrame method
     */

     /// @brief Destructor
   ~MpidOSS();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

      /// @brief Initialize device driver and state
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

      /// @brief Uninitialize device driver
    OsStatus disableDevice();
      /**<
      *  This method disables the device driver and should release any
      *  platform device resources so that the device might be used else where.
      *
      *  @NOTE this SHOULD NOT be used to mute/unmute a device. Disabling and
      *        enabling a device results in state and buffer queues being cleared.
      */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

      /// @brief Inquire if the OSS device is valid
    inline UtlBoolean isDeviceValid();

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    unsigned mNumInBuffers;   ///< The number of buffers to supply to OSS
                              ///< for audio processing.
    char* mpWaveBuffers;      ///< Array of nNumInBuffers wave buffers.
    
    int mCurBuff;

protected:    
    OsStatus initBuffers();
    void freeBuffers();
    MpAudioSample* getBuffer();
    
    void pushFrame(MpAudioSample* frm);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    //int mfdOssDev;            ///< The fd of the OSS device (e.g. /dev/dsp)
    MpOSSDeviceWrapper *pDevWrapper;


   // UtlBoolean mIsInit;       ///< Boolean indicating intialization 
                              ///  sucsessfully completed.
  

      /// @brief Copy constructor (not implemented for this class)
    MpidOSS(const MpidOSS& rMpInputDeviceDriver);

      /// @brief Assignment operator (not implemented for this class)
    MpidOSS& operator=(const MpidOSS& rhs);


};

/* ============================ INLINE METHODS ============================ */

UtlBoolean MpidOSS::isDeviceValid()
{
   return ((pDevWrapper != NULL) && pDevWrapper->isDeviceValid()); 
}

#endif  // _MpidOSS_h_
