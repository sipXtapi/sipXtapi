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


// SYSTEM INCLUDES
#include <Windows.h>
#include <os/OsSysLog.h>
#include <os/OsLock.h>
#include <utl/UtlVoidPtr.h>
#include <os/OsNotification.h>

#ifdef RTL_ENABLED
#   include <rtl_macro.h>
#else
#define RTL_BLOCK(x)
#define RTL_EVENT(x,y)
#endif

// APPLICATION INCLUDES
#include "mp/MpodWinMM.h"
#include "mp/MpOutputDeviceManager.h"

// EXTERNAL FUNCTIONS
extern void showWaveError(char *syscall, int e, int N, int line) ;  // dmaTaskWnt.cpp

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// DEFINES
#define LOW_WAVEBUF_LVL 3

#if defined(_MSC_VER) && (_MSC_VER < 1300) // if < msvc7 (2003)
#  define CBTYPE DWORD
#else
#  define CBTYPE DWORD_PTR
#endif



/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpodWinMM::MpodWinMM(const UtlString& name, 
                     unsigned nOutputBuffers)
: MpOutputDeviceDriver(name)
, mEmptyHdrVPtrListsMutex(OsMutex::Q_FIFO)
, mpNotifier(NULL)
, mWinMMDeviceId(-1)
, mDevHandle(NULL)
, mCurFrameTime(0)
, mNumOutBuffers(nOutputBuffers)
, mWaveBufSize(0)  // Unknown until enableDevice()
, mIsOpen(FALSE)
, mIsInit(FALSE)
, mTotSampleCount(0)
{
   WAVEOUTCAPS devCaps;
   // Grab the number of output devices that are available.
   UINT nInputDevs = waveOutGetNumDevs();

   // Search through the output devices looking for the input device specified.
   MMRESULT wavResult = MMSYSERR_NOERROR;
   unsigned i;
   for ( i = 0; i < nInputDevs; i++ )
   {
      MMRESULT res = waveOutGetDevCaps(i, &devCaps, sizeof(devCaps));
      if ( res != MMSYSERR_NOERROR )
      {
         wavResult = res;
      } 
      else if ( strncmp(name, devCaps.szPname, MAXPNAMELEN) == 0 )
      {
         mWinMMDeviceId = i;
         break;
      }
   }

   // Allocate the wave headers and buffer pointers for use with 
   // windows audio routines.  
   //(This does *not* include allocation of the buffers themselves -
   // that is handled in enableDevice, as we don't know the 
   // buffer size (#samplesPerFrame) until then.
   mpWaveHeaders = new WAVEHDR[mNumOutBuffers];
   if ( mpWaveHeaders == NULL )
   {
      mpWaveBuffers = NULL;
      return;
   }

   mpWaveBuffers = new LPSTR[mNumOutBuffers];
   if ( mpWaveBuffers == NULL )
   {
      delete[] mpWaveHeaders;
      mpWaveHeaders = NULL;
      return;
   }

   for ( i = 0; i < mNumOutBuffers; i++ )
   {
      mpWaveBuffers[i] = NULL;

      // Initialize the unused void pointers.
      // add mNumOutBuffers new void pointers to the list.
      mUnusedVPtrList.insert(new UtlVoidPtr());
   }

   mIsInit = TRUE;
}


// Destructor
MpodWinMM::~MpodWinMM() 
{
   // We shouldn't be enabled, assert that we aren't.
   // If we happen to still be enabled at this point, disable the device.
   //assert(!isEnabled());  // Commented out as it causes issues with unit test.
   if ( mIsInit )
   {
      return;
   }

   // Wait until the windows buffer fill thread finishes.
   //waitUntilShutDown();

   if ( isEnabled() )
   {
      disableDevice();
   }

   // Destroy all the void ptrs that we created at the beginning.
   // They can be either in the unused list where they started, or in
   // the empty header list.. Either way, they have to be destroyed.
   mEmptyHdrVPtrListsMutex.acquire();
   mEmptyHeaderList.destroyAll();
   mUnusedVPtrList.destroyAll();
   mEmptyHdrVPtrListsMutex.release();

   // Delete the sample headers and sample buffer pointers..
   unsigned i;
   for ( i = 0; i < mNumOutBuffers; i++ )
   {
      assert(mpWaveBuffers[i] == NULL);
   }
   delete[] mpWaveBuffers;
   delete[] mpWaveHeaders;
}

