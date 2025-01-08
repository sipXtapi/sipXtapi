//  
// Copyright (C) 2007-2021 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>


// SYSTEM INCLUDES
#include <os/OsSysLog.h>
//#include <initguid.h>
#include <mmdeviceapi.h>
#include <MMSystem.h>
#include <Functiondiscoverykeys_devpkey.h>
// APPLICATION INCLUDES
#include <mp/MpidWinMM.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MpResNotificationMsg.h>
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

#define SAFE_RELEASE(ptr) if ((ptr) != NULL) { (ptr)->Release(); (ptr) = NULL; }

/* //////////////////////////// PUBLIC //////////////////////////////////// */

class MpidWinMM::MpWinInputAudioDeviceNotifier : public IMMNotificationClient
{
public:

    MpWinInputAudioDeviceNotifier(const UtlString& deviceName, 
        MpInputDeviceManager& inputManager,
        UtlBoolean* isOpenPtr,
        IMMDeviceEnumerator* deviceEnumeratorPtr) : IMMNotificationClient(),
        mName(deviceName),
        mpInputDeviceManager(&inputManager),
        mpIsOpen(isOpenPtr),
        mDeviceEnumeratorPtr(deviceEnumeratorPtr)
    {
    };

    ULONG _stdcall AddRef()
    {
        return(1);
    };

    ULONG _stdcall Release()
    {
        return(1);
    };

    HRESULT _stdcall QueryInterface(REFIID riid,
                           void** ppvObject)
    {
        return(S_OK);
    };

    HRESULT _stdcall OnDefaultDeviceChanged(EDataFlow flow,
                                            ERole role,
                                            LPCWSTR defaultDeviceId)
    {
        UtlString flowString("unknown");
        UtlString roleString("unknown");

        switch (flow)
        {
        case EDataFlow::eAll:
            flowString = "all";
            break;

        case EDataFlow::eCapture:
            flowString = "capture";
            break;

        case EDataFlow::EDataFlow_enum_count:
            flowString = "enum_count";
            break;

        case EDataFlow::eRender:
            flowString = "render";
            break;
        }

        switch (role)
        {
        case ERole::eCommunications:
            roleString = "communications";
            break;

        case ERole::eConsole:
            roleString = "console";
            break;

        case ERole::eMultimedia:
            roleString = "multimedia";
            break;

        case ERole::ERole_enum_count:
            roleString = "enum_count";
            break;
        }

        UtlString deviceName;
        // Set to null when no devices available
        if (defaultDeviceId)
        {
            MpidWinMM::getWinNameForDevice(mDeviceEnumeratorPtr, defaultDeviceId, deviceName);
        }

        OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
            "MpWinInputAudioDeviceNotifier::OnDefaultDeviceChanged(%s, %s, \"%s\")",
            flowString.data(),
            roleString.data(),
            deviceName.data());

