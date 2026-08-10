//  
// Copyright (C) 2007-2026 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>


// SYSTEM INCLUDES
#include <os/OsSysLog.h>
#include <os/OsLock.h>
#include <utl/UtlVoidPtr.h>
#include <os/OsCallback.h>

#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#else
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#endif

#ifndef RTL_AUDIO_ENABLED
#     define RTL_AUDIO_BUFFER(w,x,y,z)
#     define RTL_RAW_AUDIO(v,w,x,y,z)
#endif

// APPLICATION INCLUDES
#include <mmdeviceapi.h>

#include <mp/MpodWinMM.h>
#include <mp/MpidWinMM.h>
#include <mp/MpOutputDeviceManager.h>
#include <mp/MpResNotificationMsg.h>
#include <mp/MpMMTimer.h>

// DEFINES
#define LOW_WAVEBUF_LVL 7
#define LOW_MESSAGE_QUEUE_LEN (2*LOW_WAVEBUF_LVL + 5)

//#define TEST_PRINT
/// This define enable use of MpCodec_set/getVolume() and MpCodec_set/getGain().
/// Note, this is a temporary hack, enabling volume regulation with new audio IO
/// system. We will rewrite volume regulation in better manner later, so do not
/// rely on this.
#define USE_OLD_VOLUME_REGULATION_CODE

#if defined(_MSC_VER) && (_MSC_VER < 1300) // if < msvc7 (2003)
#  define CBTYPE DWORD
#else
#  define CBTYPE DWORD_PTR
#endif

// Comment out the line below (or build with -DWMM_TRACE=1) to enable
// per-call trace instrumentation. Used during win_device_changes
// investigation; kept gated for future diagnostics during Changes 9
// and 10. Remove this and all WMM_TRACE_PRINTF call sites once those
// changes are complete and stable.
//#define WMM_TRACE 1

#ifdef WMM_TRACE
#  define WMM_TRACE_PRINTF(...) do { printf(__VA_ARGS__); fflush(stdout); } while(0)
#else
#  define WMM_TRACE_PRINTF(...) ((void)0)
#endif

// EXTERNAL FUNCTIONS
extern void showWaveError(char *syscall, int e, int N, int line) ;  // dmaTaskWnt.cpp

// EXTERNAL VARIABLES
#ifdef USE_OLD_VOLUME_REGULATION_CODE // [
// This variables are used in MpCodec.cpp to get/set audio volume. THIS IS A HACK!
extern HWAVEOUT audioOutH;
extern HWAVEOUT audioOutCallH;
#endif // USE_OLD_VOLUME_REGULATION_CODE ]

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

//
// Convention for mWinMMDeviceId vs mDevHandle:
//
//   mDevHandle = NULL          --> no live wave resource (normal when
//                                  disabled, also set on transient error
//                                  recovery paths to release the handle).
//   mWinMMDeviceId = -1         --> this object no longer represents any
//                                  Windows device. Set only at construction
//                                  time when the named device is not found.
//                                  NEVER set this from runtime error paths;
//                                  doing so makes the device permanently
//                                  unrecoverable, defeating USB
//                                  unplug/replug recovery.
//   mpTickerTimer != NULL      --> in fallback mode due to a wave-API
//                                  failure or device-removed COM event.
//                                  enableDevice will reattempt waveOutOpen
//                                  from the same mWinMMDeviceId on the
//                                  next enable.
//

class MpodWinMM::MpWinOutputAudioDeviceNotifier : public IMMNotificationClient
{
public:

    MpWinOutputAudioDeviceNotifier(MpodWinMM* outputDevice, 
        const UtlString& deviceName, 
        MpOutputDeviceManager* outputManager,
        IMMDeviceEnumerator* deviceEnumeratorPtr) : IMMNotificationClient(),
        mpOutputDevice(outputDevice),
        mName(deviceName),
        mpOutputDeviceManager(outputManager),
        mDeviceEnumeratorPtr(deviceEnumeratorPtr),
        mRefCount(1)
    {
    };

    ULONG _stdcall AddRef()
    {
        return((ULONG)::InterlockedIncrement(&mRefCount));
    };

    ULONG _stdcall Release()
    {
        return((ULONG)::InterlockedDecrement(&mRefCount));
    };

    HRESULT _stdcall QueryInterface(REFIID riid,
        void** ppvObject)
    {
        HRESULT hr = E_NOINTERFACE;

        if (ppvObject)
        {
            *ppvObject = NULL;

            if ((riid == __uuidof(IUnknown)) ||
                (riid == __uuidof(IMMNotificationClient)))
            {
                *ppvObject = static_cast<IMMNotificationClient*>(this);
                AddRef();
                hr = S_OK;
            }
        }
        else
        {
            hr = E_POINTER;
        }

        return(hr);
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
            "MpWinOutputAudioDeviceNotifier::OnDefaultDeviceChanged(%s, %s, \"%s\")",
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
            "MpWinOutputAudioDeviceNotifier::OnDeviceAdded(%s)",
            deviceName.data());

