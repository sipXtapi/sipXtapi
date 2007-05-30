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

// APPLICATION INCLUDES
#include "mp/MpidOSS.h"
#include "mp/MpodOSS.h"
#include "os/OsTask.h"

#ifdef RTL_ENABLED
#include "rtl_macro.h"
#else
#define RTL_BLOCK(x)
#define RTL_WRITE(x)
#define RTL_EVENT(x,y)
#endif


#define timediff(early, late) \
        ((late.tv_sec-early.tv_sec)*1000000+(late.tv_usec-early.tv_usec))

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
#ifndef OSS_SINGLE_DEVICE
extern MpOSSDeviceWrapperContainer mOSSContainer;
#else
//MpOSSDeviceWrapper ossSingleDriver;
#endif

//#define FULLY_NONBLOCK
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
static inline void addToTimespec(struct timespec& ts, int nsec)
{
   ts.tv_nsec += nsec;
   while (ts.tv_nsec > 1000000000)
   {
      ts.tv_nsec -= 1000000000;
      ts.tv_sec++;
   }
}


//#define DEBUG_OSS_TIMERS

#ifdef DEBUG_OSS_TIMERS
#  define dossprintf(x)  do { osPrintf(x); /*fflush(stdout);*/ } while (0)
#else
#  define dossprintf(x)
#endif


/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Default constructor
MpOSSDeviceWrapper::MpOSSDeviceWrapper()
: mfdDevice(-1)
, mbReadCap(FALSE)
, mbWriteCap(FALSE)
, mUsedSamplesPerSec(0)
, pReader(NULL)
, pWriter(NULL)
, mDeviceCap(0)
, pReaderEnabled(FALSE)
, pWriterEnabled(FALSE)
, mThreadExit(FALSE)
, mbFisrtWriteCycle(FALSE)
, musecFrameTime(0)
, mFramesRead(0)
, mFramesDropRead(0)
, mFramesWritten(0)
, mFramesWrUnderruns(0)
, mReClocks(0)
, mCTimeUp(0)
, mCTimeDown(0)
{
   int res;
   dossprintf("MpOSSDeviceWrapper::MpOSSDeviceWrapper...");

   pthread_mutex_init(&mWrMutex, NULL);
   pthread_mutex_init(&mWrMutexBuff, NULL);
   pthread_mutex_init(&mWrMutexBuff1, NULL);
   pthread_mutex_init(&mNotifyBlk, NULL);
   pthread_cond_init(&mNewTickCondition, NULL);
   pthread_cond_init(&mNull, NULL);
   pthread_cond_init(&mNewDataArrived, NULL);
   pthread_cond_init(&mNewQueueFrame, NULL);
   pthread_cond_init(&mBlockCondition, NULL);
   pthread_cond_init(&mUnBlockCondition, NULL);

   pthread_mutex_lock(&mWrMutexBuff);

   res = pthread_create(&iothread, NULL, soundCardIOWrapper, this);
   assert(res == 0);

   res = pthread_create(&notifythread, NULL, soundNotifier, this);
   assert(res == 0);

   dossprintf("^^^");
   pthread_cond_wait(&mBlockCondition, &mWrMutexBuff);
   pthread_mutex_unlock(&mWrMutexBuff);

   dossprintf("ok\n");
}

MpOSSDeviceWrapper::~MpOSSDeviceWrapper()
{
   dossprintf("MpOSSDeviceWrapper::~MpOSSDeviceWrapper\n");

   assert((pReader == NULL) || (pWriter == NULL));
   int res;

   //Stopping thread
   mThreadExit = TRUE;

   //Waiting while thread terminating
   pthread_mutex_lock(&mNotifyBlk);
   pthread_cond_signal(&mNewDataArrived);
   pthread_mutex_unlock(&mNotifyBlk);

   void *dummy;
   res = pthread_join(notifythread, &dummy);
   assert(res == 0);

   res = pthread_join(iothread, &dummy);
   assert(res == 0);

   pthread_mutex_destroy(&mWrMutex);
   pthread_mutex_destroy(&mWrMutexBuff);
   pthread_mutex_destroy(&mNotifyBlk);
   pthread_cond_destroy(&mNewTickCondition);
   pthread_cond_destroy(&mNull);
   pthread_cond_destroy(&mNewDataArrived);
   pthread_cond_destroy(&mNewQueueFrame);
   pthread_cond_destroy(&mBlockCondition);
   pthread_cond_destroy(&mUnBlockCondition);
}

