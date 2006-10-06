//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _SipConnection_h_
#define _SipConnection_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "cp/Connection.h"
#include "cp/CSeqManager.h"
#include "net/SipContactDb.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class SipMessage;
class SdpCodec;
class SdpCodecFactory;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipConnection : public Connection, public ISocketIdle, public IMediaEventListener
{
    /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum ReinviteStates
    {
        ACCEPT_INVITE = 0,
        REINVITED,
        REINVITING
    };

    /* ============================ CREATORS ================================== */

    SipConnection(const char* outboundLineAddress,
        UtlBoolean isEarlyMediaFor180Enabled = TRUE,
        CpCallManager* callMgr = NULL,
        CpCall* call = NULL,
        CpMediaInterface* mediaInterface = NULL,
        SipUserAgent* sipUA = NULL,
        int offeringDelayMilliSeconds = IMMEDIATE,
        int sessionReinviteTimer = 0,
        int availableBehavior = RING,
        const char* forwardUnconditionalUrl = NULL,
        int busyBehavior = BUSY,
        const char* forwardOnBusy = NULL);
    //:Default constructor

    virtual
        ~SipConnection();
    //:Destructor

    /* ============================ MANIPULATORS ============================== */

    virtual UtlBoolean dequeue(UtlBoolean callInFocus);
    
    virtual UtlBoolean send(SipMessage& message,
                        OsMsgQ* responseListener = NULL,
                        void* responseListenerData = NULL);

    virtual UtlBoolean dial(const char* dialString,
        const char* callerId,
        const char* callId,
        const char* callController = NULL,
        const char* originalCallConnection = NULL,
        UtlBoolean requestQueuedCall = FALSE,
        const void* pDisplay = NULL,
        const void* pSecurity = NULL,
        const char* locationHeader = NULL,
        const int bandWidth = AUDIO_MICODEC_BW_DEFAULT,
        UtlBoolean bOnHold = FALSE,
		const char* originalCallId = NULL);
    //! param: requestQueuedCall - indicates that the caller wishes to have the callee queue the call if busy

    virtual UtlBoolean originalCallTransfer(UtlString& transferTargetAddress,
        const char* transferControllerAddress,
        const char* targetCallId);
    // Initiate transfer on transfer controller connection in
    // the original call.
    // If fromAddress or toAddress are NULL it is assumed to
    // be a blind transfer.

    virtual UtlBoolean targetCallBlindTransfer(const char* transferTargetAddress,
        const char* transferControllerAddress);
    // Communicate blind transfer on transfer controller connection in
    // the target call.  This is signaled by the transfer controller in the
    // original call.

    virtual UtlBoolean transfereeStatus(int connectionState, int response);
    // Method to communicate status to original call on transferee side

    virtual UtlBoolean transferControllerStatus(int connectionState, int response);
    // Method to communicate status to target call on transfer
    // controller side

    virtual void outOfFocus() ;

    virtual UtlBoolean answer(const void* hWnd = NULL);

    virtual UtlBoolean hangUp();

    virtual UtlBoolean hold();

    virtual UtlBoolean offHold();

    virtual UtlBoolean renegotiateCodecs();

    virtual UtlBoolean silentRemoteHold();

    virtual UtlBoolean accept(int forwardOnNoAnswerTimeOut,
                              const void *pSecurity = NULL,
                              const char* locationHeader = NULL,
                              const int bandWidth = AUDIO_MICODEC_BW_DEFAULT);

    virtual UtlBoolean reject();

    virtual UtlBoolean redirect(const char* forwardAddress);

    virtual UtlBoolean canSendInfo() ;
    virtual UtlBoolean sendInfo(UtlString contentType, UtlString sContent);

    virtual UtlBoolean processMessage(OsMsg& eventMessage,
        UtlBoolean callInFocus, UtlBoolean onHook);

    void setCallerId();

    virtual UtlBoolean getRemoteAddress(UtlString* remoteAddress) const;
    virtual UtlBoolean getRemoteAddress(UtlString* remoteAddress,
        UtlBoolean leaveFieldParmetersIn) const;

    static UtlBoolean processNewFinalMessage(SipUserAgent* sipUa,
        OsMsg* eventMessage);

    void setContactType(SIPX_CONTACT_TYPE eType, Url* pToUrl = NULL) ;
    void setContactId(SIPX_CONTACT_ID contactId) { mContactId = contactId; }

	void setExternalTransport(SIPX_TRANSPORT_DATA* pTransport) { if (pTransport) { mTransport = *pTransport; }}

