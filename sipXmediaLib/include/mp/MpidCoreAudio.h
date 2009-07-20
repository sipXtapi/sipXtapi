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

#ifndef _MpidCoreAudio_h_
#define _MpidCoreAudio_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>


// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriver.h"
#include "mp/MpCoreAudioHardware.h"

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
*  @brief Container for device specific input CoreAudio driver.
*/
class MpidCoreAudio : public MpInputDeviceDriver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Default constructor.
   MpidCoreAudio(const UtlString& name,
                 MpInputDeviceManager& deviceManager);
     /**<
     *  @param name - (in) unique device driver name (e.g. "/dev/dsp",
     *         "YAMAHA AC-XG WDM Audio", etc.).
     *  @param deviceManager - (in) MpInputDeviceManager this device is to
     *         push frames to via pushFrame method.
     */

     /// @brief Destructor
   ~MpidCoreAudio();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MpInputDeviceDriver::enableDevice()
   OsStatus enableDevice(unsigned samplesPerFrame,
                         unsigned samplesPerSec,
                         MpFrameTime currentFrameTime);

     /// @copydoc MpInputDeviceDriver::disableDevice()
   OsStatus disableDevice();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// @copydoc MpInputDeviceDriver::isDeviceValid()
   UtlBoolean isDeviceValid();
   
//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   AudioUnit mInputUnit;            ///< AudioUnit input handle
   AudioConverterRef mConverter;    ///< Sample rate converter
   CoreAudioHardware mDevice;       ///< CoreAudio hardware device handle
   
   double mInputSampleRate;         ///< Device smaple rate
   MpAudioSample* mAudioFrame;      ///< Wave buffer for silence.

   void* mTempBuffer;           ///< Buffer for temporary storage
   void* mTempBufferHalf;       ///< Pointer to the center of the temp buffer
   unsigned mTempBufferHalfSize; ///< Half size of buffer
   unsigned mInputFrameSize;     ///< Device input frame size
   
     /// @brief Push audio frame to InputDeviceManager.
   void pushFrame();

     /// @brief Add frame time to <tt>mCurrentFrameTime</tt>.
   void skipFrame();
   
   bool setAudioUnitFormat(const AudioStreamBasicDescription& ds);
   
   bool initResempler(const AudioStreamBasicDescription& ds);
   
   bool setBuffers();
   
   static 
   void propertyListener(void * inRefCon,
                         AudioUnit ci,
                         AudioUnitPropertyID inID,
                         AudioUnitScope inScope,
                         AudioUnitElement inElement);
   
   static
   OSStatus renderProc(void * inRefCon,
                       AudioUnitRenderActionFlags* inActionFlags,
                       const AudioTimeStamp * inTimeStamp,
                       UInt32 inBusNumber,
                       UInt32 inNumFrames,
                       AudioBufferList * ioData);
   
   static
   OSStatus ACproc(AudioConverterRef inAudioConverter, 
                   UInt32 * ioDataPackets,
                   AudioBufferList* ioData,
                   AudioStreamPacketDescription ** inUserData,
                   void *inUData);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// @brief Copy constructor (not implemented for this class).
   MpidCoreAudio(const MpidCoreAudio& rMpInputDeviceDriver);

     /// @brief Assignment operator (not implemented for this class).
   MpidCoreAudio& operator=(const MpidCoreAudio& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpidCoreAudio_h_
