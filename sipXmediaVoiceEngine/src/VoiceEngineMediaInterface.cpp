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
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// for Windows, we need the COM version defs
#ifdef _WIN32
    #define _OLE32_
    #define _WIN32_DCOM
    #define _WIN32_WINNT 0x0400 
#endif

// APPLICATION INCLUDES
#include <utl/UtlSListIterator.h>
#include <os/OsProtectEventMgr.h>
#include "os/OsPerfLog.h"
#include <sdp/SdpCodec.h>
#include <net/SdpBody.h>
#include <net/NetBase64Codec.h>
#include "net/SipContactDb.h"

#include "mediaBaseImpl/CpMediaNetTask.h"
#include "os/IOsNatSocket.h"
#include "os/OsMediaContact.h"
#include "mediaInterface/IMediaSocket.h"
#include "mediaInterface/IMediaDeviceMgr.h" 
#include "include/VoiceEngineDatagramSocket.h"
#include "include/VoiceEngineMediaInterface.h"
#include "mediaBaseImpl/CpMediaSocketMonitorTask.h"
#include "include/VoiceEngineBufferInStream.h"
#include "include/VoiceEngineDefs.h"
#include "mediaBaseImpl/CpMediaConnection.h"
#include "include/VoiceEngineConnectionSocket.h"
#include "include/VoiceEngineSocketFactory.h"
#include "include/VoiceEngineSocketAdapter.h"
#include "ARS/ArsConnectionSocket.h"
#include "include/ArsVoiceEngineReceiveAdapter.h"

#include "include/VideoEngine.h"
#include "tapi/sipXtapi.h"

#if defined(_VXWORKS)
#   include <socket.h>
#   include <resolvLib.h>
#   include <netinet/ip.h>
#elif defined(__pingtel_on_posix__) || defined(__MACH__)
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <arpa/inet.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MINIMUM_DTMF_LENGTH 60
#define MAX_RTP_PORTS       1000

// STATIC VARIABLE INITIALIZATIONS
bool VideoEngine::sbConsoleTrace = false ;
// STATIC FUNCTIONS


class gipsve_error_callback : public error_callback
{
public:
    gipsve_error_callback(VoiceEngineMediaInterface* pMediaInterface) :
      mpMediaInterface(pMediaInterface),
          error_callback()
    {
    }
        
    virtual void error_handler(int errCode, int channel)
    {
        if (channel >= 0)
        {
            IMediaEventListener* pListener = mpMediaInterface->getMediaListener(channel) ;
            if (pListener)
            {
                // only firing with a valid connection and on certain error codes.
                CpMediaConnection* pConnection = mpMediaInterface->getMediaConnection(channel);
                if (pConnection && (10020 == errCode || 10019 == errCode))
                {                    
                    pListener->onDeviceError(IDevice_Audio, IError_DeviceUnplugged);
                }
            }
        }
        return;
    }
private:
    VoiceEngineMediaInterface* mpMediaInterface;
};


/* //////////////////////////// PUBLIC //////////////////////////////////// */


/* ============================ CREATORS ================================== */

// Constructor
VoiceEngineMediaInterface::VoiceEngineMediaInterface(IMediaDeviceMgr* pFactoryImpl,
                                                     GipsVoiceEngineLib* pVoiceEngine,
                                                     GipsVideoEnginePlatform* pVideoEngine,
                                                     const char* publicAddress,
                                                     const char* localAddress,
                                                     int numCodecs,
                                                     SdpCodec* sdpCodecArray[],
                                                     const char* locale,
                                                     int expeditedIpTos,
                                                     const ProxyDescriptor& stunServer,
                                                     const ProxyDescriptor& turnProxy,
                                                     const ProxyDescriptor& arsProxy,
                                                     const ProxyDescriptor& arsHttpProxy,
                                                     UtlBoolean bDTMFOutOfBand,
  	                                                 UtlBoolean bDTMFInBand,
  	                                                 UtlBoolean bEnableRTCP,
                                                     const char* szRtcpName,
                                                     SIPX_MEDIA_PACKET_CALLBACK pMediaPacketCallback) 
    :
    CpMediaInterface(pFactoryImpl,
                     publicAddress,
                     localAddress,
                     numCodecs,
                     sdpCodecArray,
                     locale,
                     expeditedIpTos,
                     stunServer,
                     turnProxy,
                     arsProxy,
                     arsHttpProxy,
                     bDTMFOutOfBand,
                     bDTMFInBand,
                     bEnableRTCP,
                     szRtcpName,
                     pMediaPacketCallback),
                     mbCameraUnplugged(false),
                     mpDisplay(NULL)
{
#if !defined (USE_GIPS)
    if (pVoiceEngine)
    {
        pVoiceEngine->setMediaPacketCallback(pMediaPacketCallback);
    }
    if (pVideoEngine)
    {
        pVideoEngine->setMediaPacketCallback(pMediaPacketCallback);
    }
#endif
    OS_PERF_FUNC("VoiceEngineMediaInterface::VoiceEngineMediaInterface") ;
    mpVEFactoryImpl = (VoiceEngineFactoryImpl*)pFactoryImpl ;

    mpVEFactoryImpl->getConsoleTraceOverride(mbConsoleTrace) ;

    mpMediaGuard = mpVEFactoryImpl->getLock() ;
    assert(mpMediaGuard != NULL) ;

    int rejected ;

    // Ignore passed in codecs and rebuild codec-factory based on current prefs

    UtlString audioCodecs ;
    UtlString videoCodecs ;
    mSupportedCodecs.getCodecList("audio", audioCodecs) ;
    mSupportedCodecs.getCodecList("video", videoCodecs) ;
    mSupportedCodecs.clearCodecs() ;
    mInitialCodecs.clearCodecs() ;
    mpVEFactoryImpl->buildCodecFactory(&mSupportedCodecs, audioCodecs, videoCodecs, -1, &rejected) ;
    mpVEFactoryImpl->buildCodecFactory(&mInitialCodecs, audioCodecs, videoCodecs, -1, &rejected) ;
 
    memset(&mSrtpParams, 0, sizeof(mSrtpParams));
    mpVoiceErrorCallback = NULL ;
    mpVoiceEngine = pVoiceEngine ;
    mbFocus = FALSE ;
    mpGipsVideoEngine = pVideoEngine ;
    mpVideoEngine = new VideoEngine(pVideoEngine, mbConsoleTrace) ;
#if !defined (USE_GIPS)
    if (mpVideoEngine->getVideoEngine())
    {
        mpVideoEngine->getVideoEngine()->setMediaPacketCallback(pMediaPacketCallback);
    }
#endif
      
    if (mpVoiceEngine)
    {
        mpVoiceErrorCallback = new gipsve_error_callback(this);
        mpVoiceEngine->SetObserver(*mpVoiceErrorCallback);
    }

    if (mpVideoEngine)
    {
        // init camera caps override
        bool bIgnoreCameraCaps = false ;
        mpVEFactoryImpl->applyIgnoreCameraCapsOverride(bIgnoreCameraCaps) ;
        mpVideoEngine->setIgnoreCameraCapabilties(bIgnoreCameraCaps) ;
    }
        
    mStunServer = stunServer ;
    mStunOptions = 0 ;
    mTurnProxy = turnProxy ;
    mArsProxy = arsProxy ;
    mArsHttpProxy = arsHttpProxy ;
    
    mbDTMFOutOfBand = bDTMFOutOfBand;
    mbDTMFInBand = bDTMFInBand;
    mbEnableRTCP = bEnableRTCP ;
    mRtcpName = szRtcpName ;
    mbLocalMute = FALSE ;

    if(localAddress && *localAddress)
    {
        mRtpReceiveHostAddress = localAddress;
        mLocalAddress = localAddress;
    }
    else
    {
        OsSocket::getHostIp(&mLocalAddress);
    }

    if(sdpCodecArray && numCodecs > 0)
    {
        // Assign any unset payload types
        mSupportedCodecs.bindPayloadTypes();
    }
    for (int i=0; i < MAX_VIDEO_CONNECTIONS; i++)
    {
        mVideoQuadrants[i] = -1;
    }    
}

//////////////////////////////////////////////////////////////////////////////

// Destructor
VoiceEngineMediaInterface::~VoiceEngineMediaInterface()
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::~VoiceEngineMediaInterface") ;
    OsLock lock(*mpMediaGuard) ;

    mpVoiceEngine->SetObserver(*mpVoiceErrorCallback, true) ;

    stopAudio() ;
    CpMediaConnection* pMediaConnection = NULL;    
    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (CpMediaConnection*) iterator()))
    {
        pMediaConnection->stopReadNotifyAdapters() ;
        deleteConnection(pMediaConnection->getValue()) ;
    }

    if (mpVideoEngine && mpVideoEngine->getVideoEngine())
    {
        mpVideoEngine->destroy() ;
    }

    // remove this pointer from the factory implementation
    if (mpVEFactoryImpl)
    {
        mpVEFactoryImpl->releaseInterface(this);
    }
    
    delete mpVoiceErrorCallback;
    mpVoiceErrorCallback = NULL;

    if (mpDisplay)
        delete mpDisplay ;

    // delete mpVideoEngine last -- it holds the reference for the preview window
    if (mpVideoEngine)
    {
        delete mpVideoEngine ;
        mpVideoEngine = NULL ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::release()
{      
    OS_PERF_FUNC("VoiceEngineMediaInterface::release") ;
    delete this;
}


/* ============================ MANIPULATORS ============================== */
OsStatus VoiceEngineMediaInterface::createConnection(int& connectionId,
                                                     bool bInitiating,
                                                     const char* szLocalAddress,
                                                     int localPort,
                                                     void* videoDisplay,
                                                     unsigned long flags,
                                                     void* const pSecurityAttributes,
                                                     ISocketEvent* pSocketIdleSink,
                                                     IMediaEventListener*  pMediaEventListener,
                                                     const RtpTransportOptions rtpTransportOptions,
                                                     int callHandle)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::createConnection") ;
    OsLock lock(*mpMediaGuard) ;
    int rc;
    CpMediaConnection* pMediaConnection = NULL;
    IOsNatSocket* rtpAudioSocketArray[2] = { NULL, NULL};
    IOsNatSocket* rtcpAudioSocketArray[2] = { NULL, NULL};
    OsStatus sr = OS_FAILED;

    traceAPI("VoiceEngineMediaInterface::createConnection") ;
    mpSocketIdleSink = pSocketIdleSink;

    trace(PRI_INFO, "createConnection localAddr=%s, video=%p local=%d stun=%d turn=%d, ars=%d ice=%d rtpTO=%d",
            szLocalAddress, 
            videoDisplay, 
            ((flags & CPMI_FLAGS_ENABLE_LOCAL) == CPMI_FLAGS_ENABLE_LOCAL),
            ((flags & CPMI_FLAGS_ENABLE_STUN) == CPMI_FLAGS_ENABLE_STUN),
            ((flags & CPMI_FLAGS_ENABLE_TURN) == CPMI_FLAGS_ENABLE_TURN),
            ((flags & CPMI_FLAGS_ENABLE_ARS) == CPMI_FLAGS_ENABLE_ARS),
            ((flags & CPMI_FLAGS_ENABLE_ICE) == CPMI_FLAGS_ENABLE_ICE),
            rtpTransportOptions) ;
    
    if (mpVoiceEngine)
    {
        if (pSecurityAttributes)
        {
            mpSecurityAttributes = (SIPXTACK_SECURITY_ATTRIBUTES*)pSecurityAttributes;
        }


        pMediaConnection = new CpMediaConnection() ;
        pMediaConnection->mpMediaEventListener = pMediaEventListener;

        pMediaConnection->mbEnableLocal = ((flags & CPMI_FLAGS_ENABLE_LOCAL) == CPMI_FLAGS_ENABLE_LOCAL) ;
        pMediaConnection->mbEnableSTUN = ((flags & CPMI_FLAGS_ENABLE_STUN) == CPMI_FLAGS_ENABLE_STUN) ;
        pMediaConnection->mbEnableTURN = ((flags & CPMI_FLAGS_ENABLE_TURN) == CPMI_FLAGS_ENABLE_TURN) ;
        pMediaConnection->mbEnableARS = ((flags & CPMI_FLAGS_ENABLE_ARS) == CPMI_FLAGS_ENABLE_ARS) ;
        pMediaConnection->mbEnableICE = ((flags & CPMI_FLAGS_ENABLE_ICE) == CPMI_FLAGS_ENABLE_ICE) ;

        pMediaConnection->mRtpTransport = (RTP_TRANSPORT_UDP | RTP_TRANSPORT_TCP) & rtpTransportOptions;
        RtpTcpRoles role = rtpTransportOptions & (RTP_TCP_ROLE_ACTIVE | RTP_TCP_ROLE_PASSIVE | RTP_TCP_ROLE_ACTPASS);
        pMediaConnection->setTcpRole(role);

        if (szLocalAddress && strlen(szLocalAddress))
            pMediaConnection->mLocalAddress = szLocalAddress ;
        else
            pMediaConnection->mLocalAddress = mLocalAddress ;

        pMediaConnection->mbInitiating = bInitiating ;

        startAudioSocketSupport(pMediaConnection) ;
        mpVEFactoryImpl->getVideoBitRate(pMediaConnection->mConnectionBitrate);
        mpVEFactoryImpl->getVideoFrameRate(pMediaConnection->mConnectionFramerate);
        if (videoDisplay && isDisplayValid((SIPXVE_VIDEO_DISPLAY*) videoDisplay))
        {
            startVideoSocketSupport(pMediaConnection) ;
            pMediaConnection->mbVideoStarted = true;
        }

        /*
         * Create Audio Channel
         */     
#ifdef USE_GIPS
        connectionId = mpVoiceEngine->GIPSVE_CreateChannel() ;        
#else
        connectionId = mpVoiceEngine->GIPSVE_CreateChannel(callHandle) ;        
#endif

        checkVoiceEngineReturnCode("GIPSVE_CreateChannel", connectionId, !(connectionId >= 0), true) ;     
        pMediaConnection->setValue(connectionId);

        // If the connection Id is already used -- this is an error
        assert(mMediaConnections.find(&UtlInt(connectionId)) == NULL) ;
        mMediaConnections.insert(pMediaConnection) ;

        int idleTimeout;
        mpVEFactoryImpl->getConnectionIdleTimeout(idleTimeout);
        CpMediaSocketMonitorTask::getInstance()->setIdleTimeout(idleTimeout);

        
#ifdef ENABLE_GIPS_VQMON   
        rc = mpVoiceEngine->GIPSVE_EnableVQMon(connectionId, true);
        checkVoiceEngineReturnCode("GIPSVE_EnableVQMon", connectionId, rc, true) ;
#endif

        for (int i = 0; i < 2; i++) // UDP and TCP
        {    
            if (pMediaConnection->mpRtpAudioSocketArray[i])
            {
                getMediaSocketPtr(pMediaConnection->mpRtpAudioSocketArray[i])->setAudioChannel(connectionId);
                GIPS_transport* pGipsTransport = NULL;
                pGipsTransport = dynamic_cast<GIPS_transport*>(pMediaConnection->mpAudioSocketAdapterArray[i]);
                rc = mpVoiceEngine->GIPSVE_SetSendTransport(connectionId, *pGipsTransport) ;
                checkVoiceEngineReturnCode("GIPSVE_SetSendTransport", connectionId, rc, true) ;
            }
            if (pMediaConnection->mpRtcpVideoSocketArray[i])
            {
                getMediaSocketPtr(pMediaConnection->mpRtcpVideoSocketArray[i])->setAudioChannel(connectionId);
            }
        }

        if (pMediaConnection->mpArsAudioSocket)
            pMediaConnection->mpArsAudioSocket->setAudioChannel(connectionId) ;

        if (pMediaConnection->mbVideoStarted) 
        {
            if (videoDisplay && VideoEngine::isDisplayValid((SIPXVE_VIDEO_DISPLAY*) videoDisplay))
            {
                setVideoWindowDisplay(connectionId, videoDisplay);
            }

            finalizeVideoSupport(connectionId) ;
        }

        pMediaConnection->mpCodecFactory = new SdpCodecList(mSupportedCodecs);
        pMediaConnection->mpCodecFactory->bindPayloadTypes();
                    
        if (mpVEFactoryImpl->isMuted())
        {
            muteMicrophone(true);
        }

        sr = OS_SUCCESS;
    }

    normalizeInternalPayloadTypes(connectionId, &mSupportedCodecs) ;

    return sr;
}