        return(S_OK);
    };

    HRESULT _stdcall OnDeviceAdded(LPCWSTR addedDeviceId)
    {
        UtlString deviceName;
        MpidWinMM::getWinNameForDevice(mDeviceEnumeratorPtr, addedDeviceId, deviceName);
        OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
            "MpWinInputAudioDeviceNotifier::OnDeviceAdded(%s)",
            deviceName.data());

        return(S_OK);
    };

    HRESULT _stdcall OnDeviceRemoved(LPCWSTR removedDeviceId)
    {
        UtlString deviceName;
        MpidWinMM::getWinNameForDevice(mDeviceEnumeratorPtr, removedDeviceId, deviceName);

        OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
            "MpWinInputAudioDeviceNotifier::OnDeviceRemoved(%s)",
            deviceName.data());

        return(S_OK);
    };

    HRESULT _stdcall OnDeviceStateChanged(LPCWSTR deviceId,
                                 DWORD newState)
    {
        UtlString deviceName;
        MpidWinMM::getWinNameForDevice(mDeviceEnumeratorPtr, deviceId, deviceName);
        UtlString stateString("unknown");
        bool posted = false;
        OsStatus status = OS_UNSPECIFIED;

        switch (newState)
        {
        case DEVICE_STATE_ACTIVE:
            stateString = "active";
            if (nameIsSame(deviceName, mName))
            {
                posted = true;
            }
            if (mpInputDeviceManager)
            {
                MpResNotificationMsg msg(MpResNotificationMsg::MPRNM_INPUT_DEVICE_NOW_PRESENT, deviceName);
                status = mpInputDeviceManager->postNotification(msg);
            }
            break;

        case DEVICE_STATE_DISABLED:
            stateString = "disabled";
            break;

        case DEVICE_STATE_NOTPRESENT:
            stateString = "not_present";
            if (nameIsSame(deviceName, mName))
            {
                *mpIsOpen = FALSE;
                posted = true;
                if (mpInputDeviceManager)
                {
                    MpResNotificationMsg msg(MpResNotificationMsg::MPRNM_INPUT_DEVICE_NOT_PRESENT, mName);
                    status = mpInputDeviceManager->postNotification(msg);
                }
            }
            break;

        case DEVICE_STATE_UNPLUGGED:
            stateString = "unplugged";
            break;
        }

        OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
            "MpWinInputAudioDeviceNotifier::OnDeviceStateChanged(%s, %s) same: %s, status: %d",
            deviceName.data(),
            stateString.data(),
            posted ? "true" : "false",
            status);

        return(S_OK);
    };

    HRESULT _stdcall OnPropertyValueChanged(LPCWSTR deviceId,
                                   const PROPERTYKEY key)
    {
        UtlString deviceName;
        MpidWinMM::getWinNameForDevice(mDeviceEnumeratorPtr, deviceId, deviceName);
        
        LPOLESTR* guidString = NULL;
        StringFromCLSID(key.fmtid, guidString);

        OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
            "MpWinInputAudioDeviceNotifier::OnPropertyValueChanged(%s, xxx)",
            deviceName,
            ""/*guidString*/);
        
        ::CoTaskMemFree(guidString);

        return(S_OK);
    };

    UtlString mName;
    MpInputDeviceManager* mpInputDeviceManager;
    UtlBoolean* mpIsOpen;
    IMMDeviceEnumerator* mDeviceEnumeratorPtr;
};

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
, mWinAudioDeviceChangeCallback(NULL)
, mDeviceEnumeratorPtr(NULL)
{

    // Need to do this in each thread we call CoCreateInstance
    mDeviceEnumeratorPtr = getWinDeviceEnumerator();
    if (mDeviceEnumeratorPtr != NULL)
    {
        OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
            "MpidWinMM::MpidWinMM: "
            "Got IMMDeviceEnumerator!");
        IMMDeviceCollection* winDeviceCollection = NULL;
        HRESULT result = mDeviceEnumeratorPtr->EnumAudioEndpoints(EDataFlow::eCapture, // input devices
            DEVICE_STATE_ACTIVE,
            &winDeviceCollection);

    
        // TODO: this secion needs abunch of error checking
        UINT winDeviceCount = 0;
        if (result == S_OK && winDeviceCollection)
        {
            result = winDeviceCollection->GetCount(&winDeviceCount);
            
            for (int devIndex = 0; devIndex < winDeviceCount; devIndex++)
            {
                IMMDevice* winDevicePtr = NULL;
                result = winDeviceCollection->Item(devIndex, &winDevicePtr);
                if (result == S_OK && winDevicePtr)
                {
                    LPWSTR winDeviceId = NULL;
                    result = winDevicePtr->GetId(&winDeviceId);

                    if (result == S_OK && winDeviceId)
                    {
                        IPropertyStore* winDeviceProperties = NULL;
                        result = winDevicePtr->OpenPropertyStore(STGM_READ, &winDeviceProperties);
                        if (result == S_OK && winDeviceProperties)
                        {
                            PROPVARIANT winDeviceFriendlyName;
                            PropVariantInit(&winDeviceFriendlyName);
                            result = winDeviceProperties->GetValue(PKEY_Device_FriendlyName, &winDeviceFriendlyName);
                            if (result == S_OK)
                            {
                                OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
                                    "MpidWinMM::MpidWinMM: "
                                    "windeviceId[%d]: %ls name: %ls", devIndex, winDeviceId, winDeviceFriendlyName.pwszVal);

                                PropVariantClear(&winDeviceFriendlyName);
                            }
                            SAFE_RELEASE(winDeviceProperties);
                        }
                        CoTaskMemFree(winDeviceId);
                        winDeviceId = NULL;
                    }
                    SAFE_RELEASE(winDevicePtr);
                }
            }
        }


    }

    // Register derived handler from IMMNotificationClient
    // Register for notification of device hardware availablity
    mWinAudioDeviceChangeCallback = new MpWinInputAudioDeviceNotifier(name, deviceManager, &mIsOpen, mDeviceEnumeratorPtr);
    registerDeviceEnumerator(mDeviceEnumeratorPtr, mWinAudioDeviceChangeCallback);

    OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
        "MpidWinMM::MpidWinMM:(name=\"%s\")", name.data());

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
            OsSysLog::add(FAC_MP, PRI_ERR,
                "MpidWinMM::MpidWinMM waveInGetDevCaps[%d] returned: %d", i, wavResult);
        } 
        else if (strncmp(name, devCaps.szPname, MAXPNAMELEN) == 0)
        {
            mWinMMDeviceId = i;
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpidWinMM::MpidWinMM found \"%s\" at: %d",
                devCaps.szPname,
                i);
            break;
        }
        else
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpidWinMM::MpidWinMM looking for: \"%s\" found: \"%s\"", 
                name.data(), devCaps.szPname);
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
    OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
        "MpidWinMM::~MpidWinMM \"%s\"",
        getDeviceName().data());

    // If we happen to still be enabled at this point, disable the device.
    assert(!isEnabled());
    if (isEnabled())
    {
        disableDevice();
    }

    unregisterDeviceEnumerator(mDeviceEnumeratorPtr, mWinAudioDeviceChangeCallback);
    delete mWinAudioDeviceChangeCallback;
    mWinAudioDeviceChangeCallback = NULL;

    // TODO: need to unallocate mDeviceEnumeratorPtr?

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
   //assert(nDevs != 0);  // This assert is preventing CI builds from running unit tests (CI machines don't have audio devices)

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
   // assert(wavResult == MMSYSERR_NOERROR); // This assert is preventing CI builds from running unit tests (CI machines don't have audio devices)
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

    // zero out the wave buffer.
    memset(mpWaveBuffers[n], 0, mWaveBufSize);

    // Set wave header data to initial values.
    pWave_hdr->lpData = mpWaveBuffers[n];
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
    if(!mIsOpen)
    {
        if(uMsg == WIM_DATA)
        {
            OsSysLog::add(FAC_MP, PRI_WARNING,
                "MpidWinMM::processAudioInput received WIM_DATA while !mIsOpen");
        }
        else if(uMsg == WIM_OPEN)
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpidWinMM::processAudioInput received WIM_OPEN");
            mIsOpen = TRUE;
        }
        else if(uMsg == WIM_CLOSE)
        {
            OsSysLog::add(FAC_MP, PRI_WARNING,
                "MpidWinMM::processAudioInput received WIM_CLOSE while !mIsOpen");
        }
        else
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                "MpidWinMM::processAudioInput received unexpected uMsg: %d", uMsg);
            OsSysLog::flush();
            assert(uMsg != 0);
        }
    }
    else if (uMsg == WIM_DATA)
    {
//        printf("received WIM_DATA\n"); fflush(stdout);
        WAVEHDR* pWaveHdr = (WAVEHDR*)dwParam1;
        assert(pWaveHdr->dwBufferLength 
               == (mSamplesPerFrame*sizeof(MpAudioSample)));
        assert(pWaveHdr->lpData != NULL);

        // Only process if we're enabled..
        if(mIsEnabled)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpidWinMM::processAudioInput got frame device: %d (%s)", 
                getDeviceId(), getDeviceName().data());
