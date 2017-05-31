// 
// Copyright (C) 2005-2017 SIPez LLC.  All rights reserved.
// 
// Copyright (C) 2004-2009 SIPfoundry Inc.
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
#include <sdp/SdpCodecList.h>
#include "mp/MpTypes.h"
#include "mi/CpMediaInterface.h"
#include <net/QoS.h>
#include "MaNotfTranslatorDispatcher.h"

// DEFINES
//#define HAVE_DELAY_API

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
class MpInputDeviceManager;
class MpOutputDeviceManager;
class OsSocket;
class CircularBufferPtr;

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
                             uint32_t samplesPerFrame,
                             uint32_t samplesPerSec,
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
                             UtlBoolean enableIce = FALSE,
                             OsMsgDispatcher* pDispatcher = NULL,
                             MpInputDeviceManager* pInputDeviceManager = NULL,
                             MpInputDeviceHandle inputDeviceHandle = 1,
                             UtlBoolean inputDeviceAlreadyEnabled = FALSE,
                             MpOutputDeviceManager* pOutputDeviceManager = NULL,
                             MpOutputDeviceHandle outputDeviceHandle = 1
                            );


    //! Destructor
    virtual ~CpTopologyGraphInterface();

    void release();

/* ============================ MANIPULATORS ============================== */

     /// @copydoc CpMediaInterface::createConnection()
   virtual OsStatus createConnection(int& connectionId,
                                     const char* szLocalAddress,
                                     int localPort = 0,
                                     void* videoWindowHandle = NULL,
                                     void* const pSecurityAttributes = NULL,
                                     const RtpTransportOptions rtpTransportOptions=RTP_TRANSPORT_UDP);

     /// @brief This is non-standard version of createConnection() to be used
     /// with custom OsSocket.
   virtual OsStatus createConnection(int& connectionId,
                                     OsSocket* rtpSocket,
                                     OsSocket* rtcpSocket,
                                     UtlBoolean isMulticast);
     /**<
     *  This version of createConnection() can be used if you want to override
     *  the OsSocket class in order to provide an external socket implementation.
     *
     *  @note In this case application should free socket instances by itself,
     *        because it is not freed in deleteConnection() method.
     */

     /// @copydoc CpMediaInterface::setPlcMethod()
   virtual OsStatus setPlcMethod(int connectionId,
                                 const UtlString &methodName="");
                                 
     /// @copydoc CpMediaInterface::setNotificationDispatcher()
   virtual
   OsMsgDispatcher* setNotificationDispatcher(OsMsgDispatcher* pNotificationDispatcher);

     /// @copydoc CpMediaInterface::setNotificationsEnabled()
   virtual OsStatus setNotificationsEnabled(bool enabled, 
                                            const UtlString& resourceName = NULL);

     /// Get number of bridge ports (-1 on failure).
   int getNumBridgePorts();

     /// Look up the port on the bridge to which the indicated connection is connected.
   OsStatus getConnectionPortOnBridge(int connectionId,
                                      int streamNum,
                                      int& portOnBridge);
     /**<
     *  @param[in]  connectionId - ID of connection to lookup.
     *  @param[in]  streamNum - number of an input RTP stream to lookup.
     *              Numbering starts from 0, and stream 0 is always present. 
     *  @param[out] portOnBridge - found bridge port or -1 on failure.
     *
     *  @note Output bridge port is always the same as input bridge port for
     *        stream 0.
     *
     *  @retval OS_SUCCESS on success
     *  @retval OS_NOT_FOUND if some error occurs.
     */

     /// Look up the port on the bridge to which the indicated resource is connected.
   OsStatus getResourceInputPortOnBridge(const UtlString &resourceName,
                                         int resourceOutputPortIndex,
                                         int& portOnBridge);
     /**<
     *  @warning You must be sure, that this resource is actually connected
     *           to the bridge.
     *
     *  @param[in]  resourceName - name of the resource to lookup.
     *  @param[in]  resourceOutputPortIndex - index of the resource's output port which
     *              is connected to the bridge.
     *  @param[out] portOnBridge - index of an input port to which resource
     *              is connected.
     */

     /// Look up the port on the bridge to which the indicated resource is connected.
   OsStatus getResourceOutputPortOnBridge(const UtlString &resourceName,
                                         int resourceInputPortIndex,
                                         int& portOnBridge);
     /**<
     *  @warning You must be sure, that this resource is actually connected
     *           to the bridge.
     *
     *  @param[in]  resourceName - name of the resource to lookup.
     *  @param[in]  resourceInputPortIndex - index of the resource's input port which
     *              is connected to the bridge.
     *  @param[out] portOnBridge - index of an input port to which resource
     *              is connected.
     */

     /// @copydoc CpMediaInterface::setMediaPassThrough()
   virtual OsStatus setMediaPassThrough(int connectionId,
                                        MEDIA_STREAM_TYPE mediaType,
                                        int mediaTypeStreamIndex,
                                        UtlString& receiveAddress,
                                        int rtpPort,
                                        int rtcpPort);

     /// @copydoc CpMediaInterface::setConnectionDestination()
   virtual OsStatus setConnectionDestination(int connectionId,
                                             const char* rtpHostAddress, 
                                             int rtpAudioPort,
                                             int rtcpAudioPort,
                                             int rtpVideoPort,
                                             int rtcpVideoPort);

     /// @copydoc CpMediaInterface::setConnectionDestination()
   virtual OsStatus setConnectionDestination(int connectionId,
                                             CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                             int streamIndex,
                                             const char* rtpHostAddress, 
                                             int rtpPort,
                                             int rtcpPort);

    /// @copydoc CpMediaInterface::copyPayloadIds
    virtual OsStatus copyPayloadIds(int connectionId, int numCodecs, SdpCodec* remoceCodecs[]);

     /// @copydoc CpMediaInterface::startRtpSend()
   virtual OsStatus startRtpSend(int connectionId, 
                                 int numCodecs,
                                 SdpCodec* sendCodec[]);

     /// @copydoc CpMediaInterface::setConnectionDestination()
   virtual OsStatus startRtpReceive(int connectionId,
                                    int numCodecs,
                                    SdpCodec* sendCodec[]);

     /// @copydoc CpMediaInterface::stopRtpSend()
   virtual OsStatus stopRtpSend(int connectionId);

     /// @copydoc CpMediaInterface::stopRtpReceive()
   virtual OsStatus stopRtpReceive(int connectionId);

     /// @copydoc CpMediaInterface::getConnectionCodecList()
   virtual const SdpCodecList* getConnectionCodecList(int connectionId);

     /// @copydoc CpMediaInterface::deleteConnection()
   virtual OsStatus deleteConnection(int connectionId);

     /// @copydoc CpMediaInterface::startTone()
   virtual OsStatus startTone(int toneId, UtlBoolean local, UtlBoolean remote);
     /// @copydoc CpMediaInterface::stopTone()
   virtual OsStatus stopTone();

     /// @copydoc CpMediaInterface::setRtcpTimeOffset()
   virtual OsStatus setRtcpTimeOffset(int connectionId,
                                      CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                      int streamIndex,
                                      int timeOffset);

     /// @copydoc CpMediaInterface::startChannelTone()
   virtual OsStatus startChannelTone(int connectionId, int toneId, UtlBoolean local, UtlBoolean remote) ;
     /// @copydoc CpMediaInterface::stopChannelTone()
   virtual OsStatus stopChannelTone(int connectionId) ;

     /// @copydoc CpMediaInterface::playAudio()
   virtual OsStatus playAudio(const char* url, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote,
                              UtlBoolean mixWithMic = false,
                              int downScaling = 100,
                              UtlBoolean autoStopAfterFinish = TRUE);


     /// @copydoc CpMediaInterface::playBuffer()
    virtual OsStatus playBuffer(char* buf, 
                               unsigned long bufSize,
                                uint32_t bufRate, 
                                int type, 
                                UtlBoolean repeat,
                                UtlBoolean local, 
                                UtlBoolean remote,
                                OsProtectedEvent* event = NULL,
                                UtlBoolean mixWithMic = false,
                                int downScaling = 100,
                                UtlBoolean autoStopOnFinish = TRUE);

     /// @copydoc CpMediaInterface::pauseAudio()
   virtual OsStatus pauseAudio();

     /// @copydoc CpMediaInterface::resumeAudio()
   virtual OsStatus resumeAudio();

   /// @copydoc CpMediaInterface::stopAudio()
   virtual OsStatus stopAudio();

   /// @copydoc CpMediaInterface::playCHannelAudio
   virtual OsStatus playChannelAudio(int connectionId,
                                     const char* url,
                                     UtlBoolean repeat,
                                     UtlBoolean local,
                                     UtlBoolean remote,
                                     UtlBoolean mixWithMic = false,
                                     int downScaling = 100,
                                     UtlBoolean autoStopOnFinish = TRUE);

   /// @copydoc CpMediaInterface::stopChannelAudio
   virtual OsStatus stopChannelAudio(int connectionId) ;

   /// @copydoc CpMediaInterface::recordChannelAudio
   virtual OsStatus recordChannelAudio(int connectionId,
                                       const char* szFile,
                                       CpAudioFileFormat cpFileFormat = CP_WAVE_PCM_16,
                                       UtlBoolean appendToFile = FALSE,
                                       int numChannels = 1);

   /// @copydoc CpMediaInterface::pauseRecordChannelAudio
   virtual OsStatus pauseRecordChannelAudio(int connectionId);

   /// @copydoc CpMediaInterface::resumeRecordChannelAudio
   virtual OsStatus resumeRecordChannelAudio(int connectionId);

   /// @copydoc CpMediaInterface::stopRecordChannelAudio
   virtual OsStatus stopRecordChannelAudio(int connectionId);

   /// @copydoc CpMediaInterface::recordBufferChannelAudio
   virtual OsStatus recordBufferChannelAudio(int connectionId,
                                             char* pBuffer,
                                             int bufferSize,
                                             int maxRecordTime = -1,
                                             int maxSilence = -1) ;

   /// @copydoc CpMediaInterface::stopRecordBufferAudio
   virtual OsStatus stopRecordBufferChannelAudio(int connectionId) ;

   virtual OsStatus recordCircularBufferChannelAudio(int connectionId,
                                                     CircularBufferPtr & buffer,
                                                     CpMediaInterface::CpAudioFileFormat recordingFormat,
                                                     unsigned long recordingBufferNotificationWatermark);

   virtual OsStatus stopRecordCircularBufferChannelAudio(int connectionId);

   /// Deprecated
   virtual OsStatus createPlayer(MpStreamPlayer** ppPlayer, 
                                 const char* szStream, 
                                 int flags, 
                                 OsMsgQ *pMsgQ = NULL, 
                                 const char* szTarget = NULL) ;

   /// Deprecated
   virtual OsStatus destroyPlayer(MpStreamPlayer* pPlayer);

   /// Deprecated
   virtual OsStatus createPlaylistPlayer(MpStreamPlaylistPlayer** 
                                         ppPlayer, 
                                         OsMsgQ *pMsgQ = NULL, 
                                         const char* szTarget = NULL);

   /// Deprecated
   virtual OsStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer);

   /// Deprecated
   virtual OsStatus createQueuePlayer(MpStreamQueuePlayer** ppPlayer, 
                                      OsMsgQ *pMsgQ = NULL, 
                                      const char* szTarget = NULL);

   /// Deprecated
   virtual OsStatus destroyQueuePlayer(MpStreamQueuePlayer* pPlayer);

   virtual OsStatus giveFocus();
   virtual OsStatus defocus();

   virtual void setCodecCPULimit(int iLimit);
     //:Limits the available codecs to only those within the designated
     //:limit.

     /// @copydoc CpMediaInterface::recordMic(int,int16_t*,int)
   virtual OsStatus recordMic(int ms, int16_t* pAudioBuf, int bufferSize);

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

    /// @copydoc CpMediaInterface::limitCodecs
    virtual OsStatus limitCodecs(int connectionId, const SdpCodecList& includeOnlyCodecList);

    virtual OsStatus rebuildCodecFactory(int connectionId, 
                                        int audioBandwidth, 
                                        int videoBandwidth, 
                                        UtlString& videoCodec);

    //! Set connection bitrate on the fly
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

      /// Enable/disable discontinuous transmission for given connection.
    OsStatus enableDtx(int connectionId, UtlBoolean enable);

      /// Set inactive time (in ms) after which RTP stream is actually marked inactive.
    OsStatus setRtpInactivityTimeout(int connectionId, int timeoutMs);

