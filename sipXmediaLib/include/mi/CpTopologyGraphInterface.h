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
// Copyright (C) 2005-2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
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
#include "mediaBaseImpl/CpMediaInterface.h"
#include <net/QoS.h>
#include "MaNotfTranslatorDispatcher.h"
#include "ARS/ArsConnectionSocket.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CpTopologyGraphFactoryImpl;
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
                             uint32_t samplesPerFrame,
                             uint32_t samplesPerSec,
                             const ProxyDescriptor& stunServer,
                             const ProxyDescriptor& turnServer,
                             const char* publicAddress = NULL, 
                             const char* localAddress = NULL,
                             int numCodecs = 0, 
                             SdpCodec* sdpCodecArray[] = NULL,
                             const char* pLocale = "",
                             int expeditedIpTos = QOS_LAYER3_LOW_DELAY_IP_TOS,
                             UtlBoolean enableIce = FALSE
                            );
     

    //! Destructor
    virtual ~CpTopologyGraphInterface();

    void release();

/* ============================ MANIPULATORS ============================== */

     /// @copydoc CpMediaInterface::createConnection()
   virtual OsStatus createConnection(int& connectionId,
                                     bool bInitiating,
                                     const char* szLocalAddress,
                                     int localPort = 0,
                                     void* videoWindowHandle = NULL,
                                     unsigned long flags = 0,
                                     void* const pSecurityAttributes = NULL,
                                     ISocketEvent* pIdleEvent = NULL,
                                     IMediaEventListener* pMediaEventListener = NULL,
                                     const RtpTransportOptions rtpTransportOptions=RTP_TRANSPORT_UDP,
                                     int callHandle=0);

     /// @copydoc CpMediaInterface::setPlcMethod()
   virtual OsStatus setPlcMethod(int connectionId,
                                 const UtlString &methodName="");

     /// @copydoc CpMediaInterface::setNotificationDispatcher()
   virtual
   OsMsgDispatcher* setNotificationDispatcher(OsMsgDispatcher* pNotificationDispatcher);

     /// @copydoc CpMediaInterface::setNotificationsEnabled()
   virtual OsStatus setNotificationsEnabled(bool enabled, 
                                            const UtlString& resourceName = NULL);


   /// Look up the port on the bridge to which the indicated connection is connected
   OsStatus getConnectionPortOnBridge(int connectionId, 
                                      int& portOnBridge);

     /// @brief Add an alternate Audio RTP connection destination for 
     ///        this media interface.
    virtual OsStatus addAudioArsConnectionDestination(int         connectionId,
                                                      int         iPriority,
                                                      const char* candidateIp, 
                                                      int         candidatePort) 
    {
        return OS_NOT_SUPPORTED;
    }

    virtual OsStatus addVideoArsConnectionDestination(int         connectionId,
                                                      int         iPriority,
                                                      const char* candidateIp, 
                                                      int         candidatePort)
    {
        return OS_NOT_SUPPORTED;
    }

    OsStatus addArsContacts(
                int connectionId,
                UtlSList& audioContacts,
                bool bAddAudioToConnInfo,
                UtlSList& videoContacts,
                bool bAddVideoToConnInfo)
    {
        return OS_NOT_YET_IMPLEMENTED;
    }


   /**
    * Supplies the media layer with an outgoing RTP or RTCP data packet for
    * the current channel.   See IETF RFC 1889.
    * 
    * @param type The type of packet:  AUDIO or VIDEO, RTP or RTCP
    * @param pData The bytes of the RFC 1889 packet.
    * @param nData The size of the RFC 1889 packet, in bytes.
    */
    virtual void injectMediaPacket(const int channelId,
                                  const SIPX_MEDIA_PACKET_TYPE type,
                                  const char* const pData,
                                  const size_t len)
    {
        return;
    }

    virtual OsStatus setVideoWindowDisplay(int connectionId, const void* pDisplay)
    {
        return OS_NOT_SUPPORTED;
    }



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

     /// @copydoc CpMediaInterface::deleteConnection()
   virtual OsStatus deleteConnection(int connectionId);

     /// @copydoc CpMediaInterface::startTone()
   virtual OsStatus startTone(int toneId, UtlBoolean local, UtlBoolean remote);
     /// @copydoc CpMediaInterface::stopTone()
   virtual OsStatus stopTone();

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
                              OsNotification *event = NULL);


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
                               int downScaling = 100);

     /// @copydoc CpMediaInterface::pauseAudio()
   virtual OsStatus pauseAudio();

     /// @copydoc CpMediaInterface::resumeAudio()
   virtual OsStatus resumeAudio();

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

   virtual void addToneListener(OsNotification *pListener, int connectionId);

   virtual void removeToneListener(int connectionId);

   virtual OsStatus stopRecording();

   virtual OsStatus ezRecord(int ms, 
                             int silenceLength, 
                             const char* fileName, 
                             double& duration, 
                             int& dtmfterm,
                             OsProtectedEvent* ev = NULL);

     /// @copydoc CpMediaInterface::recordMic(int,UtlString*)
   virtual OsStatus recordMic(int ms, UtlString* pAudioBuffer);

     /// @copydoc CpMediaInterface::recordMic(int, int, const char*)
   virtual OsStatus recordMic(int ms,
                              int silenceLength,
                              const char* fileName);

    //! Rebuild the codec factory on the fly
    virtual OsStatus setAudioCodecBandwidth(int connectionId, int bandWidth) ;

   virtual OsStatus rebuildCodecFactory(int connectionId, 
                                        int audioBandwidth, 
                                        int videoBandwidth, 
                                        UtlString& videoCodec);

    //! Set connection bitrate on the fly
    virtual OsStatus setConnectionBitrate(int connectionId, int bitrate) ;

    //! Set connection framerate on the fly
    virtual OsStatus setConnectionFramerate(int connectionId, int framerate) ;

    virtual OsStatus setSecurityAttributes(const void* security) ;


    virtual OsStatus addVideoRtpConnectionDestination(int         connectionId,
                                                      int         iPriority,
                                                      const char* candidateIp, 
                                                      int         candidatePort) ;

    virtual void setConnectionTcpRole(const int connectionId, const RtpTcpRoles role)
    {
        // NOT IMPLEMENTED
    }

	virtual OsStatus generateVoiceQualityReport(int         connectiond,
                                                const char* callId,
                                                char*  szReport,
                                                size_t reprotSize);

