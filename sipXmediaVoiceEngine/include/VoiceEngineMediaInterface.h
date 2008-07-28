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

#ifndef _VoiceEngineMediaInterface_h_
#define _VoiceEngineMediaInterface_h_

// SYSTEM INCLUDES
//#include <>
#ifdef WIN32
#ifndef WINCE
#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif 
#endif

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsDefs.h>
#include <os/OsMutex.h>
#include <os/OsNatDatagramSocket.h>
#include <os/OsSysLog.h>
#include <net/QoS.h>
#include <sdp/SdpCodecList.h>
#include "net/ProxyDescriptor.h"
#include "include/VoiceEngineDefs.h"
#include "include/VoiceEngineFactoryImpl.h"
#include "mediaBaseImpl/CpMediaInterface.h"
#include "mediaInterface/IMediaSocket.h"
#include "ARS/ArsConnectionSocket.h"
#include "tapi/sipXtapi.h"

#define MAX_VIDEO_CONNECTIONS     4


/**
* Type for storing a "window object handle" - in Windows,
* the application should cast their HWND to a SIPX_WINDOW_HANDLE.
*/
typedef void* SIPXVE_WINDOW_HANDLE;

/**
* Enum for specifying the type of display object
* to be used for displaying video
*/
typedef enum SIPXVE_VIDEO_DISPLAY_TYPE
{
    SIPXVE_WINDOW_HANDLE_TYPE,     /**< A handle to the window for
                                   the remote video display */
                                   SIPXVE_DIRECT_SHOW_FILTER      /**< A DirectShow render filter object for
                                                                  handling the remote video display */
} SIPXVE_VIDEO_DISPLAY_TYPE;

#ifdef WIN32
#ifndef WINCE
#undef WIN32_LEAN_AND_MEAN
#include <unknwn.h>
#include <strmif.h>
#endif 
#endif
/**
* Structure used to pass window handle/filter interface for video calls.
*/
struct SIPXVE_VIDEO_DISPLAY
{
    /** Default constructor */
    SIPXVE_VIDEO_DISPLAY()
    {
        cbSize = sizeof(SIPXVE_VIDEO_DISPLAY);
        type = SIPXVE_WINDOW_HANDLE_TYPE;
        handle = NULL;
    }

    ~SIPXVE_VIDEO_DISPLAY()
    {
        if (type == SIPXVE_DIRECT_SHOW_FILTER)
        {
#ifdef      _WIN32

            if (filter) 
            {   
                ((IUnknown*)filter)->Release() ; 
            }
#endif              
        }
    }

    /** Copy constructor */
    SIPXVE_VIDEO_DISPLAY(const SIPXVE_VIDEO_DISPLAY& ref)
    {
        cbSize = sizeof(SIPXVE_VIDEO_DISPLAY);
        type = SIPXVE_WINDOW_HANDLE_TYPE;
        handle = NULL;

        copy(ref);
    }

    SIPXVE_VIDEO_DISPLAY& operator=(const SIPXVE_VIDEO_DISPLAY& ref)
    {
        // check for assignment to self
        if (this == &ref) return *this;
        copy(ref);
        return *this;
    }    

    int cbSize;						/**< Size of structure */
    SIPXVE_VIDEO_DISPLAY_TYPE type;	/**< Type of video display */
    union
    {
        SIPXVE_WINDOW_HANDLE handle;	/**< Window handle if type SIPX_WINDOW_HANDLE_TYPE */
        void* filter;	                /**< Direct Show filter if type is DIRECT_SHOW_FILTER */
    };
private:
    void copy(const SIPXVE_VIDEO_DISPLAY& ref)
    {
#ifdef _WIN32
        IBaseFilter* oldFilter = NULL ;
        if (type == DIRECT_SHOW_FILTER)
            oldFilter = (IBaseFilter*) filter ;
#endif
        cbSize = ref.cbSize;
        type = ref.type;
        handle = ref.handle;

#ifdef _WIN32
        if (type == DIRECT_SHOW_FILTER)
        {
            if (filter) 
            {                
                ((IBaseFilter*)filter)->AddRef() ;
            }
        }
        if (oldFilter) 
            oldFilter->Release() ; 
#endif              
    }
};