OsStatus VoiceEngineMediaInterface::setUserAgent(int         connectionId,
                                                 const char* szUserAgent) 
{
    OsStatus rc = CpMediaInterface::setUserAgent(connectionId, szUserAgent) ;
    if (rc == OS_SUCCESS && mpVideoEngine)
    {
        mpVideoEngine->setUserAgent(connectionId, szUserAgent) ;
    }

    return rc ;
}


//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::getCapabilities(int connectionId,
                                                    UtlSList* pAudioContacts,
                                                    UtlSList* pVideoContacts,                                                    
                                                    SdpCodecList& supportedCodecs,
                                                    SdpSrtpParameters& srtpParameters,
                                                    int bandWidth,
                                                    int& totalBandwidth,
                                                    int& videoFramerate)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::getCapabilities") ;
    OsLock lock(*mpMediaGuard) ;  
       
    traceAPI("VoiceEngineMediaInterface::getCapabilities") ;
   
    OsStatus rc = OS_FAILED ;
    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);

    if (pMediaConn)
    {
        if (pMediaConn->mbEnableTURN && mTurnProxy.isValid())
            doEnableTurn(pMediaConn, true) ;

        // Video Sockets are delayed until here to verify that we actually want video.
        if (isDisplayValid(mpDisplay) && pMediaConn->mbVideoStarted == false)
        {
            startVideoSocketSupport(pMediaConn) ;
            finalizeVideoSupport(connectionId) ;
            pMediaConn->mbVideoStarted = true;
        }
        // Codecs
        if (bandWidth != AUDIO_MICODEC_BW_DEFAULT)
        {
            setAudioCodecBandwidth(connectionId, bandWidth);
        }
        supportedCodecs = *(pMediaConn->mpCodecFactory);
        supportedCodecs.bindPayloadTypes();        

        // Filter out video codecs
        int i = 0 ;
        const SdpCodec* pBestVideoCodec = NULL;
        const SdpCodec* pCodec ;
        while ((pCodec = supportedCodecs.getCodecByIndex(i)) != NULL)
        {
            int codecWidth, codecHeight ;
            if (pCodec->getVideoResolution(codecWidth, codecHeight))
            {
                if (isDisplayValid(mpDisplay))
                {
                    int maxWidth, maxHeight ;
                    if (mpVideoEngine->getMaxResolution(maxWidth, maxHeight))
                    {
                        if (codecWidth > maxWidth || codecHeight > maxHeight)
                        {
                            // Unsupported resolution (more then camera can support)
                            // don't increment i
                            supportedCodecs.removeCodecByType(pCodec->getCodecType()) ;
                        }
                        else
                            i++ ;
                    }
                    else
                        i++ ;
                }
                else
                {
                    // if we don't have video, remove the video codec and 
                    // don't increment i
                    supportedCodecs.removeCodecByType(pCodec->getCodecType()) ;
                }
            }
            else
                i++ ;
        }        
        memset((void*)&srtpParameters, 0, sizeof(SdpSrtpParameters));
        if (mpSecurityAttributes)
        {
            srtpParameters.securityLevel = mpSecurityAttributes->getSecurityLevel() | SRTP_SEND | SRTP_RECEIVE;
            srtpParameters.cipherType = AES_CM_128_HMAC_SHA1_80;
            strncpy((char*)srtpParameters.masterKey, (char*)mpSecurityAttributes->getSrtpKey(), 31);
        }

        totalBandwidth = 0;
        if (mpVEFactoryImpl)
        {
            // Get our set video bandwidth and translate that into a 'conference total' bandwidth
            int tempBandwidth = pMediaConn->mConnectionBitrate;
            // Threshholds are 5/70/400 - translate anything <= 70 into a total bandwidth of 28
            totalBandwidth = (tempBandwidth < 70) ? 28 : 0;
            videoFramerate = pMediaConn->mConnectionFramerate;

            // Cap framerate based on camera
            int maxFramerate ;
            if (mpVideoEngine && mpVideoEngine->getMaxFramerate(maxFramerate) && maxFramerate < videoFramerate)
            {
                videoFramerate = maxFramerate ;
            }
        }

        // Startup GIPS-side of video processing
        if (isDisplayValid(mpDisplay) && pMediaConn->mbVideoStarted == true)
        {
            if (!mpVideoEngine->isChannelCreated(connectionId)) 
            {
                GIPSVideo_CodecInst codec ;
                const SdpCodec* pSdpCodec = NULL ;
                if (getMaxVideoCodec(supportedCodecs, pSdpCodec) && pSdpCodec)
                {
                    getVideoEngineCodec(*pSdpCodec, codec) ;
                }
                else
                    assert(FALSE) ; // codec video codecs available??

                mpVideoEngine->createChannel(connectionId, &codec) ;
            }

            if (!mpVideoEngine->isCaptureDeviceSet())
                resetVideoCaptureDevice() ;
        }

        if (pAudioContacts && pVideoContacts)
        {
            bool bAddAudio = (mAudioMediaConnectivityInfo.getNumLocalCandidates() == 0) ;
            bool bAddVideo = (mVideoMediaConnectivityInfo.getNumLocalCandidates() == 0) ;

            switch (pMediaConn->mContactType)
            {
                case CONTACT_LOCAL:
                    addLocalContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addNatedContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addRelayContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addArsContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    break ;
                case CONTACT_RELAY:
                    addRelayContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addLocalContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addNatedContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addArsContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    break ;
                case CONTACT_ARS:
                    addArsContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addLocalContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addNatedContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addRelayContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    break ;
                default:
                    addNatedContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addLocalContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addRelayContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    addArsContacts(connectionId, *pAudioContacts, bAddAudio, *pVideoContacts, bAddVideo) ;
                    break ;
            }

            if ((pAudioContacts->entries() > 0) || (pVideoContacts->entries() > 0))
            {
                rc = OS_SUCCESS ;
            }
        }
        else
        {
            rc = OS_SUCCESS ;
        }
    }

    return rc ;
}


//////////////////////////////////////////////////////////////////////////////


