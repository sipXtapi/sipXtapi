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

#ifdef __linux__

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/soundcard.h>
#include <sys/mman.h>
#include <errno.h>

// APPLICATION INCLUDES
#include "mp/MpidOss.h"
#include "mp/MpodOss.h"
#include "os/OsTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#ifdef RTL_ENABLED // [
#include "rtl_macro.h"
#else // RTL_ENABLED ][
#define RTL_BLOCK(x)
#define RTL_WRITE(x)
#define RTL_EVENT(x,y)
#endif // RTL_ENABLED ]


//#define PRODUCE_STEREO_TO_DEV

#define SOUND_MAXBUFFER       1024


// STATIC VARIABLE INITIALIZATIONS
/// Silence buffer for dummy output
static const MpAudioSample gSilenceBuffer[SOUND_MAXBUFFER] = {0};
/// Buffer for dummy input
static MpAudioSample gTrashBuffer[SOUND_MAXBUFFER];

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Default constructor
MpOss::MpOss()
: mfdDevice(-1)
, mbReadCap(FALSE)
, mbWriteCap(FALSE)
, mReader(NULL)
, mWriter(NULL)
, mDeviceCap(0)
, mStReader(FALSE)
, mStWriter(FALSE)
, mStShutdown(FALSE)
, mModeChanged(TRUE)
, mUsedSamplesPerSec(0)
, mUsedSamplesPerFrame(0)
{
   int res;

   res = sem_init(&mSleepSem, 0, 0);
   assert(res == 0);
   res = sem_init(&mSignalSem, 0, 0);
   assert(res == 0);


   res = pthread_create(&mIoThread, NULL, soundCardIoWrapper, this);
   assert(res == 0);

   //Wait for thread startup
   sem_wait(&mSignalSem);
}