        return(S_OK);
    };

    HRESULT _stdcall OnDeviceRemoved(LPCWSTR removedDeviceId)
    {
        UtlString deviceName;
        MpidWinMM::getWinNameForDevice(mDeviceEnumeratorPtr, removedDeviceId, deviceName);

        OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
            "MpWinOutputAudioDeviceNotifier::OnDeviceRemoved(%s)",
            deviceName.data());

        return(S_OK);
    };

    HRESULT _stdcall OnDeviceStateChanged(LPCWSTR deviceId,
        DWORD newState)
    {
        UtlString deviceName;
        MpidWinMM::getWinNameForDevice(mDeviceEnumeratorPtr, deviceId, deviceName);
        UtlString stateString("unknown");
        UtlString flowString("unknown");
        bool posted = false;
        OsStatus status = OS_UNSPECIFIED;

        MpidWinMM::MpAudioEndpointFlow flow = MpidWinMM::MP_FLOW_UNKNOWN;
        bool flowKnown = MpidWinMM::getEndpointDataFlow(mDeviceEnumeratorPtr, deviceId, flow);
        if (flowKnown)
        {
            flowString = (flow == MpidWinMM::MP_FLOW_RENDER)   ? "render"  :
                         (flow == MpidWinMM::MP_FLOW_CAPTURE)  ? "capture" : "other";
        }

        switch (newState)
        {
        case DEVICE_STATE_ACTIVE:
            stateString = "active";
            if (!flowKnown)
            {
                OsSysLog::add(FAC_AUDIO, PRI_WARNING,
                    "MpWinOutputAudioDeviceNotifier::OnDeviceStateChanged"
                    " could not determine flow for active device %s, skipping",
                    deviceName.data());
                break;
            }
            if (flow != MpidWinMM::MP_FLOW_RENDER)
            {
                OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
                    "MpWinOutputAudioDeviceNotifier::OnDeviceStateChanged"
                    " skipping non-render active device %s flow %s",
                    deviceName.data(), flowString.data());
                break;
            }
            if (MpidWinMM::nameIsSame(deviceName, mName))
            {
                posted = true;
            }
            if (mpOutputDeviceManager)
            {
                MpResNotificationMsg msg(MpResNotificationMsg::MPRNM_OUTPUT_DEVICE_NOW_PRESENT, deviceName);
                status = mpOutputDeviceManager->postNotification(msg);
            }
            else
            {
                OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpWinOutputAudioDeviceNotifier::OnDeviceStateChanged"
                    " NULL mpOutputDeviceManager, no where to post DEVICE_NOW_PRESENT: (%s)",
                    deviceName.data());
            }
            break;

        case DEVICE_STATE_DISABLED:
            stateString = "disabled";
            break;

        case DEVICE_STATE_NOTPRESENT:
            stateString = "not_present";
            if (MpidWinMM::nameIsSame(deviceName, mName))
            {
                posted = true;
                if (mpOutputDevice)
                {
                    OsSysLog::add(FAC_AUDIO, PRI_INFO,
                        "MpWinOutputAudioDeviceNotifier::OnDeviceStateChanged"
                        " device not present, switching to MMTimer: %s",
                        deviceName.data());
                    mpOutputDevice->switchToMMTimer();
                }
            }
            else if (mpOutputDeviceManager)
            {
                if (!flowKnown)
                {
                    OsSysLog::add(FAC_AUDIO, PRI_WARNING,
                        "MpWinOutputAudioDeviceNotifier::OnDeviceStateChanged"
                        " could not determine flow for not-present device %s, skipping notification",
                        deviceName.data());
                    break;
                }
                if (flow != MpidWinMM::MP_FLOW_RENDER)
                {
                    OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
                        "MpWinOutputAudioDeviceNotifier::OnDeviceStateChanged"
                        " skipping non-render not-present device %s flow %s",
                        deviceName.data(), flowString.data());
                    break;
                }
                MpResNotificationMsg msg(MpResNotificationMsg::MPRNM_OUTPUT_DEVICE_NOT_PRESENT, deviceName);
                status = mpOutputDeviceManager->postNotification(msg);
            }
            else
            {
                OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodWinMM::OnDeviceStateChanged NULL mpOutputDeviceManager,"
                    " no where to post DEVICE_NOT_PRESENT");
            }
            break;

        case DEVICE_STATE_UNPLUGGED:
            stateString = "unplugged";
            break;
        }

        OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
            "MpWinOutputAudioDeviceNotifier::OnDeviceStateChanged(%s, %s, %s) same: %s, status: %d devMgr: %p",
            deviceName.data(),
            stateString.data(),
            flowString.data(),
            posted ? "true" : "false",
            status,
            mpOutputDeviceManager);

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
            "MpWinOutputAudioDeviceNotifier::OnPropertyValueChanged(%s, %s)",
            deviceName,
            guidString);

        ::CoTaskMemFree(guidString);

        return(S_OK);
    };

    MpodWinMM* mpOutputDevice;
    UtlString mName;
    MpOutputDeviceManager* mpOutputDeviceManager;
    IMMDeviceEnumerator* mDeviceEnumeratorPtr;
    LONG mRefCount;
};

/* ============================ CREATORS ================================== */
// Default constructor
MpodWinMM::MpodWinMM(const UtlString& name, 
                     MpOutputDeviceManager* outputManager,
                     unsigned nOutputBuffers)
: MpOutputDeviceDriver(name)
, OsCallback(NULL, NULL)
, mpOutputManger(outputManager)
, mEmptyHdrVPtrListsMutex(OsMutex::Q_FIFO)
, mWinMMDeviceId(-1)
, mDevHandle(NULL)
, mCurFrameTime(0)
, mNumOutBuffers(nOutputBuffers)
, mWaveBufSize(0)  // Unknown until enableDevice()
, mUnderrunLength(0)
, mTotSampleCount(0)
, mWinAudioDeviceChangeCallback(NULL)
, mpTickerTimer(NULL)
, mFallbackSignalCount(0)
, mFallbackDiscardCount(0)
, mFallbackFramePeriodMs(0)
, mDeviceEnumeratorPtr(NULL)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MpodWinMM::MpodWinMM(%s)", getDeviceName().data());

    mDeviceEnumeratorPtr = MpidWinMM::getWinDeviceEnumerator();
    // Register derived handler from IMMNotificationClient
    // Register for notification of device hardware availablity
    mWinAudioDeviceChangeCallback = new MpWinOutputAudioDeviceNotifier(this, name, mpOutputManger, mDeviceEnumeratorPtr);
    MpidWinMM::registerDeviceEnumerator(mDeviceEnumeratorPtr, mWinAudioDeviceChangeCallback);

   // Resolve the WinMM device index from the device name. Same
   // helper is used at the top of enableDevice for renumber
   // recovery; using it here keeps post-construction state
   // identical to what enableDevice would resolve immediately
   // afterward.
   mWinMMDeviceId = resolveDeviceIdByName();

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

   unsigned i;
   for ( i = 0; i < mNumOutBuffers; i++ )
   {
      mpWaveBuffers[i] = NULL;

      // Initialize the unused void pointers.
      // add mNumOutBuffers new void pointers to the list.
      mUnusedVPtrList.insert(new UtlVoidPtr());
   }

   // Create synchronization thread
   mCallbackEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
   mExitFlag = FALSE;

   mCallbackThread = CreateThread(NULL, 0, ThreadMMProc, this, 0, NULL);
   SetThreadPriority(mCallbackThread, REALTIME_PRIORITY_CLASS);
   //THREAD_PRIORITY_HIGHEST);
   // SetThreadPriority(mCallbackThread, THREAD_PRIORITY_LOWEST );

#ifdef DONTUSE_SLIST
   for( i = 0; i < LOW_MESSAGE_QUEUE_LEN; i++)
   {
      WinAudioData* ProgramItem = new WinAudioData;
      ProgramItem->mCbParamHdr = NULL;
      ProgramItem->mCbParamMsg = 0;

      mListFree.append(ProgramItem);
   }