OsStatus VoiceEngineMediaInterface::startRtpSend(int connectionId,
                                                 int numCodecs,
                                                 SdpCodec* sendCodecs[])
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::startRtpSend") ;
    OsLock lock(*mpMediaGuard) ;
    int i, rc;
    SdpCodec* primaryCodec = NULL;
    SdpCodec* primaryVideoCodec = NULL;
    SdpCodec* dtmfCodec = NULL;
    GIPSVE_CodecInst codecInfo;
    bool bError = false ;
    GIPSVideo_CodecInst vcodecInfo;
    memset((void*)&vcodecInfo, 0, sizeof(vcodecInfo));
   
    traceAPI("VoiceEngineMediaInterface::startRtpSend START") ;

    UtlString mimeType;
    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    for (i=0; i<numCodecs; i++) 
    {
        sendCodecs[i]->getMediaType(mimeType);
        if (SdpCodec::SDP_CODEC_TONES == sendCodecs[i]->getValue()) 
        {
            if (NULL == dtmfCodec) 
            {
                dtmfCodec = sendCodecs[i];
            }
        } 
        else if (NULL == primaryCodec && mimeType.compareTo("audio", UtlString::ignoreCase) == 0) 
        {
            primaryCodec = sendCodecs[i];
            pMediaConnection->mPrimaryCodecType = primaryCodec->getCodecType() ;
        }
        else if (NULL == primaryVideoCodec && mimeType.compareTo("video", UtlString::ignoreCase) == 0)
        {
            primaryVideoCodec = sendCodecs[i];
            pMediaConnection->mPrimaryVideoCodec = *primaryVideoCodec;
        }
    }

    if (pMediaConnection && !pMediaConnection->mRtpSendingAudio && !pMediaConnection->mRtpSendingVideo)
    {
        // If we haven't set a destination and we have set alternate destinations
        if (!pMediaConnection->mDestinationSet && pMediaConnection->mbAlternateDestinations)
        {
            bool bAudioFailure = false ;
            bool bVideoFailure = false ;
            if (!applyAlternateDestinations(connectionId, bAudioFailure, bVideoFailure))
            {
                IMediaEventListener* pListener = getMediaListener(connectionId) ;
                if (pListener)
                {
                    if (bAudioFailure)
                        pListener->onIceFailed(IDevice_Audio) ;

                    if (bVideoFailure)
                        pListener->onIceFailed(IDevice_Video) ;                    
                }
                // Allow app to tear down call
                // bError = true ;
            }
        }

        if (!bError)
        {
#ifdef SIPXTAPI_USE_GIPS_TRANSPORT
            rc = mpVoiceEngine->GIPSVE_SetSendPort(connectionId, pMediaConnection->mRtpAudioSendHostPort) ;
            checkVoiceEngineReturnCode("GIPSVE_SetSendPort", connectionId, rc, true) ;
            rc = mpVoiceEngine->GIPSVE_SetSendIP(connectionId, (char*) pMediaConnection->mRtpSendHostAddress.data()) ;
            checkVoiceEngineReturnCode("GIPSVE_SetSendIP", connectionId, rc, true) ;
#endif

            if (primaryCodec && getVoiceEngineCodec(*primaryCodec, codecInfo))
            {
                trace(PRI_DEBUG, 
                            "startRtpSend: using GIPS codec %s for id %d, payload %d", 
                            codecInfo.plname, primaryCodec->getCodecType(), 
                            primaryCodec->getCodecPayloadFormat());
                pMediaConnection->mRtpPayloadType = primaryCodec->getCodecPayloadFormat();

                // Forcing VoiceEngine to use our dynamically allocated payload types
                codecInfo.pltype = primaryCodec->getCodecPayloadFormat();            
                if (primaryCodec->getCodecType() == SdpCodec::SDP_CODEC_GIPS_ILBC)
                {
                    codecInfo.pacsize = (primaryCodec->getSampleRate()/1000)*(primaryCodec->getPacketLength()/1000);
                }

                rc = mpVoiceEngine->GIPSVE_SetSendCodec(connectionId, &codecInfo) ;
                checkVoiceEngineReturnCode("GIPSVE_SetSendCodec", connectionId, rc, true) ;

                if (dtmfCodec)
                {
                    rc = mpVoiceEngine->GIPSVE_SetDTMFPayloadType(connectionId, dtmfCodec->getCodecPayloadFormat());
                    checkVoiceEngineReturnCode("GIPSVE_SetDTMFPayloadType", connectionId, rc, true) ;
                    
                }
                pMediaConnection->mRtpSendingAudio = true ;
                pMediaConnection->mbAudioInitialized = true;

                doEnableMonitoring(true, true, false, pMediaConnection) ;
            }

            // TODO:: Look for Different Codec
            if (isVideoInitialized(connectionId) && primaryVideoCodec)
            {
                mPrimaryVideoCodec = primaryVideoCodec;
                getVideoEngineCodec(*mPrimaryVideoCodec, vcodecInfo) ;
                if (mpVideoEngine->isReceivePaused(connectionId))
                    mpVideoEngine->resumeReceive(connectionId, &vcodecInfo) ;
                else
                    mpVideoEngine->startRender(connectionId, &vcodecInfo) ;

                if (mpVideoEngine->isSendPaused(connectionId))
                    mpVideoEngine->resumeSend(connectionId) ;
                else
                    mpVideoEngine->startSend(connectionId,  &vcodecInfo) ;

                pMediaConnection->mRtpSendingVideo = true ;
                pMediaConnection->mRtpReceivingVideo = true  ;  // BOB 5/8/2008: This seems wrong.

                doEnableMonitoring(true, false, true, pMediaConnection) ;

                if (mpVideoEngine->getCaptureError())
                {
                    CpMediaConnection* pMediaConnection;
                    UtlSListIterator iterator(mMediaConnections);
                    while ((pMediaConnection = (CpMediaConnection*) iterator()))    
                    {
                        if (pMediaConnection->mpMediaEventListener)
                        {
                            pMediaConnection->mpMediaEventListener->onDeviceError(
                                IDevice_Video, IError_DeviceUnplugged);                
                        }
                    }
                }
            }
      
            if (mSrtpParams.securityLevel & SRTP_SEND)
            {
                mbIsEncrypted = true;
                rc = mpVoiceEngine->GIPSVE_StopSend(connectionId);
                checkVoiceEngineReturnCode("GIPSVE_StopSend", connectionId, rc, true) ;

                rc = mpVoiceEngine->GIPSVE_EnableSRTPSend(connectionId, 
                        mSrtpParams.cipherType, 30, AUTH_HMAC_SHA1, 16, 4, 
                        mSrtpParams.securityLevel&SRTP_SECURITY_MASK,
                        mSrtpParams.masterKey);
                checkVoiceEngineReturnCode("GIPSVE_EnableSRTPSend", connectionId, rc, true) ;
            }

            if (pMediaConnection->getHold())
            {
                rc = mpVoiceEngine->GIPSVE_PutOnHold(connectionId, false) ;
                pMediaConnection->setHold(false);
                checkVoiceEngineReturnCode("GIPSVE_PutOnHold (false)", connectionId, rc, true) ;
            }
            else
            {
                setVQMonAddresses(connectionId) ;
                if (hasAudioInputDevice())
                {
                    rc = mpVoiceEngine->GIPSVE_StartSend(connectionId) ;
                    checkVoiceEngineReturnCode("GIPSVE_StartSend", connectionId, rc, true) ;
                }
            }

            rc = mpVoiceEngine->GIPSVE_AddToConference(connectionId, true) ;
            checkVoiceEngineReturnCode("GIPSVE_AddToConference", connectionId, rc, true) ;

            if (pMediaConnection->mRtpSendingAudio)
                doEnableMonitoring(true, true, false, pMediaConnection) ;

            if (pMediaConnection->mRtpSendingVideo)
                doEnableMonitoring(true, false, true, pMediaConnection) ;
        }
    }

    traceAPI("VoiceEngineMediaInterface::startRtpSend END") ;

    return OS_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::startRtpReceive(int       connectionId,
                                                    int       numCodecs,
                                                    SdpCodec* receiveCodecs[])
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::startRtpReceive") ;
    OsLock lock(*mpMediaGuard) ;
    int rc;
    SdpCodec* primaryCodec = NULL;
    SdpCodec* primaryVideoCodec = NULL;
    SdpCodec* dtmfCodec = NULL;
    GIPSVE_CodecInst codecInfo;
    UtlString mimeType;
    UtlString videoCodecName;
    bool bHaveRestarted = false;
    int i ;

    traceAPI("VoiceEngineMediaInterface::startRtpReceive") ;

    if (mbEnableRTCP)
    {
        rc = mpVoiceEngine->GIPSVE_EnableRTCP(connectionId, TRUE) ;
        checkVoiceEngineReturnCode("GIPSVE_EnableRTCP", connectionId, rc, true) ;
        if (!mRtcpName.isNull())
        {   
            rc = mpVoiceEngine->GIPSVE_SetRTCPCNAME(connectionId, (char*) mRtcpName.data()) ;
            checkVoiceEngineReturnCode("GIPSVE_SetRTCPCNAME", connectionId, rc, true) ;
        }                        
#ifdef ENABLE_GIPS_VQMON
        rc = mpVoiceEngine->GIPSVE_EnableRTCP_XR(connectionId, true);
        checkVoiceEngineReturnCode("GIPSVE_EnableRTCP_XR", connectionId, rc, true) ;
#endif
    }
    else
    {
        rc = mpVoiceEngine->GIPSVE_EnableRTCP(connectionId, FALSE) ;
        checkVoiceEngineReturnCode("GIPSVE_EnableRTCP", connectionId, rc, true) ;
    }
    
    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;  
    if (pMediaConnection)
    {   
        if (!pMediaConnection->mRtpReceivingAudio)
        {
            normalizeInternalPayloadTypes(connectionId, numCodecs, receiveCodecs) ;
        }

        for (i=0; i<numCodecs; i++) 
        {
            receiveCodecs[i]->getMediaType(mimeType);

            if (SdpCodec::SDP_CODEC_TONES == receiveCodecs[i]->getValue()) 
            {
                if (NULL == dtmfCodec) 
                {
                    dtmfCodec = receiveCodecs[i];
                    rc = mpVoiceEngine->GIPSVE_SetDTMFPayloadType(connectionId,
                                                                  dtmfCodec->getCodecPayloadFormat());                    
                    checkVoiceEngineReturnCode("GIPSVE_SetDTMFPayloadType", connectionId, rc, true) ;
                }
            } 
            else if (NULL == primaryCodec && mimeType.compareTo("audio", UtlString::ignoreCase) == 0) 
            {
                primaryCodec = receiveCodecs[i];
                pMediaConnection->mPrimaryCodecType = primaryCodec->getCodecType() ;
                if (getVoiceEngineCodec(*primaryCodec, codecInfo))
                {
                    // Stop playout here just in case we had a previous giveFocus call. That
                    // will make the SetRecPayloadType call fail. Playout will be resumed
                    // further down.
                    rc = mpVoiceEngine->GIPSVE_StopPlayout(connectionId);
                    checkVoiceEngineReturnCode("GIPSVE_StopPlayout", connectionId, rc, true) ;

                    if (primaryCodec)
                    {
                        codecInfo.pltype = primaryCodec->getCodecPayloadFormat();                        
                    }

                    //rc = mpVoiceEngine->GIPSVE_SetRecPayloadType(connectionId, &codecInfo) ;
                    //checkVoiceEngineReturnCode("GIPSVE_SetRecPayloadType", connectionId, rc, true) ;
                }
                pMediaConnection->mRtpReceivingAudio = TRUE ;
            }
            else if (NULL == primaryVideoCodec && mimeType.compareTo("video", UtlString::ignoreCase) == 0) 
            {
                primaryVideoCodec = receiveCodecs[i];
                mPrimaryVideoCodec = primaryVideoCodec;
                pMediaConnection->mPrimaryVideoCodec = *mPrimaryVideoCodec;
                pMediaConnection->mbIsPrimaryVideoCodecSet  = true;
                mpFactoryImpl->getCodecNameByType(primaryVideoCodec->getCodecType(), videoCodecName);
                limitVideoCodecs(connectionId, videoCodecName);
                pMediaConnection->mRtpVideoPayloadType = primaryVideoCodec->getCodecPayloadFormat();  

                GIPSVideo_CodecInst vcodecInfo;
                memset(&vcodecInfo, 0, sizeof(vcodecInfo));
                UtlBoolean bFoundCodec = getVideoEngineCodec(*mPrimaryVideoCodec, vcodecInfo) ;
#if defined (USE_GIPS)
                assert(bFoundCodec == TRUE) ;
#endif

#if defined (USE_GIPS)
                if (VideoEngine::isDisplayValid(mpDisplay) &&
                    bFoundCodec)
#endif
                {
                    int maxWidth = -1 ;
                    int maxHeight = -1 ;
                    mPrimaryVideoCodec->getVideoResolution(maxWidth, maxHeight) ;

                    if ((mpVideoEngine->isChannelCreated(connectionId) == true) ||
                            mpVideoEngine->createChannel(connectionId, &vcodecInfo))
                    {
                        if (!mpVideoEngine->isPreviewing())
                        {
                            SIPXVE_VIDEO_DISPLAY* pLocalDisplay = NULL;
                            pLocalDisplay = (SIPXVE_VIDEO_DISPLAY*)
                                    mpVEFactoryImpl->getVideoPreviewDisplay();

                            if (VideoEngine::isDisplayValid(pLocalDisplay))
                                mpVideoEngine->setPreviewDisplay(*pLocalDisplay) ;

                            if (!pMediaConnection->mbVideoStarted)
                            {
                                startVideoSocketSupport(pMediaConnection) ;
                                finalizeVideoSupport(connectionId) ;
                                pMediaConnection->mbVideoStarted = true;
                            }

                            pMediaConnection->mbVideoInitialized = true ;

                            for (int indx = 0; indx < 2; indx++) // UDP and TCP
                            {
                                if (pMediaConnection->mpVideoSocketAdapterArray[indx])
                                {
                                    GIPS_transport* pGipsTransport = NULL;
                                    pGipsTransport = dynamic_cast<GIPS_transport*>(pMediaConnection->mpVideoSocketAdapterArray[indx]);

                                    mpVideoEngine->setTransport(connectionId, 
                                            *pGipsTransport) ;
                                }
                            }                        

                            mpVideoEngine->enableRTCP(connectionId, mbEnableRTCP, mRtcpName) ;
                            mpVideoEngine->startPreview(connectionId, &vcodecInfo) ;

                            // WARNING: This logic assumes that video 
                            // codecs follow audio codecs, the first video 
                            // codec of any subtype is the best, and 
                            // subtypes are grouped (e.g. VP71, VP71, VP71, 
                            // H263 H263)
                            UtlString lastSubMimeType ;
                            for (int j=i; j<numCodecs; j++) 
                            {
                                UtlString codecName ;
                                int w,h ;

                                if (receiveCodecs[j]->getVideoResolution(w, h)) // is video codec
                                {
                                    receiveCodecs[j]->getEncodingName(codecName) ;
                                    if (lastSubMimeType.compareTo(codecName, UtlString::ignoreCase) != 0)
                                    {
                                        lastSubMimeType = codecName ;
#if DELAY_SET_VP71_RECEIVE_CODEC
                                        //
                                        // Only add receive codecs if not VP71.  VideoEngine 1.5
                                        // Does not allow you to change resolutions on the fly
                                        //
                                        if (codecName.compareTo("VP71", UtlString::ignoreCase) != 0)
                                        {
#endif
                                            GIPSVideo_CodecInst recvCodec;
                                            memset(&recvCodec, 0, sizeof(recvCodec));
                                            if (getVideoEngineCodec(*receiveCodecs[j], recvCodec))
                                            {
                                                mpVideoEngine->addReceiveCodec(connectionId, &recvCodec) ;
                                            }
#if DELAY_SET_VP71_RECEIVE_CODEC
                                        }
                                        else
                                        {
                                            trace(PRI_DEBUG, "Not setting up VP71 for receive (VideoEngine 1.5 workaround)") ;
                                        }
#endif
                                    }
                                }
                            }
                            mpVideoEngine->startRender(connectionId, &vcodecInfo) ;
                        }
                    }
                }               
            }
        }

        /*
         * Optimization: Disable video support, if we didn't neg the first time.  
         * Once we disable it, we won't re-enable it.
         */             
        if (mPrimaryVideoCodec == NULL && 
                mpVideoEngine && 
                mpVideoEngine->getVideoEngine())
        {
            mpVideoEngine->releaseVideoEngine() ;
            mpVEFactoryImpl->releaseVideoEngineInstance(mpGipsVideoEngine) ;            
            mpGipsVideoEngine = NULL ;
        }

        if (mbFocus)
        {
#ifdef SIPXTAPI_USE_GIPS_TRANSPORT
            if (pMediaConnection)
            {
                rc = mpVoiceEngine->GIPSVE_SetRecPort(connectionId, pMediaConnection->mRtpAudioReceivePort + 10);
                checkVoiceEngineReturnCode("GIPSVE_SetRecPort", rc, true) ;
            }

            // This call only has meaning using internal transport
            rc = mpVoiceEngine->GIPSVE_StartListen(connectionId) ;
            checkVoiceEngineReturnCode("GIPSVE_StartListen", rc, true) ;
#endif
            if (mSrtpParams.securityLevel & SRTP_RECEIVE)
            {
                mbIsEncrypted = true;
                rc = mpVoiceEngine->GIPSVE_StopPlayout(connectionId);
                checkVoiceEngineReturnCode("GIPSVE_StopPlayout", connectionId, rc, true) ;

                rc  = mpVoiceEngine->GIPSVE_EnableSRTPReceive(connectionId, mSrtpParams.cipherType, 30, AUTH_HMAC_SHA1, 
                                                                     16, 4, mSrtpParams.securityLevel&SRTP_SECURITY_MASK,
                                                                     mSrtpParams.masterKey);
                checkVoiceEngineReturnCode("GIPSVE_EnableSRTPReceive", connectionId, rc, true) ;
            }
                
            if (hasAudioOutputDevice())
            {
                rc = mpVoiceEngine->GIPSVE_StartPlayout(connectionId) ;
                checkVoiceEngineReturnCode("GIPSVE_StartPlayout", connectionId, rc, true) ;
            }
        }
        pMediaConnection->setSocketsEnabled(true) ;
    }

    return OS_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::stopRtpSend(int connectionId)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::stopRtpSend") ;
    OsLock lock(*mpMediaGuard) ;
    int iRC ;  

    traceAPI("VoiceEngineMediaInterface::stopRtpSend") ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        pMediaConnection->mDestinationSet = false ;
        pMediaConnection->mbAlternateDestinations = false ;

        doEnableMonitoring(false, true, true, pMediaConnection) ;

        if (pMediaConnection->mRtpSendingAudio)
        {   
            pMediaConnection->mRtpSendingAudio = FALSE ;        

            iRC = mpVoiceEngine->GIPSVE_PutOnHold(connectionId, true) ;    
            checkVoiceEngineReturnCode("GIPSVE_PutOnHold (true)", connectionId, iRC, true) ;
            pMediaConnection->setHold(true);
            
            iRC = mpVoiceEngine->GIPSVE_AddToConference(connectionId, false) ;
            checkVoiceEngineReturnCode("GIPSVE_AddToConference", connectionId, iRC, true) ;
        }
    
        if (isVideoInitialized(connectionId))
        {
            if (pMediaConnection->mRtpSendingVideo)
            {   
                mpVideoEngine->pauseSend(connectionId) ;
                pMediaConnection->mRtpSendingVideo = FALSE ; 
            }        
        }
    }
    return OS_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::stopRtpReceive(int connectionId)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::stopRtpReceive") ;
    OsLock lock(*mpMediaGuard) ;
    int iRC ;

    traceAPI("VoiceEngineMediaInterface::stopRtpReceive") ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {           
        doEnableTurn(pMediaConnection, false) ;

        // Disable processing of media sockets
        pMediaConnection->setSocketsEnabled(false) ;

        if (pMediaConnection->mRtpReceivingAudio)
        {
            pMediaConnection->mRtpReceivingAudio = FALSE ;
            pMediaConnection->stopReadNotifyAdapters() ;
                       
            iRC = mpVoiceEngine->GIPSVE_StopPlayout(connectionId) ;    
            checkVoiceEngineReturnCode("GIPSVE_StopPlayout", connectionId, iRC, true) ;
            
            if (mbIsEncrypted)
            {
                iRC  = mpVoiceEngine->GIPSVE_DisableSRTPSend(connectionId);
                checkVoiceEngineReturnCode("GIPSVE_DisableSRTPSend", connectionId, iRC, true) ;
                
                iRC = mpVoiceEngine->GIPSVE_DisableSRTPReceive(connectionId);
                checkVoiceEngineReturnCode("GIPSVE_DisableSRTPReceive", connectionId, iRC, true) ;
            } 
        }
        if (pMediaConnection && pMediaConnection->mRtpReceivingVideo)
        {   
            mpVideoEngine->pauseReceive(connectionId) ;
            pMediaConnection->mRtpReceivingVideo = FALSE ; 

/*
            if (mbIsEncrypted)
            {
                iRC = mpVoiceEngine->GIPSVE_DisableSRTPReceive(connectionId);
                checkVoiceEngineReturnCode("GIPSVE_DisableSRTPReceive", connectionId, iRC, true) ;
            } 
*/
        }
    }

    return OS_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::deleteConnection(int connectionId)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::deleteConnection") ;

    OsStatus rc = OS_NOT_FOUND ;
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    CpMediaNetTask* pNetTask = NULL;    
    CpMediaSocketMonitorTask* pMonTask = NULL ;
    OsTime maxEventTime(20, 0);

    traceAPI("VoiceEngineMediaInterface::deleteConnection") ;

    mpMediaGuard->acquire() ;


    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {       
        /*
        * FIRST: Stop receiving/sending all data
        */
        stopChannelAudio(connectionId) ;
        if (pMediaConnection->mRtpSendingAudio || pMediaConnection->mRtpSendingVideo)
            stopRtpSend(connectionId) ;    
        if (pMediaConnection->mRtpReceivingAudio || pMediaConnection->mRtpReceivingVideo)
            stopRtpReceive(connectionId) ;        

        /*
         * Next, unmonitor the connections
         */
        for (int i = 0; i < 2; i++) // UDP and TCP
        {
            // Audio RTP
            if (pMediaConnection->mpRtpAudioSocketArray[i])
                doUnmonitor(pMediaConnection->mpRtpAudioSocketArray[i]->getSocket());
            // Video RTP
            if (pMediaConnection->mpRtpVideoSocketArray[i])
                doUnmonitor(pMediaConnection->mpRtpVideoSocketArray[i]->getSocket());
        }

        if (pMediaConnection->mpArsAudioSocket)
            doUnmonitor(pMediaConnection->mpArsAudioSocket->getSocket()) ;
        if (pMediaConnection->mpArsVideoSocket)
            doUnmonitor(pMediaConnection->mpArsAudioSocket->getSocket()) ;
    
        /*
         * Next, delete the GIPS channels
         */
        if (mpVideoEngine && isVideoInitialized(connectionId))
            mpVideoEngine->destroyChannel(connectionId) ;            
        int iRC = mpVoiceEngine->GIPSVE_DeleteChannel(connectionId) ;
        checkVoiceEngineReturnCode("GIPSVE_DeleteChannel", connectionId, iRC, true) ;

        // stop the read notify adapters
        // MUST be done BEFORE sockets are deleted,
        // because, as part of the stop,
        // the notify adapter's
        // socket's notifier is set to null.
        // That cannot be done if the notifier's
        // socket has been deleted out from under it.
        pMediaConnection->stopReadNotifyAdapters();

        /*
         * Next, free sockets
         */        
        for (int i = 0; i < 2; i++)
        {
            // Audio RTP
            if (pMediaConnection->mpRtpAudioSocketArray[i])
                mpVEFactoryImpl->getSocketFactory()->releaseUdpSocket(pMediaConnection->mpRtpAudioSocketArray[i]) ;            
            // Audio RTCP
            if (pMediaConnection->mpRtcpAudioSocketArray[i])
                mpVEFactoryImpl->getSocketFactory()->releaseUdpSocket(pMediaConnection->mpRtcpAudioSocketArray[i]) ;
            // Video RTP
            if (pMediaConnection->mpRtpVideoSocketArray[i])
                mpVEFactoryImpl->getSocketFactory()->releaseUdpSocket(pMediaConnection->mpRtpVideoSocketArray[i]) ;
            // Video RTCP
            if (pMediaConnection->mpRtcpVideoSocketArray[i])
                mpVEFactoryImpl->getSocketFactory()->releaseUdpSocket(pMediaConnection->mpRtcpVideoSocketArray[i]) ;
        }

        if (pMediaConnection->mpArsAudioSocket)
            mpVEFactoryImpl->getSocketFactory()->releaseArsSocket(pMediaConnection->mpArsAudioSocket) ;        
        if (pMediaConnection->mpArsVideoSocket)
            mpVEFactoryImpl->getSocketFactory()->releaseArsSocket(pMediaConnection->mpArsVideoSocket) ;

        /*
         * Finally, cleanup data structures
         */
        mMediaConnections.remove(&UtlInt(connectionId)) ;
        mpMediaGuard->release() ;               
        delete pMediaConnection ;
        rc = OS_SUCCESS ;
    }
    else
    {
        mpMediaGuard->release() ;
    }

