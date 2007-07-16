//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MpidWinMM_h_
#define _MpidWinMM_h_

// SYSTEM INCLUDES
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <MMSystem.h>

// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriver.h"

// DEFINES
#define DEFAULT_N_INPUT_BUFS 32
#define NUM_SAMPLES 80

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpInputDeviceManager;

/**
*  @brief Container for the Microsoft Windows Multimedia specific input driver.
*
*  The MpidWinMM device driver wrapper is the concrete container containing the
*  implementation of the windows multimedia wave audio input interface.
*  This is currently driven by callbacks.
*  @see MpInputDeviceDriver
*/
class MpidWinMM : public MpInputDeviceDriver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Default constructor
   MpidWinMM(const UtlString& name,
                          MpInputDeviceManager& deviceManager,
                          unsigned nInputBuffers = DEFAULT_N_INPUT_BUFS);
     /**<
     *  @param[in] name - unique device driver name
     *             (e.g. "YAMAHA AC-XG WDM Audio", etc.)
     *  @param[in] deviceManager - MpInputDeviceManager this device is to
     *             push frames to via pushFrame method
     *  @param[in] nOutputBuffers - The number of frame-sized buffers to 
     *             register with Windows to fill with frames of audio data,
     *             in case we're not fast enough to process them.
     */

     /// @brief Destructor
   virtual
   ~MpidWinMM();

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
      *  @param[in] samplesPerFrame - the number of samples in a frame of media
      *  @param[in] samplesPerSec - sample rate for media frame in samples per second
      *  @param[in] currentFrameTime - time in milliseconds for beginning of frame
      *             relative to the MpInputDeviceManager reference time
      *  @param[in] deviceId - device Id used to identify this input device to 
      *             MpInputDeviceManager for pushFrame calls.
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

     /// @brief get the windows name of the default wave input device.
   static UtlString getDefaultDeviceName();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

      /// @brief Inquire if the windows device is valid
    virtual inline UtlBoolean isDeviceValid();

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

      /// @brief Zero out a wave header, so it is ready to be filled in by windows.
    WAVEHDR* initWaveHeader(int n);
      /**<
      *  Initialize all the values in the wave header indicated by <tt>n</tt>.
      *  @param[in] n - The index into <tt>mpWaveHeaders</tt> indicating 
      *             which wave header to initialize.
      *  @returns a pointer to the wave header that was initialized.
      */

      /// @brief Processes incoming audio data.
    void processAudioInput(HWAVEIN hwi, 
                           UINT uMsg, 
                           void* dwParam1);
      /**<
      *  This method, called by the static callback function 
      *  waveInCallbackStatic, processes audio input data from the windows
      *  waveform audio functions, passing the results to the input manager.
      */

      /// @brief Callback function for receiving data from windows audio.
    static void CALLBACK waveInCallbackStatic(HWAVEIN hwi,
                                              UINT uMsg,
                                              void* dwInstance,
                                              void* dwParam1,
                                              void* dwParam2);
      /**<
      *  This static method is called by the windows waveform audio functions
      *  passing incoming audio data to be processed.
      *  This function then passes the results on to processAudioInput()
      */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    int mWinMMDeviceId;       ///< The underlying windows Device ID (not the
                              ///< logical Mp device ID)
    HWAVEIN mDevHandle;       ///< The Microsoft handle for this audio input device
    unsigned mNumInBuffers;   ///< The number of buffers to supply to windows
                              ///< for audio processing.
    unsigned mWaveBufSize;    ///< The size, in bytes, of mpWaveBuffer 
                              ///< after allocation.
    WAVEHDR* mpWaveHeaders;   ///< Array of nNumInBuffers wave headers.
    LPSTR* mpWaveBuffers;     ///< Array of nNumInBuffers wave buffers.
    UtlBoolean mIsOpen;       ///< Boolean indicating waveInOpen() completed.
    unsigned mnAddBufferFailures;  ///< The number of times that addBuffer called 
                                   ///< within the callback has failed since last enabled.


      /// @brief Copy constructor (not implemented for this class)
    MpidWinMM(const MpInputDeviceDriver& rMpInputDeviceDriver);

      /// @brief Assignment operator (not implemented for this class)
    MpidWinMM& operator=(const MpInputDeviceDriver& rhs);
};

/* ============================ INLINE METHODS ============================ */

UtlBoolean MpidWinMM::isDeviceValid()
{
   return (mWinMMDeviceId >= 0);
}


#endif  // _MpidWinMM_h_
