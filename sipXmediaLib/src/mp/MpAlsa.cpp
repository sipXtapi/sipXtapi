// 
// Copyright (C) 2007-2018 SIPez LLC.  All rights reserved.
// 
// $$ 
/////////////////////////////////////////////////////////////////////////////// 

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
#include <alsa/asoundlib.h>
#include <alsa/pcm_plugin.h>

// APPLICATION INCLUDES
#include <mp/MpidAlsa.h>
#include <mp/MpodAlsa.h>
#include <os/OsTask.h>
#include <os/OsSysLog.h>

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
#define ALSA_LATENCY_LENGTH          30

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
MpAlsa::MpAlsa()
: mbReadCap(FALSE)
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
, pPcmOut(NULL)
, pPcmIn(NULL)
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

MpAlsa::~MpAlsa()
{
   int res;
   void *dummy;

   if(mReader)
   {
     freeInputDevice();
   }
   if(mWriter)
   {
     freeOutputDevice();
   }
   assert((mReader == NULL) && (mWriter == NULL));

   threadKill();

   res = pthread_join(mIoThread, &dummy);
   assert(res == 0);
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpAlsa::setInputDevice(MpidAlsa* pIDD)
{
   OsStatus ret = OS_FAILED;
   OsSysLog::add(FAC_MP, PRI_INFO,"MpAlsa::setInputDevice %s",
		 pIDD->data());
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
      OsSysLog::add(FAC_MP, PRI_INFO,"MpAlsa::setInputDevice mWriter %s",
		    mWriter->data());

      assert (res == 0);
      ret = OS_SUCCESS;
   } else {
       OsSysLog::add(FAC_MP, PRI_INFO,"MpAlsa::setInputDevice initDevice");
      //Device didn't open
      ret = initDevice(*pIDD, pIDD->getSamplesPerSec());
   }
   if (ret != OS_SUCCESS)
   {
      mReader = NULL;
   }

   return ret;
}

OsStatus MpAlsa::setOutputDevice(MpodAlsa* pODD)
{
   OsStatus ret = OS_FAILED;
   OsSysLog::add(FAC_MP, PRI_INFO,"MpAlsa::setOutputDevice %s",
		 pODD->data());
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
      OsSysLog::add(FAC_MP, PRI_INFO,"MpAlsa::setOutputDevice mReader %s",
		    mReader->data());
      assert (res == 0);
      ret = OS_SUCCESS;
   } else {
      //Device didn't open
       OsSysLog::add(FAC_MP, PRI_INFO,"MpAlsa::setOuputDevice initDevice");
      ret = initDevice(*pODD, pODD->getSamplesPerSec());
   }
   if (ret != OS_SUCCESS)
   {
      mWriter = NULL;
   }
   return ret;
}

OsStatus MpAlsa::freeInputDevice()
{
   OsStatus ret = OS_SUCCESS;
   // Need to detach reader (noitify read/writer thread)
   // before closing down the pcm
   if (mStReader)
   {
      //It's very bad freeing device when it is enabled
      ret = detachReader();
   }

   if (mReader != NULL)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
          "MpAlsa::freeInputDevice closing ALSA input stream");
      int closeRet = snd_pcm_close(pPcmIn);
      {
          OsSysLog::add(FAC_MP, PRI_DEBUG,
              "snd_pcm_close(pPcmIn=%p) input returned: %d (%s)",
              pPcmIn, closeRet, snd_strerror(closeRet));
      }
      pPcmIn = NULL;

      OsSysLog::add(FAC_MP, PRI_DEBUG,
          "MpAlsa::freeInputDevice setting mReader to NULL");
      mReader = NULL;
   }
   else
      ret = OS_FAILED;

   if (isNotUsed())
   {
      noMoreNeeded();
   }
   return ret;
}

OsStatus MpAlsa::freeOutputDevice()
{
   OsStatus ret = OS_SUCCESS;
   // Need to detach writer (noitify read/writer thread)
   // before closing down the pcm
   if (mStWriter)
   {
      //It's very bad freeing device when it is enabled
      ret = detachWriter();
   }

   if (mWriter != NULL)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
          "MpAlsa::freeOutputDevice closing ALSA output stream");
      int closeRet = snd_pcm_close(pPcmOut);
      {
          OsSysLog::add(FAC_MP, PRI_DEBUG,
              "snd_pcm_close(pPcmOut=%p) output returned: %d (%s)",
              pPcmOut, closeRet, snd_strerror(closeRet));
      }
      pPcmOut = NULL;

      OsSysLog::add(FAC_MP, PRI_DEBUG,
          "MpAlsa::freeOutputDevice setting mWriter to NULL");
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