// DEFINES
#define GIPS_CODEC_ID_IPCMWB      "IPCMWB"
#define GIPS_CODEC_ID_ISAC        "ISAC"
#define GIPS_CODEC_ID_ISAC_LC     "ISACLC"
#define GIPS_CODEC_ID_EG711U      "EG711U"
#define GIPS_CODEC_ID_EG711A      "EG711A"
#define GIPS_CODEC_ID_PCMA        "PCMA"
#define GIPS_CODEC_ID_PCMU        "PCMU"
#define GIPS_CODEC_ID_ILBC        "iLBC"
#define GIPS_CODEC_ID_G729        "G729"
#define GIPS_CODEC_ID_GSM         "GSM"
#define GIPS_CODEC_ID_G723        "G723"
#define GIPS_CODEC_ID_VP71_CIF    "VP71-CIF"
#define GIPS_CODEC_ID_VP71_QCIF   "VP71-QCIF"
#define GIPS_CODEC_ID_VP71_SQCIF  "VP71-SQCIF"
#define GIPS_CODEC_ID_VP71_QVGA   "VP71-QVGA"
#define GIPS_CODEC_ID_VP71_VGA    "VP71-VGA"
#define GIPS_CODEC_ID_VP71_4CIF   "VP71-4CIF"
#define GIPS_CODEC_ID_VP71_16CIF  "VP71-16CIF"
#define GIPS_CODEC_ID_H263_CIF    "H263-CIF"
#define GIPS_CODEC_ID_H263_QCIF   "H263-QCIF"
#define GIPS_CODEC_ID_H263_SQCIF  "H263-SQCIF"
#define GIPS_CODEC_ID_LSVX        "LSVX"
#define GIPS_CODEC_ID_TELEPHONE   "audio/telephone-event"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SdpCodec;
class OsDatagramSocket;
class VoiceEngineFactoryImpl;
class CpMediaConnection;
class GipsVoiceEngineLib ;
class gipsve_error_callback;
class VoiceEngineNetTask ;
class VideoEngine;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class VoiceEngineMediaInterface : public CpMediaInterface
{
    /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend class gipsve_error_callback;
    /* ============================ CREATORS ================================== */

    VoiceEngineMediaInterface(IMediaDeviceMgr* pFactoryImpl,
            GipsVoiceEngineLib* pVoiceEngine,
            GipsVideoEnginePlatform* pVideoEngine,
            const char* publicAddress = NULL, 
            const char* localAddress = NULL,
            int numCodecs = 0, 
            SdpCodec* sdpCodecArray[] = NULL,
            const char* pLocale = "",
            int expeditedIpTos = QOS_LAYER3_LOW_DELAY_IP_TOS,
            const ProxyDescriptor& stunServer = ProxyDescriptor(),
            const ProxyDescriptor& turnProxy = ProxyDescriptor(),
            const ProxyDescriptor& arsProxy = ProxyDescriptor(),
            const ProxyDescriptor& arsHttpProxy = ProxyDescriptor(),
            UtlBoolean bDTMFOutOfBand=TRUE,
            UtlBoolean bDTMFInBand=TRUE,
            UtlBoolean bEnableRTCP=TRUE,
            const char* szRtcpName=NULL,
            SIPX_MEDIA_PACKET_CALLBACK pMediaPacketCallback=NULL);

    virtual
        ~VoiceEngineMediaInterface();
    //:Destructor


    /**
    * public interface for destroying this media interface
    */ 
    virtual void release();      

    /* ============================ MANIPULATORS ============================== */