MpOss::~MpOss()
{
   int res;
   void *dummy;

   assert((mReader == NULL) && (mWriter == NULL));

   threadKill();

   res = pthread_join(mIoThread, &dummy);
   assert(res == 0);
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpOss::setInputDevice(MpidOss* pIDD)
{
   OsStatus ret = OS_FAILED;
   if (mReader)
   {
      //Input device has been already set
      return ret;
   }
   mReader = pIDD;

   if (mWriter)
   {
      //Check than one device is selected when pWriter has been set.
      int res = pIDD->compareTo(*mWriter);
      assert (res == 0);
      ret = OS_SUCCESS;
   } else {
      //Device didn't open
      ret = initDevice(*pIDD);
   }
   if (ret != OS_SUCCESS)
   {
      mReader = NULL;
   }

   return ret;
}

OsStatus MpOss::setOutputDevice(MpodOss* pODD)
{
   OsStatus ret = OS_FAILED;
   if (mWriter)
   {
      //Output device has been already set
      return ret;
   }
   mWriter = pODD;

   if (mReader)
   {
      //Check than one device is selected when pReader has been set.
      int res = pODD->compareTo(*mReader);
      assert (res == 0);
      ret = OS_SUCCESS;
   } else {
      //Device didn't open
      ret = initDevice(*pODD);
   }
   if (ret != OS_SUCCESS)
   {
      mWriter = NULL;
   }
   return ret;
}

OsStatus MpOss::freeInputDevice()
{
   OsStatus ret = OS_SUCCESS;
   if (mStReader)
   {
      //It's very bad freeing device when it is enabled
      ret = detachReader();
   }

   if (mReader != NULL)
      mReader = NULL;
   else
      ret = OS_FAILED;

   if (isNotUsed())
   {
      noMoreNeeded();
   }
   return ret;
}

OsStatus MpOss::freeOutputDevice()
{
   OsStatus ret = OS_SUCCESS;
   if (mStWriter)
   {
      //It's very bad freeing device when it is enabled
      ret = detachWriter();
   }

   if (mWriter != NULL)
   {
      mWriter = NULL;
   }
   else
   {
      ret = OS_FAILED;
   }

   if (isNotUsed())
   {
      noMoreNeeded();
   }
   return ret;
}

void MpOss::noMoreNeeded()
{
   //Free OSS device If it no longer using
   freeDevice();
   MpOssContainer::excludeWrapperFromContainer(this);
}

OsStatus MpOss::attachReader()
{
   OsStatus ret = OS_FAILED;

   UtlBoolean threadSleep = (mStReader == FALSE) && (mStWriter == FALSE);

   if (!((mReader == NULL) || (mStReader == TRUE)))
   {
      assert(mbReadCap == TRUE);
      ret = setSampleRate(mReader->mSamplesPerSec, mReader->mSamplesPerFrame);
      if (ret == OS_SUCCESS) {
         mStReader = TRUE;

         if (threadSleep) 
         {
            threadWakeUp();
         }
         else
         {
            threadIoStatusChanged();
         }
      }
   }
   return ret;
}

OsStatus MpOss::attachWriter()
{
   OsStatus ret = OS_FAILED;
   UtlBoolean threadSleep = (mStReader == FALSE) && (mStWriter == FALSE);

   if (!((mWriter == NULL) || (mStWriter == TRUE)))
   {
      assert(mbWriteCap == TRUE);
      ret = setSampleRate(mWriter->mSamplesPerSec, mWriter->mSamplesPerFrame);
      if (ret == OS_SUCCESS)
      {
         mStWriter = TRUE;
         if (threadSleep) 
         {
            threadWakeUp();
         }
         else
         {
            threadIoStatusChanged();
         }
      }
   }
   return ret;
}

OsStatus MpOss::detachReader()
{
   OsStatus ret = OS_FAILED;
   if (!((mReader == NULL) || (mStReader == FALSE)))
   {
      mStReader = FALSE;
      threadIoStatusChanged();
      ret = OS_SUCCESS;
   }
   return ret;
}

OsStatus MpOss::detachWriter()
{
   OsStatus ret = OS_FAILED;
   if (!((mWriter == NULL) || (mStWriter == FALSE)))
   {
      mStWriter = FALSE;
      threadIoStatusChanged();
      ret = OS_SUCCESS;
   }
   return ret;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
OsStatus MpOss::initDevice(const char* devname)
{
   int res;
#ifdef PRODUCE_STEREO_TO_DEV
   int stereo = 1;
#else
   int stereo = 0;
#endif
   int samplesize = 8 * sizeof(MpAudioSample);

   OsStatus ret = OS_FAILED;

   mbReadCap = TRUE;
   mbWriteCap = TRUE;

   mfdDevice = open(devname, O_RDWR);
   if (mfdDevice == -1)
   {
      char temp[1024];
      char* ptr = strerror_r(errno, temp, 1024);

      //Trying only input device
      mfdDevice = open(devname, O_RDONLY);
      if (mfdDevice == -1)
      {
         //Trying only output device
         mfdDevice = open(devname, O_WRONLY);
         if (mfdDevice == -1)
         {
            OsSysLog::add(FAC_MP, PRI_EMERG,
               "OSS: could not open %s, errno = %d (%s);"
               " *** NO SOUND! ***",
               devname, errno, (ptr) ? ptr : "");
            ret = OS_INVALID_ARGUMENT;
            return ret;
         }
         else
         {
            OsSysLog::add(FAC_MP, PRI_WARNING,
               "OSS: could not open %s in duplex mode: "
               "%s (opening output only).",
               devname, (ptr) ? ptr : "");
            mbReadCap = FALSE;
         }
      }
      else
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
            "OSS: could not open %s in duplex mode: "
            "%s (opening input only).",
            devname, (ptr) ? ptr : "");
         mbWriteCap = FALSE;
      }
   }

   // magic number, reduces latency (0x0006 dma buffers of 2 ^ 0x0008 = 256 bytes each)
   // For samplig rates 8000 to 16000 0x00040007 give better latency, but it does not
   // work with sampling rates such as 32000 or 48000.
#ifdef PRODUCE_STEREO_TO_DEV
   int fragment = 0x00060008;
#else
   int fragment = 0x00060007;
#endif
   res = ioctl(mfdDevice, SNDCTL_DSP_SETFRAGMENT, &fragment);
   if(res == -1)
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "OSS: could not set fragment size; *** NO SOUND! ***\n");
      freeDevice();
      return ret;
   }

   res = ioctl(mfdDevice, SNDCTL_DSP_SAMPLESIZE, &samplesize);
   if ((res == -1) || (samplesize != (8 * sizeof(MpAudioSample))))
   {
      res = AFMT_S16_LE;
      ioctl(mfdDevice, SNDCTL_DSP_SETFMT, &res);
   }

   // Make sure the sound card has the capabilities we need
   // FIXME: Correct this code for samplesize != 16 bits
   res = AFMT_QUERY;
   ioctl(mfdDevice, SNDCTL_DSP_SETFMT, &res);
   if (res != AFMT_S16_LE)
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "OSS: could not set sample format; *** NO SOUND! ***");
      freeDevice();
      return ret;
   }

   res = ioctl(mfdDevice, SNDCTL_DSP_STEREO, &stereo);
   if (res == -1)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "OSS: could not set single channel audio!\n");
      freeDevice();
      return ret;
   }

   res = ioctl(mfdDevice, SNDCTL_DSP_GETCAPS, &mDeviceCap);
   if (res == -1)
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "OSS: could not get capabilities; *** NO SOUND! ***!\n");
      freeDevice();
      return ret;
   }

   if (!isDevCapDuplex())
   {
      char buff[8] = {0};
      OsStatus st = doOutput(buff, 8);

      mbWriteCap = (st == OS_SUCCESS);
      mbReadCap = !mbWriteCap;


      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "OSS: device %s support only %s operations",
         devname, (mbWriteCap) ? "output" : "input");

   }

   return OS_SUCCESS;
}

