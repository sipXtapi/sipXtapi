// 
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
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
#include <sdp/SdpMediaLine.h>
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

    /// @name DTLS-SRTP identity and global configuration
    ///
    /// These static methods configure DTLS-SRTP at the process level.
    /// Call them (optionally) once at application startup. If
    /// setDtlsIdentity() is never called, a self-signed ECDSA P-256
    /// certificate is auto-generated on first need (first call to
    /// getLocalDtlsFingerprint(), or first DTLS-SRTP connection).
    ///
    /// These methods have no effect unless the library is built with
    /// ENABLE_SRTP and HAVE_SSL defined.
    //@{

    /// Install the long-lived DTLS identity (certificate + private key)
    /// used for all DTLS-SRTP handshakes.
    static OsStatus setDtlsIdentity(const UtlString& certPemPath,
       const UtlString& privateKeyPemPath);
    /**
    *  @param[in] certPemPath - path to a PEM-encoded X.509 certificate.
    *  @param[in] privateKeyPemPath - path to a PEM-encoded private key
    *             matching the certificate. May be the same file as
    *             certPemPath if the key is bundled with the cert.
    *
    *  @retval OS_SUCCESS   identity loaded successfully.
    *  @retval OS_NOT_FOUND a file could not be opened.
    *  @retval OS_FAILED    the files could not be parsed, the key does
    *                       not match the cert, or the library is not
    *                       built with ENABLE_SRTP and HAVE_SSL defines.
    *
    *  @note Must be called before any DTLS-SRTP connection is created
    *        and before the first call to getLocalDtlsFingerprint() if
    *        you want to avoid triggering auto-generation.
    */

    /// Get the fingerprint of our local DTLS certificate, for the SIP
    /// layer to include in outgoing SDP as an a=fingerprint attribute.
    static OsStatus getLocalDtlsFingerprint(UtlString& fingerprint,
       const UtlString& hashAlgorithm = "SHA-256");
    /**
    *  @param[out] fingerprint - upper-case hex string with colon
    *              separators, e.g. "AB:CD:EF:...". The caller is
    *              responsible for prepending the hash algorithm name
    *              (e.g. "SHA-256 ") when building the SDP line.
    *  @param[in]  hashAlgorithm - one of "SHA-256" (default), "SHA-1",
    *              "SHA-384", "SHA-512".
    *
    *  @retval OS_SUCCESS        fingerprint returned.
    *  @retval OS_INVALID_ARGUMENT  unsupported hashAlgorithm.
    *  @retval OS_FAILED         the library is not built with
    *                            ENABLE_SRTP and HAVE_SSL defines,
    *                            or cert auto-gen failed.
    *
    *  @note If setDtlsIdentity() has not been called, this triggers
    *        auto-generation of a self-signed certificate on first use.
    */

    /// Restrict and order the list of SRTP profiles offered during the
    /// DTLS use_srtp extension negotiation (RFC 5764).
    static OsStatus setDtlsSrtpProfiles(int numProfiles,
                                        const SdpMediaLine::SdpCryptoSuiteType profiles[]);
    /**
    *  @param[in] numProfiles - number of entries in the profiles array.
    *             Zero restores the default list.
    *  @param[in] profiles - ordered list, highest preference first.
    *             Only suites supported by DTLS-SRTP's use_srtp
    *             extension are accepted:
    *             - CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM (not supported in default Windows build from contrib)
    *             - CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM (not supported in default Windows build from contrib)
    *             - CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80
    *             - CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32
    *             Other suites in SdpCryptoSuiteType (e.g. AES-192,
    *             AES-256 CM, F8) are valid for SDES-SRTP but have no
    *             registered use_srtp profile and will be rejected.
    *
    *             The two GCM suites further require that libsrtp was
    *             built with an external crypto backend (OpenSSL or NSS).
    *             A libsrtp built with no external crypto -- which is
    *             the default for the prebuilt libsrtp shipped on
    *             Windows -- will reject GCM here at API time so callers
    *             get an immediate error rather than a silent
    *             negotiate-then-fail at SRTP key install time. Linux
    *             distributions typically ship libsrtp built with NSS
    *             or OpenSSL and accept all four.
    *
    *             Capability is detected at MpSrtp::globalInitialize()
    *             time; check the FAC_MP startup log for the support
    *             matrix.
    *
    *  @retval OS_SUCCESS  profile list updated.
    *  @retval OS_INVALID_ARGUMENT  a profile in the list is not
    *                               supported by DTLS-SRTP, or the
    *                               linked-in libsrtp lacks the cipher
    *                               for it.
    *  @retval OS_FAILED   the library is not
    *                      built with ENABLE_SRTP and HAVE_SSL defines.
    *
    *  Default list (when this method is never called):
    *     AEAD_AES_128_GCM, AEAD_AES_256_GCM,
    *     AES_CM_128_HMAC_SHA1_80, AES_CM_128_HMAC_SHA1_32
    *  -- with any suites unsupported by the linked-in libsrtp filtered
    *  out before being offered on the wire.
    */

    /// Set the maximum time to wait for a DTLS handshake to complete.
    static OsStatus setDtlsHandshakeTimeout(int timeoutSeconds);
    /**
    *  @param[in] timeoutSeconds - maximum handshake duration. If the
    *             handshake does not complete within this window, the
    *             connection fires a DTLS_HANDSHAKE_FAILED notification
    *             with reason TIMEOUT. Default is 20 seconds.
    *
    *  @retval OS_SUCCESS        timeout updated.
    *  @retval OS_INVALID_ARGUMENT  timeoutSeconds <= 0.
    */

    //@}
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

   /// Configure a connection to use DTLS-SRTP. The DTLS handshake
   /// will run over the same UDP sockets as RTP, and the resulting
   /// SRTP master keys are installed automatically when the
   /// handshake completes.
   ///
   /// @note Use the non-SRTP-key overload of startRtpSend() and
   ///       startRtpReceive() for DTLS-SRTP connections -- the keys
   ///       come from the handshake, not the caller. Calling the
   ///       SDES overload on a connection already configured for
   ///       DTLS-SRTP is an error.
   ///
   /// @note ENABLE_SRTP and HAVE_SSL must be defined for this method to
   ///       establish an actual DTLS-SRTP session.
   virtual OsStatus setDtlsSrtpParams(int connectionId,
                                      CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                      const UtlString& remoteFingerprint,
                                      const UtlString& hashAlgorithm,
                                      SdpMediaLine::SdpTcpSetupAttribute role,
                                      int numProfilesOverride = 0,
                                      const SdpMediaLine::SdpCryptoSuiteType* profilesOverride = NULL);

   /// Inbound packet counts for a connection: RTP accepted, and RTCP
   /// accepted (classified, ungated, and successfully unprotected).
   ///
   /// Intended for diagnostics -- "is the peer's media arriving", "is its
   /// RTCP getting through" -- and in particular for confirming RTCP is being
   /// received at all when rtcp-mux puts it on the RTP port, where a
   /// misclassification would otherwise fail silently.
   ///
   /// The counters are incremented on the NetInTask thread and read here
   /// without synchronisation, so a value may lag the wire by a packet. That
   /// is immaterial for the questions they answer.
   ///
   /// @retval OS_NOT_FOUND - no such connection, or its input resource is gone.
   virtual OsStatus getRtpPacketCounts(int connectionId,
      CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
      int& rtpPackets,
      int& rtcpPackets);

   /// Enable RFC 5761 RTP/RTCP multiplexing on a connection: RTCP is sent to,
   /// and expected on, the RTP port instead of a separate one.
   ///
   /// Call this only once multiplexing is NEGOTIATED, never merely proposed.
   /// RFC 5761 section 5.1.1: "If the answer does not contain an
   /// 'a=rtcp-mux' attribute, the offerer MUST NOT multiplex RTP and RTCP
   /// packets on a single port."  So:
   ///   - As offerer: put a=rtcp-mux in the offer, then call this only if the
   ///     answer echoes it.  Nothing needs to be called at offer time; the
   ///     connection keeps listening on both ports meanwhile, which is what
   ///     section 5.1.1 requires.
   ///   - As answerer: call this before generating an answer that echoes
   ///     a=rtcp-mux.
   ///
   /// @note Ordering matters, in two ways:
   ///       - Call before startRtpSend() / startRtpReceive(). Those bind the
   ///         sockets into the flowgraph and start the RTCP renderer, and the
   ///         renderer needs to know which socket to write to.
   ///       - Call before the DTLS-SRTP handshake is triggered, i.e. before
   ///         whichever of setDtlsSrtpParams() / setConnectionDestination()
   ///         completes that pair. Multiplexing decides whether one DTLS
   ///         association suffices or two are required (RFC 5764 section 3),
   ///         and that cannot be changed once handshakes are under way.
   ///
   /// @note Once enabled, the rtcpPort argument of setConnectionDestination()
   ///       is ignored: RTCP goes to the peer's RTP port.
   ///
   /// @param[in] connectionId - connection to configure.
   /// @param[in] mediaType - AUDIO_STREAM or VIDEO_STREAM; audio and video are
   ///            separate m-lines and negotiate multiplexing independently.
   /// @param[in] enabled - TRUE to multiplex.
   /// @retval OS_SUCCESS - applied.
   /// @retval OS_NOT_FOUND - no such connection.
   /// @retval OS_NOT_SUPPORTED - VIDEO_STREAM in a build without VIDEO.
   /// @retval OS_INVALID_STATE - RTP is already started on this connection, so
   ///            it is too late to change the socket layout.
   virtual OsStatus setRtcpMux(int connectionId,
                               CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                               UtlBoolean enabled);
   /**
   *  The DTLS handshake begins when this method AND
   *  setConnectionDestination() have both been called for the given
   *  connectionId. The two calls may occur in either order. Until
   *  the handshake completes, any RTP sent is dropped and any RTP
   *  received is discarded (DTLS packets on the same socket are
   *  routed to the handshake engine).
   *
   *  On handshake completion or failure, a notification is dispatched
   *  via the notification dispatcher (see setNotificationDispatcher):
   *    - DTLS_HANDSHAKE_COMPLETE on success
   *    - DTLS_HANDSHAKE_FAILED on fingerprint mismatch, cert error,
   *      timeout, or protocol error
   *
   *  @param[in] connectionId - connection to configure.
   *  @param[in] mediaType - CpMediaInterface::AUDIO_STREAM or
   *             CpMediaInterface::VIDEO_STREAM
   *  @param[in] remoteFingerprint - the peer's certificate fingerprint
   *             as received in their SDP a=fingerprint attribute, in
   *             upper-case colon-separated hex (e.g. "AB:CD:..."),
   *             without the hash algorithm prefix.
   *  @param[in] hashAlgorithm - algorithm that produced
   *             remoteFingerprint (e.g. "SHA-256"). Must match what
   *             was in the peer's SDP.
   *  @param[in] role - resolved setup role from SDP a=setup:
   *             negotiation. Must be TCP_SETUP_ATTRIBUTE_ACTIVE
   *             (we are the DTLS client) or
   *             TCP_SETUP_ATTRIBUTE_PASSIVE (we are the DTLS server).
   *             TCP_SETUP_ATTRIBUTE_ACTPASS is invalid here -- the
   *             SIP layer must resolve actpass before calling.
   *  @param[in] numProfilesOverride - if non-zero, overrides the
   *             factory-level default profile list for this
   *             connection only.
   *  @param[in] profilesOverride - per-connection profile list.
   *             Ignored if numProfilesOverride is 0. The same
   *             eligibility rules as setDtlsSrtpProfiles apply: each
   *             entry must have an RFC 5764 use_srtp registration AND
   *             must be supported by the linked-in libsrtp build (GCM
   *             requires libsrtp built with OpenSSL or NSS).
   *
   *  @retval OS_SUCCESS        connection configured; handshake will
   *                            start when destination is also set.
   *  @retval OS_NOT_FOUND      invalid connectionId.
   *  @retval OS_INVALID_ARGUMENT  bad fingerprint format, unsupported
   *                            hashAlgorithm, invalid role, an override
   *                            profile not supported by DTLS-SRTP, or
   *                            an override profile whose cipher is
   *                            missing from the linked-in libsrtp
   *                            build.
   *  @retval OS_FAILED         not built with ENABLE_SRTP or HAVE_SSL, or
   *                            the connection is already configured
   *                            for SDES-SRTP.
   */

   /// Configure ICE credentials for a connection, enabling ice-lite
   /// STUN authentication on the RTP socket. Required when the remote
   /// peer is a WebRTC endpoint (e.g. a browser).
   ///
   /// When credentials are set, the STUN responder will:
   ///   - Validate USERNAME on incoming Binding Requests. Usernames are
   ///     in the form "LFRAG:RFRAG" must match localUfrag:remoteUfrag.
   ///   - Validate MESSAGE-INTEGRITY using localPwd as the HMAC-SHA1
   ///     key.
   ///   - Include MESSAGE-INTEGRITY and FINGERPRINT on outgoing
   ///     Binding Responses, signed with localPwd.
   ///
   /// Without this call, the STUN responder for the connection
   /// operates in legacy mode (no integrity validation, no signed
   /// responses), preserving the historical sipXtapi behavior for
   /// non-ICE peers.
   ///
   /// The credentials should be set after the connection's RTP socket
   /// has been created (via createConnection) and before media flows.
   /// In practice this means immediately after the SIP layer has
   /// generated the local ICE credentials for the SDP answer.
   ///
   /// Note: remotePwd is stored for forward compatibility with a future
   ///       full-ICE implementation but is not currently used; ice-lite
   ///       mode never initiates outbound STUN Binding Requests for ICE
   ///       and so does not need the peer's credentials.
   virtual OsStatus setIceCredentials(int connectionId,
                                      CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                      const UtlString& localUfrag,
                                      const UtlString& localPwd,
                                      const UtlString& remoteUfrag = "",
                                      const UtlString& remotePwd   = "");
   /**
   *  @param[in] connectionId - connection to configure.
   *  @param[in] mediaType - CpMediaInterface::AUDIO_STREAM or
   *             CpMediaInterface::VIDEO_STREAM.
   *  @param[in] localUfrag - the value advertised in our SDP answer
   *             as a=ice-ufrag. Must be non-empty.
   *  @param[in] localPwd   - the value advertised in our SDP answer
   *             as a=ice-pwd. Must be non-empty.
   *  @param[in] remoteUfrag - the peer's a=ice-ufrag from their SDP.
   *  @param[in] remotePwd   - the peer's a=ice-pwd. Stored but not
   *             used in ice-lite mode.
   *
   *  @retval OS_SUCCESS         credentials applied.
   *  @retval OS_NOT_FOUND       invalid connectionId or no RTP socket
   *                             for the requested mediaType.
   *  @retval OS_INVALID_ARGUMENT  empty localUfrag or empty localPwd.
   */

    /// Called by the ICE nomination trampoline when USE-CANDIDATE is received,
    /// in a valid STUN Binding Request.  Calls setConnectionDestination with 
    /// the nominated address and posts MI_NOTF_ICE_CANDIDATE_NOMINATED for the
    /// SIP / application layer. 
    /// Not for external use.
    void handleIceNomination(int connectionId, 
                             CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                             const char* remoteIp, 
                             int remotePort);

    /// @copydoc CpMediaInterface::copyPayloadIds
    virtual OsStatus copyPayloadIds(int connectionId, int numCodecs, SdpCodec* remoceCodecs[]);

     /// @copydoc CpMediaInterface::startRtpSend()
    virtual OsStatus startRtpSend(int connectionId,
                                  int numCodecs,
                                  SdpCodec* sendCodec[]);

    // Note:  ENABLE_SRTP must be defined for this version to encrypt SRTP
    virtual OsStatus startRtpSend(int connectionId,
                                  int numCodecs,
                                  SdpCodec* sendCodec[],
                                  SdpMediaLine::SdpCryptoSuiteType cryptoSuite,
                                  const UtlString& cryptoKey);

     /// @copydoc CpMediaInterface::startRtpReceive()
    virtual OsStatus startRtpReceive(int connectionId,
                                     int numCodecs,
                                     SdpCodec* receiveCodec[]);

    // Note:  ENABLE_SRTP must be defined for this version to decrypt SRTP
    virtual OsStatus startRtpReceive(int connectionId,
                                     int numCodecs,
                                     SdpCodec* receiveCodec[],
                                     SdpMediaLine::SdpCryptoSuiteType cryptoSuite,
                                     const UtlString& cryptoKey);

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
   virtual OsStatus startTone(const UtlString& resourceName, int toneId, UtlBoolean rfc4733Enabled = TRUE);
   /// @copydoc CpMediaInterface::stopTone()
   virtual OsStatus stopTone();
   virtual OsStatus stopTone(const UtlString& resourceName, UtlBoolean rfc4733Enabled = TRUE);

     /// @copydoc CpMediaInterface::setRtcpTimeOffset()
   virtual OsStatus setRtcpTimeOffset(int connectionId,
                                      CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                      int streamIndex,
                                      int timeOffset);

     /// @copydoc CpMediaInterface::startChannelTone()
   virtual OsStatus startChannelTone(int connectionId, int toneId, UtlBoolean local, UtlBoolean remote); // Plays both inband and rfc4733 tone
   virtual OsStatus startChannelTone(const UtlString& resourceName, int connectionId, int toneId);
   /// @copydoc CpMediaInterface::stopChannelTone()
   virtual OsStatus stopChannelTone(int connectionId) ;
   virtual OsStatus stopChannelTone(const UtlString& resourceName, int connectionId);

   /// @copydoc CpMediaInterface::startChannelOnlyTone()
   virtual OsStatus startChannelOnlyTone(int connectionId, int toneId);  // Plays RFC4733 only
   /// @copydoc CpMediaInterface::stopChannelOnlyTone()
   virtual OsStatus stopChannelOnlyTone(int connectionId);

     /// @copydoc CpMediaInterface::playAudio()
   virtual OsStatus playAudio(const char* url, 
                              UtlBoolean repeat,
                              UtlBoolean local,               // currently unused in this topology
                              UtlBoolean remote,              // currently unused in this topology
                              UtlBoolean mixWithMic = false,  // currently unused in this topology
                              int downScaling = 100,          // currently unused in this topology
                              UtlBoolean autoStopAfterFinish = TRUE);
   virtual OsStatus playAudio(const UtlString& resourceName,
                              const char* url,
                              UtlBoolean repeat,
                              UtlBoolean autoStopAfterFinish = TRUE,
                              unsigned int startOffsetMs = 0);

     /// @copydoc CpMediaInterface::playBuffer()
    virtual OsStatus playBuffer(char* buf, 
                                unsigned long bufSize,
                                uint32_t bufRate, 
                                int type, 
                                UtlBoolean repeat,
                                UtlBoolean local,               // currently unused in this topology
                                UtlBoolean remote,              // currently unused in this topology
                                OsProtectedEvent* event = NULL,
                                UtlBoolean mixWithMic = false,  // currently unused in this topology
                                int downScaling = 100,          // currently unused in this topology
                                UtlBoolean autoStopOnFinish = TRUE);
   virtual OsStatus playBuffer(const UtlString& resourceName,
                               char* buf,
                               unsigned long bufSize,
                               uint32_t bufRate,
                               int type,
                               UtlBoolean repeat,
                               OsProtectedEvent* event = NULL,
                               UtlBoolean autoStopOnFinish = TRUE,
                               unsigned int startOffsetMs = 0);

     /// @copydoc CpMediaInterface::pauseAudio()
   virtual OsStatus pauseAudio();
   virtual OsStatus pauseAudio(const UtlString& resourceName);

     /// @copydoc CpMediaInterface::resumeAudio()
   virtual OsStatus resumeAudio();
   virtual OsStatus resumeAudio(const UtlString& resourceName);

   /// @copydoc CpMediaInterface::stopAudio()
   virtual OsStatus stopAudio();
   virtual OsStatus stopAudio(const UtlString& resourceName);

   /// @copydoc CpMediaInterface::playChannelAudio
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
                                       int numChannels = 1,
                                       int maxTime = 0,
                                       int silenceLength = -1,
                                       UtlBoolean setupMultiChannelMixesAutomatically = TRUE);
   virtual OsStatus recordAudio(const UtlString& resourceName,
                                const char* szFile,
                                CpAudioFileFormat cpFileFormat = CP_WAVE_PCM_16,
                                UtlBoolean appendToFile = FALSE,
                                int numChannels = 1,
                                int maxTime = 0,
                                int silenceLength = -1,
                                UtlBoolean setupMultiChannelMixesAutomatically = TRUE);

   /// @copydoc CpMediaInterface::pauseRecordChannelAudio
   virtual OsStatus pauseRecordChannelAudio(int connectionId);
   virtual OsStatus pauseRecordAudio(const UtlString& resourceName);

   /// @copydoc CpMediaInterface::resumeRecordChannelAudio
   virtual OsStatus resumeRecordChannelAudio(int connectionId);
   virtual OsStatus resumeRecordAudio(const UtlString& resourceName);

   /// @copydoc CpMediaInterface::stopRecordChannelAudio
   virtual OsStatus stopRecordChannelAudio(int connectionId);
   virtual OsStatus stopRecordAudio(const UtlString& resourceName);

   /// @copydoc CpMediaInterface::recordBufferChannelAudio
   virtual OsStatus recordBufferChannelAudio(int connectionId,
                                             char* pBuffer,
                                             int bufferSize,
                                             int maxRecordTime = -1,
                                             int maxSilence = -1,
                                             int numChannels = 1,
                                             UtlBoolean setupMixesAutomatically = TRUE) ;
   virtual OsStatus recordBufferAudio(const UtlString& resourceName, 
                                      char* pBuffer,
                                      int bufferSize,
                                      int maxRecordTime = -1,
                                      int maxSilence = -1,
                                      int numChannels = 1,
                                      UtlBoolean setupMixesAutomatically = TRUE);

   /// @copydoc CpMediaInterface::stopRecordBufferAudio
   virtual OsStatus stopRecordBufferChannelAudio(int connectionId) ;

   virtual OsStatus recordCircularBufferChannelAudio(int connectionId,
                                                     CircularBufferPtr & buffer,
                                                     CpMediaInterface::CpAudioFileFormat recordingFormat,
                                                     unsigned long recordingBufferNotificationWatermark,
                                                     int maxRecordTime = -1,
                                                     int maxSilence = -1,
                                                     int numChannels = 1,
                                                     UtlBoolean setupMixesAutomatically = TRUE);
   virtual OsStatus recordCircularBufferAudio(const UtlString& resourceName,
                                              CircularBufferPtr& buffer,
                                              CpMediaInterface::CpAudioFileFormat recordingFormat,
                                              unsigned long recordingBufferNotificationWatermark,
                                              int maxRecordTime = -1,
                                              int maxSilence = -1,
                                              int numChannels = 1,
                                              UtlBoolean setupMixesAutomatically = TRUE);

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

   /// Query the DTLS-SRTP handshake state for a connection.
   /// Non-blocking. Useful as a poll-based alternative to the
   /// DTLS_HANDSHAKE_COMPLETE / DTLS_HANDSHAKE_FAILED notifications.
   virtual OsStatus getDtlsSrtpStatus(int connectionId,
                                      CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                      UtlBoolean& handshakeComplete,
                                      UtlBoolean& fingerprintVerified,
                                      SdpMediaLine::SdpCryptoSuiteType& negotiatedSuite);
   /**
   *  @param[in]  connectionId - connection to query.
   *  @param[in]  mediaType - CpMediaInterface::AUDIO_STREAM or
   *              CpMediaInterface::VIDEO_STREAM
   *  @param[out] handshakeComplete - TRUE if the DTLS handshake has
   *              finished (successfully or not). FALSE if still in
   *              progress or not yet started.
   *  @param[out] fingerprintVerified - TRUE if handshakeComplete is
   *              TRUE AND the peer's certificate matched the
   *              remoteFingerprint passed to setDtlsSrtpParams().
   *              Always FALSE if handshakeComplete is FALSE.
   *  @param[out] negotiatedSuite - the SRTP suite selected by the
   *              use_srtp extension negotiation. Valid only when
   *              handshakeComplete and fingerprintVerified are both
   *              TRUE; CRYPTO_SUITE_TYPE_NONE otherwise.
   *
   *  @retval OS_SUCCESS    status returned.
   *  @retval OS_NOT_FOUND  invalid connectionId, or the connection
   *                        is not configured for DTLS-SRTP.
   *  @retval OS_FAILED     not built with ENABLE_SRTP and HAVE_SSL.
   */

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
   int mInitialTopologyBridgePorts;

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

      /// Setup the mixes for recording (routes audio to multiple channels)
    void setupRecordingMixes(int numChannels);

    virtual OsStatus startRtpSendImpl(int connectionId,
                                      int numCodecs,
                                      SdpCodec* sendCodec[],
                                      SdpMediaLine::SdpCryptoSuiteType cryptoSuite,
                                      const UtlString& cryptoKey);

    virtual OsStatus startRtpReceiveImpl(int connectionId,
                                         int numCodecs,
                                         SdpCodec* receiveCodec[],
                                         SdpMediaLine::SdpCryptoSuiteType cryptoSuite,
                                         const UtlString& cryptoKey);

      /// Disabled copy constructor
    CpTopologyGraphInterface(CpTopologyGraphInterface&);

      /// Disabled assignment operator
    CpTopologyGraphInterface& operator=(const CpTopologyGraphInterface&);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpTopologyGraphInterface_h_
