// 
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
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

// Preferable output latency im msec
#define OSS_LATENCY_LENGTH          30

// Set 1 to use stereo output or 0 for mono
#define PRODUCE_STEREO_TO_DEV       1

// Should enough for storing 192kHz 16bit STEREO 20ms frame
#define SOUND_MAXBUFFER             16384


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
, mResamplerBuffer(NULL)
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
   if (mResamplerBuffer != NULL)
   {
      delete[] mResamplerBuffer;
      mResamplerBuffer = NULL;
   }

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
               "%s (opening output only) file descriptor: %d.",
               devname, (ptr) ? ptr : "", mfdDevice);
            mbReadCap = FALSE;
         }
      }
      else
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
            "OSS: could not open %s in duplex mode: "
            "%s (opening input only) file descriptor: %d.",
            devname, (ptr) ? ptr : "", mfdDevice);
         mbWriteCap = FALSE;
      }
   }
   else
   {
       OsSysLog::add(FAC_MP, PRI_DEBUG,
               "OSS opened device: %s file descriptor: %d",
               devname, mfdDevice);
   }


   res = ioctl(mfdDevice, SNDCTL_DSP_SAMPLESIZE, &samplesize);
   if ((res == -1) || (samplesize != (8 * sizeof(MpAudioSample))))
   {
      res = AFMT_S16_LE;
      ioctl(mfdDevice, SNDCTL_DSP_SETFMT, &res);
   }

   // Make sure the sound card has the capabilities we need
   // Correct this code for samplesize != 16 bits
   res = AFMT_QUERY;
   ioctl(mfdDevice, SNDCTL_DSP_SETFMT, &res);
   if (res != AFMT_S16_LE)
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "OSS: could not set sample format; *** NO SOUND! ***");
      freeDevice();
      return ret;
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

OsStatus MpOss::setSampleRate(unsigned samplesPerSec, unsigned samplesPerFrame)
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
   }
   else if ((mUsedSamplesPerSec == samplesPerSec) &&
           (mUsedSamplesPerFrame == samplesPerFrame))
   {
      return OS_SUCCESS;
   }

   return initDeviceFinal(samplesPerSec, samplesPerFrame);
}

