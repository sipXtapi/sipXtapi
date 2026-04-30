//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpDtls_h_
#define _MpDtls_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <sdp/SdpMediaLine.h>
#include <utl/UtlString.h>
#include <utl/UtlBool.h>
#include <os/OsStatus.h>
#include <os/OsMutex.h>
#include <os/OsMsgQ.h>
#include <os/OsTimer.h>
#include <os/OsCallback.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// Forward declarations to keep OpenSSL headers out of this header.
#ifdef __cplusplus
extern "C" {
#endif
   struct ssl_st;
   typedef struct ssl_st SSL;
   struct ssl_ctx_st;
   typedef struct ssl_ctx_st SSL_CTX;
   struct bio_st;
   typedef struct bio_st BIO;
#ifdef __cplusplus
}
#endif

class OsSocket;
class OsMsgDispatcher;

// CONSTANTS
// STRUCTS
// TYPEDEFS

/**
 *  @brief Per-connection DTLS-SRTP handshake engine.
 *
 *  One MpDtls instance per RTP connection. Owns the OpenSSL SSL_CTX
 *  and SSL state for the connection's DTLS handshake, drives the
 *  handshake to completion, verifies the peer's certificate against
 *  the fingerprint promised in SDP, and on success exports the SRTP
 *  master keys (RFC 5764 §4.2) and installs them in the existing
 *  MpSrtp protect/unprotect path via MpSetSrtpParamsMsg messages -- the
 *  same mechanism used by SDES-SRTP.
 *
 *  Threading model: all operations run on the media thread of the
 *  owning flowgraph, including processIncomingPacket(). The
 *  MprFromNet NetInTask-thread receive path demultiplexes inbound
 *  packets per RFC 7983 and posts a flowgraph message containing the
 *  raw DTLS bytes; processIncomingPacket() runs from inside the
 *  resulting handleMessage() call. The only cross-thread API is
 *  getStatus(), which uses a small mutex covering a few state fields.
 *
 *  Outbound DTLS bytes are written to the RTP socket directly by
 *  MpDtls (via the OsSocket pointer registered in setDestination()).
 *  No traffic flows through MprToNet for DTLS records.
 *
 *  @nosubgrouping
 */
