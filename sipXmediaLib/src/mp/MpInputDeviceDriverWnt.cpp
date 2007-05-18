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

// APPLICATION INCLUDES
#include "mp/MpInputDeviceDriverWnt.h"
#include "mp/MpInputDeviceManager.h"

// EXTERNAL FUNCTIONS
extern void showWaveError(char *syscall, int e, int N, int line) ;  // dmaTaskWnt.cpp

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpInputDeviceDriverWnt::MpInputDeviceDriverWnt(const UtlString& name, 
                                               MpInputDeviceManager& deviceManager,
                                               unsigned nInputBuffers)
: MpInputDeviceDriver(name, deviceManager)
, mWntDeviceId(-1)
, mDevHandle(NULL)
, mNumInBuffers(nInputBuffers)
, mWaveBufSize(0)  // Unknown until enableDevice()
, mIsOpen(FALSE)
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
            mWntDeviceId = i;
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
MpInputDeviceDriverWnt::~MpInputDeviceDriverWnt() 
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

OsStatus MpInputDeviceDriverWnt::enableDevice(unsigned samplesPerFrame, 
                                              unsigned samplesPerSec, 
                                              MpFrameTime currentFrameTime)
{
    OsStatus status = OS_SUCCESS;

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
    MMRESULT res = waveInOpen(&mDevHandle, mWntDeviceId,
                              &wavFormat,
#if defined(_MSC_VER) && (_MSC_VER < 1300) // if < msvc7 (2003)
                              (DWORD)waveInCallbackStatic,
                              (DWORD)this, 
#else
                              (DWORD_PTR)waveInCallbackStatic,
                              (DWORD_PTR)this, 
#endif
                              CALLBACK_FUNCTION);
    if (res != MMSYSERR_NOERROR)
    {
        // If waveInOpen failed, print out the error info,
        // invalidate the handle, and the device driver itself,
        status = OS_FAILED;
        showWaveError("MpInputDeviceDriverWnt::enableDevice", res, -1, __LINE__);
        waveInClose(mDevHandle);
        mDevHandle = NULL; // Open didn't work, reset device handle to NULL
        mWntDeviceId = -1; // Make device invalid.

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
            mWntDeviceId = -1;

            // and return OS_FAILED.
            return status;
        }
        res = waveInAddBuffer(mDevHandle, pWaveHdr, sizeof(WAVEHDR));
        if (res != MMSYSERR_NOERROR)
        {
            showWaveError("waveInAddBuffer", res, i, __LINE__);
            waveInClose(mDevHandle);
            mDevHandle = NULL;
            mWntDeviceId = -1;

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
        mWntDeviceId = -1;

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

OsStatus MpInputDeviceDriverWnt::disableDevice()
{
    OsStatus status = OS_SUCCESS;
    MMRESULT   res;
    
    if (!isDeviceValid() || !isEnabled())
    {
        return OS_FAILED;
    }

    // Indicate we are no longer enabled -- Do this first,
    // since we'll be partially disabled from here on out.
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
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpInputDeviceDriverWnt::processAudioInput(HWAVEIN hwi,
                                               UINT uMsg,
                                               void* dwParam1)
{
    if (!mIsOpen)
    {
        assert(uMsg == WIM_OPEN);
        if (uMsg == WIM_OPEN)
        {
            printf("received WIM_OPEN\n");
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

        mpInputDeviceManager->pushFrame(mDeviceId,
                                        mSamplesPerFrame,
                                        (MpAudioSample*)pWaveHdr->lpData,
                                        mCurrentFrameTime);
        // Ok, we have received and pushed a frame to the manager,
        // Now we advance the frame time.
        mCurrentFrameTime += (mSamplesPerFrame*1000)/mSamplesPerSec;
    }
    else if (uMsg == WIM_CLOSE)
    {
        printf("received WIM_CLOSE\n");
        mIsOpen = FALSE;
    }
}

void CALLBACK 
MpInputDeviceDriverWnt::waveInCallbackStatic(HWAVEIN hwi,
                                             UINT uMsg, 
                                             void* dwInstance,
                                             void* dwParam1, 
                                             void* dwParam2)
{
    assert(dwInstance != NULL);
    MpInputDeviceDriverWnt* iddWntPtr = (MpInputDeviceDriverWnt*)dwInstance;
    iddWntPtr->processAudioInput(hwi, uMsg, dwParam1);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

WAVEHDR* MpInputDeviceDriverWnt::initWaveHeader(int n)
{
    assert((n >= 0) && (n < (int)mNumInBuffers));
    assert(mpWaveHeaders != NULL);
    assert((mpWaveBuffers != NULL) && (mpWaveBuffers[n] != NULL));
    WAVEHDR& wave_hdr(mpWaveHeaders[n]);
    LPSTR    wave_data(mpWaveBuffers[n]);

    // zero out the wave buffer.
    memset(wave_data, 0, mWaveBufSize);

    // Set wave header data to initial values.
    wave_hdr.lpData = wave_data;
    wave_hdr.dwBufferLength = mWaveBufSize;
    wave_hdr.dwBytesRecorded = 0;  // Filled in by wave functions
    wave_hdr.dwUser = n;
    wave_hdr.dwFlags = 0;
    wave_hdr.dwLoops = 0;
    wave_hdr.lpNext = NULL;
    wave_hdr.reserved = 0;

    return &wave_hdr;
}

// Copy constructor (not implemented for this class)
//MpWntInputDeviceDriver::MpWntInputDeviceDriver(const MpInputDeviceDriver& rMpInputDeviceDriver) {}
// Copy constructor (not implemented for this class)
//MpWntInputDeviceDriver& MpWntInputDeviceDriver::operator =(const MpInputDeviceDriver &rhs) {}