/* ============================ MANIPULATORS ================================ */
OsStatus MpodWinMM::enableDevice(unsigned samplesPerFrame, 
                                 unsigned samplesPerSec, 
                                 MpFrameTime currentFrameTime)
{
   // If the device is not valid, let the user know it's bad.
   if ( !isDeviceValid() )
   {
      return OS_INVALID_STATE;
   }

   if ( isEnabled() )
   {
      return OS_FAILED;
   }

   // Set some wave header stat information.
   mSamplesPerFrame = samplesPerFrame;
   mSamplesPerSec = samplesPerSec;
   mCurFrameTime = currentFrameTime;
   mTotSampleCount = 0;

   // Open the wave device.
   int nChannels = 1;
   WAVEFORMATEX wavFormat;
   wavFormat.wFormatTag = WAVE_FORMAT_PCM;
   wavFormat.nChannels = nChannels;
   wavFormat.nSamplesPerSec = (DWORD)mSamplesPerSec;
   wavFormat.nAvgBytesPerSec = 
      nChannels * mSamplesPerSec * sizeof(MpAudioSample);
   wavFormat.nBlockAlign = nChannels * sizeof(MpAudioSample);
   wavFormat.wBitsPerSample = sizeof(MpAudioSample) * 8;
   wavFormat.cbSize = 0;

   MMRESULT res = waveOutOpen(&mDevHandle, mWinMMDeviceId, &wavFormat, 
                               (CBTYPE)waveOutCallbackStatic, 
                               (CBTYPE)this, 
                               CALLBACK_FUNCTION);
   if( res != MMSYSERR_NOERROR )
   {
      // If waveOutOpen failed, print out the error info,
      // invalidate the handle, and the device driver itself,
      showWaveError("MpodWinMM::enableDevice", res, -1, __LINE__);
      waveOutClose(mDevHandle);
      mDevHandle = NULL; // Open didn't work, reset device handle to NULL
      mWinMMDeviceId = -1; // Make device invalid.

      // and return OS_FAILED.
      return OS_FAILED;
   }

   // We'll be accessing the vptr and empty header lists, so acquire the mutex
   mEmptyHdrVPtrListsMutex.acquire();

   // Allocate the buffers we are going to use to send audio data to
   // windows audio.
   // Calculate the buffer length we're going to use. 
   // number of samples per frame * sample size in bytes
   mWaveBufSize = mSamplesPerFrame * sizeof(MpAudioSample); 
   unsigned i;
   for ( i = 0; i < mNumOutBuffers; i++ )
   {
      mpWaveBuffers[i] = new char[mWaveBufSize];

      // Initialize the array of pointers to empty wave headers
      // There will be as many pointers as are out buffers, as
      // they're all currently empty.
      UtlVoidPtr* pVoidPtr = (UtlVoidPtr*)mUnusedVPtrList.get();
      pVoidPtr->setValue(&mpWaveHeaders[i]);
      mEmptyHeaderList.insert(pVoidPtr);

      // The total of ptrs in the empty header list and unused vptr list 
      // should equal the number of output buffers.
      assert((mEmptyHeaderList.entries() + mUnusedVPtrList.entries()) == mNumOutBuffers);
   }

   // Release the mutex as we're done accessing the vptr and empty header lists.
   mEmptyHdrVPtrListsMutex.release();

   // Initialize the buffers to zero and prepare them.
   WAVEHDR* pWaveHdr = NULL;
   for ( i = 0; i < mNumOutBuffers; i++ ) 
   {
      pWaveHdr = initWaveHeader(i);

      res = waveOutPrepareHeader(mDevHandle, pWaveHdr, sizeof(WAVEHDR));
      if ( res != MMSYSERR_NOERROR )
      {
         showWaveError("waveOutPrepareHeader", res, i, __LINE__);
         waveOutClose(mDevHandle);
         mDevHandle = NULL;
         mWinMMDeviceId = -1;

         // and return OS_FAILED.
         return OS_FAILED;
      }
   }

   // allocate and zero out the silence buffer.
   mpSilenceBuffer = new char[mWaveBufSize];
   memset(mpSilenceBuffer, 0, mWaveBufSize);

   mIsEnabled = TRUE;

   OsStatus pushStat = OS_SUCCESS;
   // Push silent frames to the device to kick-start it,
   // so that if it is in mixer mode, notifications are sent.
   // Push the minimum we can, as this adds latency equal to the number of
   // silence frames we push (LOW_WAVEBUF_LVL frames).
   for( i = 0; pushStat == OS_SUCCESS && i < LOW_WAVEBUF_LVL; i++ )
   {
      pushStat = pushFrame(mSamplesPerFrame, (MpAudioSample*)mpSilenceBuffer, 
                           getFramePeriod());
   }

   if( pushStat != OS_SUCCESS )
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
                    "During windows device driver enable, "
                    "Error kickstart pushing silent frames!");
      waveOutClose(mDevHandle);
      mDevHandle = NULL;
      mWinMMDeviceId = -1;

      // and return OS_FAILED.
      return OS_FAILED;
   }

   return OS_SUCCESS;
}