    virtual OsStatus createConnection(int& connectionId,
        bool bInitiating,
        const char* szLocalAddress,
        int localPort,
        void* videoWindowHandle,
        unsigned long flags,
        void* const pSecurityAttributes = NULL,
        ISocketEvent* pSocketIdleSink = NULL,
        IMediaEventListener*  pMediaEventListener = NULL,
        const RtpTransportOptions rtpTransportOptions=RTP_TRANSPORT_UDP,
        int callHandle=0);

    virtual OsStatus setUserAgent(int         connectionId,
                                 const char* szUserAgent) ;

    virtual OsStatus getCapabilities(int connectionId, 
        UtlSList* pAudioContacts,
        UtlSList* pVideoContacts,
        SdpCodecList& supportedCodecs,
        SdpSrtpParameters& srtpParams,
        int bandWidth,
        int& videoBandwidth,
        int& videoFramerate);

    virtual OsStatus setVideoWindowDisplay(int connectionId, 
                                           const void* pDisplay);
    virtual const void* getVideoWindowDisplay();

    const bool isDisplayValid(const SIPXVE_VIDEO_DISPLAY* const pDisplay);


     /// @brief Returns the sample rate of the flowgraph.
   virtual uint32_t getSamplesPerSec()
   {
       return OS_NOT_SUPPORTED;
   }

     /// @brief Returns the samples per frame of the flowgraph.
   virtual uint32_t getSamplesPerFrame()
   {
       return OS_NOT_SUPPORTED;
   }

     /// @brief Returns the Media Notification dispatcher this controls.
   virtual OsMsgDispatcher* getNotificationDispatcher()
   {
       return NULL;
   }

    virtual OsStatus setSecurityAttributes(const void* pSecurity);

    virtual OsStatus startRtpSend(int connectionId, 
        int numCodecs,
        SdpCodec* sendCodec[]);
    virtual OsStatus startRtpReceive(int connectionId,
        int numCodecs,
        SdpCodec* sendCodec[]);

    virtual OsStatus stopRtpSend(int connectionId);
    virtual OsStatus stopRtpReceive(int connectionId);

    virtual OsStatus deleteConnection(int connectionId);

    virtual OsStatus startTone(int toneId, UtlBoolean local, UtlBoolean remote);
    virtual OsStatus stopTone();

    virtual OsStatus startChannelTone(int connectionId, int toneId, UtlBoolean local, UtlBoolean remote) ;
    virtual OsStatus stopChannelTone(int connectionId) ;

    virtual OsStatus playAudio(const char* url, 
        UtlBoolean repeat,
        UtlBoolean local, 
        UtlBoolean remote,
        UtlBoolean mixWithMic = false,
        int downScaling = 100,
        OsNotification *event = NULL) ;

    virtual OsStatus stopAudio();

    virtual OsStatus playChannelAudio(int connectionId,
        const char* url,
        UtlBoolean repeat,
        UtlBoolean local,
        UtlBoolean remote,
        UtlBoolean mixWithMic = false,
        int downScaling = 100,
        OsNotification *event = NULL) ;


    virtual OsStatus stopChannelAudio(int connectionId) ;

    virtual void injectMediaPacket(const int channelId,
                                  const SIPX_MEDIA_PACKET_TYPE type,
                                  const char* const pData,
                                  const size_t len);

    virtual OsStatus recordChannelAudio(int connectionId,
        const char* szFile) ;

    virtual OsStatus stopRecordChannelAudio(int connectionId) ;

   virtual OsStatus playBuffer(char* buf, 
                               unsigned long bufSize,
                               uint32_t bufRate, 
                               int type, 
                               UtlBoolean repeat,
                               UtlBoolean local, 
                               UtlBoolean remote,
                               OsProtectedEvent* event = NULL,
                               UtlBoolean mixWithMic = false,
                               int downScaling = 100);

    virtual OsStatus pauseAudio();   

   virtual OsStatus resumeAudio()
   {
       return OS_NOT_SUPPORTED;
   }

    virtual OsStatus createPlayer(MpStreamPlayer** ppPlayer, 
            const char* szStream, 
            int flags, 
            OsMsgQ *pMsgQ = NULL, 
            const char* szTarget = NULL) 
        { return OS_NOT_SUPPORTED ; } ;