#if defined(_DEBUG)
    trace(PRI_DEBUG, "VoiceEngineMediaInterface::deleteConnection completed") ;
#endif


    return rc ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::playAudio(const char* url,
                                              UtlBoolean repeat,
                                              UtlBoolean local,
                                              UtlBoolean remote,
                                              UtlBoolean mixWithMic,
                                              int downScaling,
                                              OsNotification *event)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::playAudio") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;
    bool bPlayedLocally = false ;
    CpMediaConnection* pMediaConnection = NULL;

    assert(url);

    // Stop audio if already playing
    stopAudio() ;
    
    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (CpMediaConnection*) iterator()))    
    {
        int connectionId = pMediaConnection->getValue();
        if (isSendingRtpAudio(connectionId))
        {
            rc = playChannelAudio(connectionId, url, repeat, (bPlayedLocally == false), remote, mixWithMic, downScaling) ;
            bPlayedLocally = true ;
        }
    }

    // If local is set then play it locally (if not already done)
    if (local && !bPlayedLocally)
    {
        if (mpVEFactoryImpl)
        {
            VoiceEngineBufferInStream* pInStream = new VoiceEngineBufferInStream(
                        url, repeat, this, NULL, IDevice_Audio) ;

            GIPS_FileFormats fileFormat = FILE_PCM_8KHZ ;

            // Determine if file is PCM or WAV 
            determineFileType(url, fileFormat);
            

            // The factory imply will manage the life cycle of the instream
            mpVEFactoryImpl->startPlayAudio(pInStream, fileFormat, downScaling) ;
        }
    }

    return rc;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::stopAudio()
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::stopAudio") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;
    CpMediaConnection* pMediaConnection = NULL;
    bool bStoppedLocally = false ;

    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (CpMediaConnection*) iterator()))    
    {
        int connectionId = pMediaConnection->getValue();
        stopChannelAudio(connectionId) ;
    }

    // Stop playing the global instance
    if (mpVEFactoryImpl)
    {
        mpVEFactoryImpl->stopPlay(this) ;
    }

    return rc;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::playChannelAudio(int connectionId,
                                                     const char* url,
                                                     UtlBoolean repeat,
                                                     UtlBoolean local,
                                                     UtlBoolean remote,
                                                     UtlBoolean mixWithMic,
                                                     int downScaling,
                                                     OsNotification *event)

{
    OS_PERF_FUNC("VoiceEngineMediaInterface::playChannelAudio") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;
    int iRC;
    CpMediaConnection* pMediaConnection = NULL;
    GIPS_FileFormats fileFormat;
    bool bPlayedLocally = false ;

    // Stop audio if already playing
    stopChannelAudio(connectionId) ;
    assert(url);

    if (determineFileType(url, fileFormat))
    {    
        CpMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
        IMediaEventListener* pListener = pMediaConnection ? pMediaConnection->mpMediaEventListener :
                                                            NULL;
        if (pListener && pMediaConnection && isSendingRtpAudio(connectionId))
        {            
            int connectionId = pMediaConnection->getValue();

            if (remote)
            {
                pMediaConnection->mpInStreamRemote = new VoiceEngineBufferInStream(
                        url, repeat, NULL, pListener, IDevice_Audio) ;
                // Do not pass listener to both local and remote (dup events) 
                pListener = NULL ;

                // If remote is set then play file as if it came from microphone
                iRC = mpVoiceEngine->GIPSVE_PlayPCMAsMicrophone(connectionId, 
                        (InStream*)pMediaConnection->mpInStreamRemote, mixWithMic, 
                        fileFormat, ((float) downScaling) / 100) ;
                checkVoiceEngineReturnCode("GIPSVE_PlayPCMAsMicrophone", connectionId, iRC, true) ;
                if (iRC == 0)
                {
                    rc = OS_SUCCESS;
                }
                else
                {
                    if (pMediaConnection->mpInStreamRemote)
                    {
                        pMediaConnection->mpInStreamRemote->close() ;
                    }
                    delete pMediaConnection->mpInStreamRemote ;
                    pMediaConnection->mpInStreamRemote = NULL ;                
                }
            }

            // Play Locally
            if (local)
            {
                pMediaConnection->mpInStreamLocal = new VoiceEngineBufferInStream(
                        url, repeat, pMediaConnection, pListener, IDevice_Audio) ;

                // Do not pass listener to both local and remote (dup events) 
                pListener = NULL ;

                bPlayedLocally = true ;

                iRC = mpVoiceEngine->GIPSVE_PlayPCM(connectionId, 
                        (InStream*)pMediaConnection->mpInStreamLocal, fileFormat, 
                        ((float) downScaling) / 100) ;
                checkVoiceEngineReturnCode("GIPSVE_PlayPCM", connectionId, iRC, true) ;
                if (iRC == 0)
                {
                    // Only set if not set by remote play
                    if (!remote)
                    {
                        rc = OS_SUCCESS;
                    }
                }
                else
                {
                    pMediaConnection->mpInStreamLocal->close() ;
                    delete pMediaConnection->mpInStreamLocal ;
                    pMediaConnection->mpInStreamLocal = NULL ;                
                }
            }
        }    

        if (local && !bPlayedLocally)
        {
            if (mpVEFactoryImpl)
            {
                VoiceEngineBufferInStream* pInStream = new VoiceEngineBufferInStream(
                            url, repeat, pMediaConnection, pListener, IDevice_Audio) ;

                GIPS_FileFormats fileFormat = FILE_PCM_8KHZ ;
                // Determine if file is PCM or WAV 
                determineFileType(url, fileFormat);

                // The factory imply will manage the life cycle of the instream
                mpVEFactoryImpl->startPlayAudio(pInStream, fileFormat, downScaling) ;
                if (!remote)
                {
                    rc = OS_SUCCESS;
                }
            }
        }
    }
    else
    {
        rc = OS_FAILED;
    }

    return rc;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::stopChannelAudio(int connectionId)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::stopChannelAudio") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;
    bool bStoppedLocally = false ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        int connectionId = pMediaConnection->getValue();

        // Stop playing remotely
        if (mpVoiceEngine->GIPSVE_IsPlayingFileAsMicrophone(connectionId) == 1)
        {
            if (mpVoiceEngine->GIPSVE_StopPlayingFileAsMicrophone(connectionId) == -1)
            {
                trace(PRI_ERR, 
                             "stopAudio: GIPSVE_StopPlayingFileAsMicrophone on channel %d failed with error %d",
                             connectionId,
                             mpVoiceEngine->GIPSVE_GetLastError());
                assert(0);
            }
            else
            {
                rc = OS_SUCCESS;
            }
        }

        // Stop playing Locally
        if (mpVoiceEngine->GIPSVE_IsPlayingFile(connectionId))
        {
            if (mpVoiceEngine->GIPSVE_StopPlayingFile(connectionId) != -1)
            {
                rc = OS_SUCCESS;
            }
        }

        // Stop playing the global instance
        if (mpVEFactoryImpl)
        {
            mpVEFactoryImpl->stopPlay(pMediaConnection) ;
        }

        if (pMediaConnection->mpInStreamLocal)
        {
            pMediaConnection->mpInStreamLocal->close() ;
            delete pMediaConnection->mpInStreamLocal ;
            pMediaConnection->mpInStreamLocal = NULL ;
        }

        if (pMediaConnection->mpInStreamRemote)
        {
            pMediaConnection->mpInStreamRemote->close() ;
            delete pMediaConnection->mpInStreamRemote ;
            pMediaConnection->mpInStreamRemote = NULL ;
        }
    }
    // Stop playing the global instance
    if (mpVEFactoryImpl)
    {
        rc = mpVEFactoryImpl->stopPlay() ;
    }

    return rc;
}


