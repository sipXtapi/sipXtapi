// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a 
// Contributor Agreement.  Contributors retain copyright to elements 
// licensed under a Contributor Agreement.  Licensed to the User under the 
// LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>
#include <assert.h>

// APPLICATION INCLUDES
#include "include/VoiceEngineFactoryImpl.h"
#include "include/VoiceEngineBufferInStream.h"
#include "mediaBaseImpl/CpMediaNetTask.h"
#include "mediaBaseImpl/CpMediaSocketMonitorTask.h"
#include "include/VoiceEngineSocketFactory.h"
//#include "mi/CpMediaInterfaceFactoryFactory.h"

#include "os/OsPerfLog.h"
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "os/OsTimerTask.h"
#include "os/OsProtectEventMgr.h"
#include "os/OsNameDb.h"
#include "os/OsLock.h"
#include "sdp/SdpCodec.h"
#include "sdp/SdpCodecList.h"
#include "net/NetBase64Codec.h"
#include "utl/UtlSListIterator.h"

#ifdef WIN32
#include <mmsystem.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CONFIG_PHONESET_SEND_INBAND_DTMF  "PHONESET_SEND_INBAND_DTMF"
#define MAX_MANAGED_FLOW_GRAPHS           16
#define MAX_AUDIO_DEVICES                 16
// MACROS
#if !defined(_WIN32)
#define __declspec(x)
#endif

// STATIC VARIABLE INITIALIZATIONS
//static CpMediaInterfaceFactory* spFactory = NULL;

OsMutex sGuard(OsMutex::Q_FIFO);
VoiceEngineFactoryImpl* spMediaFactory = NULL;
int iMediaFactoryCount = 0;

extern "C" __declspec(dllexport) IMediaDeviceMgr* createMediaDeviceMgr()
{
    OsLock lock(sGuard);
    iMediaFactoryCount++;
    if (NULL == spMediaFactory)
    {
        spMediaFactory = new VoiceEngineFactoryImpl();
    }
    return spMediaFactory;
}

extern "C" __declspec(dllexport) void releaseMediaDeviceMgr(void* pDevice)
{
    OsLock lock(sGuard);
    if (iMediaFactoryCount <= 1)
    {
        if (pDevice == spMediaFactory)
        {
            spMediaFactory = NULL;
        }
        IMediaDeviceMgr* pMediaDeviceMgr = (IMediaDeviceMgr*)pDevice;
        pMediaDeviceMgr->destroy();
        delete pMediaDeviceMgr;
    }
    iMediaFactoryCount--;
}

extern "C" __declspec(dllexport) int getReferenceCount()
{
    OsLock lock(sGuard);
    return iMediaFactoryCount;
}




// STATIC FUNCTIONS
static void VoiceEngineLogCallback(void* pObj, char *pData, int nLength)
{
    if (pData != NULL)
    {
#ifdef ENCODE_GIPS_LOGS
        UtlString encodedData ;        
        NetBase64Codec::encode(nLength, pData, encodedData) ;
        OsSysLog::add(FAC_VOICEENGINE, PRI_DEBUG, encodedData.data()) ;
#else
        OsSysLog::add(FAC_VOICEENGINE, PRI_DEBUG, pData) ;
#endif                      

#ifdef GIPS_LOG_FLUSH
        OsSysLog::flush() ;
#endif
    }
}

static void VideoEngineLogCallback(GIPSTraceCallbackObject obj, char *pData, int nLength)
{
    if ((nLength > 0) && pData != NULL)
    {
#ifdef ENCODE_GIPS_LOGS
        UtlString encodedData ;        
        NetBase64Codec::encode(nLength, pData, encodedData) ;        
        OsSysLog::add(FAC_VIDEOENGINE, PRI_DEBUG, encodedData.data()) ;
#else
        OsSysLog::add(FAC_VIDEOENGINE, PRI_DEBUG, pData) ;
#endif        
    }
#ifdef GIPS_LOG_FLUSH
    OsSysLog::flush() ;
#endif
}


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
VoiceEngineFactoryImpl::VoiceEngineFactoryImpl() :
    mCurrentWaveInDevice(-1),
    mCurrentWaveOutDevice(-1),
    mVideoQuality(0),
    mVideoBitRate(70),
    mVideoFrameRate(15),
    mCpu(40),
    mTrace(false),
    mbMute(false),
    mbEnableAGC(false),
    mRtpPortLock(OsMutex::Q_FIFO),
    mbDTMFInBand(TRUE),
    mIdleTimeout(30),
    mVideoCaptureDevice(""),
    mAECMode(MEDIA_AEC_CANCEL_AUTO),
    mNRMode(MEDIA_NOISE_REDUCTION_LOW),
    mpInStream(NULL),
    mpMediaNetTask(NULL),
    mpVideoEngine(NULL),
    mpVoiceEngine(NULL),
    mLocalConnectionId(-1),
    mAudioDeviceInput(MediaDeviceInfo::MDIT_AUDIO_INPUT),
    mAudioDeviceOutput(MediaDeviceInfo::MDIT_AUDIO_OUTPUT) 
{    
    OS_PERF_FUNC("VoiceEngineFactoryImpl::VoiceEngineFactoryImpl") ;

}