OsStatus MpOss::freeDevice()
{
   OsStatus ret = OS_SUCCESS;
   //Closing device
   if (mfdDevice != -1)
   {
      int res = close(mfdDevice);
      if (res != 0)
         ret = OS_FAILED;

      mfdDevice = -1;
      mUsedSamplesPerSec = 0;
      mUsedSamplesPerFrame = 0;
   }
   return ret;
}

OsStatus MpOss::setSampleRate(unsigned samplesPerSec, unsigned samplerPerFrame)
{
   // FIXME: OSS device support only one sample rate for duplex operation
   //checking requested rate with whether configured
   if ( (((mStReader && !mStWriter) ||
      (!mStReader && mStWriter)) &&
      (mUsedSamplesPerSec > 0) &&
      (mUsedSamplesPerSec != samplesPerSec)) ||
      (mStReader && mStWriter))
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "OSS: could not set different sample speed for "
         "input and output; *** NO SOUND! ***");
      return OS_INVALID_ARGUMENT;
   } else if ((mUsedSamplesPerSec == samplesPerSec) &&
              (mUsedSamplesPerFrame == samplerPerFrame)) {
      return OS_SUCCESS;
   }

   int speed = samplesPerSec;
   int res = ioctl(mfdDevice, SNDCTL_DSP_SPEED, &speed);
   // allow .5% variance
   if ((res == -1) || (abs(speed - samplesPerSec) > (int)samplesPerSec / 200))
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "OSS: could not set sample speed; *** NO SOUND! ***");
      return OS_FAILED;
   }

   // Hmm.. If it's used sample rate may be set value returned by IOCTL..
   mUsedSamplesPerSec = samplesPerSec;
   mUsedSamplesPerFrame = samplerPerFrame;
   return OS_SUCCESS;
}

OsStatus MpOss::doInput(char* buffer, int size)
{
   //Performs reading
   int bytesToRead = size;
   int bytesReadSoFar = 0;

   while (bytesReadSoFar < bytesToRead)
   {
      int bytesJustRead = read(mfdDevice, &buffer[bytesReadSoFar],
         bytesToRead - bytesReadSoFar);

      if (bytesJustRead == -1) {
         ossReset();
         return OS_FAILED;
      }
      bytesReadSoFar += bytesJustRead;
   }
   return OS_SUCCESS;
}

OsStatus MpOss::doOutput(const char* buffer, int size)
{
   //Performs writing
   int bytesToWrite = size;
   int bytesWritedSoFar = 0;

   while (bytesWritedSoFar < bytesToWrite)
   {
      int bytesJustWrite = write(mfdDevice, &buffer[bytesWritedSoFar],
         bytesToWrite - bytesWritedSoFar);
      if (bytesJustWrite == -1) {
         ossReset();
         return OS_FAILED;
      }
      bytesWritedSoFar += bytesJustWrite;
   }
   return OS_SUCCESS;
}