/* ============================ MANIPULATORS ============================== */
OsStatus MpOSSDeviceWrapper::setInputDevice(MpidOSS* pIDD)
{
   OsStatus ret = OS_FAILED;
   dossprintf("MpOSSDeviceWrapper::setInputDevice...");
   if (pReader)
   {
      //Input device has been already set
      return ret;
   }

   pReader = pIDD;

   if (pWriter)
   {
      //Check than one device is selected when pWriter has been set.
      int res = pIDD->compareTo(*pWriter);
      assert (res == 0);
      ret = OS_SUCCESS;
   } else {
      //Device didn't open
      ret = initDevice(*pIDD);
   }
   if (ret != OS_SUCCESS)
   {
      pReader = NULL;
      dossprintf("failed!\n");
      return ret;
   }
   dossprintf("ok\n");
   return ret;
}

OsStatus MpOSSDeviceWrapper::setOutputDevice(MpodOSS* pODD)
{
   OsStatus ret = OS_FAILED;
   dossprintf("MpOSSDeviceWrapper::setOutputDevice...");
   if (pWriter)
   {
      //Output device has been already set
      return ret;
   }

   pWriter = pODD;

   if (pReader)
   {
      //Check than one device is selected when pReader has been set.
      int res = pODD->compareTo(*pReader);
      assert (res == 0);
      ret = OS_SUCCESS;
   } else {
      //Device didn't open
      ret = initDevice(*pODD);
   }
   if (ret != OS_SUCCESS)
   {
      pWriter = NULL;
      dossprintf("failed!\n");
      return ret;
   }
   dossprintf("ok\n");
   return ret;
}

OsStatus MpOSSDeviceWrapper::freeInputDevice()
{
   OsStatus ret = OS_SUCCESS;
   dossprintf("MpOSSDeviceWrapper::freeInputDevice...");
   if (pReaderEnabled)
   {
      //It's very bad freeing device when it is enabled
      ret = detachReader();
   }

   if (pReader != NULL)
      pReader = NULL;
   else
      ret = OS_FAILED;

   if (isNotUsed())
   {
      noMoreNeeded();
   }

   if (ret == OS_SUCCESS)
   {
      dossprintf("ok\n");
   }
   else
   {
      dossprintf("failed!\n");
   }
   return ret;
}

OsStatus MpOSSDeviceWrapper::freeOutputDevice()
{
   OsStatus ret = OS_SUCCESS;
   dossprintf("MpOSSDeviceWrapper::freeOutputDevice...");
   if (pWriterEnabled)
   {
      //It's very bad freeing device when it is enabled
      ret = detachWriter();
   }

   if (pWriter != NULL)
   {
      pWriter = NULL;
   }
   else
   {
      ret = OS_FAILED;
   }

   if (isNotUsed())
   {
      noMoreNeeded();
   }

   if (ret == OS_SUCCESS)
   {
      dossprintf("ok\n");
   }
   else
   {
      dossprintf("failed!\n");
   }

   return ret;
}

void MpOSSDeviceWrapper::noMoreNeeded()
{
   //Free OSS device If it no longer using
   freeDevice();
#ifndef OSS_SINGLE_DEVICE
   //Remove this wrapper from container
   //mOSSContainer.excludeFromContainer(this);
   MpOSSDeviceWrapperContainer::excludeWrapperFromContainer(this);
#endif
}

OsStatus MpOSSDeviceWrapper::attachReader()
{
   dossprintf("OSS: AttachReader\n");
   OsStatus ret = OS_FAILED;
   pthread_mutex_lock(&mWrMutexBuff);
   pthread_mutex_lock(&mWrMutexBuff1);

   if (!((pReader == NULL) || (pReaderEnabled == TRUE)))
   {
      assert(mbReadCap == TRUE);

      ret = setSampleRate(pReader->mSamplesPerSec);
      if (ret == OS_SUCCESS) {
         pReaderEnabled = TRUE;
         soundIOThreadBlocking(pWriterEnabled);

         ossSetTrigger(pReaderEnabled, pWriterEnabled);
         dossprintf("A");
         char silence[10];
         doInput(silence, 10);
         dossprintf("D");
         soundIOThreadAfterBlocking();
      }
      dossprintf("ok\n");
   }
   pthread_mutex_unlock(&mWrMutexBuff1);
   pthread_mutex_unlock(&mWrMutexBuff);

   return ret;
}