OsStatus MpodWinMM::disableDevice()
{
   OsStatus status = OS_SUCCESS;
   MMRESULT   res;

   // If the device is (not valid) or (not enabled),
   // then don't do anything and return failure.
   if ( !isDeviceValid() )
   {
      // If the device is not valid, let the user know it's bad.
      return OS_INVALID_STATE;
   }
   if ( !isEnabled() )
   {
      return OS_FAILED;
   }

   // Indicate we are no longer enabled -- Do this first,
   // since we'll be partially disabled from here on out.
   mIsEnabled = FALSE;

   // Cleanup
   if ( mDevHandle == NULL )
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "During windows device driver disable, "
                    "device handle was invalid while in enabled state!");
      // The device handle seems to be in a weird state - it's not valid.
      return OS_INVALID_STATE;
   }

   // Reset performs a stop, returns all the buffers within windows multimedia
   res = waveOutReset(mDevHandle);
   if ( res != MMSYSERR_NOERROR )
   {
      showWaveError("waveOutReset", res, -1, __LINE__);
   } 

   // We'll be accessing the vptr and empty header lists, so acquire the mutex
   mEmptyHdrVPtrListsMutex.acquire();

   // clear out the empty header list, as we don't want to continue filling
   // buffers after a wave reset.  Put the cleared out entries in the unused
   // void ptr list for use when this next gets enabled.
   unsigned nEmpties = mEmptyHeaderList.entries();
   unsigned i;
   for(i = 0; i < nEmpties; i++)
   {
      mUnusedVPtrList.insert(mEmptyHeaderList.get());
   }

   // Release the mutex as we're done accessing the vptr and empty header lists.
   mEmptyHdrVPtrListsMutex.release();

   // Must unprepare the headers after a reset, but before the device is closed
   // (if this is done after waveOutClose, mDevHandle will be invalid and 
   // MMSYSERR_INVALHANDLE will be returned.
   for ( i = 0; i < mNumOutBuffers; i++ ) 
   {
      res = waveOutUnprepareHeader(mDevHandle, &mpWaveHeaders[i], sizeof(WAVEHDR));
      if ( res != MMSYSERR_NOERROR )
      {
         showWaveError("waveOutUnprepareHeader", res, i, __LINE__);
      }
   }

   res = waveOutClose(mDevHandle);
   if ( res != MMSYSERR_NOERROR )
   {
      showWaveError("waveOutClose", res, -1, __LINE__);
   }

   // Delete the buffers that were allocated in enableDevice()
   for ( i = 0; i < mNumOutBuffers; i++ )
   {
      delete[] mpWaveBuffers[i];
      mpWaveBuffers[i] = NULL;
   }

   // set the device handle to NULL, since it no longer is valid.
   mDevHandle = NULL;

   // Clear out all the wave header information.
   mSamplesPerFrame = 0;
   mSamplesPerSec = 0;
   mCurFrameTime = 0;
   mTotSampleCount = 0;

   return status;
}