/* ============================ ACCESSORS ================================= */

    /** soon to be deprecated in favor of getCapabilitiesEx
     */
   virtual OsStatus getCapabilities(int connectionId, 
                                    UtlSList* pAudioContacts,
                                    UtlSList* pVideoContacts,                                    
                                    SdpCodecList& supportedCodecs,
                                    SdpSrtpParameters& srtpParams,
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

   virtual OsStatus getMediaDeviceInfo(int connectionId,
                                       MediaDeviceInfo::MediaDeviceInfoType type,
                                       MediaDeviceInfo& info) ;

/* ============================ INQUIRY =================================== */
   //! Query whether the specified media connection is enabled for 
   //! sending RTP.
   virtual UtlBoolean isSendingRtpVideo(int connectionId);

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

      /// Create socket pair for RTP/RTCP streams.
    OsStatus createRtpSocketPair(UtlString localAddress,
                                 int localPort,
                                 SIPX_CONTACT_TYPE contactType,
                                 OsNatDatagramSocket* &rtpSocket,
                                 OsNatDatagramSocket* &rtcpSocket,
                                 CpMediaConnection* pMediaConnection);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    /** Get the next unique connection Id scoped to this flowgraph
     */
    int getNextConnectionId();

    CpMediaConnection* removeMediaConnection(int connectionId);
    OsStatus doDeleteConnection(CpMediaConnection* mediaConnection);

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
    int mExpeditedIpTos;

    UtlBoolean mEnableIce;
    UtlString mRtpReceiveHostAddress;
    UtlString mLocalAddress;
    UtlHashMap mInterfaceProperties;
    MaNotfTranslatorDispatcher mTranslatorDispatcher;  ///< Dispatcher for translating
      ///< mediaLib notification messages into abstract MediaAdapter ones.
      ///< Only used if a dispatcher is set on this interface.

    ArsConnectionSocket* getArsSocket(const ProxyDescriptor& arsProxy,
                                      const ProxyDescriptor& arsHttpsProxy,
                                      ARS_MIMETYPE mimeType,
                                      const char* szLocalAddress);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpTopologyGraphInterface_h_