OsStatus MpOSSDeviceWrapper::attachWriter()
{
   dossprintf("OSS: AttachWriter..");
   OsStatus ret = OS_FAILED;
   pthread_mutex_lock(&mWrMutexBuff);
   pthread_mutex_lock(&mWrMutexBuff1);

   if (!((pWriter == NULL) || (pWriterEnabled == TRUE)))
   {    
      assert(mbWriteCap == TRUE);

      ret = setSampleRate(pWriter->mSamplesPerSec);
      if (ret == OS_SUCCESS)
      {
         pWriterEnabled = TRUE;
         mbFisrtWriteCycle = TRUE;
         soundIOThreadBlocking(pReaderEnabled);

         ossSetTrigger(pReaderEnabled, pWriterEnabled);
         soundIOThreadAfterBlocking();
      }
      dossprintf("ok\n");
   }
   pthread_mutex_unlock(&mWrMutexBuff1);
   pthread_mutex_unlock(&mWrMutexBuff);
   
   return ret;
}

OsStatus MpOSSDeviceWrapper::detachReader()
{
   dossprintf("OSS: DetachReader..");
   OsStatus ret = OS_FAILED;
   pthread_mutex_lock(&mWrMutexBuff);
   pthread_mutex_lock(&mWrMutexBuff1);

   if (!((pReader == NULL) || (pReaderEnabled == FALSE)))
   {
      pReaderEnabled = FALSE;
      soundIOThreadBlocking(TRUE);

      ossSetTrigger(pReaderEnabled, pWriterEnabled);

      soundIOThreadAfterBlocking();
      ret = OS_SUCCESS;   
      dossprintf("ok\n");
   }

   pthread_mutex_unlock(&mWrMutexBuff1);
   pthread_mutex_unlock(&mWrMutexBuff);
   
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "OSS: Reader Statistic: Captured %d, Dropped %d",
                 mFramesRead, mFramesDropRead);
   return ret;
}

OsStatus MpOSSDeviceWrapper::detachWriter()
{
   dossprintf("OSS: DetachWriter...");
   OsStatus ret = OS_FAILED;
   pthread_mutex_lock(&mWrMutexBuff);
   pthread_mutex_lock(&mWrMutexBuff1);

   if (!((pWriter == NULL) || (pWriterEnabled == FALSE)))
   {
      pWriterEnabled = FALSE;
      soundIOThreadBlocking(TRUE);

      ossSetTrigger(pReaderEnabled, pWriterEnabled);

      soundIOThreadAfterBlocking();
      ret = OS_SUCCESS;   
      dossprintf("ok\n");
   }

   pthread_mutex_unlock(&mWrMutexBuff1);
   pthread_mutex_unlock(&mWrMutexBuff);
   
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "OSS: Writer Statistic: Played %d; Underruns %d; Reclocks %d; "
                 "TUps %d; TDowns %d",
                 mFramesWritten, mFramesWrUnderruns, mReClocks, mCTimeUp, mCTimeDown);
   dossprintf("ok\n");

   return ret;
}

void MpOSSDeviceWrapper::soundIOThreadLockUnlock(bool bLock)
{
   // Neither reading nor writing ops
   if (bLock)
   {
      //Blocking IO thread because no IO operation can occurs
      dossprintf("BW");
      pthread_cond_wait(&mBlockCondition, &mWrMutexBuff);
      dossprintf("BLK+");
   }
   else
   {
      //Unblocking IO thread
      dossprintf("BLK-");
      pthread_mutex_lock(&mNotifyBlk);
      pthread_cond_signal(&mNewDataArrived);
      pthread_mutex_unlock(&mNotifyBlk);
      dossprintf("-W");

      pthread_cond_wait(&mUnBlockCondition, &mWrMutexBuff1);
      dossprintf("-K");
   }
}

void  MpOSSDeviceWrapper::soundIOThreadAfterBlocking()
{
   if (pReaderEnabled || pWriterEnabled)
   {
      soundIOThreadLockUnlock(FALSE);
   }
   else
   {
      mUsedSamplesPerSec = 0;
   }
}