OsStatus MpOss::initDeviceFinal(unsigned samplesPerSec, unsigned samplesPerFrame)
{
   int stereo = PRODUCE_STEREO_TO_DEV;
   int res = ioctl(mfdDevice, SNDCTL_DSP_STEREO, &stereo);
   if (res == -1)
   {
      if (stereo)
      {
         stereo = 0;
         res = ioctl(mfdDevice, SNDCTL_DSP_STEREO, &stereo);
      }

      if (res == -1)
      {
         OsSysLog::add(FAC_MP, PRI_EMERG,
            "OSS: could not set needed channel audio count!\n");
         return OS_FAILED;
      }
      else
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
            "OSS: could not set stereo, using mono!\n");
      }
   }

   res = ioctl(mfdDevice, SNDCTL_DSP_GETCAPS, &mDeviceCap);
   if (res == -1)
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "OSS: could not get capabilities; *** NO SOUND! ***!\n");
      return OS_FAILED;
   }

   int duplex = isDevCapDuplex();

   // Try to find optimal internal OSS buffer size to satisfy latency constraint
   const int minSegmentBits = 5;
   
   for (int segmentBits = 8; segmentBits >= minSegmentBits; segmentBits--)
   {
      int bytesInOneFrame = 2 * samplesPerFrame * ((stereo) ? 2 : 1);
      int frmsize = samplesPerFrame * 1000 / samplesPerSec;
      int buffering = OSS_LATENCY_LENGTH * bytesInOneFrame / frmsize  + bytesInOneFrame;
      if (duplex)
      {
         // Increase buffer by 2.25 for duplex operations
         // Use addition quater because input and output can't operate on the same buffer
         buffering =  (buffering * 9) >> 2;
      }

      int reqfragsize = (1 << segmentBits);
      int cnt = buffering / reqfragsize + ((buffering % reqfragsize) ? 1 : 0);

      int minimumfrag = 2 + (buffering / (bytesInOneFrame/2));

      // We must have sufficient amount of buffers to minimize latemcy
      if  ((segmentBits > minSegmentBits) && (cnt < minimumfrag))
         continue;
      else if ((segmentBits == minSegmentBits) && (cnt < minimumfrag))
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
           "OSS: Minimum segment count has been limited to: %d giving a few amount of buffers: %d, while recommended count is %d. May be poor sound.",
           minSegmentBits, cnt, minimumfrag);
      }

      int fragment = (cnt << 16) | segmentBits;
      res = ioctl(mfdDevice, SNDCTL_DSP_SETFRAGMENT, &fragment);
      if(res == -1)
      {
         OsSysLog::add(FAC_MP, PRI_EMERG,
                       "OSS: could not set fragment size %x; *** NO SOUND! ***\n",
                       fragment);

         return OS_FAILED;
      }

      int fragsize = 0;
      res = ioctl(mfdDevice, SNDCTL_DSP_GETBLKSIZE, &fragsize);
      if (res == -1)
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
            "OSS: could not get fragment size.\n");
         break;
      }

      if (fragsize != reqfragsize)
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
            "OSS: could not set fragment size %x, using %x. Sound quality may be poor\n",
            fragment, reqfragsize);
      }
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "OSS: Use fragment size %x; Bytes in a frame:%d; Internal buffer size:%d; Minimum buffers count:%d;\n",
         fragment, bytesInOneFrame, fragsize, minimumfrag);
   
      break;
   }

   int speed = samplesPerSec;
   int wspeed = speed;
   res = ioctl(mfdDevice, SNDCTL_DSP_SPEED, &speed);
   if ((res == -1) || (speed != wspeed))
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "OSS: could not set sample speed; *** NO SOUND! ***");
      return OS_FAILED;
   }

   mUsedSamplesPerSec = samplesPerSec;
   mUsedSamplesPerFrame = samplesPerFrame;
   mStereoOps = stereo;

   if (!duplex)
   {
      char buff[8] = {0};
      OsStatus st = doOutput(buff, 8);

      mbWriteCap = (st == OS_SUCCESS);
      mbReadCap = !mbWriteCap;


      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "OSS: device support only %s operations",
         (mbWriteCap) ? "output" : "input");
   }

   //Initialize resampler
   if (mStereoOps)
   {
      int buffSize = ((mStereoOps) ? 2 : 1) * samplesPerFrame;
      assert(mResamplerBuffer == NULL);
      mResamplerBuffer = new MpAudioSample[buffSize];

      assert(buffSize * 2 <=  SOUND_MAXBUFFER);
   }

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

OsStatus MpOss::doInputRs(MpAudioSample* buffer, unsigned size)
{
   int sz = 2 * size * ((mStereoOps) ? 2 : 1);
   if (mStereoOps)
   {
      OsStatus status;
      unsigned j;
      assert (size <= mUsedSamplesPerFrame);

      status = doInput((char*)mResamplerBuffer, sz);
      for (j = 0; j < size; j++)
      {
         buffer[j] = (mResamplerBuffer[2 * j]/2) +
                     (mResamplerBuffer[2 * j + 1]/2);
      }
      return status;
   }
   else
   {
      return doInput((char*)buffer, sz);
   }

}

OsStatus MpOss::doOutputRs(const MpAudioSample* buffer, unsigned size)
{
   int sz = 2 * size * ((mStereoOps) ? 2 : 1);
   if (mStereoOps)
   {
      unsigned j;
      assert (size <= mUsedSamplesPerFrame);

      for (j = 0; j < size; j++)
      {
         mResamplerBuffer[2*j] = mResamplerBuffer[2*j + 1] = buffer[j];
      }
      return doOutput((const char*)mResamplerBuffer, sz);
   }
   else
   {
      return doOutput((const char*)buffer, sz);
   }
}

