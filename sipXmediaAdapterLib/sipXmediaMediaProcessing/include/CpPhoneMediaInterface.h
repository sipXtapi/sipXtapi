//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _CpPhoneMediaInterface_h_
#define _CpPhoneMediaInterface_h_

// SYSTEM INCLUDES
//#include <>

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsDefs.h>
#include <net/QoS.h>
#include <net/SdpCodecFactory.h>
#include "mi/CpMediaInterface.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpCallFlowGraph;
class SdpCodec;
class OsDatagramSocket;
class CpPhoneMediaConnection;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CpPhoneMediaInterface : public CpMediaInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum OutputAudioDevice
   {
      UNKNOWN = 0x0,
      HANDSET = 0x1,
      SPEAKER = 0x2,
      HEADSET = 0x4
   };

/* ============================ CREATORS ================================== */

   CpPhoneMediaInterface(CpMediaInterfaceFactoryImpl* pFactoryImpl,
                         const char* publicAddress = NULL, 
                         const char* localAddress = NULL,
                         int numCodecs = 0, 
                         SdpCodec* sdpCodecArray[] = NULL,
                         const char* pLocale = "",
                         int expeditedIpTos = QOS_LAYER3_LOW_DELAY_IP_TOS,
                         const char* szStunServer = NULL,
                         int iStunPort = PORT_NONE,
                         int iStunKeepAlivePeriodSecs = 28,
                         const char* szTurnServer = NULL,
                         int iTurnPort = PORT_NONE,
                         const char* szTurnUsername = NULL,
                         const char* szTurnPassword = NULL,
                         int iTurnKeepAlivePeriodSecs = 28,
                         bool mbEnableICE = false);
     //:Default constructor

  protected:
   virtual
   ~CpPhoneMediaInterface();
     //:Destructor
  public:

   /**
    * public interface for destroying this media interface
    */ 
   void release();

/* ============================ MANIPULATORS ============================== */


   virtual OsStatus createConnection(int& connectionId,
                                     const char* szLocalAddress,
                                     void* videoWindowHandle, 
                                     void* const pSecurityAttributes = NULL,
                                     ISocketIdle* pIdleSink = NULL,
                                     IMediaEventListener* pMediaEventListener = NULL) ;
   
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
                                    int& videoFramerate) ;

   virtual OsStatus getCapabilitiesEx(int connectionId, 
                                      int nMaxAddresses,
                                      UtlString rtpHostAddresses[], 
                                      int rtpAudioPorts[],
                                      int rtcpAudioPorts[],
                                      int rtpVideoPorts[],
                                      int rtcpVideoPorts[],
                                      int& nActualAddresses,
                                      SdpCodecFactory& supportedCodecs,
                                      SdpSrtpParameters& srtpParameters,
                                      int bandWidth,
                                      int& videoBandwidth,
                                      int& videoFramerate);

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
                              int downScaling = 100);


    virtual OsStatus playBuffer(char* buf, 
                               unsigned long bufSize,
                               int type, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote,
                              OsNotification* event = NULL,
                              UtlBoolean mixWithMic = false,
                              int downScaling = 100);

    virtual OsStatus stopAudio();

    virtual OsStatus playChannelAudio(int connectionId,
                                     const char* url,
                                     UtlBoolean repeat,
                                     UtlBoolean local,
                                     UtlBoolean remote,
                                     UtlBoolean mixWithMic = false,
                                     int downScaling = 100) ;


   virtual OsStatus stopChannelAudio(int connectionId) ;


   virtual OsStatus recordChannelAudio(int connectionId,
                                       const char* szFile) ;

   virtual OsStatus stopRecordChannelAudio(int connectionId) ;

   virtual OsStatus createPlayer(MpStreamPlayer** ppPlayer, 
                                 const char* szStream, 
                                 int flags, 
                                 OsMsgQ *pMsgQ = NULL, 
                                 const char* szTarget = NULL) ;
   virtual OsStatus destroyPlayer(MpStreamPlayer* pPlayer);
   virtual OsStatus createPlaylistPlayer(MpStreamPlaylistPlayer** 
                                         ppPlayer, 
                                         OsMsgQ *pMsgQ = NULL, 
                                         const char* szTarget = NULL);
   virtual OsStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer);
   virtual OsStatus createQueuePlayer(MpStreamQueuePlayer** ppPlayer, 
                                      OsMsgQ *pMsgQ = NULL, 
                                      const char* szTarget = NULL);
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

    virtual void setContactType(int connectionId, CONTACT_TYPE eType, CONTACT_ID contactId) ;
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