void  MpOSSDeviceWrapper::soundIOThreadBlocking(UtlBoolean bIsWorkingNow)
{
   if (bIsWorkingNow)
   {
      soundIOThreadLockUnlock(TRUE);
   }
}

UtlBoolean MpOSSDeviceWrapper::ossSetTrigger(bool read, bool write)
{
   unsigned int val = ((read) ? PCM_ENABLE_INPUT : 0) |
                      ((write) ? PCM_ENABLE_OUTPUT : 0);
   UtlBoolean res = (ioctl(mfdDevice, SNDCTL_DSP_SETTRIGGER, &val) != -1);
   if (!res)
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set OSS trigger\n");
   }
   return res;
}


UtlBoolean MpOSSDeviceWrapper::ossGetBlockSize(unsigned& block)
{
   return (ioctl(mfdDevice, SNDCTL_DSP_GETBLKSIZE, &block) == -1);
}

UtlBoolean MpOSSDeviceWrapper::ossGetODelay(unsigned& delay)
{
   return (ioctl(mfdDevice, SNDCTL_DSP_GETODELAY, &delay) == -1);
}

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
OsStatus MpOSSDeviceWrapper::initDevice(const char* devname)
{
   int res;
   int stereo = OSS_SOUND_STEREO;
   int samplesize = 8 * sizeof(MpAudioSample);

   OsStatus ret = OS_FAILED;

   mbReadCap = TRUE;
   mbWriteCap = TRUE;

   mfdDevice = open(devname, O_RDWR
#ifdef FULLY_NONBLOCK
                            | O_NONBLOCK
#endif
                   );
   if (mfdDevice == -1)
   {
      char temp[1024];
      char* ptr = strerror_r(errno, temp, 1024);


      //Trying only input device
      mfdDevice = open(devname, O_RDONLY
#ifdef FULLY_NONBLOCK
                      | O_NONBLOCK
#endif
                      );
      if (mfdDevice == -1)
      {

         //Trying only output device
         mfdDevice = open(devname, O_WRONLY
#ifdef FULLY_NONBLOCK
                         | O_NONBLOCK
#endif
                         );
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

   if ((mbReadCap == TRUE) && (mbWriteCap == TRUE))
   {
      res = ioctl(mfdDevice, SNDCTL_DSP_SETDUPLEX, 0);
      if (res == -1)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "OSS: could not set full duplex");
      }
   }
   // magic number, reduces latency (0x0004 dma buffers of 2 ^ 0x0008 = 256 bytes each)
   int fragment = 0x00040007;
   //int fragment = 0x00080007;
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
      ioctl(mfdDevice ,SNDCTL_DSP_SETFMT, &res);
   }

   // Make sure the sound card has the capabilities we need
   // FIXME: Correct this code for samplesize != 16 bits
   res = AFMT_QUERY;
   ioctl(mfdDevice ,SNDCTL_DSP_SETFMT, &res);
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
                    "OSS: could not set single channel audio");
      freeDevice();
      return ret;
   }

   res = ioctl(mfdDevice, SNDCTL_DSP_GETCAPS, &mDeviceCap);
   if (res == -1)
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not get capabilities; *** NO SOUND! ***");
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

   if (isDevCapBatch())
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "OSS: Sound card has internal buffers (increases latency)");
   }

   if (mbWriteCap)
   {
      if (!getOSpace(fullOSpace))
      {
          OsSysLog::add(FAC_MP, PRI_EMERG,
                        "OSS: this device dosen't support OSPACE ioctl!"
                        " *** NO SOUND! ***");
          freeDevice();
          return ret;
      }
   }
   else
   {
      if (!getISpace(fullOSpace))
      {
          OsSysLog::add(FAC_MP, PRI_EMERG,
                        "OSS: this device dosen't support ISPACE ioctl!"
                        " *** NO SOUND! ***");
          freeDevice();
          return ret;
      }
   }

   lastISpace = 0;
   lastOSpace = fullOSpace;

   return OS_SUCCESS;
}

OsStatus MpOSSDeviceWrapper::freeDevice()
{
   OsStatus ret = OS_SUCCESS;
   //Closing device
   if (mfdDevice != -1)
   {
      int res = close(mfdDevice);
      if (res != 0)
         ret = OS_FAILED;

      mfdDevice = -1;
   }
   return ret;
}