    // ISocketIdle::onIdleNotify method
    void onIdleNotify(OsDatagramSocket* const pSocket,
                                 SocketPurpose purpose,
                                 const int millisecondsIdle);

    // IMediaEventListener::methods
    virtual void onFileStart(IMediaEvent_DeviceTypes type);
    virtual void onFileStop(IMediaEvent_DeviceTypes type);
    virtual void onBufferStart(IMediaEvent_DeviceTypes type);
    virtual void onBufferStop(IMediaEvent_DeviceTypes type);
    virtual void onDeviceError(IMediaEvent_DeviceTypes type, IMediaEvent_DeviceErrors errCode);
    virtual void onListenerAddedToEmitter(IMediaEventEmitter* pEmitter);

    /* ============================ ACCESSORS ================================= */

    virtual UtlBoolean getSession(SipSession& session);

    virtual OsStatus getFromField(UtlString* fromField);

    virtual OsStatus getToField(UtlString* toField);

    int getNextCseq();

    /* ============================ INQUIRY =================================== */

    static UtlBoolean shouldCreateConnection(SipUserAgent& sipUa,
        OsMsg& eventMessage,
        SdpCodecFactory* codecFactory = NULL);

    virtual UtlBoolean willHandleMessage(OsMsg& eventMessage) const;

    virtual UtlBoolean isConnection(const char* callId,
        const char* toTag,
        const char* fromTag,
        UtlBoolean  strictCompare) const;

    virtual UtlBoolean isSameRemoteAddress(Url& remoteAddress) const;
    virtual UtlBoolean isSameRemoteAddress(Url& remoteAddress,
        UtlBoolean tagsMustMatch) const;

    void setSecurity(const SIPXTACK_SECURITY_ATTRIBUTES* const pSecurity);

    void getRemoteUserAgent(UtlString* userAgent);

    virtual UtlBoolean isLocallyInitiatedRemoteHold() const ;

    /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    bool prepareInviteSdpForSend(SipMessage* pMsg, int connectionId, const void* pSecurityAttribute) ;

    void setMediaDestination(const char*    hostAddress, 
                             int            audioRtpPort, 
                             int            audioRtcpPort, 
                             int            videoRtpPort, 
                             int            videoRtcpPort, 
                             const SdpBody* pRemoteBody) ;

    SIPX_CONTACT_TYPE selectCompatibleContactType(const SipMessage& request) ;
    //: Select a compatible contact given the URI

    void updateContact(Url* pContactUrl, SIPX_CONTACT_TYPE eType, Url *pToUrl = NULL) ;

    static UtlBoolean requestShouldCreateConnection(const SipMessage* sipMsg,
        SipUserAgent& sipUa,
        SdpCodecFactory* codecFactory);

    UtlBoolean doOffHold(UtlBoolean forceReInvite);

    UtlBoolean doHangUp(const char* dialString = NULL,
        const char* callerId = NULL);

    void buildFromToAddresses(const char* dialString,
        const char* callerId,
        const char* callerDisplayName,
        UtlString& fromAddress,
        UtlString& goodToAddress);

    void buildLocalContact(Url fromAddress,
        UtlString& localContact,
        Url* pToUrl = NULL) ;//for outbound call

    UtlBoolean extendSessionReinvite();

    // SIP Request handlers
    UtlBoolean processRequest(const SipMessage* request,
        UtlBoolean callInFocus, UtlBoolean onHook);
    void processInviteRequest(const SipMessage* request);
    void processReferRequest(const SipMessage* request);
    void processAckRequest(const SipMessage* request);
    void processByeRequest(const SipMessage* request);
    void processCancelRequest(const SipMessage* request);
    void processNotifyRequest(const SipMessage* request);

    // SIP Response handlers
    UtlBoolean processResponse(const SipMessage* response,
        UtlBoolean callInFocus, UtlBoolean onHook);
    void processInviteResponse(const SipMessage* request);    
    void processReferResponse(const SipMessage* request);
    void processOptionsResponse(const SipMessage* request);
    void processByeResponse(const SipMessage* request);
    void processCancelResponse(const SipMessage* request);
    void processNotifyResponse(const SipMessage* request);