/* ============================ ACCESSORS ================================= */

   virtual void setPremiumSound(UtlBoolean enabled);
   virtual int getCodecCPUCost();
      //:Calculate the current cost for our sending/receiving codecs

   virtual int getCodecCPULimit();
      //:Calculate the worst cost for our sending/receiving codecs

   virtual OsMsgQ* getMsgQ();
     //:Returns the flowgraph's message queue

   virtual OsStatus getVideoQuality(int& quality);
   virtual OsStatus getVideoBitRate(int& bitRate);
   virtual OsStatus getVideoFrameRate(int& frameRate);

     //:Returns primary codec for the connection
   virtual OsStatus setVideoWindowDisplay(const void* hWnd);
   virtual const void* getVideoWindowDisplay();

   virtual OsStatus setVideoQuality(int quality);
   virtual OsStatus setVideoParameters(int bitRate, int frameRate);

   // Returns the primary codec for the connection
   virtual OsStatus getPrimaryCodec(int connectionId, 
                                    UtlString& audioCodec,
                                    UtlString& videoCodec,
                                    int* audiopPayloadType,
                                    int* videoPayloadType,
                                    bool& isEncrypted) ;

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isSendingRtpAudio(int connectionId);
   virtual UtlBoolean isReceivingRtpAudio(int connectionId);
   virtual UtlBoolean isSendingRtpVideo(int connectionId);
   virtual UtlBoolean isReceivingRtpVideo(int connectionId);
   virtual UtlBoolean isDestinationSet(int connectionId);   
   virtual UtlBoolean canAddParty() ;
   virtual bool isVideoInitialized(int connectionId) ;
   virtual bool isAudioInitialized(int connectionId) ;
   virtual bool isAudioAvailable() ;
   virtual bool isVideoConferencing() { return false; } ;


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    bool getLocalAddresses( int connectionId,
                            UtlString& hostIp,
                            int& rtpAudioPort,
                            int& rtcpAudioPort,
                            int& rtpVideoPort,
                            int& rtcpVideoPort) ;

    bool getNatedAddresses( int connectionId,
                            UtlString& hostIp,
                            int& rtpAudioPort,
                            int& rtcpAudioPort,
                            int& rtpVideoPort,
                            int& rtcpVideoPort) ;


    bool getRelayAddresses( int connectionId,
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


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    CpPhoneMediaConnection* getMediaConnection(int connectionId);
    CpPhoneMediaConnection* removeMediaConnection(int connectionId);
    OsStatus doDeleteConnection(CpPhoneMediaConnection* mediaConnection);

   UtlString mRtpReceiveHostAddress; // Advertized as place to send RTP/RTCP
   UtlString mLocalAddress; // On which ports are bound
   MpCallFlowGraph* mpFlowGraph;
   UtlBoolean mRingToneFromFile;
   SdpCodecFactory mSupportedCodecs;
   UtlDList mMediaConnections;
   int mExpeditedIpTos;
   UtlString mStunServer ;
   int mStunPort;
   int mStunRefreshPeriodSecs ;
   UtlString mTurnServer ;
   int mTurnPort ;
   int mTurnRefreshPeriodSecs ;
   UtlString mTurnUsername ;
   UtlString mTurnPassword ;
   UtlBoolean mbEnableICE ;


   // Disabled copy constructor
   CpPhoneMediaInterface(const CpPhoneMediaInterface& rCpPhoneMediaInterface);

   // Disabled equals operator
   CpPhoneMediaInterface& operator=(const CpPhoneMediaInterface& rhs);
     

};

/* ============================ INLINE METHODS ============================ */
#endif  // _CpPhoneMediaInterface_h_