void MpAlsa::noMoreNeeded()
{
   if (mResamplerBuffer != NULL)
   {
      delete[] mResamplerBuffer;
      mResamplerBuffer = NULL;
   }

   //Free ALSA device If it no longer using
   MpAlsaContainer::excludeWrapperFromContainer(this);
}

OsStatus MpAlsa::attachReader()
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

OsStatus MpAlsa::attachWriter()
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

OsStatus MpAlsa::detachReader()
{
   OsStatus ret = OS_FAILED;
   if (!((mReader == NULL) || (mStReader == FALSE)))
   {
      mStReader = FALSE;
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MpAlsa::detachReader mStReader = FALSE");
      threadIoStatusChanged();
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MpAlsa::detachReader threadIoStatusChanged");
      ret = OS_SUCCESS;
   }
   return ret;
}

OsStatus MpAlsa::detachWriter()
{
   OsStatus ret = OS_FAILED;
   if (!((mWriter == NULL) || (mStWriter == FALSE)))
   {
      mStWriter = FALSE;
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MpAlsa::detachWriter mStWriter = FALSE");
      threadIoStatusChanged();
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MpAlsa::detachWriter threadIoStatusChanged");
      ret = OS_SUCCESS;
   }
   return ret;
}
#define BITS_PER_SAMPLE 16

/* //////////////////////////// ACCESSORS ///////////////////////////////// */


int MpAlsa::getDeviceNames(UtlContainer& deviceNames, bool capture)
{
    int deviceCount = 0;
    void** hints = NULL;
    // Get array of hints about sound iterfaces
    int hintCount = snd_device_name_hint(-1, "pcm", &hints);
    if(hintCount >= 0)
    {
        void** aHint = hints;
        // hints is a NULL terminated array of pointers
        while( *aHint != NULL)
        {
            // Looking for:
            // capture == true: Input device names
            // capture == false: Output device names
            char* ioSense = snd_device_name_get_hint(*aHint, "IOID");
            // Note: ipSense == NULL means input and output
            if(ioSense == NULL || strcmp(ioSense, capture ? "Input" : "Output") == 0)
            {
                char* name = snd_device_name_get_hint(*aHint, "NAME");
                if(name)
                {
                    // Add the name to the list
                    UtlString* newDeviceName = new UtlString(name); 
                    deviceNames.insert(newDeviceName);

                    if(OsSysLog::willLog(FAC_MP, PRI_DEBUG))
                    {
                        char* description = snd_device_name_get_hint(*aHint, "DESC");
                        OsSysLog::add(FAC_MP, PRI_DEBUG,
                                      "MpAlsa::getDeviceNames device[%d]: %s\n%s\n%s",
                                      deviceCount,
                                      name,
                                      description,
                                      ioSense ? ioSense : "Input and Output");
                        free(description);
                        description = NULL;
                    }

                    deviceCount++;
                    free(name);
                    name = NULL;
                }
            }
            if(ioSense)
            {
                free(ioSense);
                ioSense = NULL;
            }

            // next hint, NULL if no more
            aHint++;
        }

        snd_device_name_free_hint(hints);
    }
    else
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "MpAlsa::getDeviceNames snd_device_name_hint returned: %d (%s)",
                      hintCount,
                      snd_strerror(hintCount));
    }

    return(deviceCount);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
