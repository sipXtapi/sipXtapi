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
        }
    }

    // calculate the buffer length we're going to use.
    // number of samples per frame * sample size in bytes
    mWaveBufSize = NUM_SAMPLES * sizeof(MpAudioSample); 

    // Allocate the wave headers and buffers for use with windows audio routines.
    mpWaveHeaders = new WAVEHDR[mNumInBuffers];
    mpWaveBuffers = new LPSTR[mNumInBuffers];
    for (i = 0; i < mNumInBuffers; i++)
    {
        mpWaveBuffers[i] = new char[mWaveBufSize];
    }
}

// Destructor
MpInputDeviceDriverWnt::~MpInputDeviceDriverWnt() 
{
    // If we happen to still be enabled at this point, disable the device.
    assert(!isEnabled());
    if (isEnabled())
        disableDevice();

    // Delete the sample buffers..
    unsigned i;
    for (i = 0; i < mNumInBuffers; i++)
    {
        delete[] mpWaveBuffers[i];
        mpWaveBuffers[i] = NULL;
    }
    delete[] mpWaveBuffers;
}


/* ============================ MANIPULATORS ============================== */
OsStatus MpInputDeviceDriverWnt::enableDevice(unsigned samplesPerFrame, 
                                              unsigned samplesPerSec, 
                                              unsigned currentFrameTime)
{
    OsStatus status = OS_SUCCESS;

    // If the device is not valid, let the user know it's bad.
    if (!isDeviceValid())
        return OS_INVALID_STATE;  // perhaps new OsState of OS_RESOURCE_INVALID?

    if (isEnabled())
        return OS_FAILED;

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

    MMRESULT res = waveInOpen(&mDevHandle, mWntDeviceId,
                              &wavFormat, (DWORD_PTR)waveInCallbackStatic,
                              (DWORD_PTR)this, CALLBACK_FUNCTION);
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


    // Setup the buffers so windows can stuff them full of audio
    // when it becomes available from this audio input device.
    WAVEHDR* pWaveHdr = NULL;
    unsigned i;
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

    res = waveInReset(mDevHandle);
    if (res != MMSYSERR_NOERROR)
    {
        showWaveError("waveInReset", res, -1, __LINE__);
    }    
    res = waveInStop(mDevHandle);
    if (res != MMSYSERR_NOERROR)
    {
        showWaveError("waveInStop", res, -1, __LINE__);
    }
    Sleep(500);

    unsigned i;
    for (i=0; i < mNumInBuffers; i++) 
    {
        res = waveInUnprepareHeader(mDevHandle, &mpWaveHeaders[i], sizeof(WAVEHDR));
        if (res != MMSYSERR_NOERROR)
        {
            showWaveError("waveInUnprepareHeader", res, i, __LINE__);
        }
    }
    Sleep(500);

    res = waveInClose(mDevHandle);
    if (res != MMSYSERR_NOERROR)
    {
        showWaveError("waveInClose", res, -1, __LINE__);
    }

    // set the device handle to NULL, since it no longer is valid.
    mDevHandle = NULL;

    // Clear out all the wave header information.
    mSamplesPerFrame = 0;
    mSamplesPerSec = 0;
    mCurrentFrameTime = 0;

    return status;
}

void MpInputDeviceDriverWnt::processAudioInput(HWAVEIN hwi,
                                               UINT uMsg,
                                               DWORD_PTR dwParam1,
                                               DWORD_PTR dwParam2)
{
    if (!mIsOpen)
    {
        assert(uMsg == WIM_OPEN);
        if(uMsg == WIM_OPEN)
        {
            printf("received WIM_OPEN\n");
            mIsOpen = TRUE;
        }
    }
    if (uMsg == WIM_DATA)
    {
        assert(mIsOpen);
        printf("received WIM_DATA\n");
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
                                             DWORD_PTR dwInstance,
                                             DWORD_PTR dwParam1, 
                                             DWORD_PTR dwParam2)
{
    assert(dwInstance != NULL);
    MpInputDeviceDriverWnt* iddWntPtr = (MpInputDeviceDriverWnt*)dwInstance;
    iddWntPtr->processAudioInput(hwi, uMsg, dwParam1, dwParam2);
}



/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
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