    void processInviteResponseRinging(const SipMessage* response);
    void processInviteResponseBusy(const SipMessage* response);
    void processInviteResponseQueued(const SipMessage* response);
    void processInviteResponseRequestPending(const SipMessage* response);
    void processInviteResponseFailed(const SipMessage* response);
    void processInviteResponseHangingUp(const SipMessage* response);
    void processInviteResponseNormal(const SipMessage* response);
    void processInviteResponseRedirect(const SipMessage* response);
    void processInviteResponseUnknown(const SipMessage* response);

    void processInviteRequestBadTransaction(const SipMessage* request, int tag) ;
    void processInviteRequestLoop(const SipMessage* request, int tag) ;
    void processInviteRequestBadRefer(const SipMessage* request, int tag) ;
    void processInviteRequestOffering(const SipMessage* request, 
                                      int               tag,
                                      bool              doesReplaceCallLegExist,
                                      int               replaceCallLegState,
                                      UtlString&        replaceCallId,
                                      UtlString&        replaceToTag,
                                      UtlString&        replaceFromTag);


    void processInviteRequestReinvite(const SipMessage* request, int tag) ;

    void fireIncompatibleCodecsEvent(SdpCodecFactory* pSupportedCodecs,
                                     SdpCodec**       ppMatchedCodecs,
                                     int              nMatchedCodces) ;
    void fireAudioStartEvents(SIPX_MEDIA_CAUSE cause = MEDIA_CAUSE_NORMAL) ;
    void fireAudioStopEvents(SIPX_MEDIA_CAUSE cause = MEDIA_CAUSE_NORMAL) ;

    /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    CSeqManager mCSeqMgr ;

    bool mbByeAttempted;
    SipUserAgent* sipUserAgent;
    UtlString mFromTag;
    UtlString mLocationHeader;
    SipMessage* inviteMsg;
    UtlBoolean  mbCancelling;
    UtlBoolean  mbLocallyInitiatedRemoteHold ;
    SIPXTACK_SECURITY_ATTRIBUTES* mpSecurity;

    SipMessage* mReferMessage;
    UtlBoolean inviteFromThisSide;
    UtlString mLastRequestMethod;
    UtlString mRemoteContact; //last contact frield from the other side
    Url mFromUrl; // SIP address for the local side
    Url mToUrl;  //  SIP address for the remote side
    UtlString mRemoteUriStr;  //  SIP uri string for the remote side
    UtlString mLocalUriStr;  //  SIP uri string for the local side
    UtlString mContactUriStr; //  SIP uri string for the contact

    int lastLocalSequenceNumber;
    int lastRemoteSequenceNumber;
    int reinviteState;
    UtlString mRouteField;
    int mDefaultSessionReinviteTimer;
    int mSessionReinviteTimer;
    UtlString mAllowedRemote;  // Methods supported by the otherside
    UtlBoolean mIsReferSent;   // used to determine whether to send ack when sequence number is smaller
    UtlBoolean mIsAcceptSent;   // used to determine whether to accept ack when sequence number is smaller
    int mHoldCompleteAction;
    int mBandwidthId;
    UtlBoolean mIsEarlyMediaFor180;
    UtlString mLineId; //line identifier for incoming calls.
    UtlString mLocalContact;    ///< The local Contact: field value - a URI in name-addr format.
    SIPX_CONTACT_TYPE mContactType ;
    SIPX_CONTACT_ID mContactId;
    UtlBoolean mDropping ;
    UtlString mRemoteUserAgent;
    SIPX_TRANSPORT_DATA mTransport;
    UtlSList mMediaEventEmitters;

    UtlBoolean getInitialSdpCodecs(const SipMessage* sdpMessage,
        SdpCodecFactory& supportedCodecsArray,
        int& numCodecsInCommon,
        SdpCodec** &codecsInCommon,
        UtlString& remoteAddress,
        int& remotePort,
        int& remoteRtcpPort,
		int& remoteVideoRtpPort,
		int& remoteVideoRtcpPort,
        SdpSrtpParameters& localSrtpParams,
        SdpSrtpParameters& matchingSrtpParams,
        int localBandwidth,
        int& matchingBandwidth,
        int localVideoFramerate,
        int& matchingVideoFramerate);

    virtual void proceedToRinging(const SipMessage* inviteMessage,
        SipUserAgent* sipUserAgent,
        int tagNum,
        int availableBehavior);

    UtlBoolean isMethodAllowed(const char* method);

    void doBlindRefer();


    SipConnection& operator=(const SipConnection& rhs);
    //:Assignment operator (disabled)

    SipConnection(const SipConnection& rSipConnection);
    //:Copy constructor (disabled)


};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipConnection_h_