OsStatus MpAlsa::initDevice(const char* devname, int samplesPerSecond)
{
#if 0
   int res;
   int samplesize = 8 * sizeof(MpAudioSample);
#endif
    OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MpAlsa::initDevice(%s, %d) this: %p",
        devname,
        samplesPerSecond,
        this);
    unsigned suggestedRate = samplesPerSecond;
    OsStatus ret = OS_FAILED;

   mbReadCap = TRUE;
   mbWriteCap = TRUE;
   int ret1 = -1;
   int capture = 0;
   ret1 = alsaSetupPcmDevice(devname, capture, suggestedRate); 
   if (ret1 != 0)
   {
       mbWriteCap = FALSE;
   }
   capture = 1;
   ret1 = alsaSetupPcmDevice(devname, capture, suggestedRate); 
   if (ret1 != 0)
   {
       mbReadCap = FALSE;
   }

   if ((mbReadCap == FALSE) && (mbWriteCap == FALSE))
   {
       OsSysLog::add(FAC_MP, PRI_EMERG,
		     "ALSA: could not open %s, for input or output"
		     " *** NO SOUND! ***",
		     devname);
       ret = OS_INVALID_ARGUMENT;
       return ret;
   }
   else if ((mbReadCap == FALSE) && (mbWriteCap == TRUE))
   {
       OsSysLog::add(FAC_MP, PRI_WARNING,
	  "ALSA: could not open %s in duplex mode: "
	  "(opening playback only).",
	  devname);
   }
   else if ((mbReadCap == TRUE) && (mbWriteCap == FALSE))
   {
       OsSysLog::add(FAC_MP, PRI_WARNING,
	  "ALSA: could not open %s in duplex mode: "
	  "(opening capture only).",
	  devname);
   }
   else
   {
       OsSysLog::add(FAC_MP, PRI_INFO,
	  "ALSA: opened %s in duplex mode: ",
	  devname);

   }
   snd_output_t* pStdout = NULL;
   ret1 = snd_output_stdio_attach(&pStdout, stdout, 0);
#if 0
   snd_pcm_status_t* pStat;
   snd_pcm_status_alloca(&pStat);

   ret1 = snd_pcm_status(pPcmOut, pStat);
   if (ret1 != 0)
   {
       writeBufSize = 0;
   }
   else
   {
       writeBufSize = snd_pcm_status_get_avail(pStat) * (BITS_PER_SAMPLE / 8);
   }
   snd_pcm_status_free(pStat);

   snd_pcm_status_alloca(&pStat);
   ret1 = snd_pcm_status(pPcmIn, pStat);
   if (ret1 != 0)
   {
       readBufSize = 0;
   }
   else
   {
       readBufSize = snd_pcm_status_get_delay(pStat) * (BITS_PER_SAMPLE / 8);
   }
#endif
   return OS_SUCCESS;
}

OsStatus MpAlsa::setSampleRate(unsigned samplesPerSec, unsigned samplesPerFrame)
{
   // FIXME: ALSA device support only one sample rate for duplex operation
   //checking requested rate with whether configured
   if ( (((mStReader && !mStWriter) ||
      (!mStReader && mStWriter)) &&
      (mUsedSamplesPerSec > 0) &&
      (mUsedSamplesPerSec != samplesPerSec)) ||
      (mStReader && mStWriter))
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "ALSA: could not set different sample speed for "
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

// MER Fix up for ALSA device
OsStatus MpAlsa::initDeviceFinal(unsigned samplesPerSec, unsigned samplesPerFrame)
{
#if 0
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
            "ALSA: could not set needed channel audio count!\n");
         return OS_FAILED;
      }
      else
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
            "ALSA: could not set stereo, using mono!\n");
      }
   }

   res = ioctl(mfdDevice, SNDCTL_DSP_GETCAPS, &mDeviceCap);
   if (res == -1)
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "ALSA: could not get capabilities; *** NO SOUND! ***!\n");
      return OS_FAILED;
   }

   int duplex = isDevCapDuplex();

   // Try to find optimal internal ALSA buffer size to satisfy latency constraint
   const int minSegmentBits = 5;
   
   for (int segmentBits = 8; segmentBits >= minSegmentBits; segmentBits--)
   {
      int bytesInOneFrame = 2 * samplesPerFrame * ((stereo) ? 2 : 1);
      int frmsize = samplesPerFrame * 1000 / samplesPerSec;
      int buffering = ALSA_LATENCY_LENGTH * bytesInOneFrame / frmsize  + bytesInOneFrame;
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
           "ALSA: Minimum segment count has been limited to: %d giving a few amount of buffers: %d, while recommended count is %d. May be poor sound.",
           minSegmentBits, cnt, minimumfrag);
      }

      int fragment = (cnt << 16) | segmentBits;
      res = ioctl(mfdDevice, SNDCTL_DSP_SETFRAGMENT, &fragment);
      if(res == -1)
      {
         OsSysLog::add(FAC_MP, PRI_EMERG,
                       "ALSA: could not set fragment size %x; *** NO SOUND! ***\n",
                       fragment);

         return OS_FAILED;
      }

      int fragsize = 0;
      res = ioctl(mfdDevice, SNDCTL_DSP_GETBLKSIZE, &fragsize);
      if (res == -1)
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
            "ALSA: could not get fragment size.\n");
         break;
      }

      if (fragsize != reqfragsize)
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
            "ALSA: could not set fragment size %x, using %x. Sound quality may be poor\n",
            fragment, reqfragsize);
      }
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "ALSA: Use fragment size %x; Bytes in a frame:%d; Internal buffer size:%d; Minimum buffers count:%d;\n",
         fragment, bytesInOneFrame, fragsize, minimumfrag);
   
      break;
   }

   int speed = samplesPerSec;
   int wspeed = speed;
   res = ioctl(mfdDevice, SNDCTL_DSP_SPEED, &speed);
   if ((res == -1) || (speed != wspeed))
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
         "ALSA: could not set sample speed; *** NO SOUND! ***");
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
         "ALSA: device support only %s operations",
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
#else
   mUsedSamplesPerSec = samplesPerSec;
   mUsedSamplesPerFrame = samplesPerFrame;
   mStereoOps = 0;
    if (snd_pcm_state(pPcmIn) == SND_PCM_STATE_PREPARED)
    {
	if (snd_pcm_start(pPcmIn) != 0)
	{
	    OsSysLog::add(FAC_MP, PRI_INFO, "MpAlsa::initDeviceFinal could not start pPcmIn");
	}
    }
