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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
// 

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "include/VideoEngine.h"


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

VideoEngine::VideoEngine(GipsVideoEnginePlatform* pVideoEngine, bool bConsoleTrace)
    : mLock(OsMutex::Q_FIFO)
    , mMediaDeviceInfo(MediaDeviceInfo::MDIT_VIDEO_CAPTURE)
{
    sbConsoleTrace = bConsoleTrace ;
    trace(PRI_DEBUG, "VideoEngine::VideoEngine") ;

    mpVideoEngine = pVideoEngine ;

    mbAutoStart = true ;
    mbCaptureDeviceSet = false ;
    mbPreviewRendererSet = false ;
    mbRunning = false ;
    mbHaveCameraCapabilities = false ;
    mbIgnoreCameraCapabilties = false ;
    mbCameraError = false;
    memset(&mCameraCapabilities, 0, sizeof(mCameraCapabilities)) ;
    mbForceCodecReset = false ;
    
    for (int i=0; i<MAX_VE_CONNECTIONS; i++)
    {
        memset(&mSendCodec[i], 0, sizeof(GIPSVideo_CodecInst)) ;
        mbChannelCreated[i] = false ;
        mbRemoteRendererSet[i] = false ;
        mbRendering[i] = false ;
        mbSending[i] = false ;
        mbReceivePaused[i] = false ;
        mbSendPaused[i] = false ;
        mReceiveCodecs[i] = new UtlSList() ;
    }        
#if defined (GIPS_STUB)
    setIgnoreCameraCapabilties(true);
#endif

}

//////////////////////////////////////////////////////////////////////////////

VideoEngine::~VideoEngine()
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::~VideoEngine") ;

    for (int i=0; i<MAX_VE_CONNECTIONS; i++)
    {
        if (mReceiveCodecs[i])
        {
            UtlVoidPtr* pPtr ;
            UtlSListIterator itor(*mReceiveCodecs[i]) ;
            while ((pPtr = (UtlVoidPtr*) itor()) != NULL)
            {
                GIPSVideo_CodecInst* pInst = (GIPSVideo_CodecInst*) 
                        pPtr->getValue() ;
                if (pInst)
                    delete pInst ;

                mReceiveCodecs[i]->destroy(pPtr) ;
            }               
        }
    }
}

/* ============================ MANIPULATORS ============================== */

void VideoEngine::setGipsVideoEngine(GipsVideoEnginePlatform* pVideoEngine)
{
    OsLock lock(mLock) ;
    mpVideoEngine = pVideoEngine ;
}

//////////////////////////////////////////////////////////////////////////////

void VideoEngine::releaseVideoEngine() 
{
    OsLock lock(mLock) ;
    mpVideoEngine = NULL ;
}

//////////////////////////////////////////////////////////////////////////////

void VideoEngine::setIgnoreCameraCapabilties(bool bIgnore)
{
    OsLock lock(mLock) ;
    mbIgnoreCameraCapabilties = bIgnore ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::createChannel(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec) 
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::createChannel") ;

    assert(mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] == false) ;

    // Create our Channel
    if (mpVideoEngine)
    {
        int rc = mpVideoEngine->GIPSVideo_CreateChannel(voiceChannel) ;
        checkRC("GIPSVideo_CreateChannel", voiceChannel, rc , true) ;
        mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] = (rc >= 0) ;

        if (mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS])
        {
            rc = mpVideoEngine->GIPSVideo_EnableIntraRequest(voiceChannel, true) ;
            checkRC("GIPSVideo_EnableIntraRequest", voiceChannel, rc , true) ;

            if (mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS])
                doUserAgentSpecific(voiceChannel) ;
        }
    }

#if VE_INIT_CODEC_ON_CREATE_CHANNEL
    /*
        * Init to default codec
        */
    if (mpVideoEngine && pVideoCodec && mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS])
        doUpdateSendCodec(voiceChannel, pVideoCodec) ;
#endif

    return mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] ;
}

//////////////////////////////////////////////////////////////////////////////

void VideoEngine::setUserAgent(int voiceChannel, const char* szUserAgent)
{
    OsLock lock(mLock) ;
    if (mUserAgent[voiceChannel % MAX_VE_CONNECTIONS].compareTo(szUserAgent, UtlString::ignoreCase) != 0)
    {
        mUserAgent[voiceChannel % MAX_VE_CONNECTIONS] = szUserAgent ;
        if (mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS])
            doUserAgentSpecific(voiceChannel) ;
    }

}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::destroyChannel(int voiceChannel)
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::destroyChannel (%d)", voiceChannel) ;

    bool bRC = true ;
    int rc ;

    if (mpVideoEngine)
    {
        if (mbSending[voiceChannel % MAX_VE_CONNECTIONS])
        {
            rc = mpVideoEngine->GIPSVideo_StopSend(voiceChannel) ;
            checkRC("GIPSVideo_StopSend", voiceChannel, rc, true) ;
            if (rc != 0)
                rc = false ;
        }

        if (mbRendering[voiceChannel % MAX_VE_CONNECTIONS])
        {
            rc = mpVideoEngine->GIPSVideo_StopRender(voiceChannel) ;
            checkRC("GIPSVideo_StopRender", voiceChannel, rc, true) ;
            if (rc != 0)
                rc = false ;
        }


        if (mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS])
        {
            rc = mpVideoEngine->GIPSVideo_DeleteChannel(voiceChannel) ;
            checkRC("GIPSVideo_Channel", voiceChannel, rc, true) ;
            if (rc != 0)
                rc = false ;
        }

        if (    mbPreviewRendererSet && 
                mPreviewDisplay.type == SIPXVE_DIRECT_SHOW_FILTER && 
                mPreviewDisplay.filter)
        {
#ifdef WIN32
            ((IBaseFilter*) mPreviewDisplay.filter)->Release() ;
#endif
            mbPreviewRendererSet = false ;
        }            


        if (    mbRemoteRendererSet[voiceChannel % MAX_VE_CONNECTIONS] && 
                mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS].type == SIPXVE_DIRECT_SHOW_FILTER &&
                mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS].filter)
        {
#ifdef WIN32
            ((IBaseFilter*) mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS].filter)->Release() ;