#else
   // Initialize the list header.
   InitializeSListHead(&mPoolFree);
   InitializeSListHead(&mPoolSignaled);

   // Insert items into the list.
   for( i = 0; i < LOW_MESSAGE_QUEUE_LEN; i++)
   {
      WinAudioDataChain* ProgramItem = (WinAudioDataChain*)_aligned_malloc(sizeof(*ProgramItem),
         MEMORY_ALLOCATION_ALIGNMENT);
      ProgramItem->mCbParamHdr = NULL;
      ProgramItem->mCbParamMsg = 0;

      InterlockedPushEntrySList(&mPoolFree, 
         &ProgramItem->ItemEntry); 
   }
#endif

}


// Destructor
MpodWinMM::~MpodWinMM() 
{
   OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MpodWinMM::~MpodWinMM(%s)", getDeviceName().data());

   // Unregister the callback interface
   MpidWinMM::unregisterDeviceEnumerator(mDeviceEnumeratorPtr, mWinAudioDeviceChangeCallback);
   delete mWinAudioDeviceChangeCallback;
   mWinAudioDeviceChangeCallback = NULL;

   // TODO: need to unallocate mDeviceEnumeratorPtr?

   // We shouldn't be enabled, assert that we aren't.
   // If we happen to still be enabled at this point, disable the device.
   //assert(!isEnabled());  // Commented out as it causes issues with unit test.

   // Wait until the windows buffer fill thread finishes.
   //waitUntilShutDown();

   if ( isEnabled() )
   {
      disableDevice();
   }

   if (mpTickerTimer)
   {
       logFallbackSummary("~MpodWinMM");
       OsStatus status = mpTickerTimer->stop();
       if (status != OS_SUCCESS)
       {
           OsSysLog::add(FAC_MP, PRI_ERR,
               "MpodWinMM::~MpodWinMM MMTimer stop failed: %d", (int)status);
       }
       else
       {
           OsSysLog::add(FAC_MP, PRI_DEBUG,
               "MpodWinMM::~MpodWinMM MMTimer stopped");
       }
       delete mpTickerTimer;
       mpTickerTimer = NULL;
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
   int freedCount = 0;
   for ( i = 0; i < mNumOutBuffers; i++ )
   {
       if (mpWaveBuffers[i] != NULL)
       {
           delete[] mpWaveBuffers[i];
           mpWaveBuffers[i] = NULL;
           freedCount++;
       }
   }
   if (freedCount)
   {
       OsSysLog::add(FAC_MP, PRI_WARNING,
           "MpodWinMM::~MpodWinMM %d mpWaveBuffers not NULL", freedCount);
   }
   delete[] mpWaveBuffers;
   delete[] mpWaveHeaders;

   // Delete synchronization thread
   mExitFlag = TRUE;
   SetEvent(mCallbackEvent);
   WaitForSingleObject(mCallbackThread, INFINITE);

#ifdef DONTUSE_SLIST
   mListSignaled.destroyAll();
   mListFree.destroyAll();
#else
   for (;;)
   {
      WinAudioDataChain* pListEntry = 
         (WinAudioDataChain*)InterlockedPopEntrySList(&mPoolSignaled);

      if( NULL == pListEntry )
         break;

      _aligned_free(pListEntry);
   }

   for (;;)
   {
      WinAudioDataChain* pListEntry = 
         (WinAudioDataChain*)InterlockedPopEntrySList(&mPoolFree);

      if( NULL == pListEntry )
         break;

      _aligned_free(pListEntry);
   }
#endif

   OsSysLog::add(FAC_MP, PRI_DEBUG,
       "MpodWinMM::~MpodWinMM exit");
}

/* ============================ MANIPULATORS ================================ */
OsStatus MpodWinMM::enableDevice(unsigned samplesPerFrame, 
                                 unsigned samplesPerSec, 
                                 MpFrameTime currentFrameTime,
                                 OsCallback &frameTicker)
{
   OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MpodWinMM::enableDevice");
   WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu enableDevice ENTRY mIsEnabled=%d mDevHandle=%p mWinMMDeviceId=%d mpTickerTimer=%p\n",
         GetCurrentThreadId(), (int)mIsEnabled, mDevHandle, mWinMMDeviceId, mpTickerTimer);

   // Re-resolve the WinMM device index by name. Windows can
   // renumber devices between construction and now (e.g. USB
   // unplug/replug), so the cached index may be stale or now
   // belong to a different device. Always look up the current
   // index from the current device list. If the named device
   // is not currently enumerated, we treat that as
   // OS_INVALID_STATE -- same behavior as today's
   // not-found-at-construction case.
   int currentId = resolveDeviceIdByName();
   if (currentId != mWinMMDeviceId)
   {
      OsSysLog::add(FAC_MP, PRI_INFO,
         "MpodWinMM::enableDevice: WINDEVICE_RENUMBER detected for "
         "'%s': WinMM index changed from %d to %d. (If you see "
         "this in production logs, the win_device_changes Change 9 "
         "renumber-recovery path is doing real work for this "
         "customer.)",
         getDeviceName().data(),
         mWinMMDeviceId,
         currentId);
   }
   mWinMMDeviceId = currentId;

   // If the device is not valid, let the user know it's bad.
   if ( !isDeviceValid() )
   {
      return OS_INVALID_STATE;
   }

   if ( isEnabled() )
   {
      return OS_FAILED;
   }

   // Clean up fallback timer if it was activated due to a previous device failure.
   // The timer will be recreated by switchToMMTimer if needed again.
   if (mpTickerTimer)
   {
       OsSysLog::add(FAC_MP, PRI_DEBUG,
           "MpodWinMM::enableDevice stopping and deleting fallback MMTimer");
       logFallbackSummary("enableDevice");
       mpTickerTimer->stop();
       delete mpTickerTimer;
       mpTickerTimer = NULL;
   }

   WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu enableDevice past_timer_cleanup mpTickerTimer=%p mDevHandle=%p mWinMMDeviceId=%d\n",
          GetCurrentThreadId(), mpTickerTimer, mDevHandle, mWinMMDeviceId);

   // Set some wave header stat information.
   mSamplesPerFrame = samplesPerFrame;
   mSamplesPerSec = samplesPerSec;
   mCurFrameTime = currentFrameTime;
   mUnderrunLength = 0;
   mTotSampleCount = 0;
   mpTickerNotification = &frameTicker;

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

   WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu enableDevice about_to_waveOutOpen mWinMMDeviceId=%d\n",
          GetCurrentThreadId(), mWinMMDeviceId);

   MMRESULT res = waveOutOpen(&mDevHandle, mWinMMDeviceId, &wavFormat, 
                               (CBTYPE)waveOutCallbackStatic, 
                               (CBTYPE)this, 
                               CALLBACK_FUNCTION);

   WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu enableDevice waveOutOpen_returned res=%d mDevHandle=%p\n",
          GetCurrentThreadId(), (int)res, mDevHandle);

   if( res != MMSYSERR_NOERROR )
   {
      // If waveOutOpen failed, print out the error info,
      // invalidate the handle, and the device driver itself,
      showWaveError("MpodWinMM::enableDevice", res, -1, __LINE__);
      waveOutClose(mDevHandle);
      mDevHandle = NULL; // Open didn't work, reset device handle to NULL
      // mWinMMDeviceId intentionally NOT cleared (see file header
      // comment): waveOutOpen failure does not prove the device is gone.

      switchToMMTimer();

      // and return OS_FAILED.
      return OS_FAILED;
   }

