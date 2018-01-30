//
// Copyright (C) 2005-2017 SIPez LLC. All rights reserved.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Daniel Petrie dpetrie AT SIPez DOT com

#ifndef _CallManager_h_
#define _CallManager_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <cp/CpCallManager.h>
#include <cp/Connection.h>
#include <mi/CpMediaInterfaceFactoryImpl.h>
#include <net/QoS.h>

#include <tao/TaoObjectMap.h>
#include <os/OsProtectEventMgr.h>
#include <tapi/sipXtapiInternal.h>

// DEFINES
// Much larger value needed when running under Valgrind on an embedded dev board:
// #define LONG_EVENT_RESPONSE_TIMEOUTS
#ifdef LONG_EVENT_RESPONSE_TIMEOUTS
#  define CP_MAX_EVENT_WAIT_SECONDS    2592000    // 30 days in seconds
#else
#  define CP_MAX_EVENT_WAIT_SECONDS    30         // time out, seconds
#endif

#define CP_CALL_HISTORY_LENGTH 50

#define CP_MAXIMUM_RINGING_EXPIRE_SECONDS 180
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS


// FORWARD DECLARATIONS
class SdpCodec;
class CpCall;
class SipUserAgent;
class OsConfigDb;
class PtMGCP;
class TaoObjectMap;
class TaoReference;
class SdpCodecList;
class CpMultiStringMessage;
class SipSession;
class SipDialog;
class SipLineMgr;
class CpMediaInterfaceFactory;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CallManager : public CpCallManager
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   CallManager(UtlBoolean isRequiredUserIdMatch,
               SipLineMgr* lineMgrTask,
               UtlBoolean isEarlyMediaFor180Enabled,
               SdpCodecList* pCodecFactory,
               int rtpPortStart,
               int rtpPortEnd,
               const char* localAddress,
               const char* publicAddress,
               SipUserAgent* userAgent,
               int sipSessionReinviteTimer,               // Suggested value: 0
               PtMGCP* mgcpStackTask,                     // Suggested value: NULL
               const char* defaultCallExtension,          // Suggested value: NULL
               int availableBehavior,                     // Suggested value: Connection::RING
               const char* unconditionalForwardUrl,       // Suggested value: NULL
               int forwardOnNoAnswerSeconds,              // Suggested value: -1
               const char* forwardOnNoAnswerUrl,          // Suggested value: NULL
               int busyBehavior,                          // Suggested value: Connection::BUSY
               const char* sipForwardOnBusyUrl,           // Suggested value: NULL
               OsConfigDb* speedNums,                     // Suggested value: NULL
               CallTypes phonesetOutgoingCallProtocol,    // Suggested value: CallManager::SIP_CALL
               int numDialPlanDigits,                     // Suggested value: 4
               int holdType,                              // Suggested value: CallManager::NEAR_END_HOLD
               int offeringDelay,                         // Suggested value: Connection::IMMEDIATE
               const char* pLocal,                        // Suggested value: ""
               int inviteExpireSeconds,                   // Suggested value: CP_MAXIMUM_RINGING_EXPIRE_SECONDS
               int expeditedIpTos,                        // Suggested value: QOS_LAYER3_LOW_DELAY_IP_TOS
               int maxCalls,                              // Suggested value: 10
               CpMediaInterfaceFactory* pMediaFactory,    // Suggested value: NULL
               int   internalSamplerate);                 // Suggested value: 8000, 16000 or 32000

   virtual
   ~CallManager();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

    virtual UtlBoolean handleMessage(OsMsg& eventMessage);

    virtual void requestShutdown(void);

    virtual void setOutboundLine(const char* lineUrl);
    virtual void setOutboundLineForCall(const char* callId, const char* address, SIPX_CONTACT_TYPE eType = CONTACT_AUTO);

    // Operations for calls
    virtual void createCall(UtlString* callId,
                            int metaEventId = 0,
                            int metaEventType = PtEvent::META_EVENT_NONE,
                            int numMetaEventCalls = 0,
                            const char* metaEventCallIds[] = NULL,
                            UtlBoolean assumeFocusIfNoInfocusCall = TRUE);
    virtual OsStatus getCalls(int maxCalls, int& numCalls, UtlString callIds[]);

    virtual PtStatus connect(const char* callId,
                             const char* toAddress,
                             const char* fromAddress = NULL,
                             const char* desiredConnectionCallId = NULL,
                             SIPX_CONTACT_ID contactId = 0,
                             const void* pDisplay = NULL,
                             const void* pSecurity = NULL,
                             const char* locationHeader = NULL,
                             const int bandWidth=AUDIO_CODEC_BW_DEFAULT,
                             SIPX_TRANSPORT_DATA* pTransportData = NULL,
                             const RTP_TRANSPORT rtpTransportOptions = RTP_TRANSPORT_UDP) ;

    virtual PtStatus consult(const char* idleTargetCallId,
        const char* activeOriginalCallId, const char* originalCallControllerAddress,
        const char* originalCallControllerTerminalId, const char* consultAddressUrl,
        UtlString& targetCallControllerAddress, UtlString& targetCallConsultAddress);
    virtual void drop(const char* callId);
    virtual PtStatus transfer_blind(const char* callId, const char* transferToUrl,
                          UtlString* targetCallId,
                          UtlString* targetConnectionAddress = NULL);
    // Blind transfer

    virtual PtStatus transfer(const char* targetCallId, const char* originalCallId);
    // Consultative transfer
    // The couple targetCallId & targetConnectionAddress return/define
    // the transfer target connection in the resulting new transfer
    // target call

    PtStatus transfer(const char* sourceCallId, 
                      const char* sourceAddress, 
                      const char* targetCallId,
                      const char* targetAddress) ;
    //: Transfer an individual participant from one end point to another using 
    //: REFER w/replaces.


    virtual PtStatus split(const char* szSourceCallId,
                           const char* szSourceAddress,
                           const char* szTargetCallId) ;
    //: Split szSourceAddress from szSourceCallId and join it to the specified 
    //: target call id.  The source call/connection MUST be on hold prior
    //: to initiating the split/join.


    virtual void sendFlowgraphMessage(const char* callId, OsMsg& flowgraphMessage);
    virtual void toneStart(const char* callId, int toneId, UtlBoolean local, UtlBoolean remote);
    virtual void toneStop(const char* callId);
    virtual void toneChannelStart(const char* callId, const char* szRemoteAddress, int toneId, UtlBoolean local, UtlBoolean remote);
    virtual void toneChannelStop(const char* callId, const char* szRemoteAddress);    
    virtual void audioPlay(const char* callId, const char* audioUrl, UtlBoolean repeat, UtlBoolean local, UtlBoolean remote, UtlBoolean mixWithMic = false, int downScaling = 100);
    virtual void audioStop(const char* callId);
    virtual void audioChannelPlay(const char* callId, const char* szRemoteAddress, const char* audioUrl, UtlBoolean repeat, UtlBoolean local, UtlBoolean remote, UtlBoolean mixWithMic = false, int downScaling = 100);
    virtual void audioChannelStop(const char* callId, const char* szRemoteAddress);

    /// @copydoc CpCallManager::audioChannelRecordStart
    virtual OsStatus audioChannelRecordStart(const char* callId, 
                                            const char* szRemoteAddress, 
                                            const char* szFile,
                                            const CpMediaInterface::CpAudioFileFormat = CpMediaInterface::CP_WAVE_PCM_16,
                                            const UtlBoolean appendToFile = FALSE,
                                            const int numChannels = 1);

    /// Pause a file recording that is already started
    virtual OsStatus audioChannelRecordPause(const char* callId, const char* szRemoteAddress);

    /// Resume a file recording that is paused
    virtual OsStatus audioChannelRecordResume(const char* callId, const char* szRemoteAddress);

    virtual OsStatus audioChannelRecordStop(const char* callId, const char* szRemoteAddress) ;
    virtual OsStatus audioChannelRecordBufferStart(const char* callId, const char* szRemoteAddress, const char* pBuffer, const int bufferSize, const int bufferType, const int maxRecordTime, const int maxSilence);
    virtual OsStatus audioChannelRecordBufferStop(const char* callId, const char* szRemoteAddress);
    virtual OsStatus setMediaPassThrough(const UtlString& callId, const UtlString& remoteAddress, CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                         int mediaTypeStreamIndex, const UtlString& mediaRecieveAddress, int rtpPort, int rtcpPort);
    virtual OsStatus createMediaConnection(const UtlString& callId, int& connectionId);
    virtual OsStatus setRtpDestination(const UtlString& callId, int connectionId, CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                       int mediaTypeStreamIndex, const UtlString& mediaRecieveAddress, int rtpPort, int rtcpPort);
    virtual OsStatus startRtpSend(const UtlString& callId, int connectionId, const SdpCodecList& codecList);
    virtual OsStatus stopRtpSend(const UtlString& callId, int connectionId);
    virtual void bufferPlay(const char* callId, const void* audiobuf, int bufSize, int type, UtlBoolean repeat, UtlBoolean local, UtlBoolean remote);