void VoiceEngineMediaInterface::injectMediaPacket(const int channelId,
                       const SIPX_MEDIA_PACKET_TYPE type,
                            const char* const pData,
                            const size_t len)
{
    CpMediaConnection* pMediaConnection = getMediaConnection(channelId);  

    if (pMediaConnection)
    {
        if (type == SIPX_MEDIA_PACKET_AUDIO_RTP)
        {
            for (int i = 0; i < 2; i++)
            {
                if (pMediaConnection->mpAudioSocketAdapterArray[i])
                {
                    pMediaConnection->mpAudioSocketAdapterArray[i]->doSendPacket(channelId, pData, len);
                }
            }
        }
        else if (type == SIPX_MEDIA_PACKET_AUDIO_RTCP)
        {
            for (int i = 0; i < 2; i++)
            {
                if (pMediaConnection->mpAudioSocketAdapterArray[i])
                {
                    pMediaConnection->mpAudioSocketAdapterArray[i]->doSendRTCPPacket(channelId, pData, len);
                }
            }
        }
        else if (type == SIPX_MEDIA_PACKET_VIDEO_RTP)
        {
            for (int i = 0; i < 2; i++)
            {
                if (pMediaConnection->mpVideoSocketAdapterArray[i])
                {
                    pMediaConnection->mpVideoSocketAdapterArray[i]->doSendPacket(channelId, pData, len);
                }
            }
        }
        else if (type == SIPX_MEDIA_PACKET_VIDEO_RTCP)
        {
            for (int i = 0; i < 2; i++)
            {
                if (pMediaConnection->mpVideoSocketAdapterArray[i])
                {
                    pMediaConnection->mpVideoSocketAdapterArray[i]->doSendRTCPPacket(channelId, pData, len);
                }
            }
        }
    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
OsStatus VoiceEngineMediaInterface::playBuffer(char* buf, 
                               unsigned long bufSize,
                               uint32_t bufRate, 
                               int type, 
                               UtlBoolean repeat,
                               UtlBoolean local, 
                               UtlBoolean remote,
                               OsProtectedEvent* pEvent,
                               UtlBoolean mixWithMic,
                               int downScaling)

{
    OS_PERF_FUNC("VoiceEngineMediaInterface::playBuffer") ;
    OsLock lock(*mpMediaGuard) ;
    assert(buf);
    int iRC;
    UtlBoolean bPlayedLocally = false ;    
    OsStatus rc = OS_FAILED;
    CpMediaConnection* pMediaConnection = NULL;
    GIPS_FileFormats fileFormat = FILE_PCM_8KHZ ;

    // Stop audio if already playing
    stopAudio() ;
    
    // Determine if file is PCM or WAV (NOTE: Looping doesn't work on WAV files)
    determineFileType(buf, bufSize, fileFormat);                
    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (CpMediaConnection*) iterator()))    
    {
        IMediaEventListener* pListener = pMediaConnection->mpMediaEventListener ;
        int connectionId = pMediaConnection->getValue();
        iRC = 0 ;

        if (isSendingRtpAudio(connectionId))
        {
            if (remote)
            {
                pMediaConnection->mpInStreamRemote = new VoiceEngineBufferInStream(buf, 
                        bufSize, repeat, NULL, pListener, IDevice_Audio) ;

                // Do not pass listener to both local and remote (dup events) 
                pListener = NULL ;

                // If remote is set then play file as if it came from microphone
                iRC = mpVoiceEngine->GIPSVE_PlayPCMAsMicrophone(connectionId, 
                        (InStream*)pMediaConnection->mpInStreamRemote, mixWithMic, fileFormat,
                        ((float) downScaling) / 100) ;
                checkVoiceEngineReturnCode("GIPSVE_PlayPCMAsMicrophone", connectionId, iRC, true) ;
                if (iRC == 0)
                {
                    rc = OS_SUCCESS;
                }
                else
                {
                    pMediaConnection->mpInStreamRemote->close() ;
                    delete pMediaConnection->mpInStreamRemote ;
                    pMediaConnection->mpInStreamRemote = NULL ;                
                }
            }

            // Play Locally
            if (local)
            {
                pMediaConnection->mpInStreamLocal = new VoiceEngineBufferInStream(buf, 
                        bufSize, repeat, this, pListener, IDevice_Audio) ;

                // Do not pass listener to both local and remote (dup events) 
                pListener = NULL ;

                bPlayedLocally = true ;
                iRC = mpVoiceEngine->GIPSVE_PlayPCM(connectionId, 
                        (InStream*)pMediaConnection->mpInStreamLocal, fileFormat, 
                        ((float) downScaling) / 100) ;
                checkVoiceEngineReturnCode("GIPSVE_PlayPCM", connectionId, iRC, true) ;
                if (iRC == 0)
                {
                    // Only set if not set by remote play
                    if (!remote)
                    {
                        rc = OS_SUCCESS;
                    }
                }
                else
                {
                    pMediaConnection->mpInStreamLocal->close() ;
                    delete pMediaConnection->mpInStreamLocal ;
                    pMediaConnection->mpInStreamLocal = NULL ;                
                }
            }
        }    
    }

    // If local is set then play it locally (if not already done)
    if (local && !bPlayedLocally)
    {
        if (mpVEFactoryImpl)
        {
            VoiceEngineBufferInStream* pInStream = new VoiceEngineBufferInStream(
                        buf, bufSize, repeat, this, NULL, IDevice_Audio) ;

            // The factory imply will manage the life cycle of the instream
            mpVEFactoryImpl->startPlayAudio(pInStream, fileFormat, downScaling) ;
        }
    }

    if (pEvent)
    {
        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        if(OS_ALREADY_SIGNALED == pEvent->signal(true))
        {
            eventMgr->release((OsProtectedEvent*) pEvent);
        }
    }

    return rc;    
}


//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::pauseAudio()
{
    return OS_NOT_SUPPORTED ;
}


//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::startTone(int toneId,
                                              UtlBoolean local,
                                              UtlBoolean remote)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::startTone") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;
    CpMediaConnection* pMediaConnection = NULL ;
    int err = 0;

    // Play locally
    pMediaConnection = (CpMediaConnection *)mMediaConnections.first();
    if (local && !remote)
    {
        if (pMediaConnection)
        {
            int iConnectionID = pMediaConnection->getValue() ;
            mpVoiceEngine->GIPSVE_PlayDTMFTone(toneId) ;            
        } 
        else if (mpVEFactoryImpl)
        {
            mpVEFactoryImpl->playTone(toneId) ;
        }
    }


    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (CpMediaConnection*) iterator()))    
    {
        if (remote)
        {
            int connectionId = pMediaConnection->getValue();

            // Try out-of-band first if enabled
            if (mbDTMFOutOfBand)
            {
                if (mpVoiceEngine->GIPSVE_SendDTMF(connectionId, toneId, 0) == 0)
                {
                    rc = OS_SUCCESS;
                }
                else
                {
                    err = mpVoiceEngine->GIPSVE_GetLastError();
                    trace(PRI_ERR, 
                              "startTone: out-of-band SendDTMF with event nr %d returned error %d", 
                              toneId, err);
                    if (err != 0)
                    {
                        assert(0);
                    }
                }
            }

            // Then send inband DTMF if enabled
            if (mbDTMFInBand)
            {
                if (mpVoiceEngine->GIPSVE_SendDTMF(connectionId, toneId, 1) == 0)
                {
                    rc = OS_SUCCESS;
                }
                else
                {
                    // Don't log error for inband flash hook
                    if (toneId != 16)
                    {
                        err = mpVoiceEngine->GIPSVE_GetLastError();
                        trace(PRI_ERR, 
                                "startTone: inband SendDTMF with event nr %d returned error %d", 
                                toneId, err);
                        if (err != 0)
                        {
                            assert(0);
                        }
                        rc = OS_FAILED;
                    }
                }
            }
        }
    }
    
    return rc;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::stopTone()
{
    return OS_NOT_SUPPORTED ;

}

//////////////////////////////////////////////////////////////////////////////


OsStatus VoiceEngineMediaInterface::startChannelTone(int connectionId,
                                                     int toneId,
                                                     UtlBoolean local,
                                                     UtlBoolean remote)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::startChannelTone") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;
    int err = 0;

    // Play locally
    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (local && !remote)
    {
        if (pMediaConnection)
        {
            int iConnectionID = pMediaConnection->getValue() ;
            mpVoiceEngine->GIPSVE_PlayDTMFTone(toneId) ;    
        
        } 
        else if (mpVEFactoryImpl)
        {
            mpVEFactoryImpl->playTone(toneId) ;
        }
    }


    if (pMediaConnection)
    {
        if (remote)
        {
            int connectionId = pMediaConnection->getValue();

            // Try out-of-band first if enabled
            if (mbDTMFOutOfBand)
            {
                if (mpVoiceEngine->GIPSVE_SendDTMF(connectionId, toneId, 0) == 0)
                {
                    rc = OS_SUCCESS;
                }
                else
                {
                    err = mpVoiceEngine->GIPSVE_GetLastError();
                    trace(PRI_ERR, 
                              "startChannelTone: out-of-band SendDTMF with event nr %d returned error %d", 
                              toneId, err);
                    if (err != 0)
                    {
                        assert(0);
                    }
                }
            }

            // Then send inband DTMF
            if (mpVoiceEngine->GIPSVE_SendDTMF(connectionId, toneId, 1) == 0)
            {
                rc = OS_SUCCESS;
            }
            else
            {
                // Don't log error for inband flash hook
                if (toneId != 16)
                {
                    err = mpVoiceEngine->GIPSVE_GetLastError();
                    trace(PRI_ERR, 
                              "startChannelTone: inband SendDTMF with event nr %d returned error %d", 
                              toneId, err);
                    if (err != 0)
                    {
                        assert(0);
                    }
                    rc = OS_FAILED;
                }
            }
        }
    }
    
    return rc;
}


//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::recordChannelAudio(int connectionId,
                                                       const char* szFile) 
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::recordChannelAudio") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus status = OS_FAILED ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        GIPSVE_CodecInst codecInst ;
        memset(&codecInst, 0, sizeof(codecInst)) ;
        strcpy(codecInst.plname, "L16") ;
        codecInst.plfreq = 16000 ;

        if (mpVoiceEngine->GIPSVE_StartRecordingCall((char*) szFile, &codecInst) == 0)
        {
            status = OS_SUCCESS ;
        }
    }

    return status ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::stopRecordChannelAudio(int connectionId) 
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::stopRecordChannelAudio") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus status = OS_FAILED ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        if (mpVoiceEngine->GIPSVE_StopRecordingCall())
        {
            status = OS_SUCCESS ;
        }
    }

    return status ;   
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::stopChannelTone(int connectionId)
{
    return OS_NOT_SUPPORTED ;
}

//////////////////////////////////////////////////////////////////////////////


OsStatus VoiceEngineMediaInterface::giveFocus()
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::giveFocus") ;
    OsLock lock(*mpMediaGuard) ;
    mbFocus = TRUE ;
    OsStatus rc = OS_FAILED;
    int iRC ;
    CpMediaConnection* pMediaConnection = NULL ;

    traceAPI("VoiceEngineMediaInterface::giveFocus") ;
     
    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (CpMediaConnection*) iterator()))    
    {        
        if (pMediaConnection->mRtpReceivingAudio || pMediaConnection->mRtpSendingAudio)
        {
            if (hasAudioOutputDevice())
            {
                iRC = mpVoiceEngine->GIPSVE_StartPlayout(pMediaConnection->getValue()) ;
                checkVoiceEngineReturnCode("GIPSVE_StartPlayout", pMediaConnection->getValue(), iRC, true) ;
                if (iRC == 0)
                {
                    rc = OS_SUCCESS;
                }
            }
        }
    }

    if (mbLocalMute)
    {
        mbLocalMute = false ;
        if (!mpVEFactoryImpl->isMuted())
        {
            muteMicrophone(false);
        }
    }


    return rc;
}

//////////////////////////////////////////////////////////////////////////////


OsStatus VoiceEngineMediaInterface::defocus()
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::defocus") ;
    OsLock lock(*mpMediaGuard) ;
    int rc;
    mbFocus = FALSE ;
    CpMediaConnection* pMediaConnection = NULL ;        

    traceAPI("VoiceEngineMediaInterface::defocus") ;
    
    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (CpMediaConnection*) iterator()))
    {
        rc = mpVoiceEngine->GIPSVE_StopPlayout(pMediaConnection->getValue()) ;        
        checkVoiceEngineReturnCode("GIPSVE_StopPlayout", pMediaConnection->getValue(), rc, true) ;
    }

    mbLocalMute = true ;
    muteMicrophone(true);
    
    return OS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////