OsStatus MpOSSDeviceWrapper::setSampleRate(unsigned samplesPerSec)
{
   // FIXME: OSS device support only one sample rate for duplex operation
   //checking requested rate with whether configured
   if ( (((pReaderEnabled && !pWriterEnabled) ||
      (!pReaderEnabled && pWriterEnabled)) &&
      (mUsedSamplesPerSec > 0) &&
      (mUsedSamplesPerSec != samplesPerSec)) ||
      (pReaderEnabled && pWriterEnabled))
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "OSS: could not set different sample speed for "
         "input and output; *** NO SOUND! ***");
      return OS_INVALID_ARGUMENT;
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
   return OS_SUCCESS;
}

OsStatus MpOSSDeviceWrapper::doInput(char* buffer, int size)
{
   //Perform reading
   int bytesToRead = size;
   int bytesReadSoFar = 0;

   while (bytesReadSoFar < bytesToRead)
   {
      int bytesJustRead = read(mfdDevice, &buffer[bytesReadSoFar],
         bytesToRead - bytesReadSoFar);

      if (bytesJustRead == -1) {
         dossprintf("OSS::doInput failed!\n");
         return OS_FAILED;
      }
      bytesReadSoFar += bytesJustRead;
   }
   return OS_SUCCESS;
}

OsStatus MpOSSDeviceWrapper::doOutput(char* buffer, int size)
{
   //Perform writing
   int bytesToWrite = size;
   int bytesWritedSoFar = 0;

   while (bytesWritedSoFar < bytesToWrite)
   {
      int bytesJustWrite = write(mfdDevice, &buffer[bytesWritedSoFar],
         bytesToWrite - bytesWritedSoFar);

      if (bytesJustWrite == -1) {
         dossprintf("OSS::doOutput failed!\n");
         return OS_FAILED;
      }
      bytesWritedSoFar += bytesJustWrite;
   }
   return OS_SUCCESS;
}


UtlBoolean MpOSSDeviceWrapper::getISpace(int& ispace)
{
   audio_buf_info bi;
   int res = ioctl(mfdDevice, SNDCTL_DSP_GETISPACE, &bi);

   if (res != -1) {
      ispace = bi.bytes;
      lastISpace = ispace;
      RTL_EVENT("OSS::ISpace", ispace);
      return TRUE;
   }
   return FALSE;
}

UtlBoolean MpOSSDeviceWrapper::getOSpace(int& ospace)
{
   audio_buf_info bi;
   int res = ioctl(mfdDevice, SNDCTL_DSP_GETOSPACE, &bi);

   if (res != -1) {
      ospace = bi.bytes;
      lastOSpace = ospace;
      RTL_EVENT("OSS::OSpace", ospace);
      return TRUE;
   }
   return FALSE;
}

void MpOSSDeviceWrapper::performOnlyRead()
{
   OsStatus status;
   MpAudioSample* frm;
   frm = pReader->getBuffer();
   //Do reader staff
#ifdef FULLY_NONBLOCK
   for (;;)
   {
      status = doInput((char*)frm, sizeof(MpAudioSample) * pReader->mSamplesPerFrame);
      if (status == OS_SUCCESS)
      {
         pReader->pushFrame(frm);
         mFramesRead++;
      }
      else
      {
         OsTask::delay(1);
         if (errno == EAGAIN)
            continue;
         //else TODO something!!!
      }
      break;
   }
#else
   status = doInput((char*)frm, sizeof(MpAudioSample) * pReader->mSamplesPerFrame);
   if (status == OS_SUCCESS)
   {
      pReader->pushFrame(frm);
      mFramesRead++;
   }
#endif
}

#define USEC_CORRECT_STEP   1

UtlBoolean MpOSSDeviceWrapper::performReaderNoDelay()
{
   if (!pReaderEnabled)
      return FALSE;

   UtlBoolean bRes = FALSE;
   int inSizeReady;
   getISpace(inSizeReady);

   if (inSizeReady >= (int)(sizeof(MpAudioSample) * pReader->mSamplesPerFrame))
   {
      MpAudioSample* frm;
      frm = pReader->getBuffer();
      //Do reader staff
      OsStatus status = doInput((char*)frm, sizeof(MpAudioSample) * pReader->mSamplesPerFrame);

      if (status == OS_SUCCESS)
      {
         pReader->pushFrame(frm);
         mFramesRead++;

         bRes = TRUE;
      }
   }
   
   return bRes;
}