#ifndef EXCLUDE_STREAMING
    virtual void createPlayer(const char* callid, MpStreamPlaylistPlayer** ppPlayer) ;
    virtual void createPlayer(int type, const char* callid, const char* szStream, int flags, MpStreamPlayer** ppPlayer) ;
    virtual void destroyPlayer(const char* callid, MpStreamPlaylistPlayer* pPlayer)  ;
    virtual void destroyPlayer(int type, const char* callid, MpStreamPlayer* pPlayer)  ;
#endif


    // Operations for calls & connections
    virtual void acceptConnection(const char* callId,
                                  const char* address,
                                  SIPX_CONTACT_ID contactId = 0,
                                  const void* hWnd = NULL,
                                  const void* security = NULL,
                                  const char* locationHeader = NULL,
                                  const int bandWidth=AUDIO_CODEC_BW_DEFAULT,
                                  UtlBoolean sendEarlyMedia = FALSE);
                                  
    virtual void rejectConnection(const char* callId, const char* address, int errorCode = 0, const char* errorText =  "");
    virtual PtStatus redirectConnection(const char* callId, const char* address, const char* forwardAddressUrl);
    virtual void dropConnection(const char* callId, const char* address);
    virtual void getNumConnections(const char* callId, int& numConnections);
        virtual OsStatus getConnections(const char* callId, int maxConnections,
                int& numConnections, UtlString addresses[]);
    virtual OsStatus getCalledAddresses(const char* callId, int maxConnections,
                int& numConnections, UtlString addresses[]);
    virtual OsStatus getCallingAddresses(const char* callId, int maxConnections,
                int& numConnections, UtlString addresses[]);

    // Operations for calls & terminal connections
    virtual void answerTerminalConnection(const char* callId, const char* address, const char* terminalId, 
                                          const void* pDisplay = NULL, const void* pSecurity = NULL);
    virtual void holdTerminalConnection(const char* callId, const char* address, const char* terminalId);
    virtual void holdAllTerminalConnections(const char* callId);
    virtual void holdLocalTerminalConnection(const char* callId);
    virtual void unholdLocalTerminalConnection(const char* callId);
    virtual void unholdAllTerminalConnections(const char* callId);
    virtual void unholdTerminalConnection(const char* callId, const char* addresss, const char* terminalId);
    virtual void limitCodecs(const char* callId, const char* remoteAddr, const char* codecNames);
    virtual void limitCodecPreferences(const char* callId, const char* remoteAddr, const int audioBandwidth, const int videoBandwidth, const char* szVideoCodecName);
    virtual void limitCodecPreferences(const char* callId, const int audioBandwidth, const int videoBandwidth, const char* szVideoCodecName);
    virtual void silentRemoteHold(const char* callId) ;
    virtual void renegotiateCodecsTerminalConnection(const char* callId, const char* addresss, const char* terminalId);
    virtual void renegotiateCodecsAllTerminalConnections(const char* callId);
         virtual void getNumTerminalConnections(const char* callId, const char* address, int& numTerminalConnections);
         virtual OsStatus getTerminalConnections(const char* callId, const char* address,
                int maxTerminalConnections, int& numTerminalConnections, UtlString terminalNames[]);
    virtual UtlBoolean isTerminalConnectionLocal(const char* callId, const char* address, const char* terminalId);
    virtual void doGetFocus(CpCall* call);
    
    virtual OsStatus getSession(const char* callId,
                                const char* address,
                                SipSession& session);

    virtual OsStatus getSipDialog(const char* callId,
                                  const char* address,
                                  SipDialog& dialog);

    // Stimulus based operations DEPRICATED DO NOT USE
    virtual void unhold(const char* callId);
    virtual void dialString(const char* url);

    virtual UtlBoolean disconnectConnection(const char* callId, const char* addressUrl);

    virtual void setTransferType(int type);

    virtual OsStatus setCodecCPULimitCall(const char* callId, int limit, UtlBoolean bRenegotiate) ;
      //:Sets the CPU codec limit for a call.  Each connection within the call
      //:may only use a codec of the specified CPU intensity (or lesser).

    virtual OsStatus setInboundCodecCPULimit(int limit)  ;
      //:Sets the inbound call CPU limit for codecs

      /// Sets the Mic gain.
    virtual OsStatus setMicGain(const char* callId, float gain);

     /// Set gain for all inputs to the given output on bridge.
    virtual OsStatus setOutputMixWeight(const char* callId, int bridgeOutputIndex, float gain);

    virtual void setMaxCalls(int maxCalls);
    //:Set the maximum number of calls to admit to the system.

    virtual void enableStun(const char* szStunServer, 
                            int iStunPort,
                            int iKeepAlivePeriodSecs,
                            OsNotification *pNotification = NULL) ;
    //:Enable STUN for NAT/Firewall traversal

    virtual void enableTurn(const char* szTurnServer,
                            int iTurnPort,
                            const char* szUsername,
                            const char* szPassword,
                            int iKeepAlivePeriodSecs) ;

    
    virtual UtlBoolean sendInfo(const char* callId, 
                                const char* szRemoteAddress,
                                const char* szContentType,
                                const size_t nContenLength,
                                const char*  szContent);
   //: Sends an INFO message to the other party(s) on the call