void MpOss::soundIoThread()
{
   UtlBoolean bStReader;
   UtlBoolean bStWriter;
   UtlBoolean bShutdown;
   OsStatus status;

   MpAudioSample* isamples;
   const MpAudioSample* osamples;
   int frameSize = 0;
   int i;

   UtlBoolean bWakeUp = TRUE;

#ifdef PRODUCE_STEREO_TO_DEV
   int j;
   MpAudioSample imBuffer[SOUND_MAXBUFFER];
#endif

   assert(mModeChanged == TRUE);

   for (;;i++)
   {
      // To reduce latency we use flag handling instead of semaphore locking 
      while (mModeChanged)
      {
         bStReader = mStReader;
         bStWriter = mStWriter;
         bShutdown = mStShutdown;

         // Set we have caught signal
         mModeChanged = FALSE;
         if (!bStWriter && !bStReader && !bShutdown)
         {
            ossSetTrigger(false);
            // Signal we have caught modification flag  
            sem_post(&mSignalSem);
            // Wait for any IO
            sem_wait(&mSleepSem);

            if (mStShutdown) {
               return;
            }
            bWakeUp = TRUE;
         }
         else if (bShutdown)
         {
            RTL_EVENT("MpOss::io_thread", 0);
            // OSS driver killing
            ossSetTrigger(false);
            return;
         }
         else
         {
            // Signal we have caught modification flag
            sem_post(&mSignalSem);

            frameSize = mUsedSamplesPerFrame * sizeof(MpAudioSample);

            if (bStWriter || bStReader)
            {
               assert (frameSize > 0 && frameSize < SOUND_MAXBUFFER);
            }

            if (bWakeUp)
            {
               i = 0;
               ossSetTrigger(true);

               //doInput(gTrashBuffer, frameSize);
               //doOutput(gSilenceBuffer, frameSize);
               bWakeUp = FALSE;
            }
         }
      }

      // Produce heat beat for MpMediaLib
      if ((mWriter) && (!mWriter->mNotificationThreadEn))
      {
         RTL_EVENT("MpOss::io_thread", 4);
         mWriter->signalForNextFrame();
      }

      if (mbWriteCap)
      {
         osamples = (bStWriter) ? mWriter->mAudioFrame : gSilenceBuffer;
         RTL_EVENT("MpOss::io_thread", 1);
#ifdef PRODUCE_STEREO_TO_DEV
         for (j = 0; j < frameSize / 2; j++) {
            imBuffer[2*j] = imBuffer[2*j + 1] = osamples[j];
         }
         doOutput((const char*)imBuffer, frameSize * 2);
#else
         doOutput((const char*)osamples, frameSize);
#endif
      }

      if (mbReadCap)
      {
         RTL_EVENT("MpOss::io_thread", 2);
         isamples = (bStReader) ? mReader->mAudioFrame : gTrashBuffer;
#ifdef PRODUCE_STEREO_TO_DEV
         status = doInput((char*)imBuffer, frameSize * 2);
         for (j = 0; j < frameSize / 2; j++) {
            isamples[j] = (imBuffer[2*j]/2)+(imBuffer[2*j+1]/2);
         }
#else
         status = doInput((char*)isamples, frameSize);
#endif
         if ((status == OS_SUCCESS) && (bStReader))
         {
            RTL_EVENT("MpOss::io_thread", 3);
            mReader->pushFrame();
         }
      }
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */
void MpOss::threadIoStatusChanged()
{
   mModeChanged = TRUE;
   sem_wait(&mSignalSem);
}

void MpOss::threadWakeUp()
{
   mModeChanged = TRUE;
   // Wake Up
   sem_post(&mSleepSem);
   // Wait for status modification
   sem_wait(&mSignalSem);
}

void MpOss::threadKill()
{
   mStShutdown = TRUE;
   mModeChanged = TRUE;

   sem_post(&mSleepSem);
}

void* MpOss::soundCardIoWrapper(void* arg)
{
   MpOss* pOSSDW = (MpOss*)arg;

#if defined(_REALTIME_LINUX_AUDIO_THREADS) && defined(__linux__) /* [ */
   // Setup thread priority

   struct sched_param realtime;
   int res = 0;

   if(geteuid() != 0)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "_REALTIME_LINUX_AUDIO_THREADS was defined"
         " but application does not have ROOT priv.");
   }
   else
   {
      // Set the priority to the maximum allowed for the scheduling polity.
      realtime.sched_priority = sched_get_priority_max(SCHED_FIFO);
      //res = sched_setscheduler(0, SCHED_FIFO, &realtime);
      //res = pthread_setschedparam(pthread_self(), SCHED_FIFO, &realtime);
      assert(res == 0);

      // keep all memory locked into physical mem, to guarantee realtime-behavior
      res = mlockall(MCL_CURRENT|MCL_FUTURE);
      assert(res == 0);
   }
#endif /* _REALTIME_LINUX_AUDIO_THREADS ] */

   pOSSDW->soundIoThread();
   return NULL;
}

UtlBoolean MpOss::ossReset()
{
   return (ioctl(mfdDevice, SNDCTL_DSP_RESET, 0) != -1);
}

UtlBoolean MpOss::ossSetTrigger(bool turnOn)
{
   unsigned int val = ((turnOn && mbReadCap) ? PCM_ENABLE_INPUT : 0) |
                      ((turnOn && mbWriteCap) ? PCM_ENABLE_OUTPUT : 0);
   UtlBoolean res = (ioctl(mfdDevice, SNDCTL_DSP_SETTRIGGER, &val) != -1);
   if (!res)
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set OSS trigger\n");
   }
   return res;
}
#endif