#ifdef USE_OLD_VOLUME_REGULATION_CODE // [
   // This variables are used in MpCodec.cpp to get/set audio volume. THIS IS A HACK!
   audioOutH = mDevHandle;
   audioOutCallH = mDevHandle;
#endif // USE_OLD_VOLUME_REGULATION_CODE ]

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
         // mWinMMDeviceId intentionally NOT cleared.

         switchToMMTimer();

         // and return OS_FAILED.
         return OS_FAILED;
      }
   }

   mIsEnabled = TRUE;

   OsStatus pushStat = OS_SUCCESS;
   // Push silent frames to the device to kick-start it,
   // so that if it is in mixer mode, notifications are sent.
   // Push the minimum we can, as this adds latency equal to the number of
   // silence frames we push (LOW_WAVEBUF_LVL frames).
   for( i = 0; pushStat == OS_SUCCESS && i < LOW_WAVEBUF_LVL+1; i++ )
   {
      pushStat = internalPushFrame(getSamplesPerFrame(), NULL, mCurFrameTime);
      RTL_EVENT("MpodWinMM::pushFrame::driverLatencyNSamples", (i+1)*getSamplesPerFrame());
   }

   if( pushStat != OS_SUCCESS )
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
                    "During windows device driver enable, "
                    "Error kickstart pushing silent frames!");
      waveOutClose(mDevHandle);
      mDevHandle = NULL;
      // mWinMMDeviceId intentionally NOT cleared.

      // and return OS_FAILED.
      return OS_FAILED;
   }


#ifdef DONTUSE_SLIST
   for (;;)
   {
      WinAudioData* pListEntry = (WinAudioData*)mListSignaled.at(0);
      if( NULL == pListEntry )
         break;
      mListSignaled.remove(pListEntry);

      pListEntry->mCbParamHdr = NULL;
      pListEntry->mCbParamMsg = 0;

      mListFree.append(pListEntry);
   }
#else
   for (;;)
   {
      WinAudioDataChain* pListEntry = 
         (WinAudioDataChain*)InterlockedPopEntrySList(&mPoolSignaled);

      if( NULL == pListEntry )
         break;

      pListEntry->mCbParamHdr = NULL;
      pListEntry->mCbParamMsg = 0;

      InterlockedPushEntrySList(&mPoolFree, 
         &pListEntry->ItemEntry);
   }
#endif

   WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu enableDevice EXIT(success) mDevHandle=%p mWinMMDeviceId=%d mpTickerTimer=%p\n",
          GetCurrentThreadId(), mDevHandle, mWinMMDeviceId, mpTickerTimer);

   return OS_SUCCESS;
}


OsStatus MpodWinMM::disableDevice()
{
   WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu disableDevice ENTRY mIsEnabled=%d mDevHandle=%p mpTickerTimer=%p\n",
          GetCurrentThreadId(), (int)mIsEnabled, mDevHandle, mpTickerTimer);

   OsStatus status = OS_SUCCESS;
   MMRESULT   res;

   OsSysLog::add(FAC_MP, PRI_DEBUG,
       "MpodWinMM::disableDevice()");
   // If the device is (not valid) or (not enabled),
   // then don't do anything and return failure.
   if ( !isDeviceValid() )
   {
      // If the device is not valid, let the user know it's bad.
      mIsEnabled = FALSE;
      return OS_INVALID_STATE;
   }
   if ( !isEnabled() )
   {
      return OS_FAILED;
   }

   // Indicate we are no longer enabled -- Do this first,
   // since we'll be partially disabled from here on out.
   mIsEnabled = FALSE;

   // mDevHandle may be NULL here if the wave device experienced a
   // failure (NODRIVER, etc.) and was nulled by an error path before
   // disableDevice was reached. That is NOT a reason to short-circuit
   // the cleanup -- the wave handle is gone but the buffer-pointer
   // lists, allocated mpWaveBuffers, and sample-rate state still need
   // to be reset, otherwise the next enableDevice will fail in the
   // buffer-allocation loop (mUnusedVPtrList drained empty, NULL deref
   // on UtlVoidPtr::setValue). resetDevice and the waveOutClose block
   // below both already handle a NULL handle gracefully.
   if (mDevHandle == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_NOTICE,
                    "MpodWinMM::disableDevice: handle was already NULL "
                    "(prior failure?); proceeding with non-wave cleanup.");
   }

   status = resetDevice();

#ifdef USE_OLD_VOLUME_REGULATION_CODE // [
   // This variables are used in MpCodec.cpp to get/set audio volume. THIS IS A HACK!
   audioOutH = NULL;
   audioOutCallH = NULL;
#endif // USE_OLD_VOLUME_REGULATION_CODE ]

   if (mDevHandle)
   {
      res = waveOutClose(mDevHandle);
      if ( res != MMSYSERR_NOERROR )
      {
         showWaveError("waveOutClose", res, -1, __LINE__);
      }
   }

   // Delete the buffers that were allocated in enableDevice()
   for (int i = 0; i < mNumOutBuffers; i++ )
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

   WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu disableDevice EXIT status=%d mUnusedVPtrList.entries=%u mEmptyHeaderList.entries=%u\n",
          GetCurrentThreadId(), (int)status,
          mUnusedVPtrList.entries(), mEmptyHeaderList.entries());

   return status;
}

