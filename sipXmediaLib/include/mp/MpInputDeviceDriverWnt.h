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

#ifndef _MpInputDeviceDriverWnt_h_
#define _MpInputDeviceDriverWnt_h_

// SYSTEM INCLUDES

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
*  @brief Container for device specific input driver.
*
*  The MpInputDeviceDriver is the abstract container for the implementation
*  of input media drivers.  An instance of MpInputDeviceDriver is created for
*  every physical and logical input device (e.g. microphone).  A driver is
*  instantiated and then added to the MpInputDeviceManager.  The driver must
*  be enabled via the MpInputDeviceManager before it yields input frames.
*
*  The MpInputDeviceDriver obtains frames of audio from hardware and  
*  provides them to the MpInputDeviceManager which makes the frames available to 
*  MprFromInputDevice resources in flowgraphs.  The MpInputDeviceDriver may be 
*  an OsTask which runs and gets frames for the OS and hardware specific device
*  or MpInputDeviceDriver may be driven via callbacks from the OS/hardware
*  input device driver.  This is an implementation choice.
*/
class MpInputDeviceDriverWnt : public MpInputDeviceDriver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Default constructor
   MpInputDeviceDriverWnt(const UtlString& name,
                          MpInputDeviceManager& deviceManager,
                          unsigned nInputBuffers = DEFAULT_N_INPUT_BUFS);
     /**<
     *  @param name - unique device driver name (e.g. "/dev/dsp", 
     *         "YAMAHA AC-XG WDM Audio", etc.)
     *  @param deviceManager - MpInputDeviceManager this device is to
     *         push frames to via pushFrame method
     */

     /// @brief Destructor
   virtual
   ~MpInputDeviceDriverWnt();

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
      *  @param deviceId - device Id used to identify this input device to 
      *         MpInputDeviceManager for pushFrame calls.
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

      /// @brief Inquire if the windows device is valid
    virtual UtlBoolean isDeviceValid() { return (mWntDeviceId >= 0); };

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

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

    int mWntDeviceId;   ///< The underlying windows Device ID (not the logical
                        ///< Mp device ID)
    HWAVEIN mDevHandle; ///< The Microsoft handle for this audio input device
    unsigned mNumInBuffers;   ///< The number of buffers to supply to windows
                              ///< for audio processing.
    unsigned mWaveBufSize;    ///< The size, in bytes, of mpWaveBuffer 
                              ///< after allocation.
    WAVEHDR* mpWaveHeaders;   ///< Array of nNumInBuffers wave headers.
    LPSTR* mpWaveBuffers;     ///< Array of nNumInBuffers wave buffers.
    UtlBoolean mIsOpen;       ///< Boolean indicating waveInOpen() completed.


      /// @brief Copy constructor (not implemented for this class)
    MpInputDeviceDriverWnt(const MpInputDeviceDriver& rMpInputDeviceDriver);

      /// @brief Assignment operator (not implemented for this class)
    MpInputDeviceDriverWnt& operator=(const MpInputDeviceDriver& rhs);

      /// @brief Zero out a wave header, so it is ready to be filled in by windows.
    WAVEHDR* initWaveHeader(int n);
      /**<
      *  Initialize all the values in the wave header indicated by <tt>n</tt>.
      *  @param n - The index into <tt>mpWaveHeaders</tt> indicating 
      *             which wave header to initialize.
      *  @returns a pointer to the wave header that was initialized.
      */
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpInputDeviceDriverWnt_h_
