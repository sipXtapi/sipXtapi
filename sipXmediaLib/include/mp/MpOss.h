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

#ifndef _MpOss_h_
#define _MpOss_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>

#include <sys/types.h>
#include <sys/soundcard.h>

// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriver.h"
#include "mp/MpOutputDeviceDriver.h"
#include "mp/MpOssContainer.h"
#include "utl/UtlVoidPtr.h"

class MpidOss;
class MpodOss;

/**
*  @brief Wrapper for OSS device that use file descriptor to communicate.
*/
class MpOss : public UtlVoidPtr
{
   friend class MpidOss;
   friend class MpodOss;
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// @brief Constructor
   MpOss();

   /// @brief Destructor
   ~MpOss();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// @brief Connect MpidOss class driver
   OsStatus setInputDevice(MpidOss* pIDD);
   /// @brief Connect MpodOss class driver
   OsStatus setOutputDevice(MpodOss* pODD);

   /// @brief Disconnect Input driver
   OsStatus freeInputDevice();
   /// @brief Disconnect Output driver
   OsStatus freeOutputDevice();

   /// @brief Enable input device
   OsStatus attachReader();
   /// @brief Enable output device
   OsStatus attachWriter();

   /// @brief Disable input device
   OsStatus detachReader();
   /// @brief Disable output device
   OsStatus detachWriter();

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{


   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   /// @brief Inquire if the device is valid
   inline UtlBoolean isDeviceValid();

   /// @brief Inquire if the input device is connected
   inline UtlBoolean isReaderAttached();

   /// @brief Inquire if the output device is connected
   inline UtlBoolean isWriterAttached();
   inline UtlBoolean isDevCapBatch();
   inline UtlBoolean isDevCapDuplex();
   inline UtlBoolean isDevCapMmap();
   inline UtlBoolean isDevCapTrigger();

   /// @brief Inquire if OSS device is free, i.e. neither input nor output is connected
   inline UtlBoolean isNotUsed();

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   int mfdDevice;               ///< The fd of the POSIX device (e.g. /dev/dsp)
   UtlBoolean mbReadCap;        ///< Device is able to play audio
   UtlBoolean mbWriteCap;       ///< Device is able to capture audio

   MpidOss* mReader;          ///< Input driver
   MpodOss* mWriter;          ///< Output driver

   int mDeviceCap;              ///< Device capabilities

   volatile UtlBoolean mStReader;        ///< Input device is enabled
   volatile UtlBoolean mStWriter;        ///< Output device is enabled
   volatile UtlBoolean mStShutdown;      ///< Thread exit flag
   volatile UtlBoolean mModeChanged;     ///< IO settings has changed

   unsigned mUsedSamplesPerSec;          ///< Used samples rate either for IO
   unsigned mUsedSamplesPerFrame;        ///< Used frame size for IO

   MpAudioSample* mResamplerBuffer;      ///< Buffer for conversions
   UtlBoolean mStereoOps;                ///< Use stereo output

   pthread_t mIoThread;                  ///< Internal IO thread
   sem_t mSleepSem;      ///< Control IO thread sleeping
   sem_t mSignalSem;     ///< Use for parameters synchronization

   /// @brief Pre initializations for OSS device
   OsStatus initDevice(const char* devname);
   /// @brief Final initializations for OSS device
   OsStatus initDeviceFinal(unsigned samplesPerSec, unsigned samplerPerFrame);
   /// @brief Free OSS device
   OsStatus freeDevice();

   /// @brief Because OSS device works in duplex mode we must ensure that
   /// input and output driver use one sample rate and perfrom final init
   OsStatus setSampleRate(unsigned samplesPerSec, unsigned samplerPerFrame);

   /// @brief Perform input operation of OSS device
   OsStatus doInput(char* buffer, int size);
   /// @brief Perform output operation of OSS device
   OsStatus doOutput(const char* buffer, int size);

   /// @brief Perform input operation of OSS device with siutable resampler
   OsStatus doInputRs(MpAudioSample* buffer, unsigned size);
   /// @brief Perform output operation of OSS device  with siutable resampler
   OsStatus doOutputRs(const MpAudioSample* buffer, unsigned size);

   /// @brief Deinitialization and freeing sequences
   void noMoreNeeded();

   /// @brief Thread subroutine
   void soundIoThread();

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   void threadIoStatusChanged();
   void threadWakeUp();
   void threadKill();

   UtlBoolean ossSetTrigger(bool turnOn);
   UtlBoolean ossReset();

   /// @brief Thread subroutine
   static void* soundCardIoWrapper(void* arg);
};

/* ============================ INLINE METHODS ============================ */

UtlBoolean MpOss::isDeviceValid()
{
   return (mfdDevice != -1);
}

UtlBoolean MpOss::isReaderAttached()
{
   return (mReader != NULL);
}

UtlBoolean MpOss::isWriterAttached()
{
   return (mWriter != NULL);
}

UtlBoolean MpOss::isDevCapBatch()
{
   return (mDeviceCap & DSP_CAP_BATCH);
}

UtlBoolean MpOss::isDevCapDuplex()
{
   return (mDeviceCap & DSP_CAP_DUPLEX);
}

UtlBoolean MpOss::isDevCapMmap()
{
   return (mDeviceCap & DSP_CAP_MMAP);
}

UtlBoolean MpOss::isDevCapTrigger()
{
   return (mDeviceCap & DSP_CAP_TRIGGER);
}

UtlBoolean MpOss::isNotUsed()
{
   return ((mReader == NULL) && (mWriter == NULL));
}


#endif // _MpOss_h_