/* ============================ ACCESSORS ================================= */
   /**
    * Gets the number of lines made available by line manager.
    */
   virtual int getNumLines();

  /**
   * maxAddressesRequested is the number of addresses requested if available
   * numAddressesAvailable is the actual number of addresses available.
   * "addresses" is a pre-allocated array of size maxAddressesRequested
   */
   virtual OsStatus getOutboundAddresses(int maxAddressesRequested,
                                       int& numAddressesAvailable, UtlString** addresses);

   virtual UtlBoolean getCallState(const char* callId, int& state);
        virtual UtlBoolean getConnectionState(const char* callId, const char* remoteAddress, int& state);

        virtual UtlBoolean getTermConnectionState(const char* callId,
                                                                                        const char* address,
                                                                                        const char* terminal,
                                                                                        int& state);

    UtlBoolean getNextSipCseq(const char* callId,
                             const char* remoteAddress,
                             int& nextCseq);
    //:provides the next csequence number for a given call session (leg) if it exists.
    // Note: this does not protect against transaction

    void printCalls();

    void setOutGoingCallType(int callType);

    int getTransferType();

    virtual PtStatus validateAddress(UtlString& address);

    // Soon to be removed:
        virtual OsStatus getFromField(const char* callId, const char* remoteAddress,  UtlString& fromField);
        virtual OsStatus getToField(const char* callId, const char* remoteAddress,  UtlString& toField);

   virtual OsStatus getCodecCPUCostCall(const char* callId, int& cost);
     //:Gets the CPU cost for an individual connection within the specified
     //:call.

   virtual OsStatus getCodecCPULimitCall(const char* callId, int& cost);
     //:Gets the CPU cost for an individual connection within the specified
     //:call.
     // It is assumed that each connection is at worst this level; however,
     // may actually use less resources.

   virtual void getCalls(int& currentCalls, int& maxCalls);
     //:Get the current number of calls in the system and the maximum number of
     //:calls to be admitted to the system.

   virtual OsStatus getLocalContactAddresses(const char* callid,
                                             SIPX_CONTACT_ADDRESS addresses[],
                                             size_t  nMaxAddresses,
                                             size_t& nActaulAddresses) ;
     //:The available local contact addresses

   virtual CpMediaInterfaceFactory* getMediaInterfaceFactory() ;
     //: Gets the media interface factory used by the call manager
     
   virtual int getMediaConnectionId(const char* szCallId, const char* remoteAddress, void** ppInstData = NULL);
   //: Gets the Media Connection ID
   //: @param szCallId The call-id string of the call with which the connection id is associated
   //: @param remoteAddress The remote address of the call's connection, with which the connection id is associated
   //: @param ppInstData Pointer to the media interface

   virtual UtlBoolean getAudioEnergyLevels(const char*   szCallId, 
                                           const char*   szRemoteAddress,
                                           int&          iInputEnergyLevel,
                                           int&          iOutputEnergyLevel,
                                           int&          nContributors,
                                           unsigned int* pContributorSRCIds,
                                           int*          pContributorEngeryLevels) ;

   virtual UtlBoolean getAudioEnergyLevels(const char*   szCallId,                                            
                                           int&          iInputEnergyLevel,
                                           int&          iOutputEnergyLevel) ;

    virtual UtlBoolean getAudioRtpSourceIDs(const char*   szCallId, 
                                            const char*   szRemoteAddress,
                                            unsigned int& uiSendingSSRC,
                                            unsigned int& uiReceivingSSRC) ;

    virtual void getRemoteUserAgent(const char* callId, 
                                    const char* remoteAddress,
                                    UtlString& userAgent);

    virtual OsStatus getInviteHeaderValue(const char* callId, 
                                          const char* remoteAddress,
                                          const char* headerName, 
                                          UtlString& headerValueString,
                                          UtlBoolean& inviteIsFromRemote,
                                          const int headerInstanceIndex);
     //: Get the named header field value from the INVITE for the indicated call.
     //: @param callId for the CpPeerCall
     //: @param remoteAddress for the CpConnection
     //: @param headerName for the SIP INVITE header to retrieve
     //: @param inviteIsFromRemote - TRUE/FALSE if INVITE is received (vs sent)
     //: @param headerInstanceIndex - index to headers of the same name
     //: @return 
     //:     OS_SUCCESS
     //:     OS_NOT_FOUND - INVITE exists, header not found
     //:     OS_INVALID_STATE - call or INVITE does not exist

   virtual UtlBoolean canAddConnection(const char* szCallId);
   //: Can a new connection be added to the specified call?  This method is 
   //: delegated to the media interface.
   