OsStatus MpodWinMM::resetDevice()
{
    OsStatus status = OS_SUCCESS;
    MMRESULT res;

    // Wave-API reset, only meaningful if we still hold a handle.
    if (mDevHandle)
    {
        // Reset performs a stop, returns all the buffers within
        // windows multimedia.
        res = waveOutReset(mDevHandle);
        if (res != MMSYSERR_NOERROR)
        {
            showWaveError("waveOutReset", res, -1, __LINE__);
            if (res == MMSYSERR_NODRIVER)
            {
                mDevHandle = NULL;
                // mWinMMDeviceId intentionally NOT cleared.
                // Keep the ticks going until the device is switch
                // to a valid one.
                switchToMMTimer();
            }
        }
    }

    // Drain mEmptyHeaderList back into mUnusedVPtrList. This MUST
    // run regardless of whether mDevHandle was non-NULL on entry:
    // the void-pointer list is internal bookkeeping and is needed
    // for the next enableDevice's buffer-alloc loop. Skipping this
    // when mDevHandle is already NULL (e.g. signal() nulled it
    // during fallback) leaves the next enable with a starved
    // mUnusedVPtrList and a NULL deref in the buffer-alloc loop.
    mEmptyHdrVPtrListsMutex.acquire();
    unsigned nEmpties = mEmptyHeaderList.entries();
    unsigned i;
    for (i = 0; i < nEmpties; i++)
    {
        mUnusedVPtrList.insert(mEmptyHeaderList.get());
    }
    mEmptyHdrVPtrListsMutex.release();

    WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu resetDevice drain_done mUnusedVPtrList.entries=%u mEmptyHeaderList.entries=%u\n",
           GetCurrentThreadId(),
           mUnusedVPtrList.entries(), mEmptyHeaderList.entries());

    // Unprepare the wave headers, only if we still hold a handle.
    // Must happen after waveOutReset and before waveOutClose;
    // otherwise MMSYSERR_INVALHANDLE will be returned.
    if (mDevHandle)
    {
        for (i = 0; i < mNumOutBuffers; i++)
        {
            res = waveOutUnprepareHeader(mDevHandle, &mpWaveHeaders[i], sizeof(WAVEHDR));
            if (res != MMSYSERR_NOERROR)
            {
                showWaveError("waveOutUnprepareHeader", res, i, __LINE__);
            }
        }
    }

    return (status);
}

int MpodWinMM::resolveDeviceIdByName()
{
    // Walk WinMM's current waveOut device enumeration looking for
    // a device whose Pname matches our mName (the UtlString base).
    // Returns the current index (>= 0) on match, or -1 if no
    // match. Always re-queries Windows; never consults a cached
    // value. Used at construction and at the top of every
    // enableDevice call so the driver is robust to Windows
    // renumbering devices across unplug/replug.
    //
    // Continues walking after the first match to detect Pname
    // collisions (typically caused by MAXPNAMELEN truncation
    // collapsing distinct device names into the same string).
    // Logs a warning if a collision is found; first match still
    // wins, matching constructor behavior.
    UINT nDevs = waveOutGetNumDevs();
    WAVEOUTCAPS devCaps;
    int firstMatchIdx = -1;
    for (UINT i = 0; i < nDevs; i++)
    {
        MMRESULT res = waveOutGetDevCaps(i, &devCaps, sizeof(devCaps));
        if (res != MMSYSERR_NOERROR)
        {
            // Skip this index; one bad device should not prevent
            // discovery of others.
            continue;
        }
        if (strncmp(getDeviceName().data(), devCaps.szPname, MAXPNAMELEN) == 0)
        {
            if (firstMatchIdx < 0)
            {
                firstMatchIdx = (int)i;
            }
            else
            {
                // Multiple WinMM entries report the same szPname.
                // szPname is MAXPNAMELEN (32) chars including null,
                // so distinct device names that share a 31-char
                // prefix collapse into one string here. The first
                // match wins (matching constructor behavior), but
                // the choice is ambiguous: the application may have
                // intended a different device than the one we are
                // about to use.
                OsSysLog::add(FAC_MP, PRI_WARNING,
                   "MpodWinMM::resolveDeviceIdByName: device name "
                   "'%s' matches multiple WinMM entries (at least "
                   "indices %d and %d). Likely szPname truncation "
                   "(MAXPNAMELEN=%d). Using first match (%d). The "
                   "application may be opening a different device "
                   "than intended. Possible workaround: rename the "
                   "devices in Windows Sound settings to differ "
                   "within the first %d characters; this works on "
                   "some systems but depends on whether the driver "
                   "populates szPname from the editable friendly "
                   "name.",
                   getDeviceName().data(),
                   firstMatchIdx, (int)i, MAXPNAMELEN, firstMatchIdx,
                   MAXPNAMELEN - 1);
                break;  // one warning is enough
            }
        }
    }
    return firstMatchIdx;
}

OsStatus MpodWinMM::pushFrame(unsigned int numSamples,
                              const MpAudioSample* samples,
                              MpFrameTime frameTime)
{
   OsStatus status = OS_FAILED;

   if ( !isEnabled() )
   {
      // be sure to release the mutex prior to returning.
    // WHY????  Who took the mutex???
      mEmptyHdrVPtrListsMutex.release();

      return OS_FAILED;
   }

   // Only full frames are supported right now.
   // If samples == NULL, then silent (full) frame should be inserted.
   assert(mSamplesPerFrame == numSamples);

   if(samples != NULL)
   {
      RTL_RAW_AUDIO("MpodWinMM_pushFrame", mSamplesPerSec, numSamples, samples,
                    frameTime*(mSamplesPerSec/1000)/mSamplesPerFrame);
   }

   // Push the frame of real data we got here out to the windows output device,
   // using an internal method that actually performs writing to the audio
   // device.
   status = internalPushFrame(numSamples, samples, frameTime);

   // If we have a MMTimer, something has gone wrong with the output device
   // So don't touch the wave device if we don't have to.  We could get hung.
   if (mDevHandle == NULL)
   {
   }

   // If the first internalPushFrame succeeded, then go on and see if we need
   // to push any silence due to low buffers.
   else if(status == OS_SUCCESS)
   {
      // Collect some metrics -- the sample number that windows is on 
      // since waveOutOpen was called.
      MMRESULT   res;
      MMTIME mmt;
      mmt.wType = TIME_SAMPLES;
      res = waveOutGetPosition(mDevHandle, &mmt, sizeof(mmt));
      if(res != MMSYSERR_NOERROR)
      {
          showWaveError("MpodWinMM::pushFrame - waveOutGetPosition",
              res, -1, __LINE__);

          if (res == MMSYSERR_NODRIVER)
          {
              mDevHandle = NULL;
              // mWinMMDeviceId intentionally NOT cleared. A failed
              // metrics query does not prove the device is gone.

              // Keep the ticks going until the device is switch to a valid one
              switchToMMTimer();
          }
      }
      assert(mmt.wType == TIME_SAMPLES);

      // Write out some statistics, if enabled.
      DWORD drvLatencyNSamp = mTotSampleCount - mmt.u.sample;
      int numBuffersInPlay = mUnusedVPtrList.entries();
      RTL_EVENT("MpodWinMM::pushFrame::driverLatencyNSamples", drvLatencyNSamp);
      RTL_EVENT("MpodWinMM::pushFrame::emptyHeaders", mEmptyHeaderList.entries());
      RTL_EVENT("MpodWinMM::pushFrame::numBuffersInPlay", numBuffersInPlay);

      // If the number of samples held within windows MME subsystem gets below
      // LOW_WAVEBUF_LVL frames worth, count this as underrun.
//      if (drvLatencyNSamp <= LOW_WAVEBUF_LVL*80)
      if (numBuffersInPlay <= LOW_WAVEBUF_LVL)
      {
         mUnderrunLength++;
      }
      else
      {
         mUnderrunLength = 0;
      }

      // If underrun persist for more then 3 frames, inject a frame of silence.
      // Note: Underrun length was introduced, because many windows drivers
      //       generate WOM_DONE messages with slightly slower speed then real
      //       playback is going. To keep them in sync driver sometimes fire
      //       WOM_DONE twice without any delay. And at this moment (and for one
      //       frame processing interval only) number of frames held is dropped
      //       by one and then go up to normal level again. To avoid silence
      //       insertion in this frequent case we're taking into account only
      //       underruns with long enough length.
      // P.S. Number 3 here is took from my mind as a small number bigger then 1.
      if (mUnderrunLength > LOW_WAVEBUF_LVL - 3)
      {
         RTL_BLOCK("MpodWinMM::pushFrame::inject");
#ifdef TEST_PRINT // [
         printf("^");
#endif // TEST_PRINT ]

         // write out silence to the device.
         status = internalPushFrame(getSamplesPerFrame(), NULL, mCurFrameTime);
      }
   }

   return status;
}