    virtual OsStatus destroyPlayer(MpStreamPlayer* pPlayer)
        { return OS_NOT_SUPPORTED ; } ;

    virtual OsStatus createPlaylistPlayer(MpStreamPlaylistPlayer** 
            ppPlayer, 
            OsMsgQ *pMsgQ = NULL, 
            const char* szTarget = NULL)
        { return OS_NOT_SUPPORTED ; } ;

    virtual OsStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer)
        { return OS_NOT_SUPPORTED ; } ;
    virtual OsStatus createQueuePlayer(MpStreamQueuePlayer** ppPlayer, 
            OsMsgQ *pMsgQ = NULL, 
            const char* szTarget = NULL)
        { return OS_NOT_SUPPORTED ; } ;
    virtual OsStatus destroyQueuePlayer(MpStreamQueuePlayer* pPlayer)
        { return OS_NOT_SUPPORTED ; } ;

    virtual OsStatus giveFocus();
    virtual OsStatus defocus();

    virtual void setCodecCPULimit(int iLimit);
    //:Limits the available codecs to only those within the designated
    //:limit.


    virtual void addToneListener(OsNotification *pListener, int connectionId);
    virtual void removeToneListener(int connectionId);

    virtual OsStatus stopRecording();

    virtual OsStatus ezRecord(int ms, 
        int silenceLength, 
        const char* fileName, 
        double& duration, 
        int& dtmfterm,
        OsProtectedEvent* ev = NULL);

    void* const getVideoEnginePtr();   

    void setVQMonAddresses(int connectionId) ;



    virtual OsStatus setRemoteVolumeScale(const int connectionId,
        const int scale = 100) ;     

    bool resetVideoCaptureDevice() ;

