// 
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

#ifndef _CpTopologyGraphInterface_h_
#define _CpTopologyGraphInterface_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <net/SdpCodecFactory.h>
#include <mi/CpMediaInterface.h>
#include <net/QoS.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CpTopologyGraphFactoryImpl;
class CpTopologyMediaConnection;
class MpTopologyGraph;
class MpResourceTopology;
class MpResourceFactory;


/** Subsystem manager and creator of CpTopologyGraphInterfaces specialization of CpMediaInterface
 *
 */
class CpTopologyGraphInterface : public CpMediaInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    //! Default constructor
    CpTopologyGraphInterface(CpTopologyGraphFactoryImpl* factoryImpl,
                             const char* publicAddress = NULL, 
                             const char* localAddress = NULL,
                             int numCodecs = 0, 
                             SdpCodec* sdpCodecArray[] = NULL,
                             const char* pLocale = "",
                             int expeditedIpTos = QOS_LAYER3_LOW_DELAY_IP_TOS,
                             const char* stunServer = NULL,
                             int stunPort = PORT_NONE,
                             int stunKeepAlivePeriodSecs = 28,
                             const char* turnServer = NULL,
                             int turnPort = PORT_NONE,
                             const char* turnUsername = NULL,
                             const char* turnPassword = NULL,
                             int turnKeepAlivePeriodSecs = 28,
                             UtlBoolean enableIce = FALSE);
     

    //! Destructor
    virtual ~CpTopologyGraphInterface();

    void release();

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus createConnection(int& connectionId,
                                     const char* szLocalAddress,
                                     int localPort = 0,
                                     void* videoWindowHandle = NULL,
                                     void* const pSecurityAttributes = NULL,
                                     ISocketEvent* pIdleEvent = NULL,
                                     IMediaEventListener* pMediaEventListener = NULL,
                                     const RtpTransportOptions rtpTransportOptions=RTP_TRANSPORT_UDP);


      /// @copydoc CpMediaInterface::setMediaNotificationDispatcher()
   virtual OsMsgDispatcher*
   setMediaNotificationDispatcher(OsMsgDispatcher* pNoteDisper);

     /// @copydoc CpMediaInterface::setMediaNotificationsEnabled()
   virtual OsStatus setMediaNotificationsEnabled(bool enabled, 
                                                 const UtlString& resourceName = NULL);


   /// Look up the port on the bridge to which the indicated connection is connected
   OsStatus getConnectionPortOnBridge(int connectionId, 
                                      int& portOnBridge);

   virtual OsStatus setConnectionDestination(int connectionId,
                                             const char* rtpHostAddress, 
                                             int rtpAudioPort,
                                             int rtcpAudioPort,
                                             int rtpVideoPort,
                                             int rtcpVideoPort);

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
                              OsNotification *event = NULL);


    virtual OsStatus playBuffer(char* buf, 
                               unsigned long bufSize,
                               int type, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote,
                              OsProtectedEvent* event = NULL,
                              UtlBoolean mixWithMic = false,
                              int downScaling = 100);

    virtual OsStatus stopAudio();

    virtual OsStatus playChannelAudio(int connectionId,
                                     const char* url,
                                     UtlBoolean repeat,
                                     UtlBoolean local,
                                     UtlBoolean remote,
                                     UtlBoolean mixWithMic = false,
                                     int downScaling = 100,
                                     OsNotification *notify = NULL) ;


   virtual OsStatus stopChannelAudio(int connectionId) ;


   virtual OsStatus recordChannelAudio(int connectionId,
                                       const char* szFile) ;

   virtual OsStatus stopRecordChannelAudio(int connectionId) ;

   /// Deprecated
   virtual OsStatus createPlayer(MpStreamPlayer** ppPlayer, 
                                 const char* szStream, 
                                 int flags, 
                                 OsMsgQ *pMsgQ = NULL, 
                                 const char* szTarget = NULL) ;

   // Deprecated
   virtual OsStatus destroyPlayer(MpStreamPlayer* pPlayer);

   // Deprecated
   virtual OsStatus createPlaylistPlayer(MpStreamPlaylistPlayer** 
                                         ppPlayer, 
                                         OsMsgQ *pMsgQ = NULL, 
                                         const char* szTarget = NULL);

   // Deprecated
   virtual OsStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer);

   // Deprecated
   virtual OsStatus createQueuePlayer(MpStreamQueuePlayer** ppPlayer, 
                                      OsMsgQ *pMsgQ = NULL, 
                                      const char* szTarget = NULL);

   // Deprecated
   virtual OsStatus destroyQueuePlayer(MpStreamQueuePlayer* pPlayer);

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

     /// @copydoc CpMediaInterface::recordMic(UtlString*)
   virtual OsStatus recordMic(UtlString* pAudioBuffer);

     /// @copydoc CpMediaInterface::recordMic(int, int, const char*)
   virtual OsStatus recordMic(int ms,
                              int silenceLength,
                              const char* fileName);

    virtual void setContactType(int connectionId, SIPX_CONTACT_TYPE eType, SIPX_CONTACT_ID contactId) ;
     //: Set the contact type for this Phone media interface.  
     //  It is important to set the contact type BEFORE creating the 
     //  connection -- setting after the connection has been created
     //  is essentially a NOP.

    //! Rebuild the codec factory on the fly
    virtual OsStatus setAudioCodecBandwidth(int connectionId, int bandWidth) ;

   virtual OsStatus rebuildCodecFactory(int connectionId, 
                                        int audioBandwidth, 
                                        int videoBandwidth, 
                                        UtlString& videoCodec);

    //! Set conneection bitrate on the fly
    virtual OsStatus setConnectionBitrate(int connectionId, int bitrate) ;

    //! Set connection framerate on the fly
    virtual OsStatus setConnectionFramerate(int connectionId, int framerate) ;

    virtual OsStatus setSecurityAttributes(const void* security) ;

    virtual OsStatus addAudioRtpConnectionDestination(int         connectionId,
                                                      int         iPriority,
                                                      const char* candidateIp, 
                                                      int         candidatePort) ;

    virtual OsStatus addAudioRtcpConnectionDestination(int         connectionId,
                                                       int         iPriority,
                                                       const char* candidateIp, 
                                                       int         candidatePort) ;

    virtual OsStatus addVideoRtpConnectionDestination(int         connectionId,
                                                      int         iPriority,
                                                      const char* candidateIp, 
                                                      int         candidatePort) ;

    virtual OsStatus addVideoRtcpConnectionDestination(int         connectionId,
                                                       int         iPriority,
                                                       const char* candidateIp, 
                                                       int         candidatePort) ;
    
    virtual void setConnectionTcpRole(const int connectionId, const RtpTcpRoles role)
    {
        // NOT IMPLEMENTED
    }

	virtual OsStatus generateVoiceQualityReport(int         connectiond,
                                                const char* callId,
                                                UtlString&  report) ;

