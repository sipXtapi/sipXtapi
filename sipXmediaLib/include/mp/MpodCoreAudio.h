//
// Copyright (C) 2009 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpodCoreAudio_h_
#define _MpodCoreAudio_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>

// APPLICATION INCLUDES
#include "mp/MpOutputDeviceDriver.h"
#include "mp/MpCoreAudioHardware.h"


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpOutputDeviceManager;


/**
*  @brief Container for device specific output CoreAudio driver.
*/
class MpodCoreAudio : public MpOutputDeviceDriver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Default constructor.
   MpodCoreAudio(const UtlString& name);
     /**<
     *  @param name - (in) unique device driver name (e.g. "/dev/dsp",
     *         "YAMAHA AC-XG WDM Audio", etc.).
     *  @param nInputBuffers - (in) Maximum number of frames in internal buffer.
     */

     /// @brief Destructor
   ~MpodCoreAudio();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MpOutputDeviceDriver::enableDevice()
   OsStatus enableDevice(unsigned samplesPerFrame,
                         unsigned samplesPerSec,
                         MpFrameTime currentFrameTime);

     /// @copydoc MpOutputDeviceDriver::disableDevice()
   OsStatus disableDevice();

     /// @copydoc MpOutputDeviceDriver::pushFrame()
   OsStatus pushFrame(unsigned int numSamples,
                      const MpAudioSample* samples,
                      MpFrameTime frameTime);

     /// @copydoc MpOutputDeviceDriver::setTickerNotification()
   OsStatus setTickerNotification(OsNotification *pFrameTicker);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   AudioUnit mOutputUnit;           ///< AudioUnit output handle
   AudioConverterRef mConverter;    ///< Sample rate converter
   CoreAudioHardware mDevice;       ///< CoreAudio hardware device handle
   
   double mOutputSampleRate;        ///< Device smaple rate
   MpAudioSample* mAudioFrame;      ///< Wave buffer for silence.
   MpAudioSample* mAudioFrameData;  ///< Second wave buffer for actual data
   
   OsNotification *pNotificator;    ///< Notificator used for signaling next frame
   MpFrameTime mCurrentFrameTime;   ///< The current frame time for this device.

     /// @brief Signaling for next frame if notificator used do nothing otherwise
   OsStatus signalForNextFrame();

     /// @brief Adding frame time to <tt>mCurrentFrameTime</tt>.
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

     /// @brief Copy constructor (not implemented for this class)
   MpodCoreAudio(const MpodCoreAudio& rMpOutputDeviceDriver);

     /// @brief Assignment operator (not implemented for this class)
   MpodCoreAudio& operator=(const MpodCoreAudio& rhs);


};

/* ============================ INLINE METHODS ============================ */


#endif  // _MpodCoreAudio_h_
