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

#ifndef _MpOSSDeviceWrapper_h_
#define _MpOSSDeviceWrapper_h_
// OSS-specific code
#ifdef __linux__

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>

#include <sys/types.h>
#include <sys/soundcard.h>


// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriver.h"
#include "mp/MpOutputDeviceDriver.h"

// DEFINES
#define OSS_SOUND_STEREO        0

// Uncomment follow sentence for single OSS device mode
// #define OSS_SINGLE_DEVICE

#ifndef OSS_SINGLE_DEVICE
#include "mp/MpOSSDeviceWrapperContainer.h"
#include "utl/UtlVoidPtr.h"
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpidOSS;
class MpodOSS;


/**
*  @brief Wrapper for OSS device that use file descriptor to communicate.
*/
class MpOSSDeviceWrapper
#ifndef OSS_SINGLE_DEVICE
                            : public UtlVoidPtr
#endif
{
   friend class MpidOSS;
   friend class MpodOSS;
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @brief Constructor
   MpOSSDeviceWrapper();

     /// @brief Destructor
   ~MpOSSDeviceWrapper();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @brief Connect MpidOSS class driver
   OsStatus setInputDevice(MpidOSS* pIDD);
     /// @brief Connect MpodOSS class driver
   OsStatus setOutputDevice(MpodOSS* pODD);

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
   UtlBoolean isDeviceValid() { return (mfdDevice != -1); }

     /// @brief Inquire if the input device is connected
   UtlBoolean isReaderAttached() { return (pReader != NULL); }
     /// @brief Inquire if the output device is connected
   UtlBoolean isWriterAttached() { return (pWriter != NULL); }

   UtlBoolean isDevCapBatch()   { return (mDeviceCap & DSP_CAP_BATCH); }
   UtlBoolean isDevCapDuplex()  { return (mDeviceCap & DSP_CAP_DUPLEX); }
   UtlBoolean isDevCapMmap()    { return (mDeviceCap & DSP_CAP_MMAP); }
   UtlBoolean isDevCapTrigger() { return (mDeviceCap & DSP_CAP_TRIGGER); }

    /// @brief Inquire if OSS device is free, i.e. neither input nor output is connected
   UtlBoolean isNotUsed() { return ((pReader == NULL) && (pWriter == NULL)); }

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   int mfdDevice;               ///< The fd of the POSIX device (e.g. /dev/dsp)
   UtlBoolean mbReadCap;
   UtlBoolean mbWriteCap;

   unsigned mUsedSamplesPerSec; ///< Used samples rate either for input or
                                ///< output

   MpidOSS* pReader;            ///< Input driver
   MpodOSS* pWriter;            ///< Output driver

   int mDeviceCap;              ///< Device capabilities

   UtlBoolean pReaderEnabled;   ///< Input device is enabled
   UtlBoolean pWriterEnabled;   ///< Output device is enabled

   pthread_t iothread;          ///< Internal IO thread
   pthread_t notifythread;      ///< Internal notify thread

   UtlBoolean mThreadExit;      ///< Thread exit flag

   int fullOSpace;              ///< Size of assigned DMA buffer

   pthread_mutex_t mWrMutex;
   pthread_mutex_t mWrMutexBuff;
   pthread_mutex_t mWrMutexBuff1;
   pthread_mutex_t mNotifyBlk;
   pthread_cond_t mNewTickCondition;
   pthread_cond_t mNull;
   pthread_cond_t mNewDataArrived;
   pthread_cond_t mNewQueueFrame;
   pthread_cond_t mBlockCondition;
   pthread_cond_t mUnBlockCondition;
   struct timespec mWrTimeStarted;
   UtlBoolean mbFisrtWriteCycle;
   unsigned musecFrameTime;     ///< Period of frame in usec
   int musecJitterCorrect;

   /// Statistics
   unsigned mFramesRead;
   unsigned mFramesDropRead;
   unsigned mFramesWritten;
   unsigned mFramesWrittenBlocked;
   unsigned mFramesWrUnderruns;
   unsigned mReClocks;
   unsigned mCTimeUp;
   unsigned mCTimeDown;

    /// @brief Returning current count of MpAudioSample's would be playing
   int getDMAPlayingQueue();

     /// @brief Common initializations for OSS device
   OsStatus initDevice(const char* devname);
     /// @brief Free OSS device
   OsStatus freeDevice();

     /// @brief Because OSS device works in duplex mode we must ensure that
     /// input and output driver use one sample rate
   OsStatus setSampleRate(unsigned samplesPerSec);

     /// @brief Perform input operation of OSS device
   OsStatus doInput(char* buffer, int size);
     /// @brief Perform output operation of OSS device
   OsStatus doOutput(char* buffer, int size);

     /// @brief Deinitialization and freeing sequences
   void noMoreNeeded();

     /// @brief Get maximum size of data can be read without blocking
   UtlBoolean getISpace(int& ispace);
     /// @brief Get maximum size of data can be written without blocking
   UtlBoolean getOSpace(int& ospace);

   void performOnlyRead();
   void performWithWrite(UtlBoolean bReaderEn);
   
     /// @brief Perform reading and pushing frame if sufficient data are in buffer
   UtlBoolean performReaderNoDelay();
     /**<
     *  @returns TRUE if frame has been pushed
     *  @returns FALSE if are not sufficient data in buffer
     */

     /// @brief Thread subroutine
   void soundIOThread();
   void soundNotify();

     /// @brief Call when underruns occur
   void doSkipWrite(int silenceSize);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int lastISpace; ///< Previous value of ISpace during last IO
   int lastOSpace; ///< Previous value of OSpace during last IO

   void soundIOThreadLockUnlock(bool bLock);
   void soundIOThreadAfterBlocking();
   void soundIOThreadBlocking(UtlBoolean bIsWorkingNow);

   UtlBoolean ossSetTrigger(bool read, bool write);
   UtlBoolean ossGetBlockSize(unsigned& block);
   UtlBoolean ossGetODelay(unsigned& delay);

     /// @brief Thread subroutine
   static void* soundCardIOWrapper(void* arg);

   static void* soundNotifier(void *arg);

};

/* ============================ INLINE METHODS ============================ */


#endif // __linux__
#endif // _MpOSSDeviceWrapper_h_