#endif
   return OS_SUCCESS;
}
// MER for the alsa device size is actually the number of samples per frame
OsStatus MpAlsa::doInput(char* buffer, int samplesToRead)
{
   //Performs reading
   int tmpSamplesToRead = samplesToRead;
   int samplesReadSoFar = 0;
   bool loopBailFlag = false;
   while (!loopBailFlag && (samplesReadSoFar < tmpSamplesToRead))
   {
       int sampsToRead = tmpSamplesToRead - samplesReadSoFar;
       char* tmpBuffer = &buffer[samplesReadSoFar];
       if (!(sampsToRead == 0 || tmpBuffer))
       {
	   OsSysLog::add(FAC_MP, PRI_INFO, "MpAlsa::doInput should assert");
       }

       int samplesJustRead = 0;

       // This is a bit of a hack, but I cannot seem to groc enough of the state
       // machine to prevent from getting here when pPcmIn is NULL after close
       if(pPcmIn)
       {
           samplesJustRead = snd_pcm_readi(pPcmIn, tmpBuffer,sampsToRead);
       }
       else
       {
           loopBailFlag = true;
       }

       if(pPcmIn == NULL)
       {
	   OsSysLog::add(FAC_MP, PRI_INFO, "MpAlsa::doInput snd_pcm_readi skipped, pPcmIn NULL");
       }
       else if (samplesJustRead == 0)
       {
	   OsSysLog::add(FAC_MP, PRI_INFO, "MpAlsa::doInput snd_pcm_readi returned 0");
       }
       else if (samplesJustRead < 0) 
       {
	   switch (samplesJustRead) 
	   {
	   case -EAGAIN:
	       samplesJustRead = 0;
	       usleep(5000);
	       break;
	   case -EBADFD:
	   case -ENOTTY:
	   case -ENODEV:
	       // Audio device has been removed
//	       readErrorClose(samplesJustRead, TRUE, __LINE__);
	       loopBailFlag = true;
	       break;
	   default:
//	       bool dbgLog = OsSysLog::willLog(FAC_MP, PRI_DEBUG);
	       
//               if (dbgLog)
                   OsSysLog::add(FAC_MP, PRI_DEBUG, "ALSA:snd_pcm_readi returned %d",
                                 samplesJustRead);

               samplesJustRead = snd_pcm_recover(pPcmIn, samplesJustRead, 0);
       
               // If we still have an error after trying to recover, reset the device
               if (samplesJustRead < 0)
               {
//                   readErrorClose(samplesJustRead, TRUE, __LINE__);
                   loopBailFlag = true;
               }
               break;
           }
       }
       samplesReadSoFar += samplesJustRead;
   }
   if (loopBailFlag)
   {
       return OS_FAILED;
   }
   return OS_SUCCESS;
}
// MER for the alsa device size is actually the number of samples per frame
OsStatus MpAlsa::doOutput(const char* buffer, int size)
{
   //Performs writing
   int samplesToWrite = size;
   int samplesWritedSoFar = 0;

   // MER this should do snd_pcm_writei()
   while (samplesWritedSoFar < samplesToWrite)
   {
      int samplesJustWritten = snd_pcm_writei(pPcmOut, &buffer[samplesWritedSoFar],
					      samplesToWrite - samplesWritedSoFar);
      if (samplesJustWritten == 0)
      {
           OsSysLog::add(FAC_MP, PRI_DEBUG, "ALSA:snd_pcm_writei returned 0");
      }
      else if (samplesJustWritten < 0) 
      {             
	  OsSysLog::add(FAC_MP, PRI_DEBUG, "ALSA:snd_pcm_writei(%p,%p,%d) returned %d %s",
                        pPcmOut, 
                        &buffer[samplesWritedSoFar],
                        samplesToWrite - samplesWritedSoFar,
			samplesJustWritten,
                        snd_strerror(samplesJustWritten));
//	  if (!isWriteDeviceReady())
//	      break;
	  samplesJustWritten = snd_pcm_recover(pPcmOut, samplesJustWritten, 0);
      }
#ifdef TEST_PRINT
      else
      {
           OsSysLog::add(FAC_MP, PRI_DEBUG, 
                         "ALSA:snd_pcm_writei wrote: %d/%d",
                         samplesJustWritten,
                         samplesToWrite);
      }
#endif
      // still have somekind of error, close and bail
      if (samplesJustWritten < 0)
      {
	  // First close our write device so the reader doesn't have to
//	  closeDevice(false);
	  // This will tell our reader to close the device when it can
//	  writeError = samplesJustWritten;   
	  break;
      }
      samplesWritedSoFar += samplesJustWritten;
   }
   return OS_SUCCESS;
}