void VoiceEngineFactoryImpl::initialize(OsConfigDb* pConfigDb, 
                         uint32_t fameSizeMs, 
                         uint32_t maxSamplesPerSec,
                         uint32_t defaultSamplesPerSec)
{
        UtlString strOOBBandDTMF ;
    mbCreateLocalConnection = true ;
    mVideoFormat = 0 ;
    mbDTMFOutOfBand = TRUE;
    mpPreviewWindowDisplay = NULL ;
    mLocalConnectionId = -1 ;
    mbLocalConnectionInUse = false ;
    mbSpeakerAdjust = false; 
    miVolume = 90 ;
    mbEnableRTCP = true ;
    mpVoiceEngine = NULL ;
    mpVideoEngine = NULL ;
    mpStaticVideoEngine = NULL ;

    mpMediaNetTask = CpMediaNetTask::createCpMediaNetTask() ;
    mpFactory = new VoiceEngineSocketFactory(this) ;

#ifdef _WIN32
    UtlString overrideSettings ;
    UtlString temp ;
    HKEY hKey ;
    DWORD dwValue ;
    DWORD dwSize ;
    char cTemp[256] ;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIPXTAPI_OVERRIDE_KEY, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "audioTimeout", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("audioTimeout=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoFormat", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("videoFormat=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoFramerate", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("videoFramerate=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoBitrate", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("videoBitrate=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoQuality", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("videoQuality=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoCPU", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("videoCPU=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(cTemp)-1 ;
        memset(cTemp, 0, sizeof(cTemp)) ;
        if (RegQueryValueEx(hKey, "videoCodecs", 0, NULL, (LPBYTE) cTemp, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("videoCodecs=%s\n", cTemp) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(cTemp)-1 ;
        memset(cTemp, 0, sizeof(cTemp)) ;
        if (RegQueryValueEx(hKey, "videoCodecOrder", 0, NULL, (LPBYTE) cTemp, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("videoCodecOrder=%s\n", cTemp) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(cTemp)-1 ;
        memset(cTemp, 0, sizeof(cTemp)) ;
        if (RegQueryValueEx(hKey, "audioCodecs", 0, NULL, (LPBYTE) cTemp, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("audioCodecs=%s\n", cTemp) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(cTemp)-1 ;
        memset(cTemp, 0, sizeof(cTemp)) ;
        if (RegQueryValueEx(hKey, "audioCodecOrder", 0, NULL, (LPBYTE) cTemp, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("audioCodecOrder=%s\n", cTemp) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "videoQuality", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("videoQuality=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "consoleTrace", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("consoleTrace=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "enableRtcp", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("enableRtcp=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "ignoreCameraCaps", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("ignoreCameraCaps=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        dwSize = sizeof(DWORD) ;
        if (RegQueryValueEx(hKey, "mediaContactType", 0, NULL, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
        {
            temp.format("mediaContactType=%d\n", dwValue) ;
            overrideSettings.append(temp) ;
        }

        RegCloseKey(hKey) ;

        OsSysLog::add(FAC_MP, PRI_NOTICE, "Overrides:\n%s", overrideSettings.data()) ;

    }
#endif
}

void VoiceEngineFactoryImpl::destroy()
{
}

// Destructor
VoiceEngineFactoryImpl::~VoiceEngineFactoryImpl()
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::~VoiceEngineFactoryImpl") ;
    OsLock lock(sGuard) ;

    stopPlay() ;
    destroyLocalAudioConnection() ;
    shutdownCpMediaNetTask() ;
    OsTask::yield() ;       

    if (mpVideoEngine)
    {
        releaseVideoEngineInstance(mpVideoEngine) ;
        mpVideoEngine = NULL;
    }

    if (mpVoiceEngine)
    {
        releaseVoiceEngineInstance(mpVoiceEngine) ;
        mpVoiceEngine = NULL;
    }

    if (mpPreviewWindowDisplay)
    {
        delete (SIPXVE_VIDEO_DISPLAY*) mpPreviewWindowDisplay;
        mpPreviewWindowDisplay = NULL;
    }

    if (mpStaticVideoEngine)
    {
        mpStaticVideoEngine->GIPSVideo_Terminate() ;
#ifdef USE_GIPS
# ifdef _WIN32
        DeleteGipsVideoEngine(mpStaticVideoEngine) ;
# else
        delete mpStaticVideoEngine;
# endif
#else
        DeleteGipsVideoEngineStub(mpStaticVideoEngine) ;        
#endif
        mpStaticVideoEngine = NULL ;
    }

    if (mpFactory)
    {
        delete mpFactory ;
        mpFactory = NULL ;
    }

    CpMediaSocketMonitorTask::releaseInstance() ;
    OsNatAgentTask::releaseInstance();
    OsTimerTask::destroyTimerTask() ;
    OsProtectEventMgr::releaseEventMgr() ;
    OsSysLog::shutdown() ;
    OsNameDb::release() ;
}

//////////////////////////////////////////////////////////////////////////////
OsStatus VoiceEngineFactoryImpl::translateToneId(const SIPX_TONE_ID toneId,
                                             SIPX_TONE_ID&      xlateId ) const
{
    OsStatus rc = OS_SUCCESS;
    if (toneId >= '0' && toneId <= '9')
    {
        xlateId = (SIPX_TONE_ID)(toneId - '0');
    } 
    else if (toneId == ID_DTMF_STAR)
    {
        xlateId = (SIPX_TONE_ID)10;
    }
    else if (toneId == ID_DTMF_POUND)
    {
        xlateId = (SIPX_TONE_ID)11;
    }
    else if (toneId == ID_DTMF_FLASH)
    {
        xlateId = (SIPX_TONE_ID)16;
    }
    else
    {
        rc = OS_FAILED;
    }
    return rc;
}

void VoiceEngineFactoryImpl::doEnableLocalChannel(bool bEnable) const
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::doEnableLocalChannel") ;
    OsLock lock(sGuard) ;
    int rc ;

    if (bEnable)
    {
        if (mLocalConnectionId == -1)
        {
            constructGlobalInstance() ;
            assert(mbLocalConnectionInUse == false) ;
            assert(mpVoiceEngine != NULL) ;
            mLocalConnectionId = mpVoiceEngine->GIPSVE_CreateChannel() ;    
            rc = mpVoiceEngine->GIPSVE_StartPlayout(mLocalConnectionId) ;
            assert(rc == 0);
        }
    }
    else
    {
        if (mLocalConnectionId != -1)
        {
            assert(mpVoiceEngine != NULL) ;
            rc = mpVoiceEngine->GIPSVE_DeleteChannel(mLocalConnectionId) ;
            assert(rc == 0);
            mLocalConnectionId = -1 ;
            mbLocalConnectionInUse = false ;
        }
    }
}



OsStatus VoiceEngineFactoryImpl::createLocalAudioConnection()
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::createLocalAudioConnection") ;
    OsLock lock(sGuard) ;
    OsStatus rc = OS_SUCCESS;

    mbCreateLocalConnection = true ;
    doEnableLocalChannel(true) ;
    return rc;
}

OsStatus VoiceEngineFactoryImpl::destroyLocalAudioConnection()
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::destroyLocalAudioConnection") ;
    OsLock lock(sGuard) ;
    OsStatus rc = OS_SUCCESS;

    mbCreateLocalConnection = false ;
    doEnableLocalChannel(false) ;

    return rc;
}

/* ============================ MANIPULATORS ============================== */
IMediaInterface* VoiceEngineFactoryImpl::createMediaInterface( const char* publicAddress,
                                                                const char* localAddress,
                                                                int numCodecs,
                                                                SdpCodec* sdpCodecArray[],
                                                                const char* locale,
                                                                int expeditedIpTos,
                                                                const ProxyDescriptor& stunServer,
                                                                const ProxyDescriptor& turnProxy,
                                                                const ProxyDescriptor& arsProxy,
                                                                const ProxyDescriptor& arsHttpProxy,
                                                                SIPX_MEDIA_PACKET_CALLBACK pMediaPacketCallback,
                                                                UtlBoolean enableIce,
                                                                uint32_t samplesPerSec) 
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::createMediaInterface") ;

    OsLock lock(sGuard) ;
    GipsVoiceEngineLib* pVoiceEngine = NULL ;
    GipsVideoEnginePlatform* pVideoEngine = NULL ;
    
    if (mbLocalConnectionInUse)
    {
        pVoiceEngine = getNewVoiceEngineInstance() ;
        pVideoEngine = getNewVideoEngineInstance(pVoiceEngine) ;
    }
    else
    {
        doEnableLocalChannel(false) ;
        constructGlobalInstance(true, (mpPreviewWindowDisplay != NULL || pMediaPacketCallback)) ;
        pVoiceEngine = mpVoiceEngine ;
        mpVoiceEngine = NULL ;
        pVideoEngine = mpVideoEngine ;
        mpVideoEngine = NULL ;
    }

    UtlBoolean enableRtcp = mbEnableRTCP ;
    applyEnableRTCPOverride(enableRtcp) ;

    VoiceEngineMediaInterface* pMediaInterface = 
            new VoiceEngineMediaInterface(this, pVoiceEngine, 
            pVideoEngine,     
            publicAddress, localAddress, 
            numCodecs, sdpCodecArray, locale, expeditedIpTos, 
            stunServer, turnProxy,
            arsProxy, arsHttpProxy,                
            mbDTMFOutOfBand, mbDTMFInBand, enableRtcp, mRtcpName,
            pMediaPacketCallback) ;
    
    // store it in our internal list, as an int
    UtlInt* piMediaInterface = new UtlInt((int) pMediaInterface);
    mInterfaceList.insert(piMediaInterface);  

    return pMediaInterface; 
}


void VoiceEngineFactoryImpl::releaseInterface(VoiceEngineMediaInterface* pMediaInterface)
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::releaseInterface") ;
    OsLock lock(sGuard) ;
    bool bReleasedVideo = false ;

    assert(pMediaInterface != NULL) ;
    if (pMediaInterface)
    {        
        mInterfaceList.destroy(& UtlInt((int)pMediaInterface));
        if (pMediaInterface->getVideoEnginePtr())
        {
            GipsVideoEnginePlatform* pVideoEngine = (GipsVideoEnginePlatform*)pMediaInterface->getVideoEnginePtr() ;
            if (pVideoEngine)
            {
                releaseVideoEngineInstance(pVideoEngine) ;
                bReleasedVideo = true ;
            }
        }
        if (pMediaInterface->getAudioEnginePtr())
        {
            GipsVoiceEngineLib* pVoiceEngine = (GipsVoiceEngineLib*)pMediaInterface->getAudioEnginePtr() ;
            if (pVoiceEngine)
            {
                releaseVoiceEngineInstance(pVoiceEngine) ;
            }
        }
    }
}

// WARNING:: Assumes someone is externally holding a lock!!
GipsVoiceEngineLib* VoiceEngineFactoryImpl::getNewVoiceEngineInstance() const
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::getNewVoiceEngineInstance") ;
    int rc ;
    GipsVoiceEngineLib* pVoiceEngine = GetNewVoiceEngineLib() ;
    if (pVoiceEngine)
    {
        // Initialize GIPS Debug Tracing
        if (getGipsTracing())
        {
            rc = pVoiceEngine->GIPSVE_SetTraceCallback(VoiceEngineLogCallback) ;
            assert(rc == 0) ;
        }

#ifdef USE_GIPS_DLL
        char* szKey = (char*) GIPS_KEY ;
        rc = pVoiceEngine->GIPSVE_Authenticate(szKey, strlen(szKey));
        assert(rc == 0) ;
#endif

#ifdef USE_GIPS
        rc = pVoiceEngine->GIPSVE_Init(false, false, GIPS_EXPIRE_MONTH, GIPS_EXPIRE_DAY, GIPS_EXPIRE_YEAR) ;
        assert(rc == 0) ;
#endif

        rc = pVoiceEngine->GIPSVE_SetSoundDevices(mCurrentWaveInDevice, mCurrentWaveOutDevice);
        assert(rc == 0) ;

        // Save the device info last used
        UtlString results ;
        inputDeviceIndexToString(results, mCurrentWaveInDevice) ;
        mAudioDeviceInput.setSelected(results) ;
        outputDeviceIndexToString(results, mCurrentWaveOutDevice) ;
        mAudioDeviceOutput.setSelected(results) ;
        

        if (!isSpeakerAdjustSet())
        {
            rc = pVoiceEngine->GIPSVE_EnableExternalMediaProcessing(true, 
                    PLAYBACK_ALL_CHANNELS_MIXED, -1, (GIPS_media_process&) *this) ;
            assert(rc == 0) ;
        }

        // Set Other Attributes
        doSetAudioAECMode(pVoiceEngine, mAECMode) ;
        doSetAudioNoiseReductionMode(pVoiceEngine, mNRMode) ;
        doEnableAGC(pVoiceEngine, mbEnableAGC) ;        
    }

    return pVoiceEngine ;
}

// WARNING:: Assumes someone is externally holding a lock!!
void VoiceEngineFactoryImpl::releaseVoiceEngineInstance(GipsVoiceEngineLib* pVoiceEngine)
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::releaseVoiceEngineInstance") ;
    int rc ;

    assert(pVoiceEngine != NULL) ;
    if (pVoiceEngine)
    {
        rc = pVoiceEngine->GIPSVE_SetTrace(0) ;
        assert(rc == 0) ;
        rc = pVoiceEngine->GIPSVE_EnableExternalMediaProcessing(false, PLAYBACK_ALL_CHANNELS_MIXED, -1, *this) ;
        assert(rc == 0) ;
        rc = pVoiceEngine->GIPSVE_Terminate();
        assert(rc == 0) ;

#ifdef USE_GIPS
        DeleteVoiceEngineLib(pVoiceEngine); 
#else
        delete pVoiceEngine;
#endif
    }
}


// WARNING:: Assumes someone is externally holding a lock!!
GipsVoiceEngineLib* VoiceEngineFactoryImpl::getAnyVoiceEngine() const
{       
    GipsVoiceEngineLib* pRC = NULL ;

    // First: Try global instance
    if (mpVoiceEngine)
        pRC = mpVoiceEngine ;

    // Next: Session
    if (pRC == NULL)                
    {
        UtlSListIterator iterator(mInterfaceList) ;
        UtlInt* pInterfaceInt ;
        while (pInterfaceInt = (UtlInt*) iterator())
        {
            VoiceEngineMediaInterface* pInterface = 
                    (VoiceEngineMediaInterface*) pInterfaceInt->getValue();            
            if (pInterface)
            {
                if (pInterface->getAudioEnginePtr())
                {
                    pRC = (GipsVoiceEngineLib*)pInterface->getAudioEnginePtr();
                }
                break ;
            }
        }
    }

    // Last: Build it
    if (pRC == NULL)
    {
        constructGlobalInstance() ;
        pRC = mpVoiceEngine ;
    }

    return pRC ;
}

// WARNING:: Assumes someone is externally holding a lock!!
GipsVideoEnginePlatform* VoiceEngineFactoryImpl::getAnyVideoEngine() const 
{
    GipsVideoEnginePlatform* pRC = NULL ;

    // First: Try global instance
    if (mpVideoEngine)
        pRC = mpVideoEngine ;

    // Next: Session
    if (pRC == NULL)                
    {
        UtlSListIterator iterator(mInterfaceList) ;
        UtlInt* pInterfaceInt ;
        while (pInterfaceInt = (UtlInt*) iterator())
        {
            VoiceEngineMediaInterface* pInterface = 
                    (VoiceEngineMediaInterface*) pInterfaceInt->getValue();            
            if (pInterface && pInterface->getVideoEnginePtr())
            {
                pRC = (GipsVideoEnginePlatform*)pInterface->getVideoEnginePtr() ;
                break ;
            }
        }
    }

    // Last: Build it
    if (pRC == NULL)
    {
        constructGlobalInstance(false, true) ;
        pRC = mpVideoEngine ;
    }

    return pRC ;
}

// WARNING:: Assumes someone is externally holding a lock!!
GipsVideoEnginePlatform* VoiceEngineFactoryImpl::getNewVideoEngineInstance(GipsVoiceEngineLib* pVoiceEngine) const
{    
    OS_PERF_FUNC("VoiceEngineFactoryImpl::getNewVideoEngineInstance") ;
    GipsVideoEnginePlatform* pVideoEngine = NULL ;
    int rc ;

    assert(pVoiceEngine != NULL) ;
    if (pVoiceEngine)
    {
        pVideoEngine = GetNewVideoEngine();

        if (getGipsTracing())
        {
            rc = pVideoEngine->GIPSVideo_SetTraceCallback(VideoEngineLogCallback) ;
            assert(rc == 0) ;
        }

#ifdef USE_GIPS_DLL
        char* szKey = (char*) GIPS_KEY ;
        rc = pVideoEngine->GIPSVideo_Authenticate(szKey, strlen(szKey));
        assert(rc == 0);
#endif

#ifdef USE_GIPS
        rc = pVideoEngine->GIPSVideo_Init(pVoiceEngine, GIPS_EXPIRE_MONTH, GIPS_EXPIRE_DAY, GIPS_EXPIRE_YEAR) ;
        assert(rc == 0) ;
#endif 
    }

    return pVideoEngine ;
}


// WARNING:: Assumes someone is externally holding a lock!!
void VoiceEngineFactoryImpl::releaseVideoEngineInstance(GipsVideoEnginePlatform* pVideoEngine) 
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::releaseVideoEngineInstance") ;
    int rc ;

    assert(pVideoEngine != NULL) ;
    if (pVideoEngine)
    {
        rc = pVideoEngine->GIPSVideo_SetTrace(0) ;
        assert(rc == 0) ;
        OS_PERF_ADD("GIPSVideo_SetTrace") ;
        rc = pVideoEngine->GIPSVideo_Terminate() ;
        OS_PERF_ADD("GIPSVideo_Terminate") ;
        assert(rc == 0) ;

#ifdef USE_GIPS
# ifdef _WIN32
        DeleteGipsVideoEngine(pVideoEngine) ;
# else
        delete pVideoEngine ;
# endif
#else
        DeleteGipsVideoEngineStub(pVideoEngine) ;
#endif 
        // OS_PERF_ADD("(delete)") ;
    }
}


OsStatus VoiceEngineFactoryImpl::enableSpeakerVolumeAdjustment(bool bEnable) 
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::enableSpeakerVolumeAdjustment") ;
    OsLock lock(sGuard) ;

    if (mbSpeakerAdjust != bEnable)
    {
        mbSpeakerAdjust = bEnable ;

        if (mpVoiceEngine)
        {
            mpVoiceEngine->GIPSVE_EnableExternalMediaProcessing(
                    !mbSpeakerAdjust, 
                    PLAYBACK_ALL_CHANNELS_MIXED, 
                    -1, 
                    *this) ;
        }
    }
    return OS_SUCCESS ;
}


OsStatus VoiceEngineFactoryImpl::setSpeakerVolume(int iVolume) 
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::setSpeakerVolume") ;

    OsLock lock(sGuard) ;
    OsStatus rc = OS_FAILED ;    
    
    if (iVolume < 0 || iVolume > 100)
    {
        rc = OS_FAILED;
    }
    else 
    {
        if (mbSpeakerAdjust)
        {            
            GipsVoiceEngineLib* pVoiceEngine = getAnyVoiceEngine() ;
            if (pVoiceEngine)
            {            
                int gipsVolume = (int) (((float)iVolume / 100.0) * 255.0 );
                if (0 == pVoiceEngine->GIPSVE_SetSpeakerVolume(gipsVolume))
                    rc = OS_SUCCESS ;
            }
        }
        else
        {
            miVolume = iVolume ;
            rc = OS_SUCCESS ;
        }
    }

    return rc ;
}


OsStatus VoiceEngineFactoryImpl::setSpeakerDevice(const UtlString& device) 
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::setSpeakerDevice") ;
    OsLock lock(sGuard) ;
    OsStatus rc = OS_FAILED;

    UtlSListIterator iterator(mInterfaceList);
    VoiceEngineMediaInterface* pInterface = NULL;
    UtlInt* pInterfaceInt = NULL;

    mAudioDeviceOutput.reset() ;
    
    if (outputDeviceStringToIndex(mCurrentWaveOutDevice, device) == OS_SUCCESS)
    {
        mAudioDeviceOutput.setRequested(device) ;

        if (mpVoiceEngine)        
        {
            if (mpVoiceEngine->GIPSVE_SetSoundDevices(mCurrentWaveInDevice, mCurrentWaveOutDevice) == 0)
            {
                rc = OS_SUCCESS;
            }
        }
        while (pInterfaceInt = (UtlInt*)iterator())
        {
            pInterface = (VoiceEngineMediaInterface*)pInterfaceInt->getValue();
            GipsVoiceEngineLib* pGips = NULL;
            
            if (pInterface && pInterface->getAudioEnginePtr())
            {
                pGips = (GipsVoiceEngineLib*)pInterface->getAudioEnginePtr();            
                if (pGips)
                {
                    if (0 == pGips->GIPSVE_SetSoundDevices(mCurrentWaveInDevice, mCurrentWaveOutDevice))
                    {
                        rc = OS_SUCCESS;
                    }
                }
            }
        } 
    }
    
    return rc ;    
}


OsStatus VoiceEngineFactoryImpl::setMicrophoneGain(int iGain) 
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::setMicrophoneGain") ;
    OsLock lock(sGuard) ;
    OsStatus rc = OS_FAILED ;
    
    if (0 == iGain)
    {
        rc  = muteMicrophone(true);
    }
    else
    {    
        GipsVoiceEngineLib* pVoiceEngine = getAnyVoiceEngine() ;
        if (pVoiceEngine)
        {            
            int gipsGain = (int)((float)((float)iGain / 100.0) * 255.0);                
            if (iGain < 0 || iGain > 100)
            {
                rc = OS_FAILED;
            }
            else if (0 == pVoiceEngine->GIPSVE_SetMicVolume(gipsGain))
            {
                miGain = gipsGain;
                rc = OS_SUCCESS;
            }
        }
    }
    return rc ;
}

OsStatus VoiceEngineFactoryImpl::setMicrophoneDevice(const UtlString& device) 
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::setMicrophoneDevice") ;
    OsLock lock(sGuard) ;
    OsStatus rc = OS_FAILED;
    UtlSListIterator iterator(mInterfaceList);
    VoiceEngineMediaInterface* pInterface = NULL;
    UtlInt* pInterfaceInt = NULL;

    mAudioDeviceInput.reset() ;
   
    assert(device.length() > 0);
    
    if (OS_SUCCESS == inputDeviceStringToIndex(mCurrentWaveInDevice, device))
    {
        mAudioDeviceInput.setRequested(device) ;

        if (mpVoiceEngine)
        {                     
            if (0 == mpVoiceEngine->GIPSVE_SetSoundDevices(mCurrentWaveInDevice, mCurrentWaveOutDevice) )
                rc = OS_SUCCESS;
        }

        while (pInterfaceInt = (UtlInt*)iterator())
        {
            pInterface = (VoiceEngineMediaInterface*)pInterfaceInt->getValue();
            GipsVoiceEngineLib* pGips = NULL;
            
            if (pInterface && pInterface->getAudioEnginePtr())
            {
                pGips = (GipsVoiceEngineLib*)pInterface->getAudioEnginePtr();            
                if (pGips)
                {
                    if (0 == pGips->GIPSVE_SetSoundDevices(mCurrentWaveInDevice, mCurrentWaveOutDevice))
                    {
                        rc = OS_SUCCESS;
                    }
                }
            }
        } 
    }
        
    return rc ;    
}


OsStatus VoiceEngineFactoryImpl::muteMicrophone(UtlBoolean bMute) 
{    

    OS_PERF_FUNC("VoiceEngineFactoryImpl::muteMicrophone") ;
    // OsLock lock(sGuard) ;
    OsStatus rc = OS_SUCCESS;
    
    mbMute = (bMute==TRUE) ? true : false;
    
    UtlSListIterator iterator(mInterfaceList);
    UtlInt* iMediaInterface = NULL;
    while ((iMediaInterface = (UtlInt*)iterator()))
    {
        VoiceEngineMediaInterface* pInterface = (VoiceEngineMediaInterface*)iMediaInterface->getValue();
        rc = pInterface->muteMicrophone((bMute==TRUE) ? true : false);
        if (OS_FAILED == rc)
        {
            break;
        }
    }

    return rc ;
}


OsStatus VoiceEngineFactoryImpl::setAudioAECMode(const MEDIA_AEC_MODE mode)
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::setAudioAECMode") ;
    OsLock lock(sGuard) ;
    OsStatus rc = OS_SUCCESS ;

    if (mAECMode != mode)
    {

        if (mpVoiceEngine)
            doSetAudioAECMode(mpVoiceEngine, mode) ;

        // iterate through all of the VoiceEngineMedia interface pointers,
        // and set AEC for all of them
        UtlSListIterator iterator(mInterfaceList);
        VoiceEngineMediaInterface* pInterface = NULL;
        UtlInt* pInterfaceInt = NULL;        
        while (pInterfaceInt = (UtlInt*)iterator())
        {
            pInterface = (VoiceEngineMediaInterface*)pInterfaceInt->getValue();            
            if (pInterface && pInterface->getAudioEnginePtr())
            {
                doSetAudioAECMode((GipsVoiceEngineLib*)pInterface->getAudioEnginePtr(), mode) ;
            }
        }
    }

    return rc;
}


OsStatus VoiceEngineFactoryImpl::setAudioNoiseReductionMode(const MEDIA_NOISE_REDUCTION_MODE mode)
{
    OS_PERF_FUNC("VoiceEngineFactoryImpl::setAudioNoiseReductionMode") ;
    OsLock lock(sGuard) ;
    OsStatus rc = OS_SUCCESS ;

    mNRMode = mode ;

    if (mpVoiceEngine)
        doSetAudioNoiseReductionMode(mpVoiceEngine, mode) ;

    // iterate through all of the VoiceEngineMedia interface pointers,
    // and set AEC for all of them
    UtlSListIterator iterator(mInterfaceList);
    VoiceEngineMediaInterface* pInterface = NULL;
    UtlInt* pInterfaceInt = NULL;
    
    while (pInterfaceInt = (UtlInt*)iterator())
    {
        pInterface = (VoiceEngineMediaInterface*)pInterfaceInt->getValue();            
        if (pInterface && pInterface->getAudioEnginePtr())
        {
            doSetAudioNoiseReductionMode((GipsVoiceEngineLib*)pInterface->getAudioEnginePtr(), mode) ;           
        }
    }

    return rc;
}

OsStatus VoiceEngineFactoryImpl::enableAGC(UtlBoolean bEnable)
{    
    OS_PERF_FUNC("VoiceEngineFactoryImpl::enableAGC") ;
    OsLock lock(sGuard) ;
    OsStatus rc = OS_SUCCESS ;

    if (mbEnableAGC != bEnable)
    {        
        mbEnableAGC = bEnable ;

        // Set the mode on the global instance
        if (mpVoiceEngine)
            doEnableAGC(mpVoiceEngine, bEnable) ;

        // iterate through all of the VoiceEngineMedia interface pointers,
        // and set AEC for all of them
        UtlSListIterator iterator(mInterfaceList);
        VoiceEngineMediaInterface* pInterface = NULL;
        UtlInt* pInterfaceInt = NULL;
            
        while (pInterfaceInt = (UtlInt*)iterator())
        {
            pInterface = (VoiceEngineMediaInterface*)pInterfaceInt->getValue();            
            if (pInterface && pInterface->getAudioEnginePtr())
            {
                doEnableAGC((GipsVoiceEngineLib*)pInterface->getAudioEnginePtr(), bEnable) ;
            }
        }
    }

    return rc ;
}

void VoiceEngineFactoryImpl::constructGlobalInstance(bool bNoLocalChannel, bool bStartVideo) const
{    
    OS_PERF_FUNC("VoiceEngineFactoryImpl::constructGlobalInstance") ;
    OsLock lock(sGuard) ;

    if (mpVoiceEngine == NULL)
    {
        char szVersion[4096];

        mpVoiceEngine = getNewVoiceEngineInstance() ;
        mpVoiceEngine->GIPSVE_GetVersion(szVersion, 4096) ;
        OsSysLog::add(FAC_MP, PRI_INFO, szVersion) ;

        if (mbCreateLocalConnection && !bNoLocalChannel)
            doEnableLocalChannel(true) ;
    }

    if (mpVideoEngine == NULL && bStartVideo)
    {
        char szVersion[4096];

        assert(mpVideoEngine == NULL) ;
        mpVideoEngine = getNewVideoEngineInstance(mpVoiceEngine) ;
        mpVideoEngine->GIPSVideo_GetVersion(szVersion, 4096) ;
        OsSysLog::add(FAC_MP, PRI_INFO, szVersion) ;                
    }
}


OsStatus VoiceEngineFactoryImpl::enableOutOfBandDTMF(UtlBoolean bEnable)
{
    mbDTMFOutOfBand = bEnable;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::enableInBandDTMF(UtlBoolean bEnable)
{
    mbDTMFInBand = bEnable;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::enableRTCP(UtlBoolean bEnable)
{
    mbEnableRTCP = bEnable ;

    return OS_SUCCESS ;
}

OsStatus VoiceEngineFactoryImpl::setRTCPName(const char* szName)
{
    if (szName)
        mRtcpName = szName ;
    else
        mRtcpName = "user1@undefined" ;

    return OS_SUCCESS ;
}


static int sipxtapiVideoFormatToSdpFormat(SIPX_VIDEO_FORMAT formatId)
{
    int rc = 0 ;
    switch (formatId)
    {
        case VIDEO_FORMAT_CIF:
            rc = SDP_VIDEO_FORMAT_CIF ;
            break ;
        case VIDEO_FORMAT_QCIF:
            rc = SDP_VIDEO_FORMAT_QCIF ;
            break ;
        case VIDEO_FORMAT_SQCIF:
            rc = SDP_VIDEO_FORMAT_SQCIF ;
            break ;
        case VIDEO_FORMAT_QVGA:
            rc = SDP_VIDEO_FORMAT_QVGA ;
            break ;
        case VIDEO_FORMAT_VGA:
            rc = SDP_VIDEO_FORMAT_VGA ;
            break ;
        case VIDEO_FORMAT_4CIF:
            rc = SDP_VIDEO_FORMAT_4CIF ;
            break ;
        case VIDEO_FORMAT_16CIF:
            rc = SDP_VIDEO_FORMAT_16CIF ;
            break ;
        default:
            assert(false) ;
    }
    return rc ;
}

static bool shouldAllowFormat(int sdpFormat, int bitrate)
{
    bool bShouldAllow = false ;

    switch (sdpFormat)
    {
#ifdef INCLUDE_4CIF_RESOLUTION
        case SDP_VIDEO_FORMAT_4CIF:
            bShouldAllow = (bitrate >= 200) ;
            break ;
#endif
#ifdef INCLUDE_16CIF_RESOLUTION
        case SDP_VIDEO_FORMAT_16CIF:
            bShouldAllow = (bitrate >= 200) ;
            break ;
#endif
#ifdef INCLUDE_VGA_RESOLUTION
        case SDP_VIDEO_FORMAT_VGA:
            bShouldAllow = (bitrate >= 70) ;
            break ;
#endif
#ifdef INCLUDE_CIF_RESOLUTION
        case SDP_VIDEO_FORMAT_CIF:
            bShouldAllow = (bitrate >= 60) ;
            break ;
#endif
#ifdef INCLUDE_QVGA_RESOLUTION
        case SDP_VIDEO_FORMAT_QVGA:
            bShouldAllow = (bitrate >= 60) ;
            break ;
#endif
#ifdef INCLUDE_QCIF_RESOLUTION
        case SDP_VIDEO_FORMAT_QCIF:
            bShouldAllow = (bitrate >= 10) ;
            break ;
#endif
#ifdef INCLUDE_SQCIF_RESOLUTION
        case SDP_VIDEO_FORMAT_SQCIF:
            bShouldAllow = (bitrate >= 5) ;
            break ;
#endif

        default:
            break ;
    }
    return bShouldAllow ;
}



OsStatus VoiceEngineFactoryImpl::buildCodecFactory(SdpCodecList*    pFactory, 
                                                   const UtlString& sAudioPreferences, 
                                                   const UtlString& sVideoPreferences,
                                                   int videoFormat,
                                                   int* iRejected)
{
    OsLock lock(sGuard) ;

    if (videoFormat < 0)
        videoFormat = mVideoFormat ; 
    else
        mVideoFormat = videoFormat ;

    OsStatus rc = OS_FAILED;
    int iCodecs = 0;
    UtlString codec;
    UtlString codecList;
    static bool sbDumpedCodecs = false ;

    applyVideoFormatOverride(videoFormat) ;

    *iRejected = 0;
    if (pFactory)
    {
        pFactory->clearCodecs();
                 
        // add preferred codecs first
        applyAudioCodecListOverride((UtlString&)sAudioPreferences) ;
        if (sAudioPreferences.length() > 0)
        {
            UtlString audioPreferences = sAudioPreferences;
            *iRejected = pFactory->addCodecs(audioPreferences);
            rc = OS_SUCCESS;
        }
        else
        {
            codecList = DEFAULT_VOICEENGINE_CODEC_LIST ;
            *iRejected += pFactory->addCodecs(codecList);
            rc = OS_SUCCESS;
        }

        // For now include all video codecs
        rc = OS_FAILED ;
        codecList = "";
        // add preferred codecs first
        applyVideoCodecListOverride((UtlString&)sVideoPreferences) ;

        if (sVideoPreferences.length() > 0)
        {
            UtlString videoPreferences = sVideoPreferences;
            *iRejected = pFactory->addCodecs(videoPreferences);
            rc = OS_SUCCESS;

            // Filter out unwanted video codecs
            if (videoFormat != -1)
            {
                int numVideoCodecs;
                SdpCodec** videoCodecsArray = NULL;

                pFactory->getCodecs(numVideoCodecs, videoCodecsArray, "video");
                for (int codecIndex = 0; codecIndex<numVideoCodecs; codecIndex++)
                {
                    int format = videoCodecsArray[codecIndex]->getVideoFormat() ;

                    if ((sipxtapiVideoFormatToSdpFormat((SIPX_VIDEO_FORMAT) videoFormat) < format)
                        || !shouldAllowFormat(format, mVideoBitRate))
                    {
                        pFactory->removeCodecByType(videoCodecsArray[codecIndex]->getCodecType()) ;
                    }

                    delete videoCodecsArray[codecIndex];
                    videoCodecsArray[codecIndex] = NULL;
                }                
                delete[] videoCodecsArray;
            }
        }
        else
        {                
            codecList = DEFAULT_VIDEOENGINE_CODEC_LIST ;

            // Strip out disabled codecs
            switch (videoFormat)
            {                
                case 2:     // SQCIF
                    SdpCodecList::removeKeywordFromCodecList("-QCIF", codecList) ;
                case 1:     // QCIF
                    SdpCodecList::removeKeywordFromCodecList("-QVGA", codecList) ;
                case 3:     // QVGA
                    SdpCodecList::removeKeywordFromCodecList("-CIF", codecList) ;
                case 0:     // CIF
                    SdpCodecList::removeKeywordFromCodecList("-VGA", codecList) ;
                case  4:    // VGA
                case -1:    // Unspecified / ALL
                    break ;
            }

            *iRejected += pFactory->addCodecs(codecList);
            rc = OS_SUCCESS;
        }
    }

    sbDumpedCodecs = true ;

    return rc;
}



// Set the global video preview window 
OsStatus VoiceEngineFactoryImpl::setVideoPreviewDisplay(void* pDisplay)
{
    OsLock lock(sGuard) ;

    SIPXVE_VIDEO_DISPLAY* pOldDisplay = (SIPXVE_VIDEO_DISPLAY*) mpPreviewWindowDisplay ;    
    if (pDisplay)
    {
        SIPXVE_VIDEO_DISPLAY* pVideoDisplay = (SIPXVE_VIDEO_DISPLAY*) pDisplay;
        if (pVideoDisplay->handle || pVideoDisplay->filter)
        {
            mpPreviewWindowDisplay = new SIPXVE_VIDEO_DISPLAY(*pVideoDisplay) ;
        }
        else
        {
            mpPreviewWindowDisplay = NULL;
        }
    }
    else
    {
        mpPreviewWindowDisplay = NULL;
    }

    if (pOldDisplay)
        delete pOldDisplay;

    return OS_SUCCESS ;
}

void* VoiceEngineFactoryImpl::getVideoPreviewDisplay()
{
    OsLock lock(sGuard) ;

    return mpPreviewWindowDisplay;
}
        
// Update the video preview window given the specified display context.
OsStatus VoiceEngineFactoryImpl::updateVideoPreviewWindow(void* displayContext) 
{
    return OS_SUCCESS ;
}

OsStatus VoiceEngineFactoryImpl::setVideoQuality(int quality)
{
    mVideoQuality = quality;
    return OS_SUCCESS ;
}

OsStatus VoiceEngineFactoryImpl::setVideoCpuValue(int value)
{
    mCpu = value;
    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::setConnectionIdleTimeout(int idleTimeout)
{
    mIdleTimeout = idleTimeout;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::startPlayAudio(VoiceEngineBufferInStream* pStream, GIPS_FileFormats formatType, int downScaling) 
{    
    OsLock lock(sGuard) ;
    OsStatus rc = OS_FAILED ;

    stopPlay() ;
    
    constructGlobalInstance() ;
    mbLocalConnectionInUse = true ;
    mpInStream = pStream ;
    int iRC = mpVoiceEngine->GIPSVE_PlayPCM(mLocalConnectionId, mpInStream, formatType, ((float) downScaling) / 100) ;
    if (iRC == 0)
    {
        rc = OS_SUCCESS ;
    }

    return rc ;
}


OsStatus VoiceEngineFactoryImpl::stopPlay(const void* pSource) 
{
    OsLock lock(sGuard) ;
    bool bStopAudio = true ;

    if (mpVoiceEngine)
    {

        if (pSource != NULL && mpInStream != NULL)
        {
            if (mpInStream->getSource() != pSource)
            {
                bStopAudio = false ;
            }
        }

        if (bStopAudio)
        {
            if (mpVoiceEngine->GIPSVE_IsPlayingFile(mLocalConnectionId))
            {
                mpVoiceEngine->GIPSVE_StopPlayingFile(mLocalConnectionId) ;
            }

            if (mpInStream != NULL)
            {
                mpInStream->close() ;
                delete mpInStream ;
                mpInStream = NULL ;
            }

            mbLocalConnectionInUse = false ;
        }
    }

    return OS_SUCCESS ; 
}


OsStatus VoiceEngineFactoryImpl::playTone(int toneId) 
{
    OsLock lock(sGuard) ;

    constructGlobalInstance() ;
    int check = mpVoiceEngine->GIPSVE_PlayDTMFTone(toneId);
    assert(check == 0) ;
    check = mpVoiceEngine->GIPSVE_GetLastError();

    return OS_SUCCESS ; 
}

OsStatus VoiceEngineFactoryImpl::setVideoParameters(int bitRate, int frameRate)
{
    mVideoBitRate = bitRate;
    mVideoFrameRate = frameRate;
    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::setVideoBitrate(int bitrate)
{
    mVideoBitRate = bitrate;
    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::setVideoFramerate(int framerate)
{
    mVideoFrameRate = framerate;
    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const
{
    OsStatus rc = OS_FAILED;
    UtlBoolean matchFound = FALSE;

    codecName = "";

    switch (codecType)
    {
    case SdpCodec::SDP_CODEC_G729A:
        codecName = GIPS_CODEC_ID_G729;
        break;
    case SdpCodec::SDP_CODEC_TONES:
        codecName = GIPS_CODEC_ID_TELEPHONE;
        break;
    case SdpCodec::SDP_CODEC_PCMA:
        codecName = GIPS_CODEC_ID_PCMA;
        break;
    case SdpCodec::SDP_CODEC_PCMU:
        codecName = GIPS_CODEC_ID_PCMU;
        break;
    case SdpCodec::SDP_CODEC_GIPS_IPCMA:
        codecName = GIPS_CODEC_ID_EG711A;
        break;
    case SdpCodec::SDP_CODEC_GIPS_IPCMU:
        codecName = GIPS_CODEC_ID_EG711U;
        break;
    case SdpCodec::SDP_CODEC_GIPS_IPCMWB:
        codecName = GIPS_CODEC_ID_IPCMWB;
        break;
    case SdpCodec::SDP_CODEC_GIPS_ILBC:
        codecName = GIPS_CODEC_ID_ILBC;
        break;
    case SdpCodec::SDP_CODEC_GIPS_ISAC:
        codecName = GIPS_CODEC_ID_ISAC;
        break;
    case SdpCodec::SDP_CODEC_GIPS_ISAC_LC:
        codecName = GIPS_CODEC_ID_ISAC_LC;
        break;
    case SdpCodec::SDP_CODEC_GSM:
        codecName = GIPS_CODEC_ID_GSM;
        break;
    case SdpCodec::SDP_CODEC_G723:
        codecName = GIPS_CODEC_ID_G723;
        break;
    case SdpCodec::SDP_CODEC_VP71_CIF:
        codecName = GIPS_CODEC_ID_VP71_CIF;
        break;
    case SdpCodec::SDP_CODEC_H263_CIF:
        codecName = GIPS_CODEC_ID_H263_CIF;
        break;
    case SdpCodec::SDP_CODEC_VP71_QCIF:
        codecName = GIPS_CODEC_ID_VP71_QCIF;
        break;
    case SdpCodec::SDP_CODEC_H263_QCIF:
        codecName = GIPS_CODEC_ID_H263_QCIF;
        break;
    case SdpCodec::SDP_CODEC_VP71_SQCIF:
        codecName = GIPS_CODEC_ID_VP71_SQCIF;
        break;
    case SdpCodec::SDP_CODEC_H263_SQCIF:
        codecName = GIPS_CODEC_ID_H263_SQCIF;
        break;
    case SdpCodec::SDP_CODEC_VP71_QVGA:
        codecName = GIPS_CODEC_ID_VP71_QVGA;
        break;
    case SdpCodec::SDP_CODEC_VP71_VGA:   
        codecName = GIPS_CODEC_ID_VP71_VGA;
        break;
    case SdpCodec::SDP_CODEC_VP71_4CIF:
        codecName = GIPS_CODEC_ID_VP71_4CIF;
        break;
    case SdpCodec::SDP_CODEC_VP71_16CIF:    
        codecName = GIPS_CODEC_ID_VP71_16CIF;
        break;
    case SdpCodec::SDP_CODEC_LSVX:
        codecName = GIPS_CODEC_ID_LSVX;
        break ;
    default:
        OsSysLog::add(FAC_MP, PRI_DEBUG, 
                      "getCodecNameByType: unsupported codec %d", 
                      codecType); 
    }

    if (codecName != "")
    {
        matchFound = TRUE;
        rc = OS_SUCCESS;
    }

    return rc;
}


void VoiceEngineFactoryImpl::process(int    channel_no, 
                                     short* audio_10ms_16kHz, 
                                     int    len, 
                                     int    sampfreq) 
{
    float scaleFactor = (miVolume / 100.0f) ;
    if (scaleFactor != 1.0f)
    {
        for (int i=0; i<len; i++)
        {
            short sample = audio_10ms_16kHz[i] ;
            sample = (int) (((float) sample) * scaleFactor) ;
            audio_10ms_16kHz[i] = sample ;
        }
    }
}


/* ============================ ACCESSORS ================================= */

OsStatus VoiceEngineFactoryImpl::getSpeakerVolume(int& iVolume) const
{
    OsLock lock(sGuard) ;
    OsStatus rc = OS_SUCCESS ;

    if (mbSpeakerAdjust)
    {
        GipsVoiceEngineLib* pVoiceEngine = getAnyVoiceEngine() ;
        if (pVoiceEngine)
        {            
            int gipsVolume = pVoiceEngine->GIPSVE_GetSpeakerVolume();        
            iVolume = (int) ((double) ((((double)gipsVolume ) / 255.0) * 100.0) + 0.5) ;
        }
        else
        {
            iVolume = -1 ;
            rc = OS_FAILED ;
        }
    }
    else
    {
        iVolume = miVolume ;
    }
                    
    return rc ;
}

OsStatus VoiceEngineFactoryImpl::getSpeakerDevice(UtlString& device) const
{
    OsStatus rc = OS_SUCCESS ;
    
    rc = outputDeviceIndexToString(device, mCurrentWaveOutDevice);

    return rc ;
}


OsStatus VoiceEngineFactoryImpl::getMicrophoneGain(int& iGain) const
{
    OsLock lock(sGuard) ;
    OsStatus rc = OS_SUCCESS ;

    GipsVoiceEngineLib* pVoiceEngine = getAnyVoiceEngine() ;
    if (pVoiceEngine)
    {            
        double gipsGain = (double) pVoiceEngine->GIPSVE_GetMicVolume();    
        iGain = (int) ((double)((double)((gipsGain) / 255.0) * 100.0) + 0.5);
    }
    else
    {
        rc = OS_FAILED ;
    }

    return rc ;
}


OsStatus VoiceEngineFactoryImpl::getMicrophoneDevice(UtlString& device) const
{
    OsStatus rc = OS_SUCCESS ;

    rc = this->inputDeviceIndexToString(device, this->mCurrentWaveInDevice);
    return rc ;
}




bool VoiceEngineFactoryImpl::getGipsTracing() const
{
    return mTrace;
}

void VoiceEngineFactoryImpl::setGipsTracing(bool bEnable)
{
    OsLock lock(sGuard) ;

    mTrace = bEnable ;

    if (mTrace)
    {
        if (mpVoiceEngine)
            mpVoiceEngine->GIPSVE_SetTraceCallback(VoiceEngineLogCallback) ;
        if (mpVideoEngine)
            mpVideoEngine->GIPSVideo_SetTraceCallback(VideoEngineLogCallback) ;
    }
    else
    {
        if (mpVoiceEngine)
            mpVoiceEngine->GIPSVE_SetTrace(0) ;
        if (mpVideoEngine)
            mpVideoEngine->GIPSVideo_SetTrace(0) ;
    }
}

OsStatus VoiceEngineFactoryImpl::getLocalAudioConnectionId(int& connectionId) const
{
    connectionId = mLocalConnectionId ;

    return OS_SUCCESS;
}


void* const VoiceEngineFactoryImpl::getAudioEnginePtr() const
{
    OsLock lock(sGuard) ;

    constructGlobalInstance() ;
    return mpVoiceEngine ;
}

OsMutex* VoiceEngineFactoryImpl::getLock() 
{
    return &sGuard ; 
}


void* const VoiceEngineFactoryImpl::getVideoEnginePtr() const
{
    OsLock lock(sGuard) ;

    constructGlobalInstance(false, true) ;
    return mpVideoEngine;
}


MediaDeviceInfo& VoiceEngineFactoryImpl::getAudioInputDeviceInfo() 
{
    return mAudioDeviceInput ;
}


MediaDeviceInfo& VoiceEngineFactoryImpl::getAudioOutputDeviceInfo()
{
    return mAudioDeviceOutput ;
}


OsStatus VoiceEngineFactoryImpl::getVideoCaptureDevice(UtlString& videoDevice)
{
    OsLock lock(sGuard) ;

    OsStatus rc = OS_FAILED;

    if (mVideoCaptureDevice.isNull())
    {
        int gipsRc;
        char szDeviceName[256];
    
        GipsVideoEnginePlatform* pVideoEngine = getAnyVideoEngine() ;
        gipsRc = pVideoEngine->GIPSVideo_GetCaptureDevice(0, szDeviceName, sizeof(szDeviceName));
        if (0 == gipsRc)
        {
            videoDevice = szDeviceName;
            rc = OS_SUCCESS;
        }
        else
        {
            videoDevice.remove(0) ;
            rc = OS_FAILED;
        }
    }
    else
    {
        videoDevice = mVideoCaptureDevice ;
        rc = OS_SUCCESS ;
    }
    
    return rc;
}

OsStatus VoiceEngineFactoryImpl::getVideoCaptureDevices(UtlSList& videoDevices) const
{
    OsLock lock(sGuard) ;

    OsStatus rc = OS_FAILED;
    int gipsRc;
    int index = 0;
    char szDeviceName[256];

    while (1)
    {
        GipsVideoEnginePlatform* pVideoEngine = getAnyVideoEngine() ;
        gipsRc = pVideoEngine->GIPSVideo_GetCaptureDevice(index, szDeviceName, sizeof(szDeviceName));
        index++;        
        if (-1 == gipsRc)
        {            
            break;
        }
        else
        {
            videoDevices.append(new UtlString(szDeviceName));
            rc = OS_SUCCESS;
        }
    }
    return rc;
}

OsStatus VoiceEngineFactoryImpl::setVideoCaptureDevice(const UtlString& videoDevice)
{
    OsLock lock(sGuard) ;
    OsStatus rc = OS_SUCCESS;

    if (videoDevice.length() > 0)
    {
        if (videoDevice.compareTo(mVideoCaptureDevice, UtlString::ignoreCase) != 0)
        {
            mVideoCaptureDevice = videoDevice;
            UtlSListIterator iterator(mInterfaceList) ;
            UtlInt* pInterfaceInt ;
            while (pInterfaceInt = (UtlInt*) iterator())
            {
                VoiceEngineMediaInterface* pInterface = 
                        (VoiceEngineMediaInterface*) pInterfaceInt->getValue();            
                if (pInterface)
                {
                    pInterface->resetVideoCaptureDevice() ;
                }
            }
        }
    }
    return rc;
}

OsStatus VoiceEngineFactoryImpl::getVideoQuality(int& quality) const
{
    quality = mVideoQuality;
    applyVideoQualityOverride(quality) ;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::getVideoBitRate(int& bitRate) const
{
    bitRate = mVideoBitRate;
    applyVideoBitRateOverride(bitRate) ;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::getVideoFrameRate(int& frameRate) const
{
    frameRate = mVideoFrameRate;
    applyVideoFrameRateOverride(frameRate) ;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::getVideoCpuValue(int& cpuValue) const
{
    cpuValue = mCpu;
    applyVideoCpuValueOverride(cpuValue) ;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::getConnectionIdleTimeout(int& idleTimeout) const
{   
    idleTimeout = mIdleTimeout;
    applyIdleTimeoutOverride(idleTimeout) ;

    return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

OsStatus VoiceEngineFactoryImpl::getAudioAECMode(MEDIA_AEC_MODE& mode) const 
{
    mode = mAECMode ;

    return OS_SUCCESS ;
}

OsStatus VoiceEngineFactoryImpl::getAudioNoiseReductionMode(MEDIA_NOISE_REDUCTION_MODE& mode) const 
{
    mode = mNRMode ;

    return OS_SUCCESS ;
}

OsStatus VoiceEngineFactoryImpl::isAGCEnabled(UtlBoolean& bEnable) const 
{
    bEnable = mbEnableAGC ;

    return OS_SUCCESS ;
}


OsStatus VoiceEngineFactoryImpl::isOutOfBandDTMFEnabled(UtlBoolean& bEnabled) const
{
    bEnabled = mbDTMFOutOfBand;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::isInBandDTMFEnabled(UtlBoolean& bEnabled) const
{
    bEnabled = mbDTMFInBand;

    return OS_SUCCESS;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

bool VoiceEngineFactoryImpl::doSetAudioAECMode(GipsVoiceEngineLib*  pVoiceEngine, 
                                               const MEDIA_AEC_MODE mode) const
{
    bool bRC = true ;

    if (pVoiceEngine)
    {
        switch (mode)
        {
            case MEDIA_AEC_DISABLED:
                pVoiceEngine->GIPSVE_SetECType(0) ;
                pVoiceEngine->GIPSVE_SetECStatus(0) ;
                break ;
            case MEDIA_AEC_SUPPRESS:
                pVoiceEngine->GIPSVE_SetECType(1) ;
                pVoiceEngine->GIPSVE_SetECStatus(1) ;
                break ;
            case MEDIA_AEC_CANCEL:
                pVoiceEngine->GIPSVE_SetECType(0) ;
                pVoiceEngine->GIPSVE_SetECStatus(1) ;
                break ;
            case MEDIA_AEC_CANCEL_AUTO:
                pVoiceEngine->GIPSVE_SetECType(0) ;
                pVoiceEngine->GIPSVE_SetECStatus(2) ;
                break ;
            default:
                bRC = false ;
                assert(false) ;
                break ;
        }
    }
    else
    {
        bRC = false ;
    }

    return bRC ;
}


bool VoiceEngineFactoryImpl::doSetAudioNoiseReductionMode(GipsVoiceEngineLib*  pVoiceEngine, 
                                                          const MEDIA_NOISE_REDUCTION_MODE mode) const
{
    bool bRC = true ;

    if (pVoiceEngine)
    {
        switch (mode)
        {
            case MEDIA_NOISE_REDUCTION_DISABLED:
                pVoiceEngine->GIPSVE_SetNRStatus(0) ;
                pVoiceEngine->GIPSVE_SetNRpolicy(0) ;
                break ;
            case MEDIA_NOISE_REDUCTION_LOW:
                pVoiceEngine->GIPSVE_SetNRStatus(1) ;
                pVoiceEngine->GIPSVE_SetNRpolicy(0) ;
                break ;
            case MEDIA_NOISE_REDUCTION_MEDIUM:
                pVoiceEngine->GIPSVE_SetNRStatus(1) ;
                pVoiceEngine->GIPSVE_SetNRpolicy(1) ;
                break ;
            case MEDIA_NOISE_REDUCTION_HIGH:
                pVoiceEngine->GIPSVE_SetNRStatus(1) ;
                pVoiceEngine->GIPSVE_SetNRpolicy(2) ;
                break ;
            default:
                bRC = false ;
                assert(false) ;
                break ;
        }
    }
    else
    {
        bRC = false ;
    }

    return bRC ;
}


bool VoiceEngineFactoryImpl::doEnableAGC(GipsVoiceEngineLib* pVoiceEngine, 
                                         bool bEnable) const
{
    bool bRC = true ;
    int iRC ;
    int mode = bEnable ? 1 : 0 ; 

    if (pVoiceEngine)
    {
        if (pVoiceEngine->GIPSVE_GetAGCStatus() != mode)
        {
            iRC = pVoiceEngine->GIPSVE_SetAGCStatus(mode) ;
            if (iRC != 0)
            {
                assert(false) ;
                bRC = false ;
            }
            
        }
    }

    return bRC ;
}



/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

OsStatus VoiceEngineFactoryImpl::outputDeviceStringToIndex(int& deviceIndex, const UtlString& device) const
{
    OsStatus rc = OS_FAILED;
#ifdef _WIN32
    WAVEOUTCAPS outcaps ;
    int numDevices ;
    int i ;

    numDevices = waveOutGetNumDevs();
    for (i=0; i<numDevices && i<MAX_AUDIO_DEVICES; i++)
    {
        waveOutGetDevCaps(i, &outcaps, sizeof(WAVEOUTCAPS)) ;
        if (device.length() == 0 || device == UtlString(outcaps.szPname))
        {
            deviceIndex = i;
            rc = OS_SUCCESS;
            break;
        }
    }
#else
    deviceIndex = 0 ;
    rc = OS_SUCCESS;
#endif
    return rc;    
}

OsStatus VoiceEngineFactoryImpl::outputDeviceIndexToString(UtlString& device, const int deviceIndex) const
{
    OsStatus rc = OS_FAILED;
#ifdef _WIN32
    WAVEOUTCAPS outcaps ;
    int numDevices ;

    numDevices = waveOutGetNumDevs();
    if (deviceIndex < numDevices)
    {
        waveOutGetDevCaps(deviceIndex, &outcaps, sizeof(WAVEOUTCAPS)) ;
        device = outcaps.szPname;
        rc = OS_SUCCESS;
    }
#else
    device = "dev/dsp" ;
    rc = OS_SUCCESS;

#endif
    return rc;
}

OsStatus VoiceEngineFactoryImpl::inputDeviceStringToIndex(int& deviceIndex, const UtlString& device) const
{
    OsStatus rc = OS_FAILED;
#ifdef _WIN32
    WAVEINCAPS incaps ;
    int numDevices ;
    int i ;

    numDevices = waveInGetNumDevs();
    for (i=0; i<numDevices && i<MAX_AUDIO_DEVICES; i++)
    {
        waveInGetDevCaps(i, &incaps, sizeof(WAVEINCAPS)) ;
        if (device == UtlString(incaps.szPname))
        {
            deviceIndex = i;
            rc = OS_SUCCESS;
        }
    }
#else
    deviceIndex = 0 ;
    rc = OS_SUCCESS;
#endif
    return rc;    
}

OsStatus VoiceEngineFactoryImpl::inputDeviceIndexToString(UtlString& device, const int deviceIndex) const
{
    OsStatus rc = OS_FAILED;
#ifdef _WIN32
    WAVEINCAPS incaps ;
    int numDevices ;

    numDevices = waveInGetNumDevs();
    if (deviceIndex < numDevices)
    {
        waveInGetDevCaps(deviceIndex, &incaps, sizeof(WAVEINCAPS)) ;
        device = incaps.szPname;
        rc = OS_SUCCESS;
    }
#else
    device = "/dev/dsp" ;
    rc = OS_SUCCESS ;
#endif
    return rc;
}

const bool VoiceEngineFactoryImpl::isMuted() const
{
    return mbMute;
}

bool VoiceEngineFactoryImpl::isVideoSessionActive() const
{
    bool bInSession = false ;
    UtlSListIterator iterator(mInterfaceList);
    VoiceEngineMediaInterface* pInterface = NULL;
    UtlInt* pInterfaceInt = NULL;
        
    while (pInterfaceInt = (UtlInt*) iterator())
    {
        pInterface = (VoiceEngineMediaInterface*) pInterfaceInt->getValue();            
        if (pInterface && pInterface->getVideoEnginePtr())
        {
            GipsVideoEnginePlatform* pGips = 
                    (GipsVideoEnginePlatform*)pInterface->getVideoEnginePtr() ;
            if (pGips)
            {
                bInSession = true ;
                break ;
            }
        }
    }
    return bInSession ;
}

#ifdef USE_GIPS_DLL
GipsVideoEngine::~GipsVideoEngine() {} ;
#endif