OsStatus MpodWinMM::internalPushFrame(unsigned int numSamples, 
                                      const MpAudioSample* samples,
                                      MpFrameTime frameTime)
{
   // Set up our status code that we'll return - assume failure.
   OsStatus status = OS_FAILED;

   // If we have a mpTickerTimer (MMTimer) its because something has happened
   // to the output device (e.g. error or removed).  So we need to stop trying to push
   // to the output as we could hang in wave calls.
   if (mpTickerTimer == NULL)
   {
       // We'll be accessing the vptr and empty header lists, so acquire the mutex
       mEmptyHdrVPtrListsMutex.acquire();


       // If there are empty headers, we can push a frame
       if (mEmptyHeaderList.entries() > 0)
       {
           UtlVoidPtr* pvpWaveHdr = (UtlVoidPtr*)mEmptyHeaderList.get();
           WAVEHDR* pWaveHdr = (WAVEHDR*)(pvpWaveHdr->getValue());

           // Reset the voidptr to null and add it to the unused vptr list for reuse.
           pvpWaveHdr->setValue(NULL);
           mUnusedVPtrList.insert(pvpWaveHdr);

           // Cannot hold the mutex while performing wave calls.
           mEmptyHdrVPtrListsMutex.release();

           MMRESULT res = waveOutPrepareHeader(mDevHandle, pWaveHdr, sizeof(WAVEHDR));
           if (res != MMSYSERR_NOERROR)
           {
               showWaveError("MpodWinMM::internalPushFrame - waveOutPrepareHeader",
                   res, -1, __LINE__);

               if (res == MMSYSERR_NODRIVER)
               {
                   mDevHandle = NULL;
                   // mWinMMDeviceId intentionally NOT cleared.


                   // Keep the ticks going until the device is switch to a valid one
                   switchToMMTimer();
                   // Have to lie to keep the flowgraph going
                   status = OS_SUCCESS;
               }
               else
               {
                   waveOutClose(mDevHandle);
                   mDevHandle = NULL;
                   // mWinMMDeviceId intentionally NOT cleared.
                   switchToMMTimer();
                   // Have to lie to keep the flowgraph going
                   status = OS_SUCCESS;
               }

               return(status);
           }

           // We found an empty buffer, now we fill it with data or silence.
           if (samples)
           {
               memcpy(pWaveHdr->lpData, samples, sizeof(MpAudioSample) * numSamples);
#ifdef TEST_PRINT // [
               printf("|");
#endif // TEST_PRINT ]
           }
           else
           {
               memset(pWaveHdr->lpData, 0, sizeof(MpAudioSample) * numSamples);
#ifdef TEST_PRINT // [
               printf(".");
#endif // TEST_PRINT ]
           }

           // And send it on it's way to windows wave interface.
           res = waveOutWrite(mDevHandle, pWaveHdr, sizeof(WAVEHDR));
           if (res != MMSYSERR_NOERROR)
           {
               showWaveError("MpodWinMM::internalPushFrame", res, -1, __LINE__);
               // If it's more than just an unprepared header
               // (invalid handle, no driver, or a memory allocation or lock error)
               if (res == MMSYSERR_NODRIVER)
               {
                   OsSysLog::add(FAC_MP, PRI_ERR,
                       "waveOutWrite to removed device, need to switch devices or use CPU ticker");

                   mDevHandle = NULL;
                   // mWinMMDeviceId intentionally NOT cleared.

                   // Keep the ticks going until the device is switch to a valid one
                   switchToMMTimer();
                   // Have to lie and say everything is fine and keep the flowgraph going.
                   status = OS_SUCCESS;
               }

               else if (res != WAVERR_UNPREPARED)
               {
                   // Then close the device. mWinMMDeviceId is left
                   // intact -- the named device may still be valid.
                   waveOutClose(mDevHandle);
                   mDevHandle = NULL;
                   switchToMMTimer();
                   // Have to lie to keep the flowgraph going
                   status = OS_SUCCESS;
               }
           }
           else // res != MMSYSERR_NOERROR
           {
               // Increment the current frame time.
               mCurFrameTime += getFramePeriod(numSamples, mSamplesPerSec);

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
               "MpodWinMM::internalPushFrame: "
               "No free buffers! Dropping frame.");
       }
   }

   // Running on the MMTimer as something is wrong with the output device
   else
   {
#ifdef TEST_PRINT
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MpodWinMM::internalPushFrame mpTickerTimer no pushing or shoving");
#endif

      // Counted, not logged. internalPushFrame is downstream of signal()
      // on the same thread, so a divergence between these two counts in
      // the periodic line means the flowgraph stopped producing frames
      // while the fallback timer kept ticking.
      mFallbackDiscardCount++;
      if (mFallbackDiscardCount == 1)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
            "MpodWinMM::internalPushFrame discarding output frames for "
            "device '%s' while in fallback mode",
            getDeviceName().data());
      }

      // We lie, to keep the flowgraph going
      status = OS_SUCCESS;
   }

   return status;
}