// MER for the alsa device size is actually the number of samples per frame
OsStatus MpAlsa::doInputRs(MpAudioSample* buffer, unsigned size)
{
   if (mStereoOps)
   {
      OsStatus status;
      unsigned j;
      assert (size <= mUsedSamplesPerFrame);

      status = doInput((char*)mResamplerBuffer, size);
      for (j = 0; j < size; j++)
      {
         buffer[j] = (mResamplerBuffer[2 * j]/2) +
                     (mResamplerBuffer[2 * j + 1]/2);
      }
      return status;
   }
   else
   {
      return doInput((char*)buffer, size);
   }

}

// MER for the alsa device size is actually the number of samples per frame
OsStatus MpAlsa::doOutputRs(const MpAudioSample* buffer, unsigned size)
{
   if (mStereoOps)
   {
      unsigned j;
      assert (size <= mUsedSamplesPerFrame);

      for (j = 0; j < size; j++)
      {
         mResamplerBuffer[2*j] = mResamplerBuffer[2*j + 1] = buffer[j];
      }
      return doOutput((const char*)mResamplerBuffer, size);
   }
   else
   {
      return doOutput((const char*)buffer, size);
   }
}

void MpAlsa::soundIoThread()
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
            RTL_EVENT("MpAlsa::io_thread", 11);
            alsaSetTrigger(false);
            
            alsaReset();

            sem_post(&mSignalSem);

            RTL_EVENT("MpAlsa::io_thread", 12);
            // Wait for any IO
            sem_wait(&mSleepSem);

            if (mStShutdown) {
               return;
            }
            bWakeUp = TRUE;
         }
         else if (bShutdown)
         {
            RTL_EVENT("MpAlsa::io_thread", 0);
            // ALSA driver killing
            alsaSetTrigger(false);
            
            alsaReset();
            return;
         }
         else
         {
            // Signal we have caught modification flag
            sem_post(&mSignalSem);
            RTL_EVENT("MpAlsa::io_thread", 15);

            int frameSize = ((mStereoOps) ? 2 : 1) * mUsedSamplesPerFrame * 2;

            if (bStWriter || bStReader)
            {
               assert (frameSize > 0 && frameSize < SOUND_MAXBUFFER);
            }

            if (bWakeUp)
            {
               OsSysLog::add(FAC_MP, PRI_DEBUG,
                             "MpAlsa::soundIoThread bWakeUp");
               unsigned precharge;
               i = 0;
               RTL_EVENT("MpAlsa::io_thread", -1);
               alsaSetTrigger(true);
               RTL_EVENT("MpAlsa::io_thread", 16);
               if (mbWriteCap)
               {
                  // Plays out silence to set requested buffer deep
                  precharge = ALSA_LATENCY_LENGTH * mUsedSamplesPerSec / 1000;

                  for (unsigned k = 0; k < precharge; k += mUsedSamplesPerFrame)
                  {
                     unsigned sz = precharge - k;
                     if (sz > mUsedSamplesPerFrame)
                         sz = mUsedSamplesPerFrame;

                     //Use small data request unless ALSA may be confused
                     if (mbReadCap)
                     {
                        doInputRs(gTrashBuffer, mUsedSamplesPerFrame);
                        RTL_EVENT("MpAlsa::io_thread", 18);
                     }                     

                     doOutputRs(gSilenceBuffer, sz);
                     RTL_EVENT("MpAlsa::io_thread", 17);
                     
                  }
               }

               RTL_EVENT("MpAlsa::io_thread", 19);
               if ((mbWriteCap) && (mbReadCap))
               {
                  //Compensates output shift due previous data requests
                  doOutputRs(gSilenceBuffer, (mUsedSamplesPerFrame));
               }

               bWakeUp = FALSE;
               RTL_EVENT("MpAlsa::io_thread", 16);
            }
         }
      }

      // Produce heart beat for MpMediaLib
      if ((mWriter) && (!mWriter->mNotificationThreadEn))
      {
         RTL_EVENT("MpAlsa::io_thread", 4);
         mWriter->signalForNextFrame();
      }

      if (mbReadCap)
      {
         RTL_EVENT("MpAlsa::io_thread", 2);
         isamples = (bStReader) ? mReader->mAudioFrame : gTrashBuffer;
         status = doInputRs(isamples, mUsedSamplesPerFrame);

         if ((status == OS_SUCCESS) && (bStReader))
         {
            RTL_EVENT("MpAlsa::io_thread", 3);
            mReader->pushFrame();
         }
      }

      if (mbWriteCap)
      {
         osamples = (bStWriter) ? mWriter->mAudioFrame : gSilenceBuffer;
         RTL_EVENT("MpAlsa::io_thread", 1);
         status = doOutputRs(osamples, mUsedSamplesPerFrame);
      }

   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */
void MpAlsa::threadIoStatusChanged()
{
   mModeChanged = TRUE;
   sem_wait(&mSignalSem);
}

void MpAlsa::threadWakeUp()
{
   mModeChanged = TRUE;
   // Wake Up
   sem_post(&mSleepSem);
   // Wait for status modification
   sem_wait(&mSignalSem);
}

void MpAlsa::threadKill()
{
   mStShutdown = TRUE;
   mModeChanged = TRUE;

   sem_post(&mSleepSem);
}

void* MpAlsa::soundCardIoWrapper(void* arg)
{
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpAlsa::soundCardIoWrapper start MpAlsa: %p",
                arg);
   MpAlsa* pALSADW = (MpAlsa*)arg;

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

   pALSADW->soundIoThread();
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpAlsa::soundCardIoWrapper exit MpAlsa: %p",
                arg);
   return NULL;
}
// MER change this
UtlBoolean MpAlsa::alsaReset()
{
#if 0
    return (ioctl(mfdDevice, SNDCTL_DSP_RESET, 0) != -1);
#else
    return (1==1);
#endif
}

// MER change this
UtlBoolean MpAlsa::alsaSetTrigger(bool turnOn)
{
#if 0
   unsigned int val = ((turnOn && mbReadCap) ? PCM_ENABLE_INPUT : 0) |
                      ((turnOn && mbWriteCap) ? PCM_ENABLE_OUTPUT : 0);
   UtlBoolean res = (ioctl(mfdDevice, SNDCTL_DSP_SETTRIGGER, &val) != -1);
   if (!res)
   {
      OsSysLog::add(FAC_MP, PRI_EMERG,
                    "ALSA: could not set ALSA trigger fd=%d. ioctl returned: %d (%s)\n",
                    mfdDevice, errno, strerror(errno)); 
   }
   return res;
#else
    return (1==1);
#endif
}
const int MS_PER_FRAME = 10;

#define SAMPLES_PER_SEC 8000
#define MIN_SAMPLE_RATE 100
#define BITS_PER_SAMPLE 16
#define N_SAMPLES (MS_PER_FRAME*(SAMPLES_PER_SEC/1000)) /*80*/

#define BUFLEN (N_SAMPLES*(BITS_PER_SAMPLE>>3))
//
// ALSA DEFAULTS
//

#define ALSA_DEFAULT_SAMPLES_PER_SEC   (SIPX_DEFAULT_SAMPLES_PER_SEC)
#define ALSA_DEFAULT_PERIOD_TIME       (10000)
#define ALSA_DEFAULT_PERIOD_COUNT      (4)
#define ALSA_DEFAULT_BUFFER_TIME       (ALSA_DEFAULT_PERIOD_TIME * ALSA_DEFAULT_PERIOD_COUNT)


