//
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpAlsa_h_
#define _MpAlsa_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>

#include <sys/types.h>
#include <sys/soundcard.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm_plugin.h>

// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriver.h"
#include "mp/MpOutputDeviceDriver.h"
#include "mp/MpAlsaContainer.h"
#include "utl/UtlVoidPtr.h"

class MpidAlsa;
class MpodAlsa;

/**
*  @brief Wrapper for ALSA device that use file descriptor to communicate.
*/
class MpAlsa : public UtlVoidPtr
{
   friend class MpidAlsa;
   friend class MpodAlsa;
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// @brief Constructor
   MpAlsa();

   /// @brief Destructor
   ~MpAlsa();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// @brief Connect MpidAlsa class driver
   OsStatus setInputDevice(MpidAlsa* pIDD);
   /// @brief Connect MpodAlsa class driver
   OsStatus setOutputDevice(MpodAlsa* pODD);

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
   /// @brief get list of available input or output Alsa device names
   static int getDeviceNames(UtlContainer& deviceNames, bool capture);
   /**
    * @param deviceNames output - list of device names
    * @param capture input - true - get input device names
    *                        false - get output device names
    * @returns count of device names
    */

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

   /// @brief Inquire if ALSA device is free, i.e. neither input nor output is connected
   inline UtlBoolean isNotUsed();

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   UtlBoolean mbReadCap;        ///< Device is able to play audio
   UtlBoolean mbWriteCap;       ///< Device is able to capture audio

   MpidAlsa* mReader;          ///< Input driver
   MpodAlsa* mWriter;          ///< Output driver

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

   /// @brief Pre initializations for ALSA device
   OsStatus initDevice(const char* devname, int samplesPerSecond);
   /// @brief Final initializations for ALSA device
   OsStatus initDeviceFinal(unsigned samplesPerSec, unsigned samplerPerFrame);

   /// @brief Because ALSA device works in duplex mode we must ensure that
   /// input and output driver use one sample rate and perfrom final init
   OsStatus setSampleRate(unsigned samplesPerSec, unsigned samplerPerFrame);

   /// @brief Perform input operation of ALSA device
   OsStatus doInput(char* buffer, int size);
   /// @brief Perform output operation of ALSA device
   OsStatus doOutput(const char* buffer, int size);

   /// @brief Perform input operation of ALSA device with siutable resampler
   OsStatus doInputRs(MpAudioSample* buffer, unsigned size);
   /// @brief Perform output operation of ALSA device  with siutable resampler
   OsStatus doOutputRs(const MpAudioSample* buffer, unsigned size);

   /// @brief Deinitialization and freeing sequences
   void noMoreNeeded();

   /// @brief Thread subroutine
   void soundIoThread();

   // MER additions for ALSA devices
   snd_pcm_t*  pPcmOut;
   snd_pcm_t*  pPcmIn;
   int writeBufSize;
   int readBufSize;

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   void threadIoStatusChanged();
   void threadWakeUp();
   void threadKill();

   UtlBoolean alsaSetTrigger(bool turnOn);
   UtlBoolean alsaReset();

   /// @brief Thread subroutine
   static void* soundCardIoWrapper(void* arg);
   int alsaSetupPcmDevice(const char* devname, bool capture, unsigned& suggestedRate);
   int alsaSetupWarning(const char* devname, int err, bool cap, const char* pLogString);
};

/* ============================ INLINE METHODS ============================ */

UtlBoolean MpAlsa::isDeviceValid()
{
   return (pPcmIn != NULL && pPcmOut != NULL);
}

UtlBoolean MpAlsa::isReaderAttached()
{
   return (mReader != NULL);
}

UtlBoolean MpAlsa::isWriterAttached()
{
   return (mWriter != NULL);
}

UtlBoolean MpAlsa::isDevCapBatch()
{
   return (mDeviceCap & DSP_CAP_BATCH);
}

UtlBoolean MpAlsa::isDevCapDuplex()
{
   return (mDeviceCap & DSP_CAP_DUPLEX);
}

UtlBoolean MpAlsa::isDevCapMmap()
{
   return (mDeviceCap & DSP_CAP_MMAP);
}

UtlBoolean MpAlsa::isDevCapTrigger()
{
   return (mDeviceCap & DSP_CAP_TRIGGER);
}

UtlBoolean MpAlsa::isNotUsed()
{
   return ((mReader == NULL) && (mWriter == NULL));
}


#endif // _MpAlsa_h_