// TODO:
//  Need a method to shut down the device when not_present
// Another method to re-initialize when present again????                                      

                                      
OsStatus MpodWinMM::switchToMMTimer()
{
    // Don't switch to the fallback timer if the device is not enabled.
    // This prevents the COM callback thread from modifying mpTickerTimer
    // while enableDevice/disableDevice is in progress on the media task thread.
    if (!isEnabled())
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MpodWinMM::switchToMMTimer device not enabled, ignoring");
        return OS_SUCCESS;
    }

    if (mDevHandle)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
            "MpodWinMM::switchToMMTimer called with active wave device: %p %d",
            mDevHandle,
            mWinMMDeviceId);
    }

    OsStatus status = OS_FAILED;
    // If this output device is providing the ticks
    if(mpTickerNotification)
    {
        if (mpTickerTimer == NULL)
        {
            // Build a CPU based ticker as the output device is broken
            // and we need ticks to prevent the media subsystem from hanging
            mpTickerTimer = MpMMTimer::create(MpMMTimer::Notification);
            if (mpTickerTimer == NULL)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpodWinMM::switchToMMTimer NULL timer");

            }

            else
            {
                // Set the notifier/consumer of the tick messages
                status = mpTickerTimer->setNotification(this);
                if (status != OS_SUCCESS)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR,
                        "MpodWinMM::switchToMMTimer error setting timer notifier: %d", (int)status);
                }

                else
                {
                    // Start with tick period microseconds
                    unsigned int periodMicroSecs = (1000 * // milliseconds/microsecond
                        1000 * mSamplesPerFrame) / mSamplesPerSec;  // milliseconds/frame

                    // Each fallback episode counts from zero so the totals in the
                    // periodic and exit log lines are per-episode, not cumulative.
                    mFallbackSignalCount = 0;
                    mFallbackDiscardCount = 0;
                    mFallbackFramePeriodMs = (int)getFramePeriod();

                    status = mpTickerTimer->run(periodMicroSecs);
                    if (status != OS_SUCCESS)
                    {
                        OsSysLog::add(FAC_MP, PRI_ERR,
                            "MpodWinMM::switchToMMTimer error starting timer period: %d error: %d", 
                            periodMicroSecs,
                            (int)status);
                    }
                    else
                    {
                        OsSysLog::add(FAC_MP, PRI_DEBUG,
                            "MpodWinMM::switchToMMTimer successfully starting MMtimer period: %d uSec", periodMicroSecs);
                    }
                }
            }

            // Notify that this output device is removed or dead
            // Do we need to differentiate???
            if (mpOutputManger)
            {
                MpResNotificationMsg msg(MpResNotificationMsg::MPRNM_OUTPUT_DEVICE_NOT_PRESENT, getDeviceName());
                status = mpOutputManger->postNotification(msg);
            }
            else
            {
                OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodWinMM::switchToMMTimer NULL mpOutputManager, no where to post DEVICE_NOT_PRESENT");
            }
        }
        else
        {
            OsSysLog::add(FAC_MP, PRI_WARNING,
                "MpodWinMM::switchToMMTimer mpTickerTimer already set");
        }
    }
    else
    {
        OsSysLog::add(FAC_MP, PRI_WARNING,
            "MpodWinMM::switchToMMTimer NULL mpTickerNotification");
    }


    return(status);
}

/// @brief callback used by MpMMTimer when output device is not providing ticks
OsStatus MpodWinMM::signal(const intptr_t eventData)
{
    WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu signal ENTRY mIsEnabled=%d mDevHandle=%p mpTickerNotification=%p\n",
           GetCurrentThreadId(), (int)mIsEnabled, mDevHandle, mpTickerNotification);

    if (mDevHandle)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
            "MpodWinMM::signal called with active wave device: %p %d, %d buffers used. leaking device??",
            mDevHandle,
            mWinMMDeviceId,
            mUnusedVPtrList.entries());

        // Don't close device as it sometimes hangs.
        mDevHandle = NULL;
        // mWinMMDeviceId intentionally NOT cleared. The defensive
        // cleanup of mDevHandle is appropriate (we cannot prove the
        // handle is still safe to close), but invalidating the device
        // identity here was the root cause of the customer "no audio
        // after reconnect" symptom: the next enableDevice would refuse
        // because isDeviceValid() returned false. The named device may
        // still be present; let the next enableDevice retry.
    }

    // send a ticker notification tokeep the MediaTask processing frames
    if (isEnabled() && mpTickerNotification != NULL)
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MpodWinMM::signal MpMMTimer signaling notifier from device: %d",
            getDeviceName().data());
#endif
        OsStatus status = mpTickerNotification->signal(mCurFrameTime);
        if (status != OS_SUCCESS)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                "MpodWinMM::signal mpTickerNotification signal failed: %d", status);
        }
        else
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpodWinMM::signal mpTickerNotification signal succeeded");
#endif

            if (mpTickerTimer != NULL)
            {
               // Rate-limited fallback reporting. The per-tick lines above are
               // 100 Hz and unusable in production; these are ~0.2 Hz.
               // Both counters are incremented on this same thread: signal()
               // drives internalPushFrame() synchronously through the flowgraph
               // ticker, so no atomics are needed. Revisit if the flowgraph
               // ticker ever becomes a queued cross-thread dispatch.
               mFallbackSignalCount++;

               if (mFallbackSignalCount == 1)
               {
                  OsSysLog::add(FAC_MP, PRI_WARNING,
                     "MpodWinMM::signal fallback timer active for device '%s', "
                     "output audio discarded until device recovers",
                     getDeviceName().data());
               }
               else
               {
                  int framePeriodMs = mFallbackFramePeriodMs;
                  int ticksPerLog = (framePeriodMs > 0) ? (5000 / framePeriodMs) : 500;
                  if (ticksPerLog < 1) ticksPerLog = 1;

                  if ((mFallbackSignalCount % ticksPerLog) == 0)
                  {
                     OsSysLog::add(FAC_MP, PRI_NOTICE,
                        "MpodWinMM::signal fallback active %d sec: %d ticks, "
                        "%d frames discarded",
                        (mFallbackSignalCount * framePeriodMs) / 1000,
                        mFallbackSignalCount,
                        mFallbackDiscardCount);
                  }
               }
            }
        }
    }
#ifdef TEST_PRINT
    else
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MpodWinMM::signal %s MpMMTimer signal ignored enable: %s notifier: %p",
            getDeviceName().data(),
            isEnabled(),
            mpTickerNotification);
    }