int MpAlsa::alsaSetupPcmDevice(const char* devname, bool capture, unsigned& suggestedRate)
{
    //suggestedRate = 0;
    OsSysLog::add(FAC_MP, PRI_INFO, "ALSA: Opening %s for %s",
		  devname,
		  (capture ? "capture" : "playback"));

    int ret = snd_pcm_open(
		    (capture ? &pPcmIn : &pPcmOut),
		    devname,
		    (capture ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK),
		    SND_PCM_NONBLOCK);
    OsSysLog::add(FAC_MP, PRI_DEBUG, "ALSA: open %s for %s, snd_pcm_open returned: %d %s %p", 
		  devname, (capture ? "capture" : "playback"), 
		  ret, snd_strerror(ret), (capture ? pPcmIn : pPcmOut));
    if (ret != 0)
    {
	OsSysLog::add(FAC_MP, PRI_WARNING, "ALSA: can't open %s for %s, snd_pcm_open returned: %d %s", 
		      devname, (capture ? "capture" : "playback"), 
                      ret, snd_strerror(ret));
	return ret;
    }
    snd_pcm_t* pPcm = (capture ? pPcmIn : pPcmOut);

    //*****************************************************
    // Get desired parameters
    //MER a little hacky for the moment
    int chansIn;
    int chansOut;
    int blockingIO = 0;
    int       bothEnableResampling = 1;
    int       playResampling = -1;
    int       capResampling = -1;
    unsigned  bothPeriodTime = 0;
    unsigned  playPeriodTime = -1;
    unsigned  capPeriodTime = -1;
    unsigned  bothNumPeriods = 4;
    unsigned  playPeriods = -1;
    unsigned  capPeriods = -1;
    unsigned  bothBufferSize = 0;
    unsigned  playBufferSize = -1;
    unsigned  capBufferSize = -1;

    int enableResampling = (capture ? capResampling : playResampling);
    if (enableResampling == -1)
	enableResampling = bothEnableResampling;
    unsigned periodTime = (capture ? capPeriodTime : playPeriodTime);
    if (periodTime == (unsigned)-1)
	periodTime = bothPeriodTime;
    unsigned numPeriods = (capture ? capPeriods : playPeriods);
    if (numPeriods == (unsigned)-1)
	numPeriods = bothNumPeriods;
    unsigned bufferSize = (capture ? capBufferSize : playBufferSize);
    if (bufferSize == (unsigned)-1)
	bufferSize = bothBufferSize;

    // MER end of hacky
    snd_pcm_hw_params_t* pHwParms;
    snd_pcm_hw_params_alloca(&pHwParms);
    ret = snd_pcm_hw_params_any(pPcm, pHwParms);
    if (ret != 0)
    {
	return alsaSetupWarning(devname, ret, capture, "Get HW Params"); 
    }
    ret = snd_pcm_hw_params_set_rate_resample(pPcm, pHwParms, enableResampling);
    if (ret != 0)
	return alsaSetupWarning(devname, ret, capture, "Set resample");


    //***** Set basics *****

    ret = snd_pcm_hw_params_set_access(pPcm, pHwParms,
				       SND_PCM_ACCESS_RW_INTERLEAVED);
    if (ret != 0)
	return alsaSetupWarning(devname, ret, capture, "Set access");


    //***** SET FORMAT ****

    ret = snd_pcm_hw_params_set_format(pPcm, pHwParms, SND_PCM_FORMAT_S16_LE);
    if (ret != 0)
	return alsaSetupWarning(devname, ret, capture, "Set format");


    //***** Set channels *****

    int& numChans = (capture ? chansIn : chansOut);
    numChans = 1;
    ret = snd_pcm_hw_params_set_channels(pPcm, pHwParms, 1);
    if (ret != 0)
    {    
	ret = snd_pcm_hw_params_set_channels(pPcm, pHwParms, 2);
	if (ret != 0)
	    return alsaSetupWarning(devname, ret, capture, "Set channels");

	numChans = 2;
    }

    OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "MpAlsa setup %s with %d %s channels",
                  devname,
                  numChans,
                  capture ? "input" : "output");

    //***** Set rate *****

    int dir = 0;
    ret = snd_pcm_hw_params_set_rate(pPcm, pHwParms, suggestedRate, dir);
    if (ret != 0)
    {
	int origRet = ret;
	char tmp[128];
	sprintf(tmp, "Set rate to %d", suggestedRate);
	alsaSetupWarning(devname, ret, capture, tmp);

	// What rates are supported?

	unsigned minRate = 0, maxRate = 0;
	ret = snd_pcm_hw_params_get_rate_min(pHwParms, &minRate, &dir);
	if (ret != 0)
	    alsaSetupWarning(devname, ret, capture, "Get min rate");
	ret = snd_pcm_hw_params_get_rate_max(pHwParms, &maxRate, &dir);
	if (ret != 0)
	    alsaSetupWarning(devname, ret, capture, "Get max rate");

	OsSysLog::add(FAC_MP, PRI_DEBUG,
		      "ALSA: Supported rates: Min=%u, Max=%u", minRate, maxRate);

	unsigned testRates[] = { 8000, 48000, 96000, 192000, 44100, 0 };
	unsigned bestRate = 0;
	for (int i = 0; testRates[i] != 0; i++)
	{
	    ret = snd_pcm_hw_params_test_rate(pPcm, pHwParms, testRates[i], dir);
	    OsSysLog::add(FAC_MP, PRI_DEBUG, "Rate %u supported = %s",
			  testRates[i], (ret == 0) ? "Yes" : "No");
	    if (ret == 0 && bestRate == 0 && testRates[i] != SAMPLES_PER_SEC)
		bestRate = testRates[i];
	}
	if (bestRate == 0)
	    return alsaSetupWarning(devname, origRet, capture, "Find best rate");

	OsSysLog::add(FAC_MP, PRI_DEBUG,
		      "ALSA: Suggest new rate of %u", bestRate);

	suggestedRate = bestRate;
	return origRet;
    }
    else
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "ALSA: Set rate: %d",
                      suggestedRate);
    }


    //***** Set period time *****

    dir = 0;
    unsigned desiredPeriodTime = MS_PER_FRAME * 1000;   // In usecs
    unsigned chosenPeriodTime = desiredPeriodTime;
    //ret = snd_pcm_hw_params_set_period_size_near(pPcm, pHwParms,
    //                                             &chosenPeriodSize, &dir);
    ret = snd_pcm_hw_params_set_period_time(pPcm, pHwParms,
					    chosenPeriodTime, dir);
    if (ret != 0)
    {
       //osPrintf("snd_pcm_hw_params_set_period_time_near\n");
       dir = 0;
       ret = snd_pcm_hw_params_set_period_time_near(pPcm, pHwParms,
						    &chosenPeriodTime, &dir);
       if (ret != 0)
       {
	 char tmp[128];
	 sprintf(tmp, "Set period time to %u", chosenPeriodTime);
	 return alsaSetupWarning(devname, ret, capture, tmp);
       }
    }

    OsSysLog::add(FAC_MP, PRI_DEBUG, "ALSA: Set period time to %u",
					  chosenPeriodTime);

    if(desiredPeriodTime != chosenPeriodTime)
    {
		OsSysLog::add(FAC_MP, PRI_INFO, 
		   "ALSA: Period Time Differs from Requested. (Requested=%u us, Chosen=%u us)",
		   desiredPeriodTime, chosenPeriodTime);
    }

    //***** Set buffer size *****

    unsigned int bufferTime = ALSA_DEFAULT_PERIOD_COUNT * chosenPeriodTime;
    dir = 0;

    ret = snd_pcm_hw_params_set_buffer_time(pPcm, pHwParms, bufferTime, dir);

    if(ret != 0)
    {
       dir = 0;
       ret = snd_pcm_hw_params_set_buffer_time_near(pPcm, pHwParms,
						    &bufferTime, &dir);

       if(ret != 0)
		alsaSetupWarning(devname, ret, capture, "Set buffer Time");
    }

    OsSysLog::add(FAC_MP, PRI_DEBUG, "ALSA: Set buffer Time to %u",
		  bufferTime);

    ret = snd_pcm_hw_params(pPcm, pHwParms);
    if (ret != 0)
	return alsaSetupWarning(devname, ret, capture, "Set HW parms");

    //*****************************************************
    // Set desired blocking/non-blocking mode

    ret = snd_pcm_nonblock(pPcm, !blockingIO);
    if (ret != 0)
	alsaSetupWarning(devname, ret, capture, "Set non-blocking mode");

    return 0;
}

int MpAlsa::alsaSetupWarning(const char* devname, int err, bool cap, const char* pLogString)
{
    OsSysLog::add(FAC_MP, PRI_WARNING,
                  "ALSA: Initializing '%s' %s: %s failed (%s)",
                  devname,
                  (cap ? "capture" : "playback"),
                  pLogString, snd_strerror(err));
    return err;
}

#endif