// Limits the available codecs to only those within the designated limit.
void VoiceEngineMediaInterface::setCodecCPULimit(int iLimit)
{

}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::stopRecording()
{
    return OS_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::ezRecord(int ms, 
                                         int silenceLength, 
                                         const char* fileName, 
                                         double& duration,
                                         int& dtmfterm,
                                         OsProtectedEvent* ev)
{
    return OS_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::addToneListener(OsNotification *pListener, int connectionId)
{
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::removeToneListener(int connectionId)
{
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::setSecurityAttributes(const void* pSecurity)
{
    if (pSecurity)
    {
        mpSecurityAttributes = (SIPXTACK_SECURITY_ATTRIBUTES*)pSecurity;
    }

    return OS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::setRemoteVolumeScale(const int connectionId,
                                                         const int scale) 
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::setRemoteVolumeScale") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED ;

    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        float fScale = (float) scale / 100.0f ;
        int iRC = mpVoiceEngine->GIPSVE_SetChannelOutputVolumeScale(connectionId, fScale) ;
        if (iRC == 0)
        {
            rc = OS_SUCCESS ;
        }
        else
        {
            checkVoiceEngineReturnCode("GIPSVE_SetchannelOutputVolumeScale", connectionId, iRC, true) ;
        }
    }

    return rc ;
}

//////////////////////////////////////////////////////////////////////////////

bool VoiceEngineMediaInterface::resetVideoCaptureDevice() 
{ 
    bool bSuccess = false ;
    if (mpVideoEngine)
    {
        UtlString captureDevice ;
        if (mpVEFactoryImpl->getVideoCaptureDevice(captureDevice) == OS_SUCCESS)
        {
            bSuccess = mpVideoEngine->setCaptureDevice(
                    mpVEFactoryImpl,
                    captureDevice) ;
        }
        else
        {
            bSuccess = mpVideoEngine->setCaptureDevice(
                    mpVEFactoryImpl,
                    NULL) ;
        }
    }
    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////


/* ============================ ACCESSORS ================================= */

// Returns the flowgraph's message queue
OsMsgQ* VoiceEngineMediaInterface::getMsgQ()
{
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////

int adjustInputLevel(int inputLevel)
{
    static double rollingAverage = 0 ;

    // Input Levels range between 0 and 9; however some newer cameras are
    // sending levels of 1 even when mostly silent.  This are generally 
    // microphones attached to WebCams such as the MS VX-6000.

    // For now, we are suppressing 1s down to 0 when outside obvious 
    // talk spurts.


    rollingAverage += ((1.0/3.0) * (inputLevel - rollingAverage)) ;

    if (rollingAverage < 1.1)
        inputLevel = 0 ; 

    return inputLevel ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::getAudioEnergyLevels(int& iInputEnergyLevel,
                                                         int& iOutputEnergyLevel) 
{
    OsLock lock(*mpMediaGuard) ;

    // Get Input/Output Levels
    iInputEnergyLevel = adjustInputLevel(mpVoiceEngine->GIPSVE_GetInputLevel()) ;
    iOutputEnergyLevel = mpVoiceEngine->GIPSVE_GetOutputLevel(-1) ;   

    return OS_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::getAudioEnergyLevels(int connectionId,
                                                         int& iInputEnergyLevel,
                                                         int& iOutputEnergyLevel,
                                                         int& nContributors,
                                                         unsigned int* pContributorSRCIds,
                                                         int* pContributorEngeryLevels) 
{
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;

    assert(nContributors >= 0) ;

    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);
    if (pMediaConn)
    {
        // Get Input/Output Levels
        iInputEnergyLevel = adjustInputLevel(mpVoiceEngine->GIPSVE_GetInputLevel()) ;
        iOutputEnergyLevel = mpVoiceEngine->GIPSVE_GetOutputLevel(connectionId) ;

        // Get contributor info        
        if (nContributors > 0)
        {
            int nCCSRCBlock = nContributors * sizeof(int) ;
            unsigned char* cCCSRCs = (unsigned char*) malloc(nCCSRCBlock) ;
            unsigned char* cEnergyLevels = (unsigned char*) malloc(nContributors) ;
            unsigned int ignored ;

            nContributors = 0 ;

            // Verify memory allocation
            if (cCCSRCs && cEnergyLevels)
            {
                if (mpVoiceEngine->GIPSVE_GetRemoteSSRC(connectionId, &ignored, cCCSRCs, &nCCSRCBlock, cEnergyLevels) == 0)
                {
                    rc = OS_SUCCESS ;

                    if (nCCSRCBlock > 0)
                    {
                        nContributors = nCCSRCBlock / 4 ;
                        for (int i=0; i<nContributors; i++)
                        {
                            pContributorSRCIds[i] = ((unsigned int*) cCCSRCs)[i] ;
                            pContributorEngeryLevels[i] = (int) cEnergyLevels[i] ;
                        }
                    }
                }

                // Clean up allocated memory
                if (cCCSRCs)
                    free(cCCSRCs) ;

                if (cEnergyLevels)
                    free(cEnergyLevels) ;
            }
        }
    }

    return rc;    
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::getAudioRtpSourceIDs(int           connectionId,
                                                         unsigned int& uiSendingSSRC,
                                                         unsigned int& uiReceivingSSRC) 
{
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;

    CpMediaConnection* pMediaConn = getMediaConnection(connectionId);
    if (pMediaConn)
    {      
        rc = OS_SUCCESS ;

        uiSendingSSRC = mpVoiceEngine->GIPSVE_GetSendSSRC(connectionId) ;
        if (mpVoiceEngine->GIPSVE_GetRemoteSSRC(connectionId, &uiReceivingSSRC) != 0)
        {
            uiReceivingSSRC = 0 ; 
        }
    }

    return rc ;
}

//////////////////////////////////////////////////////////////////////////////
OsStatus VoiceEngineMediaInterface::getAudioRtcpStats(const int connectionId,
                                                      SIPX_RTCP_STATS* const pStats)
{
    OsStatus rc = OS_FAILED;

    if (mpVoiceEngine)
    {
        GIPSVE_CallStatistics stats ;
        memset(&stats, 0, sizeof(GIPSVE_CallStatistics)) ;
        memset(pStats, 0, sizeof(SIPX_RTCP_STATS)) ;
        pStats->cbSize = sizeof(SIPX_RTCP_STATS) ;

        if (mpVoiceEngine->GIPSVE_RTCPStat(connectionId, &stats) == 0)
        {
            pStats->fraction_lost = stats.fraction_lost ;
            pStats->cum_lost = stats.cum_lost ;
            pStats->ext_max = stats.ext_max ;
            pStats->jitter = stats.jitter ;
            pStats->RTT = stats.RTT ;
            pStats->bytesSent = stats.bytesSent ;
            pStats->packetsSent = stats.packetsSent ;
            pStats->bytesReceived = stats.bytesReceived ;
            pStats->packetsReceived = stats.packetsReceived ;

            rc = OS_SUCCESS;
        }
    }
    return rc;
}

//////////////////////////////////////////////////////////////////////////////
OsStatus VoiceEngineMediaInterface::getVideoRtcpStats(const int connectionId,
                                                      SIPX_RTCP_STATS* const pStats)
{
    OsStatus rc = OS_FAILED;

    if (mpVideoEngine)
    {
        GIPSVideo_CallStatistics stats ;
        memset(&stats, 0, sizeof(GIPSVideo_CallStatistics)) ;
        memset(pStats, 0, sizeof(SIPX_RTCP_STATS)) ;
        pStats->cbSize = sizeof(SIPX_RTCP_STATS) ;

        if (mpVideoEngine->getRTCPStats(connectionId, &stats) == 0)
        {
            pStats->fraction_lost = stats.fraction_lost ;
            pStats->cum_lost = stats.cum_lost ;
            pStats->ext_max = stats.ext_max ;
            pStats->jitter = stats.jitter ;
            pStats->RTT = stats.RTT ;
            pStats->bytesSent = stats.bytesSent ;
            pStats->packetsSent = stats.packetsSent ;
            pStats->bytesReceived = stats.bytesReceived ;
            pStats->packetsReceived = stats.packetsReceived ;

            rc = OS_SUCCESS;
        }
    }
    return rc;
}


//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::getMediaDeviceInfo(int connectionId,
                                                       MediaDeviceInfo::MediaDeviceInfoType type,
                                                       MediaDeviceInfo& info) 
{
    OsStatus rc = OS_FAILED;

    switch (type)
    {
    case MediaDeviceInfo::MDIT_AUDIO_INPUT:
        if (mpVEFactoryImpl)
        {
            info = mpVEFactoryImpl->getAudioInputDeviceInfo() ;
            rc = OS_SUCCESS ;
        }
        break ;
    case MediaDeviceInfo::MDIT_AUDIO_OUTPUT:
        if (mpVEFactoryImpl)
        {
            info = mpVEFactoryImpl->getAudioOutputDeviceInfo() ;
            rc = OS_SUCCESS ;
        }
        break ;
    case MediaDeviceInfo::MDIT_VIDEO_CAPTURE:
        if (mpVideoEngine)
        {
            info = mpVideoEngine->getVideoCaptureDeviceInfo() ;
            rc = OS_SUCCESS ;
        }
        break ;
    default:
        assert(false) ; // unsupport typed
        break ;
    }

    return rc ;

}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::generateVoiceQualityReport(int connectionId,
                                                               const char* callId,
                                                               char*  szReport,
                                                               size_t reportSize) 
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::generateVoiceQualityReport") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_FAILED;

#ifdef ENABLE_GIPS_VQMON
    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection && pMediaConnection->getRtpAudioSocket())
    {
        OsDateTime time ;
        UtlString startReadTime ;
        UtlString endReadTime ;
        UtlString startWriteTime ;
        UtlString endWriteTime ;
        UtlBoolean bSuccess ;

        bSuccess = pMediaConnection->getRtpAudioSocket()->getSocket()->getFirstReadTime(time) ;
        time.getIsoTimeStringZ(startReadTime) ;
        pMediaConnection->getRtpAudioSocket()->getSocket()->getLastReadTime(time) ;
        time.getIsoTimeStringZ(endReadTime) ;        
        bSuccess = pMediaConnection->getRtpAudioSocket()->getSocket()->getFirstWriteTime(time) && bSuccess ;
        time.getIsoTimeStringZ(startWriteTime) ;
        pMediaConnection->getRtpAudioSocket()->getSocket()->getLastWriteTime(time) ;
        time.getIsoTimeStringZ(endWriteTime) ;   

        // Only generate a report if we have read/written data
        if (bSuccess)
        {
            unsigned char cBuf[10240] ;
            unsigned int  nLength = sizeof(cBuf) ;
            memset(cBuf, 0, sizeof(cBuf)) ;

            int iRC = mpVoiceEngine->GIPSVE_GetVQMonSIPReport(
                    connectionId,
                    cBuf,
                    &nLength,
                    (char*) callId,
                    (char*) callId,
                    (char*) startWriteTime.data(),
                    (char*) endWriteTime.data(),
                    (char*) startReadTime.data(),
                    (char*) endReadTime.data()) ;
            checkVoiceEngineReturnCode("GIPSVE_GetVQMonSIPReport", connectionId, iRC, true) ;
            if (iRC == 0)
            {
                UtlString buf((char*) cBuf) ;
                size_t index = buf.index("RemoteMetrics:", 0, UtlString::ignoreCase) ;
                if (index != UTL_NOT_FOUND)
                    buf.remove(index) ;

                strncpy(szReport, buf, reportSize);
                rc = OS_SUCCESS ;
            }
        }
    }
#endif

    return rc ;
}


/* ============================ INQUIRY =================================== */

UtlBoolean VoiceEngineMediaInterface::canAddParty() 
{
    OsLock lock(*mpMediaGuard) ;
    int iLoad = mpVoiceEngine->GIPSVE_GetCPULoad() ;
    bool bRet = true;

    assert((iLoad >=0) && (iLoad <=100)) ;

    if (isVideoConferencing())
    {
        if (getNumConnections() < 4)
        {
            bRet = (iLoad < 69);
        }
        else
        {
            bRet = false;
        }
    }
    else
    {
        bRet = (iLoad < 69);
    }

    return bRet ;
}


//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::startAudioSocketSupport(CpMediaConnection* pMediaConn) 
{    
    OsLock lock(*mpMediaGuard) ;
    OS_PERF_FUNC("VoiceEngineMediaInterface::startAudioSocketSupport") ;
    CpMediaNetTask* pNetTask = CpMediaNetTask::getCpMediaNetTask() ;
    CpMediaSocketMonitorTask* pMonTask = CpMediaSocketMonitorTask::getInstance() ;
    IOsNatSocket* rtpAudioSocketArray[2] = { NULL, NULL};
    IOsNatSocket* rtcpAudioSocketArray[2] = { NULL, NULL};

    traceAPI("VoiceEngineMediaInterface::startAudioSocketSupport") ;

    if (pMediaConn && mpVEFactoryImpl && pNetTask)
    {
        /*
         * Create Sockets
         */ 

        // we always need both UDP and TCP sockets for TCP media transport (UDP sockets are needed for STUN relective requests
        // to a STUN server
        int numAudioSockets = 0;    
        rtpAudioSocketArray[numAudioSockets] = mpVEFactoryImpl->getSocketFactory()->getUdpSocket(
                mpVoiceEngine,
                NULL, 
                TYPE_AUDIO_RTP,
                pMediaConn->mLocalAddress,
                pMediaConn->mbEnableSTUN ? mStunServer : ProxyDescriptor(),
                pMediaConn->mbEnableTURN ? mTurnProxy : ProxyDescriptor());

        if (pMediaConn->mbEnableSTUN)
            mAudioMediaConnectivityInfo.setStunServer(mStunServer.getAddress()) ;
        if (pMediaConn->mbEnableTURN)
            mAudioMediaConnectivityInfo.setTurnServer(mTurnProxy.getAddress()) ;
        if (rtpAudioSocketArray[numAudioSockets])
            mAudioMediaConnectivityInfo.setUPNP(rtpAudioSocketArray[numAudioSockets]->getUpnpMappedPort() != PORT_NONE) ;
        
        if (mbEnableRTCP)
        {
            rtcpAudioSocketArray[numAudioSockets] = mpVEFactoryImpl->getSocketFactory()->getUdpSocket(
                    mpVoiceEngine,
                    NULL, 
                    TYPE_AUDIO_RTCP,
                    pMediaConn->mLocalAddress,
                    pMediaConn->mbEnableSTUN ? mStunServer : ProxyDescriptor(),
                    pMediaConn->mbEnableTURN ? mTurnProxy : ProxyDescriptor());

        }
        numAudioSockets ++;        
                
        if (pMediaConn->mRtpTransport != RTP_TRANSPORT_UDP)
        {
            rtpAudioSocketArray[numAudioSockets] = mpVEFactoryImpl->getSocketFactory()->getTcpSocket(
                    mpVoiceEngine,
                    NULL, 
                    TYPE_AUDIO_RTP,
                    0,
                    NULL, 
                    pMediaConn->mLocalAddress);
                        
            if (mbEnableRTCP)
            {                    
                rtcpAudioSocketArray[numAudioSockets] = mpVEFactoryImpl->getSocketFactory()->getTcpSocket(
                        mpVoiceEngine,
                        NULL, 
                        TYPE_AUDIO_RTCP,
                        0,
                        NULL, 
                        pMediaConn->mLocalAddress);                    
            }
            numAudioSockets++ ;
        }

        if (mArsProxy.isValid() && pMediaConn->mbEnableARS)
        {
            pMediaConn->mpArsAudioSocket = mpVEFactoryImpl->getSocketFactory()->getArsSocket(
                mArsProxy, mArsHttpProxy, ARS_MIMETYPE_AUDIO, pMediaConn->mLocalAddress) ;

            pMediaConn->mpArsAudioRAdapter = new ArsVoiceEngineReceiveAdapter(MEDIA_TYPE_AUDIO,
                    mpVoiceEngine,
                    NULL,
                    mpMediaGuard) ;

            pMediaConn->mpArsAudioSocket->setPacketHandler(pMediaConn->mpArsAudioRAdapter) ;
        
            mAudioMediaConnectivityInfo.setArsServer(mArsProxy.getAddress()) ;
            mAudioMediaConnectivityInfo.setArsHttpsProxy(mArsHttpProxy.getAddress()) ;
        }

        /*
         * Bind/plug-in sockets
         */ 
        for (int i = 0; i < numAudioSockets; i++)
        {
            if (mpSocketIdleSink && pMonTask)
            {
                pMonTask->monitor(
                        rtpAudioSocketArray[i]->getSocket(),
                        mpSocketIdleSink,
                        RTP_AUDIO);
            }
                        
            pMediaConn->mpRtpAudioSocketArray[i] = rtpAudioSocketArray[i];
            pMediaConn->mpRtcpAudioSocketArray[i] = rtcpAudioSocketArray[i];

            pMediaConn->mpAudioSocketAdapterArray[i] = new 
                    VoiceEngineSocketAdapter(MEDIA_TYPE_AUDIO, 
                    pMediaConn->mpRtpAudioSocketArray[i], 
                    pMediaConn->mpRtcpAudioSocketArray[i],
                    mpMediaPacketCallback) ;

            pMediaConn->mRtpAudioReceivePort = rtpAudioSocketArray[i]->getSocket()->getLocalHostPort() ;
            if (rtcpAudioSocketArray[i])
                pMediaConn->mRtcpAudioReceivePort = rtcpAudioSocketArray[i]->getSocket()->getLocalHostPort() ; 
        }

        if (pMediaConn->mpArsAudioSocket)                    
        {
            if (pMediaConn->mbInitiating)
            {
                ARS_COOKIE cookie ;
                memset(cookie.data, 0xBA, sizeof(cookie.data)) ;
                if (pMediaConn->mpArsAudioSocket->listenArs(cookie))
                    pNetTask->addInputSource(pMediaConn->mpArsAudioSocket);
            }
            if (mpSocketIdleSink && pMonTask)
            {
                pMonTask->monitor(
                        pMediaConn->mpArsAudioSocket,
                        mpSocketIdleSink,
                        RTP_AUDIO);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::startVideoSocketSupport(CpMediaConnection* pMediaConn) 
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::startVideoSocketSupport") ;

    CpMediaSocketMonitorTask* pMonTask = CpMediaSocketMonitorTask::getInstance() ;

    OsLock lock(*mpMediaGuard) ;

    if (mpGipsVideoEngine == NULL)
    {
        mpGipsVideoEngine = mpVEFactoryImpl->getNewVideoEngineInstance(mpVoiceEngine) ;
        mpVideoEngine->setGipsVideoEngine(mpGipsVideoEngine) ;
    }

    CpMediaNetTask* pNetTask = NULL;    
    IOsNatSocket* rtpVideoSocketArray[2] = {NULL, NULL};
    IOsNatSocket* rtcpVideoSocketArray[2] = {NULL,NULL};

    traceAPI("VoiceEngineMediaInterface::startVideoSocketSupport") ;

    if (pMediaConn && (pMediaConn->mpRtpVideoSocketArray[0] == NULL) && mpVideoEngine)
    {      
        pNetTask = CpMediaNetTask::getCpMediaNetTask();

        /*
         * Create Sockets
         */

        // we always need both UDP and TCP sockets for TCP media transport (UDP sockets are needed for STUN relective requests
        // to a STUN server
        int rtpVidSockIndex = 0;
        rtpVideoSocketArray[rtpVidSockIndex] = mpVEFactoryImpl->getSocketFactory()->getUdpSocket(
                mpVoiceEngine,
                mpVideoEngine->getVideoEngine(), 
                TYPE_VIDEO_RTP,
                pMediaConn->mLocalAddress,
                pMediaConn->mbEnableSTUN ? mStunServer : ProxyDescriptor(),
                pMediaConn->mbEnableTURN ? mTurnProxy : ProxyDescriptor());

        if (pMediaConn->mbEnableSTUN)
            mVideoMediaConnectivityInfo.setStunServer(mStunServer.getAddress()) ;
        if (pMediaConn->mbEnableTURN)
            mVideoMediaConnectivityInfo.setTurnServer(mTurnProxy.getAddress()) ;
        if (rtpVideoSocketArray[rtpVidSockIndex])
            mVideoMediaConnectivityInfo.setUPNP(rtpVideoSocketArray[rtpVidSockIndex]->getUpnpMappedPort() != PORT_NONE) ;
            
        if (mbEnableRTCP)
        {                
            rtcpVideoSocketArray[rtpVidSockIndex] = mpVEFactoryImpl->getSocketFactory()->getUdpSocket(
                    mpVoiceEngine,
                    mpVideoEngine->getVideoEngine(), 
                    TYPE_VIDEO_RTCP,
                    pMediaConn->mLocalAddress,
                    pMediaConn->mbEnableSTUN ? mStunServer : ProxyDescriptor(),
                    pMediaConn->mbEnableTURN ? mTurnProxy : ProxyDescriptor());
        }              
        rtpVidSockIndex++;  
        
        if ((pMediaConn->mRtpTransport & RTP_TRANSPORT_TCP) == RTP_TRANSPORT_TCP)
        {
            rtpVideoSocketArray[rtpVidSockIndex] = mpVEFactoryImpl->getSocketFactory()->getTcpSocket(
                    mpVoiceEngine,
                    mpVideoEngine->getVideoEngine(), 
                    TYPE_VIDEO_RTP,
                    0,
                    NULL, 
                    pMediaConn->mLocalAddress);

            if (mbEnableRTCP)
            {
                rtcpVideoSocketArray[rtpVidSockIndex] = mpVEFactoryImpl->getSocketFactory()->getTcpSocket(
                        mpVoiceEngine,
                        mpVideoEngine->getVideoEngine(), 
                        TYPE_VIDEO_RTCP,
                        0,
                        NULL, 
                        pMediaConn->mLocalAddress);
            }                
            rtpVidSockIndex++;  
        }          

        if (mArsProxy.isValid() && pMediaConn->mbEnableARS)
        {
            pMediaConn->mpArsVideoSocket = mpVEFactoryImpl->getSocketFactory()->getArsSocket(
                mArsProxy, mArsHttpProxy, ARS_MIMETYPE_VIDEO, pMediaConn->mLocalAddress) ;

            pMediaConn->mpArsVideoRAdapter = new ArsVoiceEngineReceiveAdapter(MEDIA_TYPE_VIDEO,
                    NULL,
                    mpGipsVideoEngine,
                    mpMediaGuard) ;

            pMediaConn->mpArsVideoSocket->setPacketHandler(pMediaConn->mpArsVideoRAdapter) ;
            mVideoMediaConnectivityInfo.setArsServer(mArsProxy.getAddress()) ;
            mVideoMediaConnectivityInfo.setArsHttpsProxy(mArsHttpProxy.getAddress()) ;

        }


        /*
         * Bind/Plug-in Sockets
         */

        for (int i = 0; i < rtpVidSockIndex; i++)
        {
            if (mpSocketIdleSink && pMonTask)
            {
                pMonTask->monitor(
                        rtpVideoSocketArray[i]->getSocket(),
                        mpSocketIdleSink,
                        RTP_VIDEO);
            }
                            
            pMediaConn->mpRtpVideoSocketArray[i] = rtpVideoSocketArray[i];
            pMediaConn->mpRtcpVideoSocketArray[i] = rtcpVideoSocketArray[i];

            pMediaConn->mpVideoSocketAdapterArray[i] = new 
                    VoiceEngineSocketAdapter(MEDIA_TYPE_VIDEO,
                    pMediaConn->mpRtpVideoSocketArray[i], 
                    pMediaConn->mpRtcpVideoSocketArray[i],
                    mpMediaPacketCallback) ;
            
            pMediaConn->mRtpVideoReceivePort = rtpVideoSocketArray[i]->getSocket()->getLocalHostPort() ;
            if (rtcpVideoSocketArray[i])
                pMediaConn->mRtcpVideoReceivePort = rtcpVideoSocketArray[i]->getSocket()->getLocalHostPort() ; 
        }

        if (pMediaConn->mpArsVideoSocket)
        {
            if (pMediaConn->mbInitiating)
            {
                ARS_COOKIE cookie ;
                memset(cookie.data, 0xBA, sizeof(cookie.data)) ;
                if (pMediaConn->mpArsVideoSocket->listenArs(cookie))
                    pNetTask->addInputSource(pMediaConn->mpArsVideoSocket);
            }

            if (mpSocketIdleSink && pMonTask)
            {
                pMonTask->monitor(
                        pMediaConn->mpArsVideoSocket,
                        mpSocketIdleSink,
                        RTP_VIDEO);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::finalizeVideoSupport(int connectionId)
{
    CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        for (int i = 0; i < 2; i++) // UDP and TCP
        {
            if (pMediaConnection->mpRtpVideoSocketArray[i])
            {
                getMediaSocketPtr(pMediaConnection->mpRtpVideoSocketArray[i])->setVideoChannel(connectionId);
            }
            if (pMediaConnection->mpRtcpVideoSocketArray[i])
            {
                getMediaSocketPtr(pMediaConnection->mpRtcpVideoSocketArray[i])->setVideoChannel(connectionId);
            }
        }

        if (pMediaConnection->mpArsVideoSocket)
            pMediaConnection->mpArsVideoSocket->setVideoChannel(connectionId) ;
    }
}

//////////////////////////////////////////////////////////////////////////////


/* //////////////////////////// PRIVATE /////////////////////////////////// */



UtlBoolean VoiceEngineMediaInterface::getVoiceEngineCodec(const SdpCodec& pCodec, GIPSVE_CodecInst& codecInfo)
{
    OsLock lock(*mpMediaGuard) ;
    UtlString codecName;
    UtlBoolean matchFound = FALSE;
    int iCodecs;

    if (mpFactoryImpl->getCodecNameByType(pCodec.getCodecType(), codecName))
    {
        if ((iCodecs=mpVoiceEngine->GIPSVE_GetNofCodecs()) != -1)
        {
            for (int i=0; i<iCodecs; ++i)
            {
                if (mpVoiceEngine->GIPSVE_GetCodec(i, &codecInfo) == 0)
                {
                    if (codecName.compareTo(codecInfo.plname, UtlString::ignoreCase) == 0)
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

UtlBoolean VoiceEngineMediaInterface::getVideoEngineCodec(const SdpCodec& codec, GIPSVideo_CodecInst& vcodecInfo)
{
    OsLock lock(*mpMediaGuard) ;
    UtlString codecName;
    UtlBoolean matchFound = FALSE;
    int iCodecs;

    if (mpFactoryImpl->getCodecNameByType(codec.getCodecType(), codecName))
    {
        if ((iCodecs = mpVideoEngine->getVideoEngine()->GIPSVideo_GetNofCodecs()) != -1)
        {
            for (int i=0; i<iCodecs; ++i)
            {
                memset((void*)&vcodecInfo, 0, sizeof(vcodecInfo));
                if (mpVideoEngine->getVideoEngine()->GIPSVideo_GetCodec(i, &vcodecInfo) == 0)
                {
                    if (strncmp(codecName.data(), vcodecInfo.plname, 4) == 0)
                    {
                        // Init various settings within VE codec:
                        vcodecInfo.pltype = codec.getCodecPayloadFormat();
                        lookupResolution(codec.getVideoFormat(), vcodecInfo.width, vcodecInfo.height) ;

                        int quality, cpu, framerate, bitrate ;

                        mpVEFactoryImpl->getVideoQuality(quality);
                        mpVEFactoryImpl->getVideoBitRate(bitrate);
                        mpVEFactoryImpl->getVideoFrameRate(framerate);
                        mpVEFactoryImpl->getVideoCpuValue(cpu);

                        vcodecInfo.quality = quality ;
                        vcodecInfo.bitRate = bitrate ;
                        vcodecInfo.frameRate = framerate ;
                        // VP71 allows you to specify the VP71 CPU usage as a codec 
                        // specific argument.
                        if (codecName.contains("VP71"))
                        {
                            vcodecInfo.codecSpecific = cpu ;
                        } 
                        else if (codecName.contains("263"))
                        {                            
                            vcodecInfo.codecSpecific = GIPS_H263_DROP_P_FRAMES ;
                        }

                        GIPSCameraCapability caps ;
                        if (mpVideoEngine->getCameraCapabilities(&caps))
                        {
                            // Do select anything above camera max FPS
                            if (caps.maxFPS < vcodecInfo.frameRate)
                            {
                                trace(PRI_NOTICE, 
                                        "Clamping video FPS from %d to %d (max camera capabilities)",
                                        vcodecInfo.frameRate,
                                        caps.maxFPS) ;

                                vcodecInfo.frameRate = caps.maxFPS ;
                            }
                        }


                        matchFound = TRUE;
                        break;
                    }
                }
            }
        }
    }

    return matchFound;
}

//////////////////////////////////////////////////////////////////////////////

void* const VoiceEngineMediaInterface::getAudioEnginePtr()
{
    OsLock lock(*mpMediaGuard) ;
    return mpVoiceEngine;
}

//////////////////////////////////////////////////////////////////////////////

void* const VoiceEngineMediaInterface::getVideoEnginePtr()
{
/*
    OsLock lock(*mpMediaGuard) ;
    return mpVideoEngine;
*/
    return mpGipsVideoEngine ;
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::setVQMonAddresses(int connectionId) 
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::setVQMonAddresses") ;
#ifdef ENABLE_GIPS_VQMON
    CpMediaConnection* pMediaConn = getMediaConnection(connectionId) ;

    if (pMediaConn && pMediaConn->getRtpAudioSocket())
    {
        UtlString rtpHostAddress ;
        int rtpAudioPort ;

        if (pMediaConn->mContactType == CONTACT_RELAY || pMediaConn->mContactType == CONTACT_ARS)
        {                      
            if (!pMediaConn->getRtpAudioSocket()->getTurnIp(&rtpHostAddress, &rtpAudioPort))
            {
                rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                rtpHostAddress = mRtpReceiveHostAddress ;
            }
        }
        else if (pMediaConn->mContactType == CONTACT_AUTO || pMediaConn->mContactType == CONTACT_NAT_MAPPED)
        {
            if (!pMediaConn->getRtpAudioSocket()->getMappedIp(&rtpHostAddress, &rtpAudioPort))
            {
                rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                rtpHostAddress = mRtpReceiveHostAddress ;
            }
        }
        else if (pMediaConn->mContactType == CONTACT_LOCAL)
        {
            rtpHostAddress = pMediaConn->getRtpAudioSocket()->getSocket()->getLocalIp();
            rtpAudioPort = pMediaConn->getRtpAudioSocket()->getSocket()->getLocalHostPort();
            if (rtpAudioPort <= 0)
            {
                rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                rtpHostAddress = mRtpReceiveHostAddress ;
            }
        }                

        unsigned long localAddr = inet_addr(rtpHostAddress) ;
        unsigned long remoteAddr = inet_addr(pMediaConn->mRtpSendHostAddress) ;
        
        int rc ;
        rc = mpVoiceEngine->GIPSVE_VQMonIPInfo(connectionId, 
                (unsigned char*) &localAddr, 
                rtpAudioPort, 
                (unsigned char*) &remoteAddr, 
                pMediaConn->mRtpAudioSendHostPort) ;
        checkVoiceEngineReturnCode("GIPSVE_VQMonIPInfo", connectionId, rc, true) ;
    }
#endif
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::muteMicrophone(const bool bMute)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::muteMicrophone") ;
    OsLock lock(*mpMediaGuard) ;
    OsStatus rc = OS_SUCCESS;
    int iRC ;
    
    traceAPI("VoiceEngineMediaInterface::muteMicrophone") ;
    
    UtlSListIterator iterator(mMediaConnections);
    CpMediaConnection* pMediaConnection = NULL ;
    while ((pMediaConnection = (CpMediaConnection*) iterator()))
    {
        iRC = mpVoiceEngine->GIPSVE_MuteMic(pMediaConnection->getValue(), bMute) ;
        checkVoiceEngineReturnCode("GIPSVE_MuteMic", pMediaConnection->getValue(), iRC, true) ;
        if (0 != iRC)
        {
            rc = OS_FAILED;
        }               
    }       
    return rc;
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::setVideoWindowDisplay(int connectionId, const void* pDisplay)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::setVideoWindowDisplay") ;
    OsLock lock(*mpMediaGuard) ;
    
    OsStatus rc = OS_SUCCESS;

    traceAPI("VoiceEngineMediaInterface::setVideoWindowDisplay") ;

    if (isDisplayValid((SIPXVE_VIDEO_DISPLAY*) pDisplay) && mpVideoEngine)
    {
        if (mpDisplay)
            delete mpDisplay ;
        mpDisplay = new SIPXVE_VIDEO_DISPLAY(*(SIPXVE_VIDEO_DISPLAY*)pDisplay);

        CpMediaConnection* pMediaConnection = getMediaConnection(connectionId);
        mpVideoEngine->setRemoteDisplay(connectionId, *mpDisplay) ;

        // Set Preview display also (incase it isn't set)
        SIPXVE_VIDEO_DISPLAY* pLocalDisplay = NULL;
        pLocalDisplay = (SIPXVE_VIDEO_DISPLAY*) mpVEFactoryImpl->getVideoPreviewDisplay();
        if (VideoEngine::isDisplayValid(pLocalDisplay))
            mpVideoEngine->setPreviewDisplay(*pLocalDisplay) ;
    }
    return rc;
}

//////////////////////////////////////////////////////////////////////////////


int VoiceEngineMediaInterface::checkVoiceEngineReturnCode(const char* szAPI, 
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
        lastError = mpVoiceEngine->GIPSVE_GetLastError();        

#if defined(_WIN32) && defined(_DEBUG)
        UtlString consoleOutput ;
        OsDateTime::getLocalTimeString(consoleOutput) ;
        char cTemp[128] ;

        consoleOutput.append(" ") ;
        consoleOutput.append(szAPI) ;

        sprintf(cTemp, " returned %d (lastError=%d)\n", returnCode, lastError) ;
        consoleOutput.append(cTemp) ;

        OutputDebugString(consoleOutput) ;
#endif
        trace(PRI_DEBUG, "VoiceEngine API %s returned %d (lastError=%d)", 
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

int VoiceEngineMediaInterface::checkVideoEngineReturnCode(const char* szAPI, 
                                                          int         connectionId,
                                                          int         returnCode, 
                                                          bool        bAssertOnError) 
{
    int lastError = 0;
/*

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
#ifdef VIDEO
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

        OsSysLog::add(FAC_MP, PRI_DEBUG, "%s %s returned %d (lastError=%d)", 
                cId, 
                szAPI,
                returnCode,
                lastError) ;

        if (bAssertOnError)
        {
            assert(false) ;
        }
#endif
    }
*/
    return lastError;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::enableConferencing(int connectionId)
{
/*
    OsLock lock(*mpMediaGuard) ;

    int rc = 0;
    int numConnections =  mMediaConnections.entries();
    int handle = 0;
    CpMediaConnection* pMediaConnection = NULL ; 

    if (numConnections > 1 && !mbConferenceEnabled)
    {
        UtlSListIterator iterator(mMediaConnections);
        while ((pMediaConnection = (CpMediaConnection*) iterator()))    
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
*/
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::disableConferencing(int connectionId)
{
/*
    OS_PERF_FUNC("VoiceEngineMediaInterface::disableConferencing") ;
#ifdef VIDEO
    OsLock lock(*mpMediaGuard) ;
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
                CpMediaConnection* pMediaConn = (CpMediaConnection*)mMediaConnections.first();
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
*/
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::renderVideoQuadrant(int handle, int connectionId)
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::renderVideoQuadrant") ;
/*
#ifdef VIDEO
    OsLock lock(*mpMediaGuard) ;
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
*/
}

//////////////////////////////////////////////////////////////////////////////

int VoiceEngineMediaInterface::getVideoQuadrantHandle(int connectionId)
{
    int handle = -1;
/*
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
*/
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

//////////////////////////////////////////////////////////////////////////////

/* ============================ FUNCTIONS ================================= */


// normalize internal (GIPS) payload types
void VoiceEngineMediaInterface::normalizeInternalPayloadTypes(int connectionId, SdpCodecList* pFactory)
{    
    int numCodecs ;
    SdpCodec** pCodecs ;

    // First pass, find highest dynamic payloadType
    pFactory->getCodecs(numCodecs, pCodecs) ;
    normalizeInternalPayloadTypes(connectionId, numCodecs, pCodecs) ;

    for (int i=0; i<numCodecs; i++)
    {
        delete pCodecs[i] ;
    }
    delete [] pCodecs ;
}

//////////////////////////////////////////////////////////////////////////////

void VoiceEngineMediaInterface::normalizeInternalPayloadTypes(int connectionId, 
                                                              int numCodecs, 
                                                              SdpCodec* pCodecs[])
{
    OS_PERF_FUNC("VoiceEngineMediaInterface::normalizeInternalPayloadTypes") ;

    int dynamicPayloadStart = SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC + 1 ;
    int numGipsCodecs ;
    int rc ;

    // First pass, find highest dynamic payloadType and then add 10
    for (int i=0; i<numCodecs; i++)
    {
        if (pCodecs[i]->getCodecPayloadFormat() >= dynamicPayloadStart)
            dynamicPayloadStart = pCodecs[i]->getCodecPayloadFormat() + 1 ;
    }
    dynamicPayloadStart += 10 ;

    // Second set all of the GIPS audio codecs payload types
    numGipsCodecs = mpVoiceEngine->GIPSVE_GetNofCodecs() ;
    for (int i=0; i<numGipsCodecs; i++)
    {
        GIPS_CodecInst codec ;
        if (mpVoiceEngine->GIPSVE_GetCodec(i, &codec) == 0)
        {
            bool bSet = false ;
            for (int j=0; j<numCodecs; j++)
            {
                UtlString mimeType ;
                pCodecs[j]->getMediaType(mimeType) ;
                if (mimeType.compareTo(MIME_TYPE_AUDIO, UtlString::ignoreCase) == 0)
                {
                    UtlString codecName ;
                    pCodecs[j]->getEncodingName(codecName) ;
                    if (codecName.compareTo(codec.plname, UtlString::ignoreCase) == 0)
                    {
                        if (codec.pltype != pCodecs[j]->getCodecPayloadFormat())
                        {
                            trace(PRI_DEBUG, "Resetting payload type for %s from %d to %d",
                                    codec.plname, codec.pltype, pCodecs[j]->getCodecPayloadFormat()) ;
                            codec.pltype = pCodecs[j]->getCodecPayloadFormat() ;
                            rc = mpVoiceEngine->GIPSVE_SetRecPayloadType(connectionId, &codec) ;
                            checkVoiceEngineReturnCode("GIPSVE_SetRecPayloadType", connectionId, rc, true) ;
                        }
                        bSet = true ;
                        break ;
                    }
                }
            }

            if (!bSet)
            {
                if (codec.pltype != dynamicPayloadStart)
                {
                    if (codec.pltype > SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC+1)
                    {
                        trace(PRI_DEBUG, "Resetting payload type for %s from %d to %d",
                                codec.plname, codec.pltype, dynamicPayloadStart) ;
                        codec.pltype = dynamicPayloadStart++ ;
                        rc = mpVoiceEngine->GIPSVE_SetRecPayloadType(connectionId, &codec) ;
                        checkVoiceEngineReturnCode("GIPSVE_SetRecPayloadType", connectionId, rc, true) ;
                    }
                }
            }
        }
    }
}
                          
//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::getMaxVideoResolution(SdpCodecList& supportedCodecs, 
                                                          int&             width, 
                                                          int&             height) 
{
    OsStatus rc = OS_FAILED ;
    int maxWidth = -1 ;
    int maxHeight = -1 ;

    size_t nCodecs = supportedCodecs.getCodecCount() ;
    for (size_t i=0; i<nCodecs; i++)
    {
        int w, h ;
        const SdpCodec* pCodec = supportedCodecs.getCodecByIndex(i) ;
        if (pCodec && pCodec->getVideoResolution(w, h)) 
        {
            if ((w > maxWidth) || (h > maxHeight))
            {
                maxWidth = w ;
                maxHeight = h ;
                rc = OS_SUCCESS ;
            }
        }
    }

    if (rc == OS_SUCCESS)
    {
        width = maxWidth ;
        height = maxHeight ;
    }
    return rc ;   
}

//////////////////////////////////////////////////////////////////////////////

OsStatus VoiceEngineMediaInterface::getMaxVideoCodec(SdpCodecList& supportedCodecs, 
                                                     const SdpCodec*& codec) 
{
    OsStatus rc = OS_FAILED ;
    int maxWidth = -1 ;
    int maxHeight = -1 ;

    size_t nCodecs = supportedCodecs.getCodecCount() ;
    for (size_t i=0; i<nCodecs; i++)
    {
        int w, h ;
        const SdpCodec* pCodec = supportedCodecs.getCodecByIndex(i) ;
        if (pCodec && pCodec->getVideoResolution(w, h))
        {
            if ((w > maxWidth) || (h > maxHeight))
            {
                codec = pCodec ;
                maxWidth = w ;
                maxHeight = h ;
                rc = OS_SUCCESS ;
            }
        }
    }

    return rc ;   
}

//////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////

UtlBoolean VoiceEngineMediaInterface::isAudioAvailable()
{
    OsLock lock(*mpMediaGuard) ;
    bool bAvailable = false;

    if (getNumConnections() == 0)
    {
        if (mpVoiceEngine && mpVoiceEngine->GIPSVE_CheckIfAudioIsAvailable(1, 1) == 0)
        {
            bAvailable = true;
        }
    }
    else
    {
        // Assume audio is available since we already have a connection. GIPS does the 
        // wrong thing here in reporting false for any subsequent connections.
        bAvailable = true;
    }
    return bAvailable;
}


//////////////////////////////////////////////////////////////////////////////

UtlBoolean VoiceEngineMediaInterface::getCodecTypeByName(const UtlString& codecName, SdpCodec::SdpCodecTypes& codecType)
{
    UtlBoolean matchFound = FALSE;

    codecType = SdpCodec::SDP_CODEC_UNKNOWN;

    if (codecName == GIPS_CODEC_ID_G729)
    {
        codecType = SdpCodec::SDP_CODEC_G729A;
    } 
    else if (codecName == GIPS_CODEC_ID_TELEPHONE)
    {
        codecType = SdpCodec::SDP_CODEC_TONES;
    }
    else if (codecName == GIPS_CODEC_ID_PCMA)
    {
        codecType = SdpCodec::SDP_CODEC_PCMA;
    }
    else if (codecName == GIPS_CODEC_ID_PCMU)
    {
        codecType = SdpCodec::SDP_CODEC_PCMU;
    }
    else if (codecName == GIPS_CODEC_ID_EG711A)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_IPCMA;
    }
    else if (codecName == GIPS_CODEC_ID_EG711U)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_IPCMU;
    }
    else if (codecName == GIPS_CODEC_ID_IPCMWB)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_IPCMWB;
    }
    else if (codecName == GIPS_CODEC_ID_ILBC)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_ILBC;
    }
    else if (codecName == GIPS_CODEC_ID_ISAC)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_ISAC;
    }
    else if (codecName == GIPS_CODEC_ID_ISAC_LC)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_ISAC_LC;
    }
    else if (codecName == GIPS_CODEC_ID_GSM)
    {
        codecType = SdpCodec::SDP_CODEC_GSM;
    }
    else if (codecName == GIPS_CODEC_ID_G723)
    {
        codecType = SdpCodec::SDP_CODEC_G723;
    }
    else if (codecName == GIPS_CODEC_ID_VP71_CIF)
    {
        codecType = SdpCodec::SDP_CODEC_VP71_CIF;
    }
    else if (codecName == GIPS_CODEC_ID_H263_CIF)
    {
        codecType = SdpCodec::SDP_CODEC_H263_CIF;
    }
#ifdef INCLUDE_QCIF_RESOLUTION
    else if (codecName == GIPS_CODEC_ID_VP71_QCIF)
    {
        codecType = SdpCodec::SDP_CODEC_VP71_QCIF;
    }
    else if (codecName == GIPS_CODEC_ID_H263_QCIF)
    {
        codecType = SdpCodec::SDP_CODEC_H263_QCIF;
    }
#endif
#ifdef INCLUDE_SQCIF_RESOLUTION
    else if (codecName == GIPS_CODEC_ID_VP71_SQCIF)
    {
        codecType = SdpCodec::SDP_CODEC_VP71_SQCIF;
    }
    else if (codecName == GIPS_CODEC_ID_H263_SQCIF)
    {
        codecType = SdpCodec::SDP_CODEC_H263_SQCIF;
    }
#endif
#ifdef INCLUDE_QVGA_RESOLUTION
    else if (codecName == GIPS_CODEC_ID_VP71_QVGA)
    {
        codecType = SdpCodec::SDP_CODEC_VP71_QVGA;
    }
#endif
#ifdef INCLUDE_VGA_RESOLUTION
    else if (codecName == GIPS_CODEC_ID_VP71_VGA)
    {
        codecType = SdpCodec::SDP_CODEC_VP71_VGA;
    }
#endif
#ifdef INCLUDE_4CIF_RESOLUTION
    else if (codecName == GIPS_CODEC_ID_VP71_4CIF)
    {
        codecType = SdpCodec::SDP_CODEC_VP71_4CIF;
    }
#endif
#ifdef INCLUDE_16CIF_RESOLUTION
    else if (codecName == GIPS_CODEC_ID_VP71_16CIF)
    {
        codecType = SdpCodec::SDP_CODEC_VP71_16CIF;
    }
#endif
    else if (codecName == GIPS_CODEC_ID_LSVX)
    {
        codecType = SdpCodec::SDP_CODEC_LSVX ;
    }

    if (codecType != SdpCodec::SDP_CODEC_UNKNOWN)
    {
        matchFound = TRUE;
    }

    return matchFound;
}

const bool VoiceEngineMediaInterface::isDisplayValid(const SIPXVE_VIDEO_DISPLAY* const pDisplay)
{
    bool bRet = false;
    if (pDisplay && pDisplay->cbSize && (pDisplay->handle || pDisplay->filter))
    {
        bRet = true;
    }
    return bRet;
}

UtlBoolean VoiceEngineMediaInterface::determineFileType(const char* szFile, GIPS_FileFormats& type) 
{
    UtlBoolean bRC = false ;
    FILE *fp;
    
    if ((fp=fopen(szFile,"rb")) != NULL)
    {
        char cBuffer[6] ;
        // Determine if file is PCM or WAV
        memset(cBuffer, 0, 6);
        fgets(cBuffer, 5, fp);
        if (strcmp(cBuffer, "RIFF") == 0)
        {
            type = FILE_WAV;
        }
        else
        {
            type = FILE_PCM_8KHZ;
        }
        fclose(fp);
        bRC = true ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////
const void* VoiceEngineMediaInterface::getVideoWindowDisplay()
{
    return mpDisplay;
}

UtlBoolean VoiceEngineMediaInterface::determineFileType(const char* pBuf, int nLength, GIPS_FileFormats& type) 
{
    UtlBoolean bRC = false ;

    if (nLength > 4)
    {
        char cBuffer[6] ;
        memset(cBuffer, 0, 6);
        memcpy(cBuffer, pBuf, 4);
        if (strcmp(cBuffer, "RIFF") == 0)
        {
            type = FILE_WAV;
        }
        else
        {
            type = FILE_PCM_8KHZ;
        }
        bRC = true ;
    }

    return bRC ;
}


/* ============================ FUNCTIONS ================================= */