int MpOSSDeviceWrapper::getDMAPlayingQueue()
{
   /*      Full_DMA_Buffer = fullOSpace
   *   +------------------------------------------+
   *  [(Recording_data)(Playing_data) free_space  ]
   *   \_____________/               \___________/
   *    getISpace()                   getOSpace()
   */

   int cIsp, cOsp;
   getISpace(cIsp);
   getOSpace(cOsp);

   int iPlQueue = (fullOSpace - cIsp - cOsp);
   RTL_EVENT("MpOSSDeviceWrapper::iPlQueue", iPlQueue);
   return iPlQueue;
}

void MpOSSDeviceWrapper::doSkipWrite(int silenceSize)
{
   assert (silenceSize > 0);
   {
      char silenceData[silenceSize];
      memset(silenceData, 0, silenceSize);
      doOutput(silenceData, silenceSize);
   }
   mFramesWrUnderruns++;

   // Informing writer to correct next frame time
   pWriter->skipFrame();
}

void MpOSSDeviceWrapper::performWithWrite(UtlBoolean bReaderEn)
{
   int res;
   int bTimedOut = 0;
   struct timespec tmCurr;

   RTL_BLOCK("MpOSSDeviceWrapper::pww");

   if (mbFisrtWriteCycle)
   {
      dossprintf("FIRST!!!");
      musecFrameTime = (1000000 * pWriter->mSamplesPerFrame) / pWriter->mSamplesPerSec;
      mbFisrtWriteCycle = FALSE;

      unsigned silenceSize;
      int ospace;
      res = getOSpace(ospace);
      silenceSize = ospace;
      assert(res != 0);

      if (pWriter->mSamplesPerFrame * sizeof(MpAudioSample)  < silenceSize)
      {
         silenceSize = pWriter->mSamplesPerFrame * sizeof(MpAudioSample) ;
      }

      {
         struct timeval before;
         struct timeval after;
         char silenceData[silenceSize];
         int ospace2;

         gettimeofday(&before, NULL);
         memset(silenceData, 0, silenceSize);
         doOutput(silenceData, silenceSize);
         gettimeofday(&after, NULL);

         int delta = timediff(before, after);
         res = getOSpace(ospace2);

         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "Timediff: %d usec; Size=%d; OSpace=%d (%d)",
                       delta, silenceSize, ospace, ospace2);
      }

      int iSpace;
      getISpace(iSpace);
      if (iSpace > 0)
      {
          char dummyData[iSpace];
          doInput(dummyData, iSpace);
      }

      // Ensure buffer would not emptying
      musecJitterCorrect = 0;
      clock_gettime(CLOCK_REALTIME, &mWrTimeStarted);
   }
   else
   {
      res = clock_gettime(CLOCK_REALTIME, &tmCurr);
      assert(res == 0);
      int usec_delta = ((tmCurr.tv_sec - mWrTimeStarted.tv_sec) * 1000000
                        + (tmCurr.tv_nsec - mWrTimeStarted.tv_nsec) / 1000);

      if (abs(usec_delta) > ((int)musecFrameTime * 2))
      {
         mWrTimeStarted.tv_sec = tmCurr.tv_sec;
         mWrTimeStarted.tv_nsec = tmCurr.tv_nsec;

         mReClocks++;

         // Ensure buffer would not emptying
         musecJitterCorrect = 0;
      }
      else
      {
         RTL_EVENT("MpOSSDeviceWrapper::pww", 2);
         int currecntOSpace;
         int correctOSpace = 0;
         getOSpace(currecntOSpace);
         if (bReaderEn)
         {
            /*
            getISpace(correctOSpace);
            int pSize = pReader->mSamplesPerFrame * sizeof(MpAudioSample);
            if (correctOSpace > 3 * pSize)
            {
               // Drop packet
               char silenceData[pSize];
               doInput(silenceData, pSize);
               doInput(silenceData, pSize);
               getISpace(correctOSpace);
               getOSpace(currecntOSpace);
               mFramesDropRead += 2;
            }
           */
         }

         if (((currecntOSpace /*- correctOSpace*/))
              < (int)( pWriter->mSamplesPerFrame * sizeof(MpAudioSample)))
         {
            // Buffer is fulling
            musecJitterCorrect += USEC_CORRECT_STEP;
            mCTimeUp++;

            RTL_EVENT("OSS::mucorrect", 10000 + musecJitterCorrect);
         }
         else if ((fullOSpace - correctOSpace - currecntOSpace)
                   < int(2 * pWriter->mSamplesPerFrame * sizeof(MpAudioSample)))
         {
            // Buffer is emptying
            musecJitterCorrect -= USEC_CORRECT_STEP;
            mCTimeDown++;

            RTL_EVENT("OSS::mucorrect", 10000 + musecJitterCorrect);
         }
      }
   }

   // Assuming 1000us is a maximum device delay
   int nsDeviceDelay = 1000 * 1000;
   if (pReaderEnabled)
   {
      nsDeviceDelay = 1200 * 1000;
   }
   RTL_EVENT("MpOSSDeviceWrapper::pww", 3);


   unsigned bnFrameSize;
   MpAudioSample* samples;

   addToTimespec(mWrTimeStarted, musecFrameTime * 1000 - nsDeviceDelay +
      musecJitterCorrect * 1000);

   if (bReaderEn)
   {
      UtlBoolean bRes;
      do
      {
         bRes = performReaderNoDelay(); 
      } while ((bRes == TRUE) && (lastISpace > pReader->mSamplesPerFrame * sizeof(MpAudioSample)));
   }

   if (!pWriter->mNotificationThreadEn)
   {
      pWriter->signalForNextFrame();
   }
   // Trying get samples already stored in queue
   samples = pWriter->popFrame(bnFrameSize);
   if (samples == NULL)
   {
      res = pthread_cond_timedwait(&mNewQueueFrame, &mWrMutex, &mWrTimeStarted);
      if (res == ETIMEDOUT)
      {
         bTimedOut = 1;
      }
      else
      {
         assert(res == 0);
         samples = pWriter->popFrame(bnFrameSize);
         assert(samples != NULL);
      }
   }

   if (bReaderEn)
      performReaderNoDelay();

   if (!bTimedOut)
   {
      RTL_EVENT("MpOSSDeviceWrapper::pww", 4);
      doOutput((char*)samples, bnFrameSize);
      mFramesWritten++;
   }
   else
   {
      RTL_EVENT("MpOSSDeviceWrapper::pww", 5);
      int silenceSize = pWriter->mSamplesPerFrame * sizeof(MpAudioSample);
      if (getDMAPlayingQueue() < (silenceSize * 13) / 10)
      {
         RTL_BLOCK("MpOSSDeviceWrapper::underruns");
         doSkipWrite(silenceSize);
      }
      else
      {
         RTL_BLOCK("MpOSSDeviceWrapper::skip");
         OsTask::delay(1);
      }
   }
   addToTimespec(mWrTimeStarted, nsDeviceDelay);
   RTL_EVENT("MpOSSDeviceWrapper::pww", 6);