class MpDtls
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ TYPES ===================================== */

   enum DtlsRole
   {
      DTLS_ROLE_CLIENT,    ///< "active" in SDP a=setup
      DTLS_ROLE_SERVER     ///< "passive" in SDP a=setup
   };

   enum DtlsState
   {
      DTLS_STATE_IDLE,         ///< Created, no params set yet.
      DTLS_STATE_ARMED,        ///< Params set; waiting on destination
                               ///< (or vice versa) to start handshake.
      DTLS_STATE_HANDSHAKING,  ///< Handshake in progress.
      DTLS_STATE_ACTIVE,       ///< Handshake complete, fingerprint verified,
                               ///< SRTP keys installed.
      DTLS_STATE_FAILED        ///< Handshake or verification failed.
   };

   enum FailureReason
   {
      DTLS_FAIL_NONE,
      DTLS_FAIL_FINGERPRINT_MISMATCH,
      DTLS_FAIL_PROTOCOL_ERROR,
      DTLS_FAIL_TIMEOUT,
      DTLS_FAIL_INTERNAL
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor.
   MpDtls();

     /// Destructor. Tears down OpenSSL state.
   virtual ~MpDtls();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Static helper: build and post an MpSetDtlsParamsMsg targeted at
     /// the resource named `targetResourceName` in the flowgraph whose
     /// queue is `flowgraphMessageQueue`. Mirrors MpSrtp::setSrtpParams.
     /// Used to pass the pointer to 'this' class to MprToNet and MprFromNet.
   static OsStatus setDtlsParams(const UtlString& targetResourceName,
                                 OsMsgQ& flowgraphMessageQueue,
                                 MpDtls* pDtls);

     /// Set the maximum time to wait for any DTLS handshake to complete.
     /// Process-wide; applies to all subsequent handshakes.
     /// Default is 20 seconds.
   static OsStatus setHandshakeTimeoutSeconds(int seconds);

     /// Set the process-wide default list of SRTP profiles to offer in
     /// the DTLS use_srtp extension. Per-connection overrides via
     /// setParams() take precedence over this default.
     ///
     /// Pass numProfiles=0 to reset to the engine's built-in default
     /// (AEAD_AES_128_GCM, AEAD_AES_256_GCM,
     ///  AES_CM_128_HMAC_SHA1_80, AES_CM_128_HMAC_SHA1_32).
     ///
     /// Returns OS_INVALID_ARGUMENT if any profile in the list either:
     ///   - has no DTLS-SRTP equivalent (e.g. AES-192, F8), or
     ///   - is not supported by the linked-in libsrtp build (typically
     ///     GCM on a libsrtp built without an external crypto backend).
     /// The full list is rejected; the previous default is preserved.
   static OsStatus setDefaultProfiles(int numProfiles,
                                      const SdpMediaLine::SdpCryptoSuiteType profiles[]);

     /// Returns TRUE if the given suite is eligible for DTLS-SRTP
     /// negotiation: it has both an RFC 5764 use_srtp profile registration
     /// AND the linked-in libsrtp build was compiled with the underlying
     /// cipher. Returns FALSE for SDES-only suites (AES-192, AES-256 CM,
     /// F8) and for DTLS-eligible suites whose cipher is missing from the
     /// runtime libsrtp (typically GCM on a libsrtp built without an
     /// external crypto backend).
     ///
     /// Requires that MpSrtp::globalInitialize() has run.
   static UtlBoolean isSrtpCryptoSuiteDtlsEligible(SdpMediaLine::SdpCryptoSuiteType suite);

     /// Set the resource names of the From/To-Net resources whose flowgraph
     /// queues should receive the SRTP key-install messages once the DTLS
     /// handshake completes. Also remembers the queue for those messages.
   void setSrtpInstallTargets(const UtlString& fromNetResourceName,
                              const UtlString& toNetResourceName,
                              OsMsgQ* flowgraphMessageQueue);
     /**
     *  @param[in] fromNetResourceName - resource name of the MprFromNet
     *             that will receive the unprotect-key install message.
     *  @param[in] toNetResourceName - resource name of the MprToNet
     *             that will receive the protect-key install message.
     *  @param[in] flowgraphMessageQueue - the flowgraph's message queue;
     *             both messages are posted here. May be NULL to clear.
     *
     *  Caller owns the queue; MpDtls only borrows the pointer for the
     *  lifetime of the connection. This method must be called before
     *  setParams() / setDestination() so the keys can be installed when
     *  the handshake completes.
     */

     /// Set the notification dispatcher used to deliver
     /// DTLS_HANDSHAKE_COMPLETE / DTLS_HANDSHAKE_FAILED notifications.
     /// Pointer is borrowed; not owned. May be NULL to disable
     /// notifications (e.g. for app-side polling-only mode).
   void setNotificationDispatcher(OsMsgDispatcher* pDispatcher);

     /// Set the connection ID this engine belongs to. Used to populate
     /// the connectionId field of dispatched notifications so the app
     /// can correlate them with calls.
   void setConnectionId(int connectionId);

     /// Configure the DTLS-SRTP parameters for this connection.
     ///
     /// Together with setDestination(), this arms the engine. When both
     /// have been called, the handshake begins on the next call into
     /// processIncomingPacket() (server role) or immediately via
     /// startHandshake() (client role).
   OsStatus setParams(const UtlString& remoteFingerprint,
                      const UtlString& hashAlgorithm,
                      DtlsRole role,
                      int numProfiles,
                      const SdpMediaLine::SdpCryptoSuiteType* profiles);
     /**
     *  @param[in] remoteFingerprint - peer's cert fingerprint from their
     *             SDP a=fingerprint, upper-case colon-separated hex.
     *  @param[in] hashAlgorithm - "SHA-256", "SHA-1", "SHA-384", "SHA-512".
     *  @param[in] role - DTLS_ROLE_CLIENT or DTLS_ROLE_SERVER (resolved
     *             from a=setup negotiation).
     *  @param[in] numProfiles - number of SRTP profiles in the array.
     *             Zero means use the factory-default list.
     *  @param[in] profiles - ordered list, highest preference first.
     *
     *  @retval OS_SUCCESS           params accepted.
     *  @retval OS_INVALID_ARGUMENT  bad fingerprint format, unsupported
     *                               hash, invalid role, or unsupported
     *                               profile.
     *  @retval OS_FAILED            ENABLE_SRTP and HAVE_SSL not defined.
     */

     /// Set the destination address (peer RTP IP/port) and the socket
     /// to write outbound DTLS records on.
     ///
     /// Together with setParams(), this arms the engine.
   OsStatus setDestination(OsSocket* rtpSocket,
                           const UtlString& remoteAddress,
                           int remotePort);
     /**
     *  @param[in] rtpSocket - the connection's RTP socket. MpDtls writes
     *             outbound DTLS records directly to this socket. Caller
     *             retains ownership.
     *  @param[in] remoteAddress - peer IP address.
     *  @param[in] remotePort - peer RTP port.
     *
     *  @retval OS_SUCCESS  destination registered; handshake will start
     *                      if setParams() has also been called.
     */

     /// If we're the DTLS client, send our ClientHello. Called by the
     /// owning code when setParams() and setDestination() have both
     /// been satisfied.
     ///
     /// No-op if we're the server (we wait for the peer's ClientHello).
     /// No-op if state is not ARMED.
   OsStatus startHandshake();

     /// Feed an inbound DTLS record to the engine. Called from
     /// MprFromNet::handleMessage on the media thread, after the
     /// NetInTask thread has demultiplexed by RFC 7983 first byte.
     ///
     /// On entry the engine may be ARMED (server role accepting
     /// initial ClientHello), HANDSHAKING, or ACTIVE (post-handshake
     /// keepalive / shutdown alert). All other states drop the packet.
   OsStatus processIncomingPacket(const char* data, int dataLen);

     /// Called on the media thread by MprFromNet::handleMessage when
     /// the DTLS retransmit timer has fired. Drives any pending
     /// retransmits OpenSSL has queued and re-arms the timer.
   void handleRetransmit();

     /// Called on the media thread by MprFromNet::handleMessage when
     /// the hard handshake-timeout timer has fired. If still
     /// handshaking, transitions to FAILED with reason TIMEOUT.
   void handleHandshakeTimeout();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Cross-thread status query. Cheap; takes mStatusLock only.
     /// Returns OS_FAILED if ENABLE_SRTP and HAVE_SSL are not defined.
   OsStatus getStatus(DtlsState& state,
                  FailureReason& failureReason,
                  SdpMediaLine::SdpCryptoSuiteType& negotiatedSuite,
                  UtlBoolean& fingerprintVerified) const;

     /// Convenience: TRUE if the handshake has completed successfully.
   UtlBoolean isActive() const;

    /// Convert enums to strings for logging
   static const char* dtlsRoleName(DtlsRole role);
   static const char* dtlsStateName(DtlsState state);
   static const char* failureReasonName(FailureReason reason);

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Lazy SSL_CTX/SSL setup. Runs on first transition out of IDLE.
     /// Caller must hold mLock.
   OsStatus initSsl();

     /// Tear down SSL_CTX/SSL. Caller must hold mLock.
   void teardownSsl();

     /// Drive the handshake state machine: feed inbound bytes (already
     /// pushed into mpInBio by caller), call SSL_do_handshake, drain
     /// any outbound bytes from mpOutBio to the socket. Caller must
     /// hold mLock.
   OsStatus driveHandshake();

     /// Pull all available outbound bytes from mpOutBio and write them
     /// to the socket. Caller must hold mLock.
   OsStatus drainOutgoing();

     /// If a DTLS record arrived before the engine was ready to process
     /// it, mPendingInboundRecord holds it. This helper snapshots and
     /// clears the buffer under mLock, then calls processIncomingPacket
     /// with the snapshot (without holding mLock during the call -- it
     /// will reacquire). Safe to call at any state transition; no-op if
     /// nothing is buffered.
   void replayPendingInboundRecord();

     /// Called when SSL_do_handshake returns success: verify the peer's
     /// fingerprint, export keying material, install SRTP keys via
     /// MpSetSrtpParamsMsg messages. Caller must hold mLock.
   OsStatus onHandshakeComplete();

     /// Compute the peer cert fingerprint and compare against
     /// mRemoteFingerprint. Caller must hold mLock.
   UtlBoolean verifyPeerFingerprint();

     /// Update cross-thread-visible status fields atomically.
   void updateStatus(DtlsState state,
                     FailureReason failureReason,
                     SdpMediaLine::SdpCryptoSuiteType negotiatedSuite,
                     UtlBoolean fingerprintVerified);

     /// Translate an OpenSSL SRTP profile constant to our SdpCryptoSuiteType.
   static SdpMediaLine::SdpCryptoSuiteType srtpProfileFromOpenSslId(int sslProfileId);

     /// Translate one of our SdpCryptoSuiteType values to the OpenSSL
     /// profile name string. Returns NULL for suites with no DTLS-SRTP
     /// equivalent.
   static const char* openSslSrtpProfileName(SdpMediaLine::SdpCryptoSuiteType suite);

     /// Build the comma-separated SRTP profile string OpenSSL wants for
     /// SSL_CTX_set_tlsext_use_srtp().
   static UtlString buildOpenSslProfileList(int numProfiles,
                                            const SdpMediaLine::SdpCryptoSuiteType* profiles);

     /// Read DTLSv1_get_timeout from OpenSSL and arm (or disarm) the
     /// retransmit timer accordingly. Caller must hold mLock.
   void armRetransmitTimer();

     /// Disarm the retransmit timer. Caller must hold mLock.
   void disarmRetransmitTimer();

     /// Start the hard handshake-timeout timer. Caller must hold mLock.
   void armHandshakeTimeout();

     /// Stop the hard handshake-timeout timer. Caller must hold mLock.
   void disarmHandshakeTimeout();

     /// Post a no-payload MpResourceMsg to the flowgraph queue. Used by
     /// the timer callbacks to marshal back to the media thread.
   void postTimerMessage(int msgType);

     /// Post DTLS_HANDSHAKE_COMPLETE notification to the dispatcher.
     /// Caller must hold mLock.
   void postCompleteNotification();

     /// Post DTLS_HANDSHAKE_FAILED notification to the dispatcher,
     /// carrying the failure reason as the int payload.
     /// Caller must hold mLock.
   void postFailedNotification(FailureReason reason);

     /// Static callback wired to mpRetransmitTimer. Runs on the timer
     /// thread; just posts a flowgraph message.
   static void retransmitTimerCallback(const intptr_t userData,
                                       const intptr_t eventData);

     /// Static callback wired to mpHandshakeTimeoutTimer.
   static void handshakeTimeoutCallback(const intptr_t userData,
                                        const intptr_t eventData);

   /* ----- members ------------------------------------------------------- */

   // OpenSSL state. Touched only on the media thread.
   SSL_CTX*    mpSslCtx;
   SSL*        mpSsl;
   BIO*        mpInBio;        ///< OpenSSL reads here; we write to it.
   BIO*        mpOutBio;       ///< OpenSSL writes here; we read from it.

   // Configuration.
   DtlsRole    mRole;                ///< client vs server
   UtlString   mRemoteFingerprint;   ///< upper-case hex with colons
   UtlString   mHashAlgorithm;       ///< canonical "SHA-256" etc.
   UtlString   mProfileList;         ///< OpenSSL-format colon list
   UtlBoolean  mParamsSet;
   UtlBoolean  mDestinationSet;

   // First inbound DTLS record that arrived before we were ready to
   // process it (i.e. before setParams + setDestination both completed).
   // Replayed once we transition out of IDLE/unready-ARMED. Subsequent
   // early-arrivals are dropped on the floor; the peer's DTLS retransmit
   // will resend if needed -- but holding the very first one avoids the
   // 1-second OpenSSL retransmit timer firing in the common race where
   // peer's ClientHello arrives microseconds before our setParams/
   // setDestination sequence completes.
   UtlString   mPendingInboundRecord;

   // Outbound socket / destination for DTLS messaging
   OsSocket*   mpRtpSocket;          ///< Borrowed; not owned.
   UtlString   mRemoteAddress;
   int         mRemotePort;

   // SRTP install plumbing.
   UtlString   mFromNetResourceName;
   UtlString   mToNetResourceName;
   OsMsgQ*     mpFlowgraphQueue;     ///< Borrowed; not owned.

   // Notification plumbing.
   OsMsgDispatcher* mpNotificationDispatcher;  ///< Borrowed; not owned.
   int              mConnectionId;             ///< For notification correlation.

   // Cross-thread-visible status.
   mutable OsMutex                    mStatusLock;
   DtlsState                          mState;
   FailureReason                      mFailureReason;
   SdpMediaLine::SdpCryptoSuiteType   mNegotiatedSuite;
   UtlBoolean                         mFingerprintVerified;

   // Coarse lock around OpenSSL state. In practice only the media
   // thread touches it, but the mutex documents the invariant and
   // gives us a place to expand if we ever break that assumption.
   mutable OsMutex                    mLock;

   // Retransmit timer (re-armable). Fires on timer thread; callback
   // posts MPRM_DTLS_RETRANSMIT to the flowgraph queue.
   OsCallback*                        mpRetransmitCallback;
   OsTimer*                           mpRetransmitTimer;

   // Hard handshake-timeout timer (single-shot). Fires on timer thread;
   // callback posts MPRM_DTLS_HANDSHAKE_TIMEOUT to the flowgraph queue.
   OsCallback*                        mpHandshakeTimeoutCallback;
   OsTimer*                           mpHandshakeTimeoutTimer;

   // Process-wide handshake timeout, in seconds. Read by each instance
   // when entering HANDSHAKING. Default 20.
   static int                         sHandshakeTimeoutSeconds;

   // Process-wide default SRTP profile list (set by setDefaultProfiles).
   // sDefaultProfileCount == 0 means "no explicit default; use the
   // engine's built-in fallback". MAX is the largest list we support
   // (one entry per SdpCryptoSuiteType that maps to a DTLS-SRTP profile).
   enum { MAX_DEFAULT_PROFILES = 8 };
   static SdpMediaLine::SdpCryptoSuiteType sDefaultProfiles[MAX_DEFAULT_PROFILES];
   static int                              sDefaultProfileCount;
   static OsMutex                          sDefaultProfilesLock;

     /// Disabled
   MpDtls(const MpDtls& rhs);
     /// Disabled
   MpDtls& operator=(const MpDtls& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDtls_h_