/* ============================ ACCESSORS ================================= */

    /** soon to be deprecated in favor of getCapabilitiesEx
     */
    virtual OsStatus getCapabilities(int connectionId, 
                                     UtlString& rtpHostAddress, 
                                     int& rtpAudioPort,
                                     int& rtcpAudioPort,
                                     int& rtpVideoPort,
                                     int& rtcpVideoPort, 
                                     SdpCodecList& supportedCodecs,
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
                                      SdpCodecList& supportedCodecs,
                                      SdpSrtpParameters& srtpParameters,
                                      int bandWidth,
                                      int& videoBandwidth,
                                      int& videoFramerate);

   //! Calculate the current cost for the current set of 
   //! sending/receiving codecs.
   virtual int getCodecCPUCost();

   //! Calculate the worst case cost for the current set of 
   //! sending/receiving codecs.
   virtual int getCodecCPULimit();

     // @copydoc CpMediaInterface::getSamplesPerSec()
   virtual uint32_t getSamplesPerSec();

     /// @copydoc CpMediaInterface::getSamplesPerFrame()
   virtual uint32_t getSamplesPerFrame();

     /// @copydoc CpMediaInterface::getMsgQ()
   virtual OsMsgQ* getMsgQ();

   /// @copydoc CpMediaInterface::getNotificationDispatcher()
   virtual OsMsgDispatcher* getNotificationDispatcher();

   /// @copydoc CpMediaInterface::setMixWeightForOutput
   virtual OsStatus setMixWeightForOutput(int bridgeOutputPort, float weight);

   /// @copydoc CpMediaInterface::setMixWeightsForOutput()
   virtual OsStatus setMixWeightsForOutput(int bridgeOutputPort, int numWeights, float weights[]);

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

     /// Get receiving and sending SSRCs for given connection.
   virtual OsStatus getAudioRtpSourceIDs(int connectionId,
                                         unsigned int& uiSendingSSRC,
                                         unsigned int& uiReceivingSSRC);
     /**<
     *  WARNING! uiReceivingSSRC is not filled right now. Someday it should
     *  be made an array and then we'll be able to fill it in.
     */

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

   /// Get a latency of the input part of the RTP or local connection
   virtual OsStatus getConnectionInputLatency(int &latency,
                                              MpConnectionID connectionId,
                                              int streamId=0);
   /**<
   *  @param[out] latency - returned latency value.
   *  @param[in]  connectionId - connection ID to get latency for. Use -1 for local mic.
   *  @param[in]  streamId - stream ID to get latency for. Use 0 for default.
   *
   *  @retval OS_SUCCESS if latency has been successfully calculated.
   *  @retval OS_NOT_FOUND if connection or stream is not found.
   */

   /// Get a latency of the output part of the RTP or local connection
   virtual OsStatus getConnectionOutputLatency(int &latency,
                                               MpConnectionID connectionId=-1);
   /**<
   *  @param[out] latency - returned latency value.
   *  @param[in]  connectionId - connection ID to get latency for. Use -1 for local speaker.
   *
   *  @retval OS_SUCCESS if latency has been successfully calculated.
   *  @retval OS_NOT_FOUND if connection or stream is not found.
   */

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
                                 UtlBoolean isMulticast,
                                 SIPX_CONTACT_TYPE contactType,
                                 OsSocket* &rtpSocket,
                                 OsSocket* &rtcpSocket);
      /**<
      *  For RTP/RTCP port pair will be set next free port pair.
      *  
      *  @param[in] localAddress - Address to bind to (for multihomed hosts).
      *  @param[in] localPort - Local port to bind to (0 for auto select).
      *  @param[in] isMulticast - Is requested stream multicast or not?
      *  @param[in] contactType - Contact type (see SIPX_CONTACT_TYPE).
      *  @param[out] rtpSocket - Created socket for RTP stream.
      *  @param[out] rtcpSocket - Created socket for RTCP stream.
      */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int mLastConnectionId;
   MpResourceFactory* mpResourceFactory;
   MpTopologyGraph* mpTopologyGraph;
   SdpCodecList mSupportedCodecs;
   UtlDList mMediaConnections;
   int mExpeditedIpTos;

   MpInputDeviceManager*  mpInputDeviceManager;
   MpInputDeviceHandle    mInputDeviceHandle;
   MpOutputDeviceManager* mpOutputDeviceManager;
   MpOutputDeviceHandle   mOutputDeviceHandle;

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
   MaNotfTranslatorDispatcher mTranslatorDispatcher;  ///< Dispatcher for translating
             ///< mediaLib notification messages into abstract MediaAdapter ones.
             ///< Only used if a dispatcher is set on this interface.

      /// Get the next unique connection Id scoped to this flowgraph
    int getNextConnectionId();

      /// Create media connection structure to store connection params.
    CpTopologyMediaConnection* createMediaConnection(int& connectionId, UtlBoolean isMcast);

      /// Free media connection structure.
    OsStatus deleteMediaConnection(CpTopologyMediaConnection* mediaConnection);

      /// Get media connection structure by its connectionID.
    CpTopologyMediaConnection* getMediaConnection(int connectionId);

      /// Enable discarding of our RTP stream, looped back to us.
    OsStatus discardLoopbackRtp(CpTopologyMediaConnection* mediaConnection);
      /**<
      *  This is the case, when you have to turn off similar OS functionality.
      *  E.g. this is the case, when you're running two or more connections
      *  on the same computer with the same multicast address and want to
      *  receive each others packets.
      */

      /// Stop receiving RTP on selected media connection.
    void stopRtpReceive(CpTopologyMediaConnection* mediaConnection);

      /// Stop receiving RTP on selected media connection.
    void stopRtpSend(CpTopologyMediaConnection* mediaConnection);

      /// Set mixing weight from all connection's streams to the given output port.
    OsStatus setConnectionWeightOnBridge(CpTopologyMediaConnection *mediaConnection,
                                         int destPort,
                                         float weight);

      /// Set mixing weight from all connection's streams to the given output port.
    OsStatus setConnectionToConnectionWeight(CpTopologyMediaConnection *srcConnection,
                                             int destConnectionId,
                                             float weight);

      /// Set the Mic gain.
    OsStatus setMicGain(float gain);

      /// Set mixing weight from the Mic to all other Bridge ports.
    OsStatus setMicWeightOnBridge(float weight);

      /// Disabled copy constructor
    CpTopologyGraphInterface(CpTopologyGraphInterface&);

      /// Disabled assignment operator
    CpTopologyGraphInterface& operator=(const CpTopologyGraphInterface&);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpTopologyGraphInterface_h_