#endif
            mbRemoteRendererSet[voiceChannel % MAX_VE_CONNECTIONS] = false ;
        }
    
        mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] = false ;
        mbPreviewRendererSet = false ;
        mbRemoteRendererSet[voiceChannel % MAX_VE_CONNECTIONS] = false ;
        mbRendering[voiceChannel % MAX_VE_CONNECTIONS] = false ;
        mbSending[voiceChannel % MAX_VE_CONNECTIONS] = false ;
    }
    else
        bRC = false ;

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::destroy()
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::destroy") ;
    int rc = 0 ;
/*
    if (mpVideoEngine)
    {
        rc = mpVideoEngine->GIPSVideo_Stop() ;
        mbRunning = false ;
    }

    for (int i=0; i<MAX_VE_CONNECTIONS; i++)
    {
        destroyChannel(i) ;               
    }
*/        
    return (rc == 0) ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::setTransport(int voiceChannel, GIPS_transport& transport)
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::setTransport") ;
    int rc = -1 ;

    assert(mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] == true) ;

    if (mpVideoEngine)
    {
        rc = mpVideoEngine->GIPSVideo_SetSendTransport(voiceChannel, &transport) ;
        checkRC("GIPSVideo_SetSendTransport", voiceChannel, rc, true);
    }

    return (rc == 0) ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::startPreview(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec)
{       
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::startPreview") ;
    int rc ;
    
    if (mpVideoEngine && 
            mbRunning == false &&
            doStartup(voiceChannel, true, false, false, false, pVideoCodec))
    {
        // assert(mbCaptureDeviceSet == true) ;        
        assert(mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] == true) ;
        assert(pVideoCodec != NULL) ;
        assert(isDisplayValid(&mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS]) == true) ;   // We cannot add the remote display late according to GIPS

        bool bSetPreview = doSetPreviewRenderer(voiceChannel) ;
        bool bSetRemote = doSetRemoteRenderer(voiceChannel) ;

        if (bSetRemote)
        {
            // If the codec changed, reset it
//              if (pVideoCodec && memcmp(pVideoCodec, &mSendCodec[voiceChannel % MAX_VE_CONNECTIONS], sizeof(GIPSVideo_CodecInst)) != 0)
//              {
//                    memcpy(&mSendCodec[voiceChannel % MAX_VE_CONNECTIONS], pVideoCodec, sizeof(GIPSVideo_CodecInst)) ;
//                    logCodec("setSendCodec (startPreview)", &mSendCodec[voiceChannel % MAX_VE_CONNECTIONS]) ;
//                    rc = mpVideoEngine->GIPSVideo_SetSendCodec(voiceChannel, &mSendCodec[voiceChannel % MAX_VE_CONNECTIONS], false) ;
//                    mbForceCodecReset = (rc < 0) ;
//                    checkRC("GIPSVideo_SetSendCodec", voiceChannel, rc, mbCaptureDeviceSet);                    
//              }

            rc = mpVideoEngine->GIPSVideo_Run();
            checkRC("GIPSVideo_Run", voiceChannel, rc, true);

            mbRunning = (rc == 0) ;
        }
    }

    return mbRunning ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::addReceiveCodec(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec)
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VoiceEngine::addReceiveCodec") ;

    assert (mbRendering[voiceChannel % MAX_VE_CONNECTIONS] == false) ;
    
    int rc ;
    rc = mpVideoEngine->GIPSVideo_SetReceiveCodec(voiceChannel, pVideoCodec) ;
    // BUG BUG BUG: Gips returns -1 on QCIF H263
    checkRC("GIPSVideo_SetReceiveCodec", voiceChannel, rc, false);
    logCodec("addReceiveCodec", pVideoCodec) ;


    GIPSVideo_CodecInst* pCopy = new GIPSVideo_CodecInst ;
    if (pCopy)
    {
        memcpy(pCopy, pVideoCodec, sizeof(GIPSVideo_CodecInst)) ;
        mReceiveCodecs[voiceChannel % MAX_VE_CONNECTIONS]->append(new UtlVoidPtr(pCopy)) ;
    }

    return (rc == 0) ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::startRender(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec)
{

#if defined (GIPS_STUB)
    return true;
#endif

    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::startRendering") ;
    
    bool bResetSendingCodec = mbForceCodecReset ;
    if (mpVideoEngine && 
            mbRendering[voiceChannel % MAX_VE_CONNECTIONS] == false &&
            doStartup(voiceChannel, true, true, false, false, pVideoCodec))
    {
        // assert(mbCaptureDeviceSet == true) ;        
        assert(mbRunning == true) ;
        assert(mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] == true) ;            
        int rc ;

        doUpdateReceiveCodec(voiceChannel, pVideoCodec) ;

        rc = mpVideoEngine->GIPSVideo_StartRender(voiceChannel) ;
        checkRC("GIPSVideo_StartRender", voiceChannel, rc, true);

        if (rc == 0)
            mbRendering[voiceChannel % MAX_VE_CONNECTIONS] = true ;
    }
    else if (mpVideoEngine &&  mbRendering[voiceChannel % MAX_VE_CONNECTIONS] == true)
    {
        doUpdateReceiveCodec(voiceChannel, pVideoCodec) ;
    }
    
    return mbRendering[voiceChannel % MAX_VE_CONNECTIONS] ;
}


//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::startSend(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec)    
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::startSend") ;

    if (    mpVideoEngine &&
            mbSending[voiceChannel % MAX_VE_CONNECTIONS] == false &&
            doStartup(voiceChannel, true, true, true, false, NULL))
    {
        int rc ;
#if !defined (GIPS_STUB)        
        assert(mbRunning == true) ;
        assert(mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] == true) ;            
        assert(mbRendering[voiceChannel % MAX_VE_CONNECTIONS] == true) ;