/* ============================ ACCESSORS ================================= */

    /** soon to be deprecated in favor of getCapabilitiesEx
     */
    virtual OsStatus getCapabilities(int connectionId, 
                                     UtlString& rtpHostAddress, 
                                     int& rtpAudioPort,
                                     int& rtcpAudioPort,
                                     int& rtpVideoPort,
                                     int& rtcpVideoPort, 
                                     SdpCodecFactory& supportedCodecs,
                                     SdpSrtpParameters& srtpParams,
                                     int bandWidth,
                                     int& videoBandwidth,
                                     int& videoFramerate);
    
   /**
    * replacement for getCapabilities
    */
   virtual OsStatus getCapabilitiesEx(int connectionId, 
                                      int nMaxAddresses,
                                      UtlString rtpHostAddresses[], 
                                      int rtpAudioPorts[],
                                      int rtcpAudioPorts[],
                                      int rtpVideoPorts[],
                                      int rtcpVideoPorts[],
                                      RTP_TRANSPORT transportTypes[],
                                      int& nActualAddresses,
                                      SdpCodecFactory& supportedCodecs,
                                      SdpSrtpParameters& srtpParameters,
                                      int bandWidth,
                                      int& videoBandwidth,
                                      int& videoFramerate);

   virtual void setPremiumSound(UtlBoolean enabled);

   //! Calculate the current cost for the current set of 
   //! sending/receiving codecs.
   virtual int getCodecCPUCost();

   //! Calculate the worst case cost for the current set of 
   //! sending/receiving codecs.
   virtual int getCodecCPULimit();

   //!Returns the flowgraph's message queue
   virtual OsMsgQ* getMsgQ();

   /// @copydoc CpMediaInterface::getMediaNotificationDispatcher()
   virtual OsMsgDispatcher* getMediaNotificationDispatcher();


   virtual OsStatus getVideoQuality(int& quality);

   virtual OsStatus getVideoBitRate(int& bitRate);

   virtual OsStatus getVideoFrameRate(int& frameRate);

   // Returns the primary codec for the connection
   virtual OsStatus getPrimaryCodec(int connectionId, 
                                    UtlString& audioCodec,
                                    UtlString& videoCodec,
                                    int* audiopPayloadType,
                                    int* videoPayloadType,
                                    bool& isEncrypted);

   virtual OsStatus setVideoWindowDisplay(const void* hWnd);

   virtual const void* getVideoWindowDisplay();

   virtual OsStatus getAudioEnergyLevels(int& iInputEnergyLevel,
                                         int& iOutputEnergyLevel)
        { return OS_NOT_SUPPORTED ;} ;

   virtual OsStatus getAudioEnergyLevels(int connectionId,
                                         int& iInputEnergyLevel,
                                         int& iOutputEnergyLevel,
                                         int& nContributors,
                                         unsigned int* pContributorSRCIds,
                                         int* pContributorEngeryLevels) 
        { return OS_NOT_SUPPORTED ;} ;

   virtual OsStatus getAudioRtpSourceIDs(int connectionId,
                                         unsigned int& uiSendingSSRC,
                                         unsigned int& uiReceivingSSRC) 
        { return OS_NOT_SUPPORTED ;} ;

   virtual OsStatus enableAudioTransport(int connectionId, UtlBoolean bEnable)
   {
       return OS_NOT_SUPPORTED; 
   };

   virtual OsStatus enableVideoTransport(int connectionId, UtlBoolean bEnable)
   {
       return OS_NOT_SUPPORTED; 
   };


   // Note: the followingproperties may be set or get, but have no effect
   // Set a media property on the media interface
    /*
     * Media interfaces that wish to interoperate should implement the following properties
     * and values:
     *
     * Property Name                  Property Values
     * =======================        ===============
     * "audioInput1.muteState"        "true", "false" for systems that may have a microphone for each conference or 2-way call
     * "audioInput1.device"           same value as szDevice in sipxAudioSetCallInputDevice
     * "audioOutput1.deviceType"      "speaker", "ringer" same as sipxAudioEnableSpeaker, but for specific conference or 2-way call
     * "audioOutput1.ringerDevice"    same value as szDevice in sipxAudioSetRingerOutputDevice 
     * "audioOutput1.speakerDevice"   same values as szDevice in sipxAudioSetCallOutputDevice
     * "audioOutput1.volume"          string value of iLevel in sipxAudioSetVolume
     */
   virtual OsStatus setMediaProperty(const UtlString& propertyName,
                                     const UtlString& propertyValue);

   //! Get a media property on the media interface
   virtual OsStatus getMediaProperty(const UtlString& propertyName,
                                     UtlString& propertyValue);

   //! Set a media property associated with a connection
   virtual OsStatus setMediaProperty(int connectionId,
                                     const UtlString& propertyName,
                                     const UtlString& propertyValue);

   //! Get a media property associated with a connection
   virtual OsStatus getMediaProperty(int connectionId,
                                     const UtlString& propertyName,
                                     UtlString& propertyValue);

   virtual UtlString getType() { return "CpTopologyGraphInterface"; };

