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
#include <os/OsSysLog.h>

// APPLICATION INCLUDES
#include "mp/MpidWinMM.h"
#include "mp/MpInputDeviceManager.h"

// EXTERNAL FUNCTIONS
extern void showWaveError(char *syscall, int e, int N, int line) ;  // dmaTaskWnt.cpp

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// DEFINES
#if defined(_MSC_VER) && (_MSC_VER < 1300) // if < msvc7 (2003)
#  define CBTYPE DWORD
#else
#  define CBTYPE DWORD_PTR
#endif



/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpidWinMM::MpidWinMM(const UtlString& name, 
                     MpInputDeviceManager& deviceManager,
                     unsigned nInputBuffers)
: MpInputDeviceDriver(name, deviceManager)
, mWinMMDeviceId(-1)
, mDevHandle(NULL)
, mNumInBuffers(nInputBuffers)
, mWaveBufSize(0)  // Unknown until enableDevice()
, mIsOpen(FALSE)
, mnAddBufferFailures(0)
{
    WAVEINCAPS devCaps;
    // Grab the number of input devices that are available.
    UINT nInputDevs = waveInGetNumDevs();

    // Search through the input devices looking for the input device specified.
    MMRESULT wavResult = MMSYSERR_NOERROR;
    unsigned i;
    for (i = 0; i < nInputDevs; i++)
    {
        MMRESULT res = waveInGetDevCaps(i, &devCaps, sizeof(devCaps));
        if (res != MMSYSERR_NOERROR)
        {
            wavResult = res;
        } 
        else if (strncmp(name, devCaps.szPname, MAXPNAMELEN) == 0)
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
    mpWaveHeaders = new WAVEHDR[mNumInBuffers];
    mpWaveBuffers = new LPSTR[mNumInBuffers];
    for (i = 0; i < mNumInBuffers; i++)
    {
        mpWaveBuffers[i] = NULL;
    }
}

// Destructor
MpidWinMM::~MpidWinMM() 
{
    // If we happen to still be enabled at this point, disable the device.
    assert(!isEnabled());
    if (isEnabled())
    {
        disableDevice();
    }

    // Delete the sample headers and sample buffer pointers..
    unsigned i;
    for (i = 0; i < mNumInBuffers; i++)
    {
        assert(mpWaveBuffers[i] == NULL);
        if (mpWaveBuffers[i] != NULL)
        {
            delete[] mpWaveBuffers[i];
            mpWaveBuffers[i] = NULL;
        }
    }
    delete[] mpWaveBuffers;
    delete[] mpWaveHeaders;
}


/* ============================ MANIPULATORS ============================== */

OsStatus MpidWinMM::enableDevice(unsigned samplesPerFrame, 
                                 unsigned samplesPerSec, 
                                 MpFrameTime currentFrameTime)
{
    OsStatus status = OS_SUCCESS;

    // reset the number of addBuffer failures, as we're starting fresh now.
    mnAddBufferFailures = 0;

    // If the device is not valid, let the user know it's bad.
    if (!isDeviceValid())
    {
        return OS_INVALID_STATE;  // perhaps new OsState of OS_RESOURCE_INVALID?
    }

    if (isEnabled())
    {
        return OS_FAILED;
    }

    // Set some wave header stat information.
    mSamplesPerFrame = samplesPerFrame;
    mSamplesPerSec = samplesPerSec;
    mCurrentFrameTime = currentFrameTime;

    // Do stuff to enable device.
    int nChannels = 1;
    WAVEFORMATEX wavFormat;
    wavFormat.wFormatTag = WAVE_FORMAT_PCM;
    wavFormat.nChannels = nChannels;
    wavFormat.nSamplesPerSec = mSamplesPerSec;
    wavFormat.nAvgBytesPerSec = 
        nChannels * mSamplesPerSec * sizeof(MpAudioSample);
    wavFormat.nBlockAlign = nChannels * sizeof(MpAudioSample);
    wavFormat.wBitsPerSample = sizeof(MpAudioSample) * 8;
    wavFormat.cbSize = 0;

    // Tell windows to open the input audio device.  This doesn't
    // tell it to send the data to our callback yet, just to get it ready
    // to do so..
    MMRESULT res = waveInOpen(&mDevHandle, mWinMMDeviceId,
                              &wavFormat,
                              (CBTYPE)waveInCallbackStatic,
                              (CBTYPE)this, 
                              CALLBACK_FUNCTION);
    if (res != MMSYSERR_NOERROR)
    {
        // If waveInOpen failed, print out the error info,
        // invalidate the handle, and the device driver itself,
        status = OS_FAILED;
        showWaveError("MpidWinMM::enableDevice", res, -1, __LINE__);
        waveInClose(mDevHandle);
        mDevHandle = NULL; // Open didn't work, reset device handle to NULL
        mWinMMDeviceId = -1; // Make device invalid.

        // and return OS_FAILED.
        return status;
    }


    // Allocate the buffers we are going to use to receive audio data from
    // the windows audio input callback.
    // Calculate the buffer length we're going to use. 
    // number of samples per frame * sample size in bytes
    mWaveBufSize = mSamplesPerFrame * sizeof(MpAudioSample); 
    unsigned i;
    for (i = 0; i < mNumInBuffers; i++)
    {
        mpWaveBuffers[i] = new char[mWaveBufSize];
    }


    // Setup the buffers so windows can stuff them full of audio
    // when it becomes available from this audio input device.
    WAVEHDR* pWaveHdr = NULL;
    for (i=0; i < mNumInBuffers; i++) 
    {
        pWaveHdr = initWaveHeader(i);

        res = waveInPrepareHeader(mDevHandle, pWaveHdr, sizeof(WAVEHDR));
        if (res != MMSYSERR_NOERROR)
        {
            showWaveError("waveInPrepareHeader", res, i, __LINE__);
            waveInClose(mDevHandle);
            mDevHandle = NULL;
            mWinMMDeviceId = -1;

            // and return OS_FAILED.
            return status;
        }
        res = waveInAddBuffer(mDevHandle, pWaveHdr, sizeof(WAVEHDR));
        if (res != MMSYSERR_NOERROR)
        {
            showWaveError("waveInAddBuffer", res, i, __LINE__);
            waveInClose(mDevHandle);
            mDevHandle = NULL;
            mWinMMDeviceId = -1;

            // and return OS_FAILED.
            return status;
        }
    }

    // Tell windows to start sending audio data to the callback.
    res = waveInStart(mDevHandle);
    if (res != MMSYSERR_NOERROR)
    {
        // If waveInStart failed, print out the error info,
        // invalidate the handle and the device driver itself,
        status = OS_FAILED;
        showWaveError("waveInStart", res, -1, __LINE__);
        waveInClose(mDevHandle);
        mDevHandle = NULL;
        mWinMMDeviceId = -1;

        // and return OS_FAILED.
        return status;
    }

    // If enableDevice failed, return indicating failure.
    if (status == OS_SUCCESS)
    {
        mIsEnabled = TRUE;
    }

    return status;
}

OsStatus MpidWinMM::disableDevice()
{
    OsStatus status = OS_SUCCESS;
    MMRESULT   res;
    
    if (!isDeviceValid() || !isEnabled())
    {
        return OS_FAILED;
    }

    // Indicate we are no longer enabled -- Do this first,
    // since we'll be partially disabled from here on out.
    // It is very important that this happen *before* waveInReset,
    // as the callback will continue to add and process buffers
    // while waveInReset is called causing a deadlock.
    mIsEnabled = FALSE;

    // Cleanup
    if (mDevHandle == NULL)
    {
        return OS_INVALID_STATE;
    }

    // Reset performs a stop, resets the buffers, and marks them
    // for being sent to the callback.
    // The remaining data in the windows buffers *IS* sent to the callback,
    // So be sure to watch for it and drop it on the floor.
    res = waveInReset(mDevHandle);
    if (res != MMSYSERR_NOERROR)
    {
        showWaveError("waveInReset", res, -1, __LINE__);
    } 

    // Must unprepare the headers after a reset, but before the device is closed
    // (if this is done after waveInClose, mDevHandle will be invalid and 
    // MMSYSERR_INVALHANDLE will be returned.
    unsigned i;
    for (i=0; i < mNumInBuffers; i++) 
    {
        res = waveInUnprepareHeader(mDevHandle, &mpWaveHeaders[i], sizeof(WAVEHDR));
        if (res != MMSYSERR_NOERROR)
        {
            showWaveError("waveInUnprepareHeader", res, i, __LINE__);
        }
    }

    res = waveInClose(mDevHandle);
    if (res != MMSYSERR_NOERROR)
    {
        showWaveError("waveInClose", res, -1, __LINE__);
    }

    // Delete the buffers that were allocated in enableDevice()
    for (i = 0; i < mNumInBuffers; i++)
    {
        delete[] mpWaveBuffers[i];
        mpWaveBuffers[i] = NULL;
    }

    // set the device handle to NULL, since it no longer is valid.
    mDevHandle = NULL;

    // Clear out all the wave header information.
    mSamplesPerFrame = 0;
    mSamplesPerSec = 0;
    mCurrentFrameTime = 0;

    return status;
}

/* ============================ ACCESSORS ================================= */

/* ////////////////////////// PUBLIC STATIC ///////////////////////////////// */
UtlString MpidWinMM::getDefaultDeviceName()
{
   UtlString devName = "";

   // Get windows default input device name
   unsigned nDevs = waveInGetNumDevs();
   if (nDevs == 0)
   {
      OsSysLog::add(FAC_AUDIO, PRI_ERR, 
                    "MpidWinMM::getDefaultDeviceName: "
                    "No input audio devices present!");
   }
   assert(nDevs != 0);

   MMRESULT wavResult = MMSYSERR_NOERROR;
   WAVEINCAPS devCaps;
   int defaultWinDeviceId = 0;
   wavResult = 
      waveInGetDevCaps(defaultWinDeviceId, &devCaps, sizeof(devCaps));
   if (wavResult != MMSYSERR_NOERROR)
   {
      OsSysLog::add(FAC_AUDIO, PRI_ERR, 
                    "MpodWinMM::getDefaultDeviceName: "
                    "Couldn't get default input device capabilities!");
      showWaveError("WINDOWS_DEFAULT_DEVICE_HACK",
                    wavResult, -1, __LINE__);
   }
   else
   {
      devName = devCaps.szPname;
   }
   assert(wavResult == MMSYSERR_NOERROR);
   return devName;
}


/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */

WAVEHDR* MpidWinMM::initWaveHeader(int n)
{
    assert((n >= 0) && (n < (int)mNumInBuffers));
    assert(mpWaveHeaders != NULL);
    assert((mpWaveBuffers != NULL) && (mpWaveBuffers[n] != NULL));
    WAVEHDR* pWave_hdr = &(mpWaveHeaders[n]);
    LPSTR    wave_data(mpWaveBuffers[n]);

    // zero out the wave buffer.
    memset(wave_data, 0, mWaveBufSize);

    // Set wave header data to initial values.
    pWave_hdr->lpData = wave_data;
    pWave_hdr->dwBufferLength = mWaveBufSize;
    pWave_hdr->dwBytesRecorded = 0;  // Filled in by wave functions
    pWave_hdr->dwUser = n;
    pWave_hdr->dwFlags = 0;
    pWave_hdr->dwLoops = 0;
    pWave_hdr->lpNext = NULL;
    pWave_hdr->reserved = 0;

    return pWave_hdr;
}
void MpidWinMM::processAudioInput(HWAVEIN hwi,
                                  UINT uMsg,
                                  void* dwParam1)
{
    if (!mIsOpen)
    {
        assert(uMsg == WIM_OPEN);
        if (uMsg == WIM_OPEN)
        {
            //printf("received WIM_OPEN\n");
            mIsOpen = TRUE;
        }
    }
    if (uMsg == WIM_DATA)
    {
        //printf("received WIM_DATA\n");
        assert(mIsOpen);
        WAVEHDR* pWaveHdr = (WAVEHDR*)dwParam1;
        assert(pWaveHdr->dwBufferLength 
               == (mSamplesPerFrame*sizeof(MpAudioSample)));
        assert(pWaveHdr->lpData != NULL);

        // Only process if we're enabled..
        if(mIsEnabled)
        {
           mpInputDeviceManager->pushFrame(mDeviceId,
                                           mSamplesPerFrame,
                                           (MpAudioSample*)pWaveHdr->lpData,
                                           mCurrentFrameTime);
           // Ok, we have received and pushed a frame to the manager,
           // Now we advance the frame time.
           mCurrentFrameTime += (mSamplesPerFrame*1000)/mSamplesPerSec;

           // Put the wave header back in the pool..
           MMRESULT res = MMSYSERR_NOERROR;

           res = waveInAddBuffer(mDevHandle, pWaveHdr, sizeof(WAVEHDR));
           if (res != MMSYSERR_NOERROR)
           {
              showWaveError("waveInAddBuffer", res, -1, __LINE__);
              mnAddBufferFailures++;
              if(mnAddBufferFailures >= mNumInBuffers)
              {
                 waveInClose(mDevHandle);
                 mDevHandle = NULL;
                 mWinMMDeviceId = -1;
              }
           }
        }
    }
    else if (uMsg == WIM_CLOSE)
    {
        printf("received WIM_CLOSE\n");
        mIsOpen = FALSE;
    }
}

void CALLBACK 
MpidWinMM::waveInCallbackStatic(HWAVEIN hwi,
                                UINT uMsg, 
                                void* dwInstance,
                                void* dwParam1, 
                                void* dwParam2)
{
    assert(dwInstance != NULL);
    MpidWinMM* iddWntPtr = (MpidWinMM*)dwInstance;
    assert((uMsg == WIM_OPEN) || (hwi == iddWntPtr->mDevHandle));
    iddWntPtr->processAudioInput(hwi, uMsg, dwParam1);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