OsStatus MpodWinMM::pushFrame(unsigned int numSamples,
                              const MpAudioSample* samples,
                              MpFrameTime frameTime)
{
   OsStatus status = OS_FAILED;
   if ( !isEnabled() )
   {
      // be sure to release the mutex prior to returning.
      mEmptyHdrVPtrListsMutex.release();
      return OS_FAILED;
   }

   // Only full frames are supported right now.
   assert(mSamplesPerFrame == numSamples);

   // We'll be accessing the vptr and empty header lists, so acquire the mutex
   mEmptyHdrVPtrListsMutex.acquire();

   // If there are empty headers, we can push a frame
   if( mEmptyHeaderList.entries() > 0)
   {
      UtlVoidPtr* pvpWaveHdr = (UtlVoidPtr*)mEmptyHeaderList.get();
      WAVEHDR* pWaveHdr = (WAVEHDR*)(pvpWaveHdr->getValue());

      // Reset the voidptr to null and add it to the unused vptr list for reuse.
      pvpWaveHdr->setValue(NULL);
      mUnusedVPtrList.insert(pvpWaveHdr);

      // Cannot hold the mutex while performing wave calls.
      mEmptyHdrVPtrListsMutex.release();

      MMRESULT res = waveOutPrepareHeader(mDevHandle, pWaveHdr, sizeof(WAVEHDR));
      if ( res != MMSYSERR_NOERROR )
      {
         showWaveError("waveOutPrepareHeader", res, -1, __LINE__);
         waveOutClose(mDevHandle);
         mDevHandle = NULL;
         mWinMMDeviceId = -1;

         // and return OS_FAILED.
         return OS_FAILED;
      }

      // We found an empty buffer, now we fill it.
      memcpy(pWaveHdr->lpData, samples, sizeof(MpAudioSample)*numSamples);

      // And send it on it's way to windows wave interface.
      res = waveOutWrite(mDevHandle, pWaveHdr, sizeof(WAVEHDR));
      if( res != MMSYSERR_NOERROR )
      {
         showWaveError("MpodWinMM::pushFrame", res, -1, __LINE__);
         // If it's more than just an unprepared header
         // (invalid handle, no driver, or a memory allocation or lock error)
         if( res != WAVERR_UNPREPARED )
         {
            // Then close and invalidate this device.
            waveOutClose(mDevHandle);
            mDevHandle = NULL;
            mWinMMDeviceId = -1;
         }
      }
      else // res != MMSYSERR_NOERROR
      {
         // Increment the current frame time.
         mCurFrameTime += frameTime;

         // Increase our sample count.
         mTotSampleCount += numSamples;

         status = OS_SUCCESS;
      }
   } 
   else // mNumEmptyBuffers
   {
      // Release the mutex as it was released in the if() case..
      // We don't need to hold it anymore, 
      // and we're done accessing vptr and empty header lists.
      mEmptyHdrVPtrListsMutex.release();

      // No buffers are empty! Cannot push this frame!
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "MpodWinMM::pushFrame: "
                    "No free buffers! Dropping frame.");
   }
   return status;
}

OsStatus MpodWinMM::setTickerNotification(OsNotification *pFrameTicker)
{
   mpNotifier = pFrameTicker;
   return OS_SUCCESS;
}


/* ////////////////////////// PUBLIC STATIC ///////////////////////////////// */
UtlString MpodWinMM::getDefaultDeviceName()
{
   UtlString devName = "";

   // Get windows default output device name
   unsigned nDevs = waveOutGetNumDevs();
   if (nDevs == 0)
   {
      OsSysLog::add(FAC_AUDIO, PRI_ERR, 
                    "MpodWinMM::getDefaultDeviceName: "
                    "No output audio devices present!");
   }
   assert(nDevs != 0);

   MMRESULT wavResult = MMSYSERR_NOERROR;
   WAVEOUTCAPS devCaps;
   int defaultWinDeviceId = 0;
   wavResult = 
      waveOutGetDevCaps(defaultWinDeviceId, &devCaps, sizeof(devCaps));
   if (wavResult != MMSYSERR_NOERROR)
   {
      OsSysLog::add(FAC_AUDIO, PRI_ERR, 
                    "MpodWinMM::getDefaultDeviceName: "
                    "Couldn't get default output device capabilities!");
      showWaveError("MpodWinMM::getDefaultDeviceName: ", wavResult, -1, __LINE__);
   }
   else
   {
      devName = devCaps.szPname;
   }
   assert(wavResult == MMSYSERR_NOERROR);

   return devName;
}