/* ============================ INQUIRY =================================== */


   //! Query whether the specified media connection is enabled for 
   //! sending RTP.
   virtual UtlBoolean isSendingRtpAudio(int connectionId);

   //! Query whether the specified media connection is enabled for 
   //! sending RTP.
   virtual UtlBoolean isSendingRtpVideo(int connectionId);

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isReceivingRtpAudio(int connectionId);

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isReceivingRtpVideo(int connectionId);

   //! Query whether the specified media connection has a destination 
   //! specified for sending RTP.
   virtual UtlBoolean isDestinationSet(int connectionId);

   //! Query whether a new party can be added to this media interfaces
   virtual UtlBoolean canAddParty();

   //! Query whether the connection has started sending or receiving video
   virtual UtlBoolean isVideoInitialized(int connectionId);

   //! Query whether the connection has started sending or receiving audio
   virtual UtlBoolean isAudioInitialized(int connectionId);

   //! Query if the audio device is available.
   virtual UtlBoolean isAudioAvailable();

   //! Query if we are mixing a video conference
   virtual UtlBoolean isVideoConferencing(){return(FALSE);};

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    UtlBoolean getLocalAddresses( int connectionId,
                                  UtlString& hostIp,
                                  int& rtpAudioPort,
                                  int& rtcpAudioPort,
                                  int& rtpVideoPort,
                                  int& rtcpVideoPort) ;

    UtlBoolean getNatedAddresses( int connectionId,
                                  UtlString& hostIp,
                                  int& rtpAudioPort,
                                  int& rtcpAudioPort,
                                  int& rtpVideoPort,
                                  int& rtcpVideoPort) ;


    UtlBoolean getRelayAddresses( int connectionId,
                                  UtlString& hostIp,
                                  int& rtpAudioPort,
                                  int& rtcpAudioPort,
                                  int& rtpVideoPort,
                                  int& rtcpVideoPort) ;


    OsStatus addLocalContacts(  int connectionId, 
                                int nMaxAddresses,
                                UtlString rtpHostAddresses[], 
                                int rtpAudioPorts[],
                                int rtcpAudioPorts[],
                                int rtpVideoPorts[],
                                int rtcpVideoPorts[],
                                int& nActualAddresses) ;

    OsStatus addNatedContacts(  int connectionId, 
                                int nMaxAddresses,
                                UtlString rtpHostAddresses[], 
                                int rtpAudioPorts[],
                                int rtcpAudioPorts[],
                                int rtpVideoPorts[],
                                int rtcpVideoPorts[],
                                int& nActualAddresses) ;

    OsStatus addRelayContacts(  int connectionId, 
                                int nMaxAddresses,
                                UtlString rtpHostAddresses[], 
                                int rtpAudioPorts[],
                                int rtcpAudioPorts[],
                                int rtpVideoPorts[],
                                int rtcpVideoPorts[],
                                int& nActualAddresses) ;

    void applyAlternateDestinations(int connectionId) ;

      /// Create socket pair for RTP/RTCP streams.
    OsStatus createRtpSocketPair(UtlString localAddress,
                                 int localPort,
                                 SIPX_CONTACT_TYPE contactType,
                                 OsDatagramSocket* &rtpSocket,
                                 OsDatagramSocket* &rtcpSocket);
      /**<
      *  For RTP/RTCP port pair will be set next free port pair.
      *  
      *  @param[in] localAddress - Address to bind to (for multihomed hosts).
      *  @param[in] localPort - Local port to bind to (0 for auto select).
      *  @param[in] contactType - Contact type (see SIPX_CONTACT_TYPE).
      *  @param[out] rtpSocket - Created socket for RTP stream.
      *  @param[out] rtcpSocket - Created socket for RTCP stream.
      */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /** Get the next unique connection Id scoped to this flowgraph
     */
    int getNextConnectionId();

    CpTopologyMediaConnection* getMediaConnection(int connectionId);
    CpTopologyMediaConnection* removeMediaConnection(int connectionId);
    OsStatus doDeleteConnection(CpTopologyMediaConnection* mediaConnection);

    /** Disabled copy constructor
     */
    CpTopologyGraphInterface(CpTopologyGraphInterface&);

    /** Disabled assignment operator
     */
    CpTopologyGraphInterface& operator=(const CpTopologyGraphInterface&);


    int mLastConnectionId;
    MpResourceTopology* mpInitialResourceTopology;
    MpResourceFactory* mpResourceFactory;
    MpTopologyGraph* mpTopologyGraph;
    SdpCodecFactory mSupportedCodecs;
    UtlDList mMediaConnections;
    int mExpeditedIpTos;

    UtlString mStunServer;
    int mStunPort;
    int mStunRefreshPeriodSecs;
    UtlString mTurnServer;
    int mTurnPort;
    int mTurnRefreshPeriodSecs;
    UtlString mTurnUsername;
    UtlString mTurnPassword;
    UtlBoolean mEnableIce;

    UtlString mRtpReceiveHostAddress;
    UtlString mLocalAddress;
    UtlHashMap mInterfaceProperties;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpTopologyGraphInterface_h_