#endif
           mpInputDeviceManager->pushFrame(mDeviceId,
                                           mSamplesPerFrame,
                                           (MpAudioSample*)pWaveHdr->lpData,
                                           mCurrentFrameTime);

           // Ok, we have received and pushed a frame to the manager,
           // Now we advance the frame time.
           mCurrentFrameTime += (mSamplesPerFrame*1000)/mSamplesPerSec;
        }
        else
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpidWinMM::processAudioInput input device: %d (%s) disabled", 
                getDeviceId(), getDeviceName().data());
        }

        if(mIsEnabled)
        {
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

              if (res == MMSYSERR_NODRIVER)
              {
                  mIsOpen = FALSE;
                  MpResNotificationMsg msg(MpResNotificationMsg::MPRNM_INPUT_DEVICE_NOT_PRESENT, getDeviceName());
                  /*OsStatus status =*/ mpInputDeviceManager->postNotification(msg);
              }
           }
        }
    }
    else if (uMsg == WIM_CLOSE)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MpidWinMM::processAudioInput received WIM_CLOSE");
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

IMMDeviceEnumerator* MpidWinMM::getWinDeviceEnumerator()
{
    IMMDeviceEnumerator* deviceEnumeratorPtr = NULL;

    HRESULT result = CoInitialize(NULL);
    if (result != S_OK)
    {
        OsSysLog::add(FAC_AUDIO, PRI_ERR,
            "MpidWinMM::getWinDeviceEnumeratpr CoInitialize failed with error: %p", result);
    }

    // TODO This should be done once, per thread???

    result = CoCreateInstance(__uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_INPROC_SERVER, //CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)& deviceEnumeratorPtr);
    if(result != S_OK || deviceEnumeratorPtr == NULL)
    {
        OsSysLog::add(FAC_AUDIO, PRI_ERR,
            "MpidWinMM::getWinDeviceEnumeratpr: "
            "Couldn't get IMMDeviceEnumerator result: %p!", result);
    }

    return(deviceEnumeratorPtr);
}

void MpidWinMM::registerDeviceEnumerator(IMMDeviceEnumerator* deviceEnumeratorPtr, IMMNotificationClient* winAudioDeviceChangeCallback)
{
    if (deviceEnumeratorPtr)
    {
        HRESULT result = deviceEnumeratorPtr->RegisterEndpointNotificationCallback(winAudioDeviceChangeCallback);
        if (result != S_OK)
        {
            OsSysLog::add(FAC_AUDIO, PRI_ERR,
                "MpidWinMM::registerDeviceEnumerator: "
                "Couldn't RegisterEndpointNotificationCallback result: %u!", result);
        }
        else
        {
            OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
                "MpidWinMM::registerDeviceEnumerator: "
                "RegisterEndpointNotificationCallback succeeded!");
        }
    }
    else
    {
        OsSysLog::add(FAC_AUDIO, PRI_ERR,
            "MpidWinMM::registerDeviceEnumerator NULL deviceEnumeratorPtr unable to register mWinAudioDeviceChangeCallback");
    }
}