/* ============================ INQUIRY =================================== */
   int getTotalNumberOutgoingCalls() { return mnTotalOutgoingCalls;}
   int getTotalNumberIncomingCalls() { return mnTotalIncomingCalls;}

   virtual void onCallDestroy(CpCall* pCall);
   virtual void yieldFocus(CpCall* call);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    UtlString mOutboundLine;
    UtlBoolean dialing;
    UtlBoolean mOffHook;
    UtlBoolean speakerOn;
    UtlBoolean flashPending;
    SipUserAgent* sipUserAgent;
    int mSipSessionReinviteTimer;
    CpCall* infocusCall;
    UtlSList callStack;
    UtlString mDialString;
    int mOutGoingCallType;
    PtMGCP* mpMgcpStackTask;
    int mNumDialPlanDigits;
    int mHoldType;
    SdpCodecList* mpCodecFactory;
    int mTransferType;
    UtlString mLocale;
    int mMessageEventCount;
    int mnTotalIncomingCalls;
    int mnTotalOutgoingCalls;
    int mExpeditedIpTos;
    UtlString mCallManagerHistory[CP_CALL_HISTORY_LENGTH];
    UtlBoolean mIsEarlyMediaFor180;
    SipLineMgr*       mpLineMgrTask;     //Line manager
    UtlBoolean mIsRequredUserIdMatch;
    // mMaxCalls can be changed by code running in other threads.
    volatile int mMaxCalls;    
    UtlString mStunServer ;
    int mStunPort ;
    int mStunKeepAlivePeriodSecs ;
    UtlString mTurnServer ;
    int mTurnPort ;
    UtlString mTurnUsername; 
    UtlString mTurnPassword;
    int mTurnKeepAlivePeriodSecs ;

    CpMediaInterfaceFactory* mpMediaFactory;

    // Private accessors
    void pushCall(CpCall* call);
    CpCall* popCall();
    CpCall* removeCall(CpCall* call);
    int getCallStackSize();
    UtlBoolean changeCallFocus(CpCall* callToTakeFocus);
    CpCall* findHandlingCall(const char* callId);
    CpCall* findHandlingCall(int callIndex);
    CpCall* findHandlingCall(const OsMsg& eventMessage);
    CpCall* findFirstQueuedCall();
    void getCodecs(int& numCodecs, SdpCodec**& codecArray);
    void addHistoryEvent(const char* messageLogString);

    void doHold();

    OsStatus doCreateCall(const char* callId,
                          int metaEventId = 0,
                          int metaEventType = PtEvent::META_EVENT_NONE,
                          int numMetaEventCalls = 0,
                          const char* metaEventCallIds[] = NULL,
                          UtlBoolean assumeFocusIfNoInfocusCall = TRUE);

    void doConnect(const char* callId,
                   const char* addressUrl,
                   const char* szDesiredConnectionCallId,
                   SIPX_CONTACT_ID contactId = 0,
                   const void* pDisplay = NULL,
                   const void* pSecurity = NULL,
                   const char* locationHeader = NULL,
                   const int bandWidth = AUDIO_CODEC_BW_DEFAULT,
                   SIPX_TRANSPORT_DATA* pTransport = NULL,
                   const RtpTransportOptions rtpTransportOptions = RTP_TRANSPORT_UDP) ;

    void doEnableStun(const UtlString& szStunServer, 
                      int              iServerPort,
                      int              iKeepAlivePeriodSecs, 
                      OsNotification*  pNotification) ;
    //:Enable STUN for NAT/Firewall traversal                           

    void doEnableTurn(const UtlString& turnServer, 
                      int              iTurnPort,
                      const UtlString& turnUsername,
                      const UtlString& szTurnPassword,
                      int              iKeepAlivePeriodSecs) ;
    //:Enable TURN for NAT/Firewall traversal                           

           CallManager(const CallManager& rCallManager);
     //:Copy constructor (disabled)

           CallManager& operator=(const CallManager& rhs);
     //:Assignment operator (disabled)

#ifdef _WIN32
    UtlBoolean IsTroubleShootingModeEnabled();
#endif
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CallManager_h_