#endif

        doUpdateReceiveCodec(voiceChannel, pVideoCodec) ;
        doUpdateSendCodec(voiceChannel, pVideoCodec) ;

        rc = mpVideoEngine->GIPSVideo_StartSend(voiceChannel) ;
        checkRC("GIPSVideo_StartSend", voiceChannel, rc, mbCaptureDeviceSet) ;

        mbSending[voiceChannel % MAX_VE_CONNECTIONS] = (rc == 0) || !mbCaptureDeviceSet ;

        UtlString params ;
        params.format("bitrate:%d maxbitrate:%d framerate:%d res:%dx%d, q:%d l:%d other:%d",
                pVideoCodec->bitRate,
                pVideoCodec->maxBitRate,
                pVideoCodec->frameRate,
                pVideoCodec->width,
                pVideoCodec->height,
                pVideoCodec->quality,
                pVideoCodec->level,
                pVideoCodec->codecSpecific) ;
        mMediaDeviceInfo.setParameters(params) ;
    }

    return mbSending[voiceChannel % MAX_VE_CONNECTIONS] ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::pauseSend(int voiceChannel)
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::pauseSend") ;
    int rc ;

    if (mpVideoEngine && 
        mbSendPaused[voiceChannel % MAX_VE_CONNECTIONS] == false && 
        //mbCaptureDeviceSet &&
        mbSending[voiceChannel % MAX_VE_CONNECTIONS])
    {
#if !defined (GIPS_STUB)
        assert(mbRunning == true) ;
        assert(mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] == true) ;           
        assert(mbSending[voiceChannel % MAX_VE_CONNECTIONS] == true) ;
#endif
        rc = mpVideoEngine->GIPSVideo_StopSend(voiceChannel) ;
        checkRC("GIPSVideo_StopSend", voiceChannel, rc, mbCaptureDeviceSet) ;        
        mbSending[voiceChannel % MAX_VE_CONNECTIONS] = false ;
        mbSendPaused[voiceChannel % MAX_VE_CONNECTIONS] = true ;
    }
    
    return !mbSending[voiceChannel % MAX_VE_CONNECTIONS] ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::pauseReceive(int voiceChannel)
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::pauseReceive") ;
    int rc ;

    if (mpVideoEngine && 
            mbReceivePaused[voiceChannel % MAX_VE_CONNECTIONS] == false && 
            mbRendering[voiceChannel % MAX_VE_CONNECTIONS])
    {
        // assert(mbCaptureDeviceSet == true) ;        
#if !defined (GIPS_STUB)
        assert(mbRunning == true) ;
        assert(mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] == true) ;            
        assert(mbRendering[voiceChannel % MAX_VE_CONNECTIONS] == true) ;
#endif

        rc = mpVideoEngine->GIPSVideo_StopRender(voiceChannel) ;
        checkRC("GIPSVideo_StopRender", voiceChannel, rc, true) ;
        mbRendering[voiceChannel % MAX_VE_CONNECTIONS] = !(rc == 0) ;
        mbReceivePaused[voiceChannel % MAX_VE_CONNECTIONS] = (rc == 0) ;
    }
    
    return !mbRendering[voiceChannel % MAX_VE_CONNECTIONS] ;
}

//////////////////////////////////////////////////////////////////////////////


bool VideoEngine::resumeSend(int voiceChannel)
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::resumeSend") ;
    int rc ;

    if (mpVideoEngine && 
            mbSendPaused[voiceChannel % MAX_VE_CONNECTIONS] == true)
    {
        // assert(mbCaptureDeviceSet == true) ;
        
#if !defined (GIPS_STUB)
        assert(mbRunning == true) ;
        assert(mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] == true) ;            
        assert(mbSending[voiceChannel % MAX_VE_CONNECTIONS] == false) ;
#endif

        rc = mpVideoEngine->GIPSVideo_StartSend(voiceChannel) ;
        checkRC("GIPSVideo_StartSend", voiceChannel, rc, true) ;
        mbSending[voiceChannel % MAX_VE_CONNECTIONS] = (rc == 0) ;
        mbSendPaused[voiceChannel % MAX_VE_CONNECTIONS] = false ;
    }

    return mbSending[voiceChannel % MAX_VE_CONNECTIONS] ;
}


//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::resumeReceive(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec)
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::resumeReceive") ;
    int rc ;
    
    if (mpVideoEngine && 
            mbReceivePaused[voiceChannel % MAX_VE_CONNECTIONS] == true)
    {
#if !defined (GIPS_STUB)
        assert(mbRunning == true) ;
        assert(mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] == true) ;            
        assert(mbRendering[voiceChannel % MAX_VE_CONNECTIONS] == false) ;
#endif
        if (pVideoCodec && memcmp(pVideoCodec, &mSendCodec[voiceChannel % MAX_VE_CONNECTIONS], sizeof(GIPSVideo_CodecInst)) != 0)
        {                
            if (startRender(voiceChannel, pVideoCodec))
                mbReceivePaused[voiceChannel % MAX_VE_CONNECTIONS] = false ;
        }
        else
        {
            rc = mpVideoEngine->GIPSVideo_StartRender(voiceChannel) ;
            checkRC("GIPSVideo_StartRender", voiceChannel, rc, true) ;
            mbRendering[voiceChannel % MAX_VE_CONNECTIONS] = (rc == 0) ;
            mbRendering[voiceChannel % MAX_VE_CONNECTIONS] = true ;
            mbReceivePaused[voiceChannel % MAX_VE_CONNECTIONS] = false ;
        }
    }

    return mbRendering[voiceChannel % MAX_VE_CONNECTIONS] ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::enableRTCP(int         voiceChannel, 
                bool        bEnable,
                const char* szRTCPCName)
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::enableRTCP") ;
    int rc = -1 ;
    
    if (mpVideoEngine)
    {
        if (bEnable && szRTCPCName && *szRTCPCName)
        {
            rc = mpVideoEngine->GIPSVideo_SetRTCPCNAME(voiceChannel, (char*) szRTCPCName) ;
            checkRC("GIPSVE_SetRTCPCNAME", voiceChannel, rc, false) ;
        }

        rc = mpVideoEngine->GIPSVideo_EnableRTCP(voiceChannel, bEnable) ;
        checkRC("GIPSVE_EnableRTCP", voiceChannel, rc, false) ;
    }

    return (rc == 0) ;
}

/* ============================ ACCESSORS ================================= */