void MpidWinMM::unregisterDeviceEnumerator(IMMDeviceEnumerator* deviceEnumeratorPtr, IMMNotificationClient* winAudioDeviceChangeCallback)
{
    if (winAudioDeviceChangeCallback)
    {
        if (deviceEnumeratorPtr)
        {
            HRESULT result = deviceEnumeratorPtr->UnregisterEndpointNotificationCallback(winAudioDeviceChangeCallback);
            if (result != S_OK)
            {
                OsSysLog::add(FAC_AUDIO, PRI_ERR,
                    "MpidWinMM::unregisterDeviceEnumerator: "
                    "Couldn't UnregisterEndpointNotificationCallback result: %u!", result);
            }
            else
            {
                OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
                    "MpidWinMM::unregisterDeviceEnumerator: "
                    "UnregisterEndpointNotificationCallback succeeded!");
            }
        }
        else
        {
            OsSysLog::add(FAC_AUDIO, PRI_ERR,
                "MpidWinMM::unregisterDeviceEnumerator NULL deviceEnumeratorPtr unable to unregister mWinAudioDeviceChangeCallback");
        }
    }
    else
    {
        OsSysLog::add(FAC_AUDIO, PRI_ERR,
            "MpidWinMM::unregisterDeviceEnumerator NULL winAudioDeviceChangeCallback unable to unregister mWinAudioDeviceChangeCallback");
    }
}