void MpOss::soundIoThread()
{
   UtlBoolean bStReader;
   UtlBoolean bStWriter;
   UtlBoolean bShutdown;
   OsStatus status;

   MpAudioSample* isamples;
   const MpAudioSample* osamples;
   int i;

   UtlBoolean bWakeUp = TRUE;
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
            RTL_EVENT("MpOss::io_thread", 10);
            ioctl(mfdDevice, SNDCTL_DSP_SYNC, NULL);

            RTL_EVENT("MpOss::io_thread", 11);
            ossSetTrigger(false);
            
            ossReset();

            sem_post(&mSignalSem);

            RTL_EVENT("MpOss::io_thread", 12);
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
            
            ossReset();
            return;
         }
         else
         {
            // Signal we have caught modification flag
            sem_post(&mSignalSem);
            RTL_EVENT("MpOss::io_thread", 15);

            int frameSize = ((mStereoOps) ? 2 : 1) * mUsedSamplesPerFrame * 2;

            if (bStWriter || bStReader)
            {
               assert (frameSize > 0 && frameSize < SOUND_MAXBUFFER);
            }

            if (bWakeUp)
            {
               unsigned precharge;
               i = 0;
               RTL_EVENT("MpOss::io_thread", -1);
               ossSetTrigger(true);
               RTL_EVENT("MpOss::io_thread", 16);
               if (mbWriteCap)
               {
                  // Plays out silence to set requested buffer deep
                  precharge = OSS_LATENCY_LENGTH * mUsedSamplesPerSec / 1000;

                  for (unsigned k = 0; k < precharge; k += mUsedSamplesPerFrame)
                  {
                     unsigned sz = precharge - k;
                     if (sz > mUsedSamplesPerFrame)
                         sz = mUsedSamplesPerFrame;

                     //Use small data request unless OSS may be confused
                     if (mbReadCap)
                     {
                        doInputRs(gTrashBuffer, mUsedSamplesPerFrame >> 3);
                        RTL_EVENT("MpOss::io_thread", 18);
                     }                     

                     doOutputRs(gSilenceBuffer, sz);
                     RTL_EVENT("MpOss::io_thread", 17);
                     
                  }
               }

               RTL_EVENT("MpOss::io_thread", 19);
               if ((mbWriteCap) && (mbReadCap))
               {
                  //Compensates output shift due previous data requests
                  doOutputRs(gSilenceBuffer, (mUsedSamplesPerFrame >> 2));
               }

               bWakeUp = FALSE;
               RTL_EVENT("MpOss::io_thread", 16);
            }
         }
      }

      // Produce heart beat for MpMediaLib
      if ((mWriter) && (!mWriter->mNotificationThreadEn))
      {
         RTL_EVENT("MpOss::io_thread", 4);
         mWriter->signalForNextFrame();
      }

      if (mbReadCap)
      {
         RTL_EVENT("MpOss::io_thread", 2);
         isamples = (bStReader) ? mReader->mAudioFrame : gTrashBuffer;
         status = doInputRs(isamples, mUsedSamplesPerFrame);

         if ((status == OS_SUCCESS) && (bStReader))
         {
            RTL_EVENT("MpOss::io_thread", 3);
            mReader->pushFrame();
         }
      }

      if (mbWriteCap)
      {
         osamples = (bStWriter) ? mWriter->mAudioFrame : gSilenceBuffer;
         RTL_EVENT("MpOss::io_thread", 1);
         status = doOutputRs(osamples, mUsedSamplesPerFrame);
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

   //struct sched_param realtime;
   int res = 0;

   if(geteuid() != 0)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "_REALTIME_LINUX_AUDIO_THREADS was defined"
         " but application does not have ROOT priv.");
   }
   else
   {
      // TODO: fix this
      // Set the priority to the maximum allowed for the scheduling polity.
      //realtime.sched_priority = sched_get_priority_max(SCHED_FIFO);
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
      const char* errorString = strerror(errno); 
      OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set OSS trigger fd=%d. ioctl returned: %d (%s)\n",
                    mfdDevice, errno, errorString);
   }
   return res;
}
#endif