//    void receiveMedia(int connectionId, bool bAudio, bool bRtp, const char* pData, int nData) ;

    /* ============================ ACCESSORS ================================= */

    virtual OsMsgQ* getMsgQ();
    //:Returns the flowgraph's message queue

    virtual OsStatus getAudioEnergyLevels(int& iInputEnergyLevel,
        int& iOutputEnergyLevel) ;

    virtual OsStatus getAudioEnergyLevels(int connectionId,
        int& iInputEnergyLevel,
        int& iOutputEnergyLevel,
        int& nContributors,
        unsigned int* pContributorSRCIds,
        int* pContributorEngeryLevels) ;

    virtual OsStatus getAudioRtpSourceIDs(int connectionId,
        unsigned int& uiSendingSSRC,
        unsigned int& uiReceivingSSRC) ;

    virtual OsStatus getAudioRtcpStats(const int connectionId,
        SIPX_RTCP_STATS* const pStats);

    virtual OsStatus generateVoiceQualityReport(int         connectionId,
        const char* callId,
        char*  szReport,
        size_t reprotSize) ;

    /* ============================ INQUIRY =================================== */

    virtual UtlBoolean isAudioAvailable();

    OsStatus muteMicrophone(const bool bMute);

    virtual UtlBoolean canAddParty() ;
    void* const getAudioEnginePtr();

     /// @copydoc CpMediaInterface::setPlcMethod()
   virtual OsStatus setPlcMethod(int connectionId,
                                 const UtlString &methodName="")
   {
        return OS_NOT_SUPPORTED;
   }
   virtual OsMsgDispatcher* setNotificationDispatcher(OsMsgDispatcher* pNotificationDispatcher)
   {
        return NULL;
   }

   virtual OsStatus
   setNotificationsEnabled(bool enabled, 
                           const UtlString& resourceName = NULL)
   {
       return OS_NOT_SUPPORTED;
   }

   virtual OsStatus recordMic(int ms,
                              int silenceLength,
                              const char* fileName)
   {
       return OS_NOT_SUPPORTED;
   }

   virtual OsStatus setMediaProperty(const UtlString& propertyName,
                                     const UtlString& propertyValue)
   {
       return OS_NOT_SUPPORTED;
   }

   //! Get a media property on the media interface
   virtual OsStatus getMediaProperty(const UtlString& propertyName,
                                     UtlString& propertyValue)
   {
       return OS_NOT_SUPPORTED;
   }

   //! Set a media property associated with a connection
   virtual OsStatus setMediaProperty(int connectionId,
                                     const UtlString& propertyName,
                                     const UtlString& propertyValue)
   {
       return OS_NOT_SUPPORTED;
   }

   //! Get a media property associated with a connection
   virtual OsStatus getMediaProperty(int connectionId,
                                     const UtlString& propertyName,
                                     UtlString& propertyValue)
   {
       return OS_NOT_SUPPORTED;
   }

     ///< Get the specific type of this media interface
   virtual UtlString getType()
   {
       return "VoiceEngineMediaInterface";
   }
    /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    void startAudioSocketSupport(CpMediaConnection* pMediaConn) ;
    void startVideoSocketSupport(CpMediaConnection* pMediaConn) ;

    void finalizeVideoSupport(int connectionId);


    virtual int checkVoiceEngineReturnCode(const char* szAPI, 
        int         connectionId,
        int         returnCode, 
        bool        bAssertOnError) ;

    virtual int checkVideoEngineReturnCode(const char* szAPI,
        int connectionId,
        int         returnCode, 
        bool        bAssertOnError) ;

    virtual void enableConferencing(int connectionId);
    virtual void disableConferencing(int connectionId);
    UtlBoolean determineFileType(const char* szFile, GIPS_FileFormats& type) ;
    UtlBoolean determineFileType(const char* pBuf, int nLength, GIPS_FileFormats& type) ;

    // normalize internal (GIPS) payload types
    virtual void normalizeInternalPayloadTypes(int connectionId, SdpCodecList* pFactory) ;
    virtual void normalizeInternalPayloadTypes(int connectionId, int numCodecs, SdpCodec* codecs[]) ;

    OsStatus getMaxVideoResolution(SdpCodecList& supportedCodecs, 
                                   int&             width, 
                                   int&             height) ;

    OsStatus getMaxVideoCodec(SdpCodecList& supportedCodecs, 
                              const SdpCodec*& codec) ;



    /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    CpMediaConnection* removeMediaConnection(int connecctionId);
    void renderVideoQuadrant(int handle, int connectionId);

    UtlBoolean getCodecTypeByName(const UtlString& codecName, SdpCodec::SdpCodecTypes& codecType);
    UtlBoolean getVoiceEngineCodec(const SdpCodec& pCodec, GIPSVE_CodecInst& cInst);
    UtlBoolean getVideoEngineCodec(const SdpCodec& pCodec, GIPSVideo_CodecInst& codecInfo);

    int getVideoQuadrantHandle(int connectionId);
    void releaseVideoQuadrantHandle(int handle);
    int findVideoQuadrantHandle(int connectionId);
    void clearVideoQuadrantHandles();

    UtlString mRtpReceiveHostAddress ; 
    UtlString mLocalAddress ; 
    UtlBoolean mRingToneFromFile ;
    int mExpeditedIpTos ;
    bool mbCameraUnplugged;

    UtlBoolean mbFocus ;
    UtlBoolean mbDTMFInBand;
    UtlBoolean mbDTMFOutOfBand;
    UtlString  mRtcpName;
    GipsVoiceEngineLib* mpVoiceEngine ;
    GipsVideoEnginePlatform* mpGipsVideoEngine ;
    VideoEngine* mpVideoEngine ;
    SIPXVE_VIDEO_DISPLAY* mpDisplay;
    void startRtpReceiveVideo(int channelId);
    gipsve_error_callback* mpVoiceErrorCallback;
    int mVideoQuadrants[MAX_VIDEO_CONNECTIONS];
    VoiceEngineFactoryImpl* mpVEFactoryImpl;
};

/* ============================ INLINE METHODS ============================ */


#endif  // _VoiceEngineMediaInterface_h_