void MpidWinMM::getWinNameForDevice(IMMDeviceEnumerator* deviceEnumeratorPtr, const LPCWSTR winDeviceId, UtlString& deviceName)
{
    deviceName = "";
    if (deviceEnumeratorPtr)
    {
        IMMDevice* winDevicePtr = NULL;
        HRESULT result = deviceEnumeratorPtr->GetDevice(winDeviceId, &winDevicePtr);
        if (result == S_OK && winDevicePtr)
        {
            IPropertyStore* winDeviceProperties = NULL;
            result = winDevicePtr->OpenPropertyStore(STGM_READ, &winDeviceProperties);
            if (result == S_OK && winDeviceProperties)
            {
                PROPVARIANT winDeviceFriendlyName;
                PropVariantInit(&winDeviceFriendlyName);
                result = winDeviceProperties->GetValue(PKEY_Device_FriendlyName, &winDeviceFriendlyName);
                if (result == S_OK)
                {
                    char deviceNameChar[256];
                    size_t nameSize = wcstombs(deviceNameChar, winDeviceFriendlyName.pwszVal, sizeof(deviceNameChar)-1);
                    if (nameSize > 0 && deviceNameChar[0])
                    {
                        deviceName = deviceNameChar;
                        
                        OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
                            "MpidWinMM::getWinNameForDevice: "
                            "windeviceId: %ls name: %s", winDeviceId, deviceNameChar);
                    }
                    else
                    {
                        OsSysLog::add(FAC_AUDIO, PRI_ERR,
                            "MpidWinMM::getWinNameForDevice: "
                            "windeviceId: %ls name not valid???? size: %zu", winDeviceId, nameSize);
                    }

                    PropVariantClear(&winDeviceFriendlyName);
                }
                SAFE_RELEASE(winDeviceProperties);
            }
            SAFE_RELEASE(winDevicePtr);
        }
        else
        {
            OsSysLog::add(FAC_AUDIO, PRI_WARNING,
                "MpidWinMM::getWinNameForDevice: "
                "Failed to get IMMDevice resu;t: %d",
                result);
        }
    }
    else
    {
        OsSysLog::add(FAC_AUDIO, PRI_WARNING,
            "MpidWinMM::getWinNameForDevice: "
            "Failed to get IMMDeviceEnumerator");
    }
}

bool MpidWinMM::nameIsSame(const UtlString& a, const UtlString& b)
{
    bool nameSame = false;
    int lenA = a.length();
    int lenB = b.length();
    if (lenA > 1 && lenB > 1)
    {
        if (lenA > lenB)
        {
            if (a.index(b) == 0)
            {
                nameSame = true;
            }
            else if (b.index(a) == 0)
            {
                nameSame = true;
            }
        }
    }

    OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
        "MpidWinMM::nameIsSame(\"%s\", \"%s\") %s",
        a.data(),
        b.data(),
        nameSame ? "true" : "false");

    return(nameSame);
};