#ifdef FULLY_NONBLOCK
   if (!pWriter->mNotificationThreadEn)
   {
      RTL_BLOCK("MpOSSDeviceWrapper::pww time correction");
      res = pthread_cond_timedwait(&mNull, &mWrMutex, &mWrTimeStarted);
      assert (res == ETIMEDOUT);
   }
#endif

   RTL_EVENT("MpOSSDeviceWrapper::pww", 7);
}


void MpOSSDeviceWrapper::soundIOThread()
{
   int res;
   int bLock = TRUE;

   UtlBoolean bOldReader = FALSE;
   UtlBoolean bOldWriter = FALSE;

   UtlBoolean bModeChanged = FALSE;

   UtlBoolean bLocalReader = FALSE;
   UtlBoolean bLocalWriter = FALSE;

   for (;;)
   {
      bLocalReader = pReaderEnabled;
      bLocalWriter = pWriterEnabled;
      bModeChanged = (bLocalReader != bOldReader) || (bLocalWriter != bOldWriter);

      if (!bLocalReader && !bLocalWriter)
      {
         bLock = TRUE;
         dossprintf("LOCK\n");
      }

      if ((bLock) || (bModeChanged))
      {
         dossprintf("i");
         pthread_mutex_lock(&mWrMutexBuff);
         dossprintf("p");
         pthread_cond_signal(&mBlockCondition);
         pthread_mutex_unlock(&mWrMutexBuff);

         //Wait for thread blocking there
         //If current thread not using (reader & writer are disabled)
         // then blocking occure there
         res = pthread_cond_wait(&mNewDataArrived, &mNotifyBlk);
         dossprintf("u");

         // Updating flags
         if (bLock) {
            bOldReader = pReaderEnabled;
            bOldWriter = pWriterEnabled;
         }
         else
         {
            bOldReader = bLocalReader;
            bOldWriter = bLocalWriter;
         }
         bLock = FALSE;

         dossprintf("$");

         pthread_mutex_lock(&mWrMutexBuff1);
         pthread_cond_signal(&mUnBlockCondition);
         pthread_mutex_unlock(&mWrMutexBuff1);

         if (mThreadExit) {
            dossprintf("WEWExit flag recived\n");
            return;
         }
         dossprintf("#");
         continue;
      }

      // Begin IO
      if (bLocalReader && !bLocalWriter)
      {
         dossprintf("R");
         performOnlyRead();
      }
      else if (bLocalWriter)
      {
         dossprintf("W");
         performWithWrite(bLocalReader);
      }

      bOldReader = bLocalReader;
      bOldWriter = bLocalWriter;
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void* MpOSSDeviceWrapper::soundCardIOWrapper(void* arg)
{
   MpOSSDeviceWrapper* pOSSDW = (MpOSSDeviceWrapper*)arg;

   // Setup thread priority

#if defined(_REALTIME_LINUX_AUDIO_THREADS) && defined(__linux__) /* [ */
   {
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
         res = sched_setscheduler(0, SCHED_FIFO, &realtime);
         //res = pthread_setschedparam(pthread_self(), SCHED_FIFO, &realtime);
         assert(res == 0);

         // keep all memory locked into physical mem, to guarantee realtime-behaviour
         res = mlockall(MCL_CURRENT|MCL_FUTURE);
         assert(res == 0);
      }
   }
#endif /* _REALTIME_LINUX_AUDIO_THREADS ] */
   pthread_mutex_lock(&pOSSDW->mWrMutex);
   pthread_mutex_lock(&pOSSDW->mNotifyBlk);

   pOSSDW->soundIOThread();

   pthread_mutex_unlock(&pOSSDW->mWrMutex);
   pthread_mutex_unlock(&pOSSDW->mNotifyBlk);
   return NULL;
}


void MpOSSDeviceWrapper::soundNotify()
{
   struct timespec tspec;

   int firstWrite = 1;
   int bNotify = 0;

   int res;
   struct sched_param realtime;
   realtime.sched_priority = 1;
   res = pthread_setschedparam(pthread_self(), SCHED_FIFO, &realtime);
   //assert(res == 0);

   pthread_mutex_t notifyMutex = PTHREAD_MUTEX_INITIALIZER;
   pthread_mutex_lock(&notifyMutex);

   for (;;)
   {
      if (mThreadExit) {
         dossprintf("DFEExit flag recived\n");
         pthread_mutex_unlock(&notifyMutex);
         return;
      }

      bNotify = (  pWriterEnabled
                && pWriter
                && pWriter->mNotificationThreadEn
                && pWriter->isNotificationNeeded());
      if (bNotify)
      {
         RTL_BLOCK("MpOSSDeviceWrapper::notifyThread");
         if (firstWrite) {
            clock_gettime(CLOCK_REALTIME, &tspec);
            firstWrite = 0;
         }
         pWriter->signalForNextFrame();

         int delta_delay;
         delta_delay =  ((1000000 * pWriter->mSamplesPerFrame) / pWriter->mSamplesPerSec) * 1000
                        + musecJitterCorrect * 1000;

         pthread_cond_timedwait(&mNull, &notifyMutex, &tspec);

         addToTimespec(tspec, delta_delay);
      } else {
         firstWrite = 1;
         OsTask::delay(1);
      }
   }
}


void* MpOSSDeviceWrapper::soundNotifier(void *arg)
{
   MpOSSDeviceWrapper* pOSSDW = (MpOSSDeviceWrapper*)arg;
   pOSSDW->soundNotify();
   return NULL;
}


#endif // __linux__