#endif

    WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu signal EXIT\n", GetCurrentThreadId());

    return(OS_SUCCESS);
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
   // assert(nDevs != 0); // This assert is preventing CI builds from running unit tests (CI machines don't have audio devices)

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
   // assert(wavResult == MMSYSERR_NOERROR); // This assert is preventing CI builds from running unit tests (CI machines don't have audio devices)

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
   WMM_TRACE_PRINTF("WMM_TRACE: tid=%lu finalizeProcessedHeader ENTRY mIsEnabled=%d mUnusedVPtrList.entries=%u mEmptyHeaderList.entries=%u\n",
          GetCurrentThreadId(), (int)mIsEnabled,
          mUnusedVPtrList.entries(), mEmptyHeaderList.entries());

   mEmptyHdrVPtrListsMutex.acquire();

   // Check to see if we have any free pointers, and if we're enabled.
   if(isEnabled() && mUnusedVPtrList.entries() > 0)
   {
      // Grab an unused voidptr, set it's value to pWaveHdr,
      UtlVoidPtr* pvptr = (UtlVoidPtr*)(mUnusedVPtrList.get());
      pvptr->setValue(pWaveHdr);
      // And add it to the empty header list for use by pushFrame.
      mEmptyHeaderList.insert(pvptr);
      mEmptyHdrVPtrListsMutex.release();

      // send a ticker notification so that more frames can be sent.
      if(mpTickerNotification != NULL)
      {
#ifdef TEST_PRINT
         OsSysLog::add(FAC_MP, PRI_DEBUG,
             "MpodWinMM::finalizeProcessedHeader signaling notifier from device: %d",
             getDeviceName().data());
#endif
         mpTickerNotification->signal(mCurFrameTime);
      }
      else
      {
#ifdef TEST_PRINT
          OsSysLog::add(FAC_MP, PRI_DEBUG,
              "MpodWinMM::finalizeProcessedHeader NULL tickerNotifier for device: %s",
              getDeviceName().data());
#endif
      }
   }
   else
   {
       mEmptyHdrVPtrListsMutex.release();
   }

}

DWORD WINAPI MpodWinMM::ThreadMMProc(LPVOID lpMessage)
{
   MpodWinMM* oddWinMMPtr = (MpodWinMM*)lpMessage;

   for (;;)
   {
      DWORD res = WaitForSingleObject(oddWinMMPtr->mCallbackEvent, INFINITE);
      assert (WAIT_OBJECT_0 == res);

      if (oddWinMMPtr->mExitFlag == TRUE)
      {
          OsSysLog::add(FAC_MP, PRI_DEBUG,
              "MpodWinMM::ThreadMMProc exiting, mExitFlag == TRUE");
         return 0;
      }

      for (;;)
      {
#ifdef DONTUSE_SLIST
         WinAudioData* pListEntry = (WinAudioData*)oddWinMMPtr->mListSignaled.at(0);
         if( NULL == pListEntry )
            break;
         oddWinMMPtr->mListSignaled.remove(pListEntry);

         UINT msg = pListEntry->mCbParamMsg;
         WAVEHDR* hdr = pListEntry->mCbParamHdr;

         oddWinMMPtr->mListFree.append(pListEntry);
#else
         WinAudioDataChain* pListEntry = 
            (WinAudioDataChain*)InterlockedPopEntrySList(&oddWinMMPtr->mPoolSignaled);

         if (pListEntry == NULL)
            break;

         UINT msg = pListEntry->mCbParamMsg;
         WAVEHDR* hdr = pListEntry->mCbParamHdr;

         InterlockedPushEntrySList(&oddWinMMPtr->mPoolFree, &pListEntry->ItemEntry);
#endif

         switch(msg)
         {
         case WOM_OPEN:
#ifdef TEST_PRINT // [
            printf("WOM_OPEN\n");
#endif // TEST_PRINT ]
            OsSysLog::add(FAC_MP, PRI_INFO, 
               "Windows output device driver callback "
               "device open (WOM_OPEN).");
            break;
         case WOM_DONE:
#ifdef TEST_PRINT
             OsSysLog::add(FAC_MP, PRI_DEBUG, "MpodWinMM::ThreadMMProc WOM_DONE received");
#endif
            oddWinMMPtr->finalizeProcessedHeader(hdr);
            break;
         case WOM_CLOSE:
#ifdef TEST_PRINT // [
            printf("WOM_CLOSE\n");
#endif // TEST_PRINT ]
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
   }
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpodWinMM::ThreadMMProc exiting");
}

void MpodWinMM::logFallbackSummary(const char* context)
{
   if (mFallbackSignalCount == 0 && mFallbackDiscardCount == 0)
   {
      return;
   }

   int framePeriodMs = mFallbackFramePeriodMs;
   int elapsedSec = (framePeriodMs > 0)
                    ? (mFallbackSignalCount * framePeriodMs) / 1000
                    : 0;

   OsSysLog::add(FAC_MP, PRI_NOTICE,
      "MpodWinMM::%s leaving fallback for device '%s' after %d ticks "
      "(%d sec), %d frames discarded",
      context,
      getDeviceName().data(),
      mFallbackSignalCount,
      elapsedSec,
      mFallbackDiscardCount);
}

/* //////////////////////// PROTECTED STATIC //////////////////////////////// */
void CALLBACK 
MpodWinMM::waveOutCallbackStatic(HWAVEOUT hwo,
                                 UINT uMsg, 
                                 void* dwInstance,
                                 void* dwParam1, 
                                 void* dwParam2)
{
   // It's no good to use asserts there 
   assert(dwInstance != NULL);
   MpodWinMM* oddWinMMPtr = (MpodWinMM*)dwInstance;
  // assert(uMsg == WOM_OPEN || hwo == oddWinMMPtr->mDevHandle);
   if (!((uMsg == WOM_OPEN) || (hwo == oddWinMMPtr->mDevHandle)))
   {
      OutputDebugStringA("[MpodWinMM::waveOutCallbackStatic] msg_failed\n");
      return;
   }

#ifdef DONTUSE_SLIST
   WinAudioData* pListEntry = (WinAudioData*)oddWinMMPtr->mListFree.at(0);
   if( NULL == pListEntry )
   {
      assert(!"All list exhousted");
   }
   oddWinMMPtr->mListFree.remove(pListEntry);

   pListEntry->mCbParamHdr = (WAVEHDR*)dwParam1;
   pListEntry->mCbParamMsg = uMsg;

   oddWinMMPtr->mListSignaled.append(pListEntry);

#else

   WinAudioDataChain* pListEntry = 
      (WinAudioDataChain*)InterlockedPopEntrySList(&oddWinMMPtr->mPoolFree);

   if( NULL == pListEntry )
   {
      assert(!"All list exhousted");
   }

   pListEntry->mCbParamHdr = (WAVEHDR*)dwParam1;
   pListEntry->mCbParamMsg = uMsg;

   InterlockedPushEntrySList(&oddWinMMPtr->mPoolSignaled, 
      &pListEntry->ItemEntry);
#endif

   SetEvent(oddWinMMPtr->mCallbackEvent);
}

/* ///////////////////////////// PRIVATE //////////////////////////////////// */