/* //////////////////////////// PROTECTED /////////////////////////////////// */
WAVEHDR* MpodWinMM::initWaveHeader(int n)
{
   assert((n >= 0) && (n < (int)mNumOutBuffers));
   assert(mpWaveHeaders != NULL);
   assert((mpWaveBuffers != NULL) && (mpWaveBuffers[n] != NULL));
   WAVEHDR* pWave_hdr = &(mpWaveHeaders[n]);
   LPSTR    wave_data(mpWaveBuffers[n]);

   // zero out the wave buffer.
   memset(wave_data, 0, mWaveBufSize);

   // Set wave header data to initial values.
   pWave_hdr->lpData = wave_data;
   pWave_hdr->dwBufferLength = mWaveBufSize;
   pWave_hdr->dwBytesRecorded = 0;  // Filled in by us later
   pWave_hdr->dwUser = n;
   pWave_hdr->dwFlags = WHDR_DONE;  // Hack: indicate that the wave header has finished processing by WMM
   pWave_hdr->dwLoops = 0;
   pWave_hdr->lpNext = NULL;
   pWave_hdr->reserved = 0;

   return pWave_hdr;
}

void MpodWinMM::finalizeProcessedHeader(WAVEHDR* pWaveHdr)
{
   // CAUTION: THIS IS CALLED FROM THE WAVE CALLBACK CONTEXT!
   OsLock lock(mEmptyHdrVPtrListsMutex);
   // Check to see if we have any free pointers, and if we're enabled.
   if(isEnabled() && mUnusedVPtrList.entries() > 0)
   {
      // Grab an unused voidptr, set it's value to pWaveHdr,
      UtlVoidPtr* pvptr = (UtlVoidPtr*)(mUnusedVPtrList.get());
      pvptr->setValue(pWaveHdr);
      // And add it to the empty header list for use by pushFrame.
      mEmptyHeaderList.insert(pvptr);

      // Collect some metrics -- the sample number that windows is on 
      // (since waveOutOpen)
      MMTIME mmt;
      mmt.wType = TIME_SAMPLES;
      waveOutGetPosition(mDevHandle, &mmt, sizeof(mmt));
      assert(mmt.wType == TIME_SAMPLES);

      DWORD drvLatencyNSamp = mTotSampleCount - mmt.u.sample;
      RTL_EVENT("MpodWinMM.callback.driverLatencyNSamples", drvLatencyNSamp);

      // If the number of samples held within windows MME subsystem gets below
      // 2 frames worth, inject a frame of silence.
      if(drvLatencyNSamp < LOW_WAVEBUF_LVL*80)
      {
         OsStatus pushStat = OS_SUCCESS;

         unsigned nSilenceFramesToPush = 1;
         unsigned j;
         for(j = 0; j < nSilenceFramesToPush; j++)
         {
            pushStat = 
               pushFrame(mSamplesPerFrame, 
                         (MpAudioSample*)mpSilenceBuffer, 
                         getFramePeriod());
            if( pushStat != OS_SUCCESS )
            {
               OsSysLog::add(FAC_MP, PRI_ERR, 
                  "During windows output device tick (addEmptyHeader) low buffer "
                  "level silent frame pumped, pushFrame failed!\n");
            }
         }
      }

      // send a ticker notification so that more frames can be sent.
      if(mpNotifier != NULL)
      {
         mpNotifier->signal(mCurFrameTime);
      }
   }
}

/* //////////////////////// PROTECTED STATIC //////////////////////////////// */
void CALLBACK 
MpodWinMM::waveOutCallbackStatic(HWAVEOUT hwo,
                                 UINT uMsg, 
                                 void* dwInstance,
                                 void* dwParam1, 
                                 void* dwParam2)
{
   assert(dwInstance != NULL);
   MpodWinMM* oddWinMMPtr = (MpodWinMM*)dwInstance;
   assert(uMsg == WOM_OPEN || hwo == oddWinMMPtr->mDevHandle);
   
   switch(uMsg)
   {
   case WOM_OPEN:
      OsSysLog::add(FAC_MP, PRI_INFO, 
                    "Windows output device driver callback "
                    "device open (WOM_OPEN).");
      break;
   case WOM_DONE:
      // dwParam1 is a WAVEHDR* when uMsg == WOM_DONE, per windows documentation
      oddWinMMPtr->finalizeProcessedHeader((WAVEHDR*)dwParam1);
      break;
   case WOM_CLOSE:
      OsSysLog::add(FAC_MP, PRI_INFO, 
                    "Windows output device driver callback "
                    "device closed (WOM_CLOSE).");
      break;
   default:
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "Windows output device driver callback "
                    "sending unknown message!");
   }
}

/* ///////////////////////////// PRIVATE //////////////////////////////////// */