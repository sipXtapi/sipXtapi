// Copyright 2007,2008 AOL LLC.
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

#ifndef _VIDEO_ENGINE_H
#define _VIDEO_ENGINE_H


// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "include/VoiceEngineMediaInterface.h"
#include "include/VoiceEngineDefs.h"
#include "utl/UtlSList.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlVoidPtr.h"

#define MAX_VE_CONNECTIONS                  4

#define VE_INIT_CODEC_ON_CREATE_CHANNEL     0   // HACK: Added to workaround a
                                                // a camera where changing 
                                                // resolutions caused a problem

// FORWARD DECLARATIONS
class VideoEngine;
class VoiceEngineFactoryImpl ;

//////////////////////////////////////////////////////////////////////////////

#if 0
class SetCaptureDeviceTask : public OsTask
{
public:
    SetCaptureDeviceTask(VideoEngine*            pVideoEngine, 
                         OsMutex*                pLock, 
                         VoiceEngineFactoryImpl* pFactoryImpl, 
                         const char*             szDevice) ;

    virtual ~SetCaptureDeviceTask() ;

    virtual int run(void* pArg) ;

    bool isRunning() const ;

protected:
    VideoEngine* mpVideoEngine;
    OsMutex* mpLock;
    UtlString mCameraRequested ;
    VoiceEngineFactoryImpl* mpFactoryImpl;   
    bool mbIsRunning ;
} ;
#endif

//////////////////////////////////////////////////////////////////////////////

class VideoEngine
{
#if 0
    friend SetCaptureDeviceTask ;
#endif

    /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    /* ============================ CREATORS ================================== */
    VideoEngine(GipsVideoEnginePlatform* pVideoEngine, bool bConsoleTrace) ;

    virtual ~VideoEngine() ;

    /* ============================ MANIPULATORS ============================== */

    void setGipsVideoEngine(GipsVideoEnginePlatform* pVideoEngine) ;
    void releaseVideoEngine() ;
    void setIgnoreCameraCapabilties(bool bIgnore) ;
    bool createChannel(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec)  ;
    void setUserAgent(int voiceChannel, const char* szUserAgent) ;
    bool destroyChannel(int voiceChannel) ;
    bool destroy() ;
    bool setTransport(int voiceChannel, GIPS_transport& transport) ;
    bool startPreview(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec) ;
    bool addReceiveCodec(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec) ;
    bool startRender(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec) ;
    bool startSend(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec) ; 
    bool pauseSend(int voiceChannel) ;
    bool pauseReceive(int voiceChannel) ;
    bool resumeSend(int voiceChannel) ;
    bool resumeReceive(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec) ;
    bool enableRTCP(int         voiceChannel, 
                    bool        bEnable,
                    const char* szRTCPCName) ;

    /* ============================ ACCESSORS ================================= */
    
    GipsVideoEnginePlatform* getVideoEngine() const ;
    bool getCaptureError() const ;
    void setCaptureError(bool bError) ;
    bool setCaptureDevice(VoiceEngineFactoryImpl* pImpl, const char* szDevice)  ;
    void setPreviewDisplay(SIPXVE_VIDEO_DISPLAY previewDisplay)  ;
    void setRemoteDisplay(int voiceChannel, SIPXVE_VIDEO_DISPLAY remoteDisplay) ;
    bool getCameraCapabilities(GIPSCameraCapability* pCaps) const;
    bool getMaxFramerate(int& frameRate)  const;
    bool getMaxResolution(int& width, int& height) const ;

    static void logCameraCapabilities(const char* szDevice, GIPSCameraCapability* pCaps) ;
    static void logCodec(const char* context, GIPSVideo_CodecInst* codec) ;

    /* ============================ INQUIRY =================================== */
 
    static bool isDisplayValid(const SIPXVE_VIDEO_DISPLAY* const pDisplay) ;

    bool isChannelCreated(int voiceChannel) const ;        
    bool isReceivePaused(int voiceChannel) const ;
    bool isSendPaused(int voiceChannel) const ;
    bool isCaptureDeviceSet() const ;

    //////////////////////////////////////////////////////////////////////////////

    bool isPreviewing() const ;
    
    /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    int checkRC(const char* szAPI, 
               int         connectionId,
               int         returnCode, 
               bool        bAssertOnError) ;
    void doUserAgentSpecific(int voiceChannel)  ;
    bool doUpdateReceiveCodec(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec) ;
    bool doUpdateSendCodec(int voiceChannel, GIPSVideo_CodecInst* pVideoCodec) ;
    bool doGetCameraCapabilities(const char* szDevice, GIPSCameraCapability* pCaps) ;
    bool doSetPreviewRenderer(int voiceChannel) ;
    bool doSetRemoteRenderer(int voiceChannel) ;

    // return true if all is running
    bool doStartup(int  voiceChannel,
                   bool bCreateChannel,
                   bool bStartPreview,
                   bool bStartRender,
                   bool bStartSend,
                   GIPSVideo_CodecInst* pVideoCodec) ;
 
    static void trace(OsSysLogPriority priority, const char* format, ...) ;

    /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    GipsVideoEnginePlatform* mpVideoEngine ;
    SIPXVE_VIDEO_DISPLAY     mPreviewDisplay ;
    bool                     mbAutoStart ;
    bool                     mbCaptureDeviceSet ;
    bool                     mbPreviewRendererSet ;
    bool                     mbRunning ;

    SIPXVE_VIDEO_DISPLAY     mRemoteDisplay[MAX_VE_CONNECTIONS] ;
    GIPSVideo_CodecInst      mSendCodec[MAX_VE_CONNECTIONS] ;
    UtlSList*                mReceiveCodecs[MAX_VE_CONNECTIONS] ;
    bool                     mbChannelCreated[MAX_VE_CONNECTIONS]  ;
    bool                     mbRemoteRendererSet[MAX_VE_CONNECTIONS] ;
    bool                     mbRendering[MAX_VE_CONNECTIONS] ;
    bool                     mbSending[MAX_VE_CONNECTIONS] ;
    bool                     mbReceivePaused[MAX_VE_CONNECTIONS] ;
    bool                     mbSendPaused[MAX_VE_CONNECTIONS] ;
    UtlString                mUserAgent[MAX_VE_CONNECTIONS] ;

    UtlString                mCameraRequested;
    UtlString                mCameraSelected;
    bool                     mbCameraError;

    bool                     mbHaveCameraCapabilities ;
    bool                     mbIgnoreCameraCapabilties ;
    GIPSCameraCapability     mCameraCapabilities ;
    static bool              sbConsoleTrace ;
    bool                     mbForceCodecReset ;   
    mutable OsMutex          mLock;
} ;

//////////////////////////////////////////////////////////////////////////////

#endif