GipsVideoEnginePlatform* VideoEngine::getVideoEngine() const
{ 
    return mpVideoEngine; 
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::getCaptureError() const
{
    OsLock lock(mLock) ;
    return mbCameraError ;
}

//////////////////////////////////////////////////////////////////////////////

void VideoEngine::setCaptureError(bool bError, const char* szCause)
{ 
    OsLock lock(mLock) ;
    mbCameraError = bError ;

    if (szCause && *szCause)
    {

        mMediaDeviceInfo.appendErrors(szCause) ;
    }
    else
    {
        mMediaDeviceInfo.appendErrors("Unknown Error\n") ;
    }
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::setCaptureDevice(VoiceEngineFactoryImpl* pImpl, const char* szDevice) 
{
    OsLock lock(mLock) ;


    mMediaDeviceInfo.reset() ;
    mMediaDeviceInfo.setRequested(szDevice) ;
    
    trace(PRI_DEBUG, "VideoEngine::setCaptureDevice") ;
    mbCaptureDeviceSet = false ;
    if (mpVideoEngine)        
    {
        // Make sure we can get atleast 1 device
        char cDevice[128] ;
        memset(cDevice, 0, sizeof(cDevice)) ;
        if (mpVideoEngine->GIPSVideo_GetCaptureDevice(0, cDevice, sizeof(cDevice)-1) == 0)
        {
            // Use first camera if no camera is specified
            if (mMediaDeviceInfo.getRequested().length() == 0)
                mMediaDeviceInfo.setRequested(cDevice) ;

            // Set camera
            int setRC = 0 ;
            if ((setRC = mpVideoEngine->GIPSVideo_SetCaptureDevice(
                    mMediaDeviceInfo.getRequested().data(), 
                    mMediaDeviceInfo.getRequested().length())) == 0)
            {
                trace(PRI_DEBUG, "Capture Device set to %s", mMediaDeviceInfo.getRequested().data()) ;
                mMediaDeviceInfo.setSelected(mMediaDeviceInfo.getRequested());
                mbCaptureDeviceSet = true ;
            }
            else
            {
                UtlString error ;
                error.format("%s: rc=%d, lastError=%d",
                        mMediaDeviceInfo.getRequested().data(),
                        setRC,
                        mpVideoEngine->GIPSVideo_GetLastError()) ;
                mMediaDeviceInfo.appendErrors(error) ;

                trace(PRI_DEBUG, "FAILED Capture Device %s", mMediaDeviceInfo.getRequested().data()) ;

                // Find another device
                int index=0;
                while (mpVideoEngine->GIPSVideo_GetCaptureDevice(index++, cDevice, sizeof(cDevice)-1) == 0)
                {
                    // Ignore what we just tried
                    if (mMediaDeviceInfo.getRequested().compareTo(cDevice) != 0)
                    {                           
                        if ((setRC = mpVideoEngine->GIPSVideo_SetCaptureDevice(cDevice, strlen(cDevice))) == 0)
                        {
                            trace(PRI_DEBUG, "Capture Device set to %s", cDevice) ;
                            mMediaDeviceInfo.setSelected(cDevice) ;
                            mbCaptureDeviceSet = true ;
                            break ;
                        }
                        else
                        {
                            trace(PRI_DEBUG, "FAILED Capture Device %s", cDevice) ;

                            error.format("%s: rc=%d, lastError=%d\n",
                                    cDevice,
                                    setRC,
                                    mpVideoEngine->GIPSVideo_GetLastError()) ;
                            mMediaDeviceInfo.appendErrors(error) ;
                        }
                    }
                }
            }
        }

        if (!mMediaDeviceInfo.getSelected().isNull())
        {
            mbHaveCameraCapabilities = doGetCameraCapabilities(
                    mMediaDeviceInfo.getSelected(), 
                    &mCameraCapabilities) ;            
        }
        else
        {
            setCaptureError(true, "No usable cameras found") ;
        }

        trace(PRI_INFO, 
                "SetCaptureDevice requested=%s selected=%s, success=%d",
                mMediaDeviceInfo.getRequested().data(),
                mMediaDeviceInfo.getSelected().data(),
                mbCaptureDeviceSet) ;
    }
    
    return mbCaptureDeviceSet ;
}

//////////////////////////////////////////////////////////////////////////////

void VideoEngine::setPreviewDisplay(SIPXVE_VIDEO_DISPLAY previewDisplay) 
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::setPreviewDisplay") ;
    mPreviewDisplay = previewDisplay ;
}

//////////////////////////////////////////////////////////////////////////////

void VideoEngine::setRemoteDisplay(int voiceChannel, SIPXVE_VIDEO_DISPLAY remoteDisplay) 
{
    OsLock lock(mLock) ;
    trace(PRI_DEBUG, "VideoEngine::setRemoteDisplay") ;
    mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS] = remoteDisplay ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::getCameraCapabilities(GIPSCameraCapability* pCaps) const
{
    OsLock lock(mLock) ;
    bool bRC = false ;

    if (mbHaveCameraCapabilities && !mbIgnoreCameraCapabilties)
    {
        memcpy(pCaps, &mCameraCapabilities, sizeof(GIPSCameraCapability)) ;
        bRC = true ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::getMaxFramerate(int& frameRate) const
{
    OsLock lock(mLock) ;
    bool bRC = false ;

    if (mbHaveCameraCapabilities && mCameraCapabilities.maxFPS > 0 && 
            !mbIgnoreCameraCapabilties)
    {
        frameRate = mCameraCapabilities.maxFPS ;
        bRC = true ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::getMaxResolution(int& width, int& height) const
{
    OsLock lock(mLock) ;
    bool bRC = false ;

    if (mbHaveCameraCapabilities && mCameraCapabilities.width > 0 &&
            mCameraCapabilities.height > 0 && !mbIgnoreCameraCapabilties)
    {
        width = mCameraCapabilities.width  ;
        height = mCameraCapabilities.height ;
        bRC = true ;
    }

    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

int VideoEngine::getRTCPStats(int channel, GIPSVideo_CallStatistics* stats) 
{
    OsLock lock(mLock) ;
    int rc = -1;
    
    if (mpVideoEngine)
        rc = mpVideoEngine->GIPSVideo_RTCPStat(channel, stats) ;

    return rc ;
}

//////////////////////////////////////////////////////////////////////////////

MediaDeviceInfo& VideoEngine::getVideoCaptureDeviceInfo()
{
    return mMediaDeviceInfo ;
}

//////////////////////////////////////////////////////////////////////////////

void VideoEngine::logCameraCapabilities(const char* szDevice, GIPSCameraCapability* pCaps)
{
    if (pCaps && szDevice)
    {
        OsSysLog::add(FAC_MP, PRI_INFO, "Capture Device: %s type=%d maxFPS=%d, resolution=%dx%d",
                szDevice,
                pCaps->type,
                pCaps->maxFPS,
                pCaps->width,
                pCaps->height) ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void VideoEngine::logCodec(const char* context, GIPSVideo_CodecInst* codec)
{
    trace(PRI_INFO, "%s - pltype=%d plname=%s bitRate=%d maxBitrate=%d frameRate=%d height=%d width=%d quality=%d codecSpecific=%d",
            context,
            codec->pltype,
            codec->plname,
            codec->bitRate,
            codec->maxBitRate,
            codec->frameRate,
            codec->height,
            codec->width,
            codec->quality,
            codec->codecSpecific) ;
}


/* ============================ INQUIRY =================================== */

bool VideoEngine::isDisplayValid(const SIPXVE_VIDEO_DISPLAY* const pDisplay) 
{
    bool bRet = false;
    if (pDisplay && pDisplay->cbSize && (pDisplay->handle || pDisplay->filter))
    {
        bRet = true;
    }
    return bRet;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::isChannelCreated(int voiceChannel) const
{ 
    OsLock lock(mLock) ; 
    return mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS] ; 
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::isReceivePaused(int voiceChannel) const
{ 
    OsLock lock(mLock) ; 
    return mbReceivePaused[voiceChannel % MAX_VE_CONNECTIONS] ; 
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::isSendPaused(int voiceChannel) const
{ 
    OsLock lock(mLock) ; 
    return mbSendPaused[voiceChannel % MAX_VE_CONNECTIONS] ; 
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::isCaptureDeviceSet() const
{ 
    OsLock lock(mLock) ; 
    return mbCaptureDeviceSet; 
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::isPreviewing() const
{ 
    OsLock lock(mLock) ; 
    return mbRunning; 
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

int VideoEngine::checkRC(const char* szAPI, 
                        int         connectionId,
                        int         returnCode, 
                        bool        bAssertOnError) 
{
    int lastError = 0;
    char cId[16] ;
    sprintf(cId, " cId=%d ", connectionId);
    if (returnCode == 0)
    {
#if defined(_WIN32) && defined(_DEBUG)
        UtlString consoleOutput ;
        OsDateTime::getLocalTimeString(consoleOutput) ;

        consoleOutput.append(" ") ;
        consoleOutput.append(szAPI) ;
        consoleOutput.append(cId);
        consoleOutput.append(" returned 0\n") ;

        OutputDebugString(consoleOutput) ;
#endif
    }
    else
    {
        lastError = mpVideoEngine->GIPSVideo_GetLastError() ;
#if defined(_WIN32) && defined(_DEBUG)
        UtlString consoleOutput ;
        OsDateTime::getLocalTimeString(consoleOutput) ;
        char cTemp[128] ;

        consoleOutput.append(" ") ;
        consoleOutput.append(szAPI) ;

        sprintf(cTemp, " cId=%d returned %d (lastError=%d)\n", connectionId, returnCode, lastError) ;
        consoleOutput.append(cTemp) ;

        OutputDebugString(consoleOutput) ;
#endif

        trace(PRI_ERR, "%s %s returned %d (lastError=%d)", 
                cId, 
                szAPI,
                returnCode,
                lastError) ;

        if (bAssertOnError)
        {
            assert(false) ;
        }
    }
    return lastError;
}

//////////////////////////////////////////////////////////////////////////////

void VideoEngine::doUserAgentSpecific(int voiceChannel) 
{
    UtlString userAgent = mUserAgent[voiceChannel % MAX_VE_CONNECTIONS] ;
    userAgent.toLower() ;
    if (userAgent.contains("rtc/"))
    {
//            printf("%s: DO RTC HACK\n", userAgent.data()) ;

        int rc = mpVideoEngine->GIPSVideo_SetInverseH263Logic(voiceChannel, true) ;

        checkRC("GIPSVideo_SetInverseH263Logic", voiceChannel, rc, true) ;                       
        trace(PRI_INFO, "Remote UserAgent: %s, use inverse h263 logic",
                userAgent.data()) ;
        
    }
    else
    {
//            printf("%s: NO HACK\n", userAgent.data()) ;

        trace(PRI_INFO, "Remote UserAgent: %s, use normal h263 logic",
                userAgent.data()) ;
        
    }
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::doUpdateReceiveCodec(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec)    
{
    bool bFound = false ;
    int rc = 0 ;

    if (mReceiveCodecs[voiceChannel % MAX_VE_CONNECTIONS])
    {
        UtlVoidPtr* pPtr ;
        UtlSListIterator itor(*mReceiveCodecs[voiceChannel % MAX_VE_CONNECTIONS]) ;
        while ((pPtr = (UtlVoidPtr*) itor()) != NULL)
        {
            GIPSVideo_CodecInst* pInst = (GIPSVideo_CodecInst*) 
                    pPtr->getValue() ;

            if (pInst->pltype == pVideoCodec->pltype)
            {
                if (memcmp(pInst, pVideoCodec, sizeof(GIPSVideo_CodecInst)) != 0)
                {
                    memcpy(pInst, pVideoCodec, sizeof(GIPSVideo_CodecInst)) ;

                    UtlString name(pVideoCodec->plname) ;
                    name.toUpper() ;
                    bool bForceRefresh = name.contains("VP71") ;

                    if (bForceRefresh)
                    {
                        trace(PRI_INFO, "Resetting received codec (force=%d)", bForceRefresh) ;
                        rc = mpVideoEngine->GIPSVideo_SetReceiveCodec(voiceChannel, pInst, bForceRefresh) ;
                        checkRC("GIPSVideo_SetReceiveCodec", voiceChannel, rc, true);
                        logCodec("doUpdateReceiveCodec (found)", pVideoCodec) ;
                    }
                }

                bFound = true ;
                break ;
            }
        }               
    }


    if (!bFound)
    {
        rc = mpVideoEngine->GIPSVideo_SetReceiveCodec(voiceChannel, pVideoCodec) ;
        checkRC("GIPSVideo_SetReceiveCodec", voiceChannel, rc, true);
        logCodec("doUpdateReceiveCodec (not found)", pVideoCodec) ;    
    }

    return (rc == 0) ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::doUpdateSendCodec(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec)    
{
    int rc = 0 ;

    if (pVideoCodec && memcmp(pVideoCodec, &mSendCodec[voiceChannel % MAX_VE_CONNECTIONS], sizeof(GIPSVideo_CodecInst)) != 0)
    {
        memcpy(&mSendCodec[voiceChannel % MAX_VE_CONNECTIONS], pVideoCodec, sizeof(GIPSVideo_CodecInst)) ;
        rc = mpVideoEngine->GIPSVideo_SetSendCodec(voiceChannel, &mSendCodec[voiceChannel % MAX_VE_CONNECTIONS], false) ;
        checkRC("GIPSVideo_SetSendCodec", voiceChannel, rc, true);
        logCodec("doUpdateSendCodec", &mSendCodec[voiceChannel % MAX_VE_CONNECTIONS]) ;
    }

    return (rc == 0) ;
}


//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::doGetCameraCapabilities(const char* szDevice, GIPSCameraCapability* pCaps)
{
#if defined (GIPS_STUB)
    return false;
#endif 
    bool bFound = false ;
    char cDevice[128] ;
    GIPSCameraCapability caps ;

    memset(pCaps, 0, sizeof(GIPSCameraCapability)) ;

    int i = 0 ;
    strcpy(cDevice, szDevice) ;
    while (mpVideoEngine->GIPSVideo_GetCaptureCapabilities(cDevice, (int) sizeof(cDevice), i, &caps) != -1)
    {
        logCameraCapabilities(cDevice, &caps) ;
        bFound = true ;
        if (caps.width > pCaps->width || caps.height > pCaps->height)
        {
            // Ignore camera caps greater then out max resolution
            if ((caps.width <= 640) && (caps.height <= 480))
            {
                memcpy(pCaps, &caps, sizeof(GIPSCameraCapability)) ;
            }
        }
        i++ ;
    }

    return bFound ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::doSetPreviewRenderer(int voiceChannel)
{
    int  rc ;

#if !defined (GIPS_STUB)
    assert(mbPreviewRendererSet == false) ; // Should not reset renderer
#endif

    mbPreviewRendererSet = false ;
    if (isDisplayValid(&mPreviewDisplay))
    {
        if (mPreviewDisplay.type == SIPXVE_WINDOW_HANDLE_TYPE)
        {           
#if !defined (GIPS_STUB)
#  if defined (_WIN32)
            rc = mpVideoEngine->GIPSVideo_AddLocalRenderer(
                    (HWND) mPreviewDisplay.handle, 0, 0, 0, 1, 1);
#  elif defined (__MACH__)
            rc = mpVideoEngine->GIPSVideo_AddLocalRenderer(
                    (WindowRef) mPreviewDisplay.handle, 0, 0, 0, 1, 1);
#  else
            rc = 0 ;            
#  endif
            checkRC("GIPSVideo_AddLocalRenderer", voiceChannel, rc, true) ;
            mbPreviewRendererSet = (rc == 0) ;
#endif
        }
#if defined (_WIN32) && !defined (GIPS_STUB)
        else if (mPreviewDisplay.type == SIPXVE_DIRECT_SHOW_FILTER && mPreviewDisplay.filter)
        {
            ((IBaseFilter*) mPreviewDisplay.filter)->AddRef() ;
            rc = mpVideoEngine->GIPSVideo_AddLocalRenderer((IBaseFilter*) mPreviewDisplay.filter);
            checkRC("GIPSVideo_AddLocalRenderer", voiceChannel, rc, true) ;
            mbPreviewRendererSet = (rc == 0) ;
        }
        else
        {
            OsSysLog::add(FAC_MP, PRI_ERR, 
                    "Invalid display type; unable to set preview renderer") ;                
            assert(false) ;
        }
#else
    mbPreviewRendererSet = true ;
#endif                
    }
    return mbPreviewRendererSet ;
}

//////////////////////////////////////////////////////////////////////////////

bool VideoEngine::doSetRemoteRenderer(int voiceChannel)
{
    int rc ;

    assert(mbRemoteRendererSet[voiceChannel % MAX_VE_CONNECTIONS] == false) ; // Should not reset renderer

    mbRemoteRendererSet[voiceChannel % MAX_VE_CONNECTIONS] = false ;
    if (isDisplayValid(&mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS]))
    {
        // Setup Remote Renderer
        if (mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS].type == SIPXVE_WINDOW_HANDLE_TYPE)
        {
#if defined (_WIN32) && !defined(GIPS_STUB)
            rc = mpVideoEngine->GIPSVideo_AddRemoteRenderer(voiceChannel, 
                    (HWND) mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS].handle, 0, 0, 0, 1, 1);                     
            checkRC("GIPSVideo_AddRemoteRenderer", voiceChannel, rc, true) ;
            mbRemoteRendererSet[voiceChannel % MAX_VE_CONNECTIONS] = (rc == 0) ;
#elif defined (__MACH__)
            rc = mpVideoEngine->GIPSVideo_AddRemoteRenderer(voiceChannel, 
                    (WindowRef) mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS].handle, 0, 0, 0, 1, 1);                     
            checkRC("GIPSVideo_AddRemoteRenderer", voiceChannel, rc, true) ;
            mbRemoteRendererSet[voiceChannel % MAX_VE_CONNECTIONS] = (rc == 0) ;
#endif
        }
#if defined (_WIN32) && !defined (GIPS_STUB)
        else if (mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS].type == SIPXVE_DIRECT_SHOW_FILTER)
        {
            ((IBaseFilter*) mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS].filter)->AddRef() ;
            rc = mpVideoEngine->GIPSVideo_AddRemoteRenderer(voiceChannel, 
                    (IBaseFilter*) mRemoteDisplay[voiceChannel % MAX_VE_CONNECTIONS].filter);    
            checkRC("GIPSVideo_AddRemoteRenderer", voiceChannel, rc, true) ;
            mbRemoteRendererSet[voiceChannel % MAX_VE_CONNECTIONS] = (rc == 0) ;
        }
        else
        {
            OsSysLog::add(FAC_MP, PRI_ERR, 
                    "Invalid display type; unable to set remote renderer") ;
            assert(false) ;
        }
#endif         
    }

    return mbRemoteRendererSet[voiceChannel % MAX_VE_CONNECTIONS] ;
}

// return true if all is running
bool VideoEngine::doStartup(int  voiceChannel,
                            bool bCreateChannel,
                            bool bStartPreview,
                            bool bStartRender,
                            bool bStartSend,
                            GIPSVideo_CodecInst* pVideoCodec)
{
#if 0
    bool bRunning = false ;

    if (mbAutoStart)
    {
        bRunning = true ;

        // Create Channel
        if (bCreateChannel && !mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS])
        {
            bRunning = createChannel(voiceChannel) ;
        }

        // Start Preview
        if (bRunning && bStartPreview && !mbRunning)
        {
            if (pVideoCodec)
                bRunning = startPreview(voiceChannel, pVideoCodec) ;
            else
                bRunning = false ;
        }

        // Start Receive/Render
        if (bRunning && bStartRender && !mbRendering[voiceChannel % MAX_VE_CONNECTIONS])
        {
            if (pVideoCodec)
                bRunning = startRender(voiceChannel, pVideoCodec) ;
            else
                bRunning = false ;
        }

        // Start Send
        if (bRunning && bStartSend && !mbSending[voiceChannel % MAX_VE_CONNECTIONS])
        {
            bRunning = startSend(voiceChannel) ;
        }
    } 
    else
    {
        bRunning = (
                (!bCreateChannel || mbChannelCreated[voiceChannel % MAX_VE_CONNECTIONS]) &&
                (!bStartPreview || mbRunning) &&
                (!bStartRender || mbRendering[voiceChannel % MAX_VE_CONNECTIONS]) &&
                (!bStartSend || mbSending[voiceChannel % MAX_VE_CONNECTIONS])
                ) ;
    }

    trace(PRI_DEBUG, "doStartup(%d %d %d %d) returning %d",
            bCreateChannel,
            bStartPreview,
            bStartRender,
            bStartSend,
            bRunning) ;

    return bRunning ;
#else
return true ;
#endif
}

void VideoEngine::trace(OsSysLogPriority priority, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    /* Guess we need no more than 128 bytes. */
    int n, size = 128;
    char *p;

    p = (char*) malloc(size) ;
    
    while (p != NULL)
    {
        /* Try to print in the allocated space. */
#ifdef _WIN32
        n = _vsnprintf (p, size, format, args);
#else
        n = vsnprintf (p, size, format, args);
#endif

        /* If that worked, return the string. */
        if (n > -1 && n < size)
        {
            break;
        }
        /* Else try again with more space. */
        if (n > -1)    /* glibc 2.1 */
            size = n+1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        if ((p = (char*) realloc (p, size)) == NULL)
        {
            break;
        }
    }

    if (p != NULL)
    {
        OsSysLog::add(FAC_MP, priority, "%s", p) ;
        if (sbConsoleTrace)
        {
            printf("%s\n", p) ;            
        }
        free(p) ;
    }

    va_end(args) ;
}

//////////////////////////////////////////////////////////////////////////////

/* //////////////////////////// PRIVATE /////////////////////////////////// */



/*
rc = mpVideoEngine->GIPSVideo_EnableSRTPSend(
                                connectionId, mSrtpParams.cipherType, 
                                30, AUTH_HMAC_SHA1, 16, 4, 
                                mSrtpParams.securityLevel & SRTP_SECURITY_MASK,
                                mSrtpParams.masterKey);

                        checkVideoEngineReturnCode("GIPSVideo_EnableSRTPSend", connectionId, rc, true) ;

if (mSrtpParams.securityLevel & SRTP_RECEIVE)
            {
                rc = mpVideoEngine->GIPSVideo_EnableSRTPReceive(
                        connectionId,
                        mSrtpParams.cipherType,
                        30,
                        AUTH_HMAC_SHA1, 
                        16,
                        4,
                        mSrtpParams.securityLevel&SRTP_SECURITY_MASK,
                        mSrtpParams.masterKey);

                checkVideoEngineReturnCode("GIPSVideo_EnableSRTPReceive", connectionId, rc, true) ;
            }

UtlBoolean VoiceEngineMediaInterface::getVideoEngineCodec(const SdpCodec& pCodec, GIPSVideo_CodecInst& codecInfo)
{
OsLock lock(mVoiceEngineGuard) ;
UtlString codecName;
UtlBoolean matchFound = FALSE;
int iCodecs;

if (getCodecNameByType(pCodec.getCodecType(), codecName))
{
    if ((iCodecs=mpVideoEngine->GIPSVideo_GetNofCodecs()) != -1)
    {
        for (int i=0; i<iCodecs; ++i)
        {
            memset((void*)&codecInfo, 0, sizeof(codecInfo));
            if (mpVideoEngine->GIPSVideo_GetCodec(i, &codecInfo) == 0)
            {
                if (strncmp(codecName.data(), codecInfo.plname, 4) == 0)
                {
                    matchFound = TRUE;
                    break;
                }
            }
        }
    }
}
return matchFound;
}


void VoiceEngineMediaInterface::enableConferencing(int connectionId)
{
#ifdef VIDEO
OsLock lock(mVoiceEngineGuard) ;

int rc = 0;
int numConnections =  mMediaConnections.entries();
int handle = 0;
VoiceEngineMediaConnection* pMediaConnection = NULL ; 

if (numConnections > 1 && !mbConferenceEnabled)
{
    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (VoiceEngineMediaConnection*) iterator()))    
    {
        rc = mpVideoEngine->GIPSVideo_Conferencing(pMediaConnection->getValue(), true) ;
        checkVideoEngineReturnCode("GIPSVideo_Conferencing (true)", pMediaConnection->getValue(), rc, true) ;

        handle = getVideoQuadrantHandle(pMediaConnection->getValue());
        renderVideoQuadrant(handle, pMediaConnection->getValue());
    }
    mbConferenceEnabled = true;
}
else if (numConnections > 1)
{
    rc = mpVideoEngine->GIPSVideo_Conferencing(connectionId, true) ;
    checkVideoEngineReturnCode("GIPSVideo_Conferencing (true)", connectionId, rc, true) ;
    handle = getVideoQuadrantHandle(connectionId);
    renderVideoQuadrant(handle, connectionId);
}
#endif
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::disableConferencing(int connectionId)
{
OS_PERF_FUNC("VoiceEngineMediaInterface::disableConferencing") ;
#ifdef VIDEO
OsLock lock(mVoiceEngineGuard) ;
#ifdef WIN32
char cFoo[128];
sprintf(cFoo, "VoiceEngineMediaInterface::disableConferencing cId %d\n", connectionId);
OutputDebugString(cFoo);
#endif
int rc = 0;
int numConnections = 0;

if (mbConferenceEnabled)
{
    int handle = findVideoQuadrantHandle(connectionId);
    releaseVideoQuadrantHandle(handle);
    // This call returns -1 but seems to function ok - don't assert 
    rc = mpVideoEngine->GIPSVideo_Conferencing(connectionId, false);
    checkVideoEngineReturnCode("GIPSVideo_Conferencing (false)", connectionId, rc, false);

    if ((numConnections=getNumConnections()) >= 1)
    {
        if (numConnections == 1)
        {
            VoiceEngineMediaConnection* pMediaConn = (VoiceEngineMediaConnection*)mMediaConnections.first();
            int onlyConnection = pMediaConn->getValue();

            handle = findVideoQuadrantHandle(onlyConnection);
            releaseVideoQuadrantHandle(handle);

            rc = mpVideoEngine->GIPSVideo_Conferencing(onlyConnection, false);
            checkVideoEngineReturnCode("GIPSVideo_Conferencing (only, false)", onlyConnection, rc, false);
#ifdef WIN32
            rc = mpVideoEngine->GIPSVideo_ConfigureMixer(onlyConnection, 0,
                    (HWND) mpDisplay->handle, 1, 1, 0, 0, 1, 1);         
            mbConferenceEnabled = false;
#endif
        }
//            rc = mpVideoEngine->GIPSVideo_Run();
//            checkVideoEngineReturnCode("GIPSVideo_Run", connectionId, rc, true);
    }
}
#endif
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::renderVideoQuadrant(int handle, int connectionId)
{
OS_PERF_FUNC("VoiceEngineMediaInterface::renderVideoQuadrant") ;

#ifdef VIDEO
OsLock lock(mVoiceEngineGuard) ;
int rc = -1;
#ifdef WIN32
switch (handle)
{
case 0:
    rc = mpVideoEngine->GIPSVideo_ConfigureMixer(connectionId, 0,
            (HWND) mpDisplay->handle, 0,
            1, 0, 0, 0.5f, 0.5f);                     
    checkVideoEngineReturnCode("GIPSVideo_ConfigureMixer", connectionId, rc, false) ;
    break;
case 1:
    rc = mpVideoEngine->GIPSVideo_ConfigureMixer(connectionId, 0,
            (HWND) mpDisplay->handle, 0,
            1, 0.5f, 0, 1.0f, 0.5f);                     
    checkVideoEngineReturnCode("GIPSVideo_ConfigureMixer", connectionId, rc, false) ;
    break;
case 2:
    rc = mpVideoEngine->GIPSVideo_ConfigureMixer(connectionId, 0,
            (HWND) mpDisplay->handle, 0,
            1, 0, 0.5, 0.5, 1);                     
    checkVideoEngineReturnCode("GIPSVideo_ConfigureMixer", connectionId, rc, false) ;
    break;
case 3:
    rc = mpVideoEngine->GIPSVideo_ConfigureMixer(connectionId, 0,
            (HWND) mpDisplay->handle, 0,
            1, 0.5, 0.5, 1, 1);                     
    checkVideoEngineReturnCode("GIPSVideo_ConfigureMixer", connectionId, rc, false) ;
    break;
}
#endif
//    rc = mpVideoEngine->GIPSVideo_Run();
//    checkVideoEngineReturnCode("GIPSVideo_Run", connectionId, rc, false) ;
#endif
}

//////////////////////////////////////////////////////////////////////////////

int VoiceEngineMediaInterface::getVideoQuadrantHandle(int connectionId)
{
int handle = -1;

for (int i=0; i < MAX_VIDEO_CONNECTIONS; i++)
{
    if (mVideoQuadrants[i] == -1)
    {
        mVideoQuadrants[i] = connectionId;
        handle = i;
        break;
    }
}
#ifdef WIN32
char cFoo[128];
sprintf(cFoo, "VoiceEngineMediaInterface::getVideoQuadrantHandle cId %d = handle %d\n", connectionId, handle);
OutputDebugString(cFoo);
#endif
return handle;
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::releaseVideoQuadrantHandle(int handle)
{
if (handle < MAX_VIDEO_CONNECTIONS)
{
    mVideoQuadrants[handle] = -1;
}
#ifdef WIN32
char cFoo[128];
sprintf(cFoo, "VoiceEngineMediaInterface::releaseVideoQuadrantHandle handle %d\n", handle);
OutputDebugString(cFoo);
#endif
}

//////////////////////////////////////////////////////////////////////////////

int VoiceEngineMediaInterface::findVideoQuadrantHandle(int connectionId)
{
int handle = -1;

for (int i=0; i < MAX_VIDEO_CONNECTIONS; i++)
{
    if (mVideoQuadrants[i] == connectionId)
    {
        handle = i;
        break;
    }
}
return handle;
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::clearVideoQuadrantHandles()
{
for (int i=0; i < MAX_VIDEO_CONNECTIONS; i++)
{
    mVideoQuadrants[i] = -1;
}
}
*/
