//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <mp/MpDtls.h>

#ifdef HAVE_SSL
#  include <openssl/ssl.h>
#  include <openssl/x509.h>
#  include <openssl/evp.h>
#  include <openssl/bio.h>
#  include <openssl/err.h>
#  include <openssl/srtp.h>
#endif

#include <stdio.h>
#include <string.h>

// APPLICATION INCLUDES
#include <mp/MpDtlsIdentity.h>
#include <mp/MpSetDtlsParamsMsg.h>
#include <mp/MpSrtp.h>
#include <mp/MpResourceMsg.h>
#include <mp/MpResNotificationMsg.h>
#include <mp/MprnIntMsg.h>
#include <os/OsLock.h>
#include <os/OsSysLog.h>
#include <os/OsSocket.h>
#include <os/OsTimer.h>
#include <os/OsCallback.h>
#include <os/OsTime.h>
#include <os/OsMsgDispatcher.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// RFC 5764 §4.2 keying material exporter label.
static const char* const DTLS_SRTP_EXPORTER_LABEL = "EXTRACTOR-dtls_srtp";
static const int         DTLS_SRTP_EXPORTER_LABEL_LEN = 19;

// STATIC VARIABLE INITIALIZATIONS

int MpDtls::sHandshakeTimeoutSeconds = 20;

SdpMediaLine::SdpCryptoSuiteType
   MpDtls::sDefaultProfiles[MpDtls::MAX_DEFAULT_PROFILES] = {
      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE,
      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE,
      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE,
      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE,
      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE,
      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE,
      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE,
      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE
   };
int     MpDtls::sDefaultProfileCount = 0;
OsMutex MpDtls::sDefaultProfilesLock(OsMutex::Q_FIFO);
// LOCAL HELPERS

#ifdef HAVE_SSL
namespace
{
   /// OpenSSL verify callback: accept any peer cert. We verify the
   /// fingerprint ourselves after the handshake completes.
   int dtlsVerifyCallbackAcceptAll(int /*preverifyOk*/, X509_STORE_CTX* /*ctx*/)
   {
      return 1;
   }

   /// Format raw bytes as upper-case hex pairs separated by colons.
   void formatFingerprint(const unsigned char* raw,
                          unsigned int rawLen,
                          UtlString& outString)
   {
      outString.remove(0);
      char hex[4];
      for (unsigned int i = 0; i < rawLen; i++)
      {
         if (i > 0)
         {
            outString.append(":");
         }
         sprintf(hex, "%02X", raw[i]);
         outString.append(hex);
      }
   }

   /// Map a hash algorithm name to an OpenSSL EVP_MD*.
   const EVP_MD* evpMdFromName(const UtlString& hashAlgorithm)
   {
      if (hashAlgorithm.compareTo("SHA-256", UtlString::ignoreCase) == 0 ||
          hashAlgorithm.compareTo("SHA256",  UtlString::ignoreCase) == 0)
      {
         return EVP_sha256();
      }
      if (hashAlgorithm.compareTo("SHA-1", UtlString::ignoreCase) == 0 ||
          hashAlgorithm.compareTo("SHA1",  UtlString::ignoreCase) == 0)
      {
         return EVP_sha1();
      }
      if (hashAlgorithm.compareTo("SHA-384", UtlString::ignoreCase) == 0 ||
          hashAlgorithm.compareTo("SHA384",  UtlString::ignoreCase) == 0)
      {
         return EVP_sha384();
      }
      if (hashAlgorithm.compareTo("SHA-512", UtlString::ignoreCase) == 0 ||
          hashAlgorithm.compareTo("SHA512",  UtlString::ignoreCase) == 0)
      {
         return EVP_sha512();
      }
      return NULL;
   }

   /// Log the topmost OpenSSL error, if any.
   void logOpenSslError(const char* where)
   {
      unsigned long err = ERR_peek_last_error();
      if (err != 0)
      {
         char buf[256];
         ERR_error_string_n(err, buf, sizeof(buf));
         OsSysLog::add(FAC_MP, PRI_ERR, "%s: OpenSSL error: %s", where, buf);
         ERR_clear_error();
      }
   }

   /// Per-suite key + salt sizes (libsrtp expectations).
   /// Returns 0 if the suite has no DTLS-SRTP equivalent.
   int srtpMasterKeyLen(SdpMediaLine::SdpCryptoSuiteType suite)
   {
      switch (suite)
      {
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80:
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32:
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM:
         return 16;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM:
         return 32;
      default:
         return 0;
      }
   }

   int srtpMasterSaltLen(SdpMediaLine::SdpCryptoSuiteType suite)
   {
      switch (suite)
      {
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80:
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32:
         return 14;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM:
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM:
         return 12;
      default:
         return 0;
      }
   }
}
#endif // HAVE_SSL

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpDtls::MpDtls()
   : mpSslCtx(NULL)
   , mpSsl(NULL)
   , mpInBio(NULL)
   , mpOutBio(NULL)
   , mRole(DTLS_ROLE_CLIENT)
   , mParamsSet(FALSE)
   , mDestinationSet(FALSE)
   , mpRtpSocket(NULL)
   , mRemotePort(0)
   , mpFlowgraphQueue(NULL)
   , mpNotificationDispatcher(NULL)
   , mConnectionId(-1)
   , mStatusLock(OsMutex::Q_FIFO)
   , mState(DTLS_STATE_IDLE)
   , mFailureReason(DTLS_FAIL_NONE)
   , mNegotiatedSuite(SdpMediaLine::CRYPTO_SUITE_TYPE_NONE)
   , mFingerprintVerified(FALSE)
   , mLock(OsMutex::Q_FIFO)
   , mpRetransmitCallback(NULL)
   , mpRetransmitTimer(NULL)
   , mpHandshakeTimeoutCallback(NULL)
   , mpHandshakeTimeoutTimer(NULL)
{
}

MpDtls::~MpDtls()
{
   // Stop and destroy timers FIRST, before taking mLock. OsTimer::stop()
   // synchronously waits for any in-flight callback to complete; if the
   // callback is currently waiting on mLock to post a message, holding
   // mLock here would deadlock. After stop() returns, no more callbacks
   // can fire on this object.
   if (mpRetransmitTimer != NULL)
   {
      mpRetransmitTimer->stop();
      delete mpRetransmitTimer;
      mpRetransmitTimer = NULL;
   }
   if (mpRetransmitCallback != NULL)
   {
      delete mpRetransmitCallback;
      mpRetransmitCallback = NULL;
   }
   if (mpHandshakeTimeoutTimer != NULL)
   {
      mpHandshakeTimeoutTimer->stop();
      delete mpHandshakeTimeoutTimer;
      mpHandshakeTimeoutTimer = NULL;
   }
   if (mpHandshakeTimeoutCallback != NULL)
   {
      delete mpHandshakeTimeoutCallback;
      mpHandshakeTimeoutCallback = NULL;
   }

#ifdef HAVE_SSL
   OsLock lock(mLock);
   teardownSsl();
#endif

   OsSysLog::add(FAC_MP, PRI_DEBUG, "MpDtls::~MpDtls: destroyed");
}

// static
OsStatus MpDtls::setDtlsParams(const UtlString& targetResourceName,
                               OsMsgQ& flowgraphMessageQueue,
                               MpDtls* pDtls)
{
   MpSetDtlsParamsMsg message(targetResourceName, pDtls);
   return flowgraphMessageQueue.send(message, OsTime::OS_INFINITY);
}

// static
OsStatus MpDtls::setHandshakeTimeoutSeconds(int seconds)
{
   if (seconds <= 0)
   {
      return OS_INVALID_ARGUMENT;
   }
   // Plain int write. Read once-per-handshake from a single thread,
   // and updates are app-startup affairs. No barrier needed.
   sHandshakeTimeoutSeconds = seconds;
   return OS_SUCCESS;
}

// static
OsStatus MpDtls::setDefaultProfiles(int numProfiles, const SdpMediaLine::SdpCryptoSuiteType profiles[])
{
   if (numProfiles < 0 || (numProfiles > 0 && profiles == NULL))
   {
      return OS_INVALID_ARGUMENT;
   }
   if (numProfiles > MAX_DEFAULT_PROFILES)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::setDefaultProfiles: numProfiles=%d exceeds MAX_DEFAULT_PROFILES=%d",
         numProfiles, MAX_DEFAULT_PROFILES);
      return OS_INVALID_ARGUMENT;
   }

   // Validate every entry first; reject the whole list if any is unsupported.
   // Two distinct rejection reasons, each with its own log message:
   //   1. The suite has no DTLS-SRTP use_srtp registration (e.g. AES-192,
   //      F8). It might be a valid SDES-SRTP suite, but won't survive
   //      RFC 5764 negotiation.
   //   2. The suite has a use_srtp registration AND a libsrtp policy
   //      mapping, but the linked-in libsrtp build was compiled without
   //      the underlying cipher (typically GCM on a libsrtp built with
   //      no external crypto backend). Negotiating it would succeed at
   //      DTLS handshake time but fail at SRTP key install.
   //
   // This is stricter than buildOpenSslProfileList's silent-skip semantics
   // because here the caller is explicitly asking for a default policy --
   // if they ask for AES-192 or for GCM on a non-GCM libsrtp, they should
   // be told it's not going to happen.
   for (int i = 0; i < numProfiles; i++)
   {
      if (openSslSrtpProfileName(profiles[i]) == NULL)
      {
         OsSysLog::add(FAC_MP, PRI_ERR,
            "MpDtls::setDefaultProfiles: profile[%d]=%d has no DTLS-SRTP equivalent",
            i, profiles[i]);
         return OS_INVALID_ARGUMENT;
      }
      if (!MpSrtp::isCryptoSuiteSupported(profiles[i]))
      {
         OsSysLog::add(FAC_MP, PRI_ERR,
            "MpDtls::setDefaultProfiles: profile[%d]=%s is not supported by "
            "the linked-in libsrtp build (rebuild libsrtp against an "
            "external crypto backend such as OpenSSL or NSS to enable it)",
            i, SdpMediaLine::SdpCryptoSuiteTypeString[profiles[i]]);
         return OS_INVALID_ARGUMENT;
      }
   }

   // Install.
   {
      OsLock lock(sDefaultProfilesLock);
      for (int i = 0; i < numProfiles; i++)
      {
         sDefaultProfiles[i] = profiles[i];
      }
      sDefaultProfileCount = numProfiles;
   }

   OsSysLog::add(FAC_MP, PRI_INFO,
      "MpDtls::setDefaultProfiles: installed %d profile(s)%s",
      numProfiles,
      (numProfiles == 0) ? " (reset to built-in default)" : "");
   return OS_SUCCESS;
}

/* ============================ MANIPULATORS ============================== */

void MpDtls::setSrtpInstallTargets(const UtlString& fromNetResourceName,
                                   const UtlString& toNetResourceName,
                                   OsMsgQ* flowgraphMessageQueue)
{
   OsLock lock(mLock);
   mFromNetResourceName = fromNetResourceName;
   mToNetResourceName   = toNetResourceName;
   mpFlowgraphQueue     = flowgraphMessageQueue;
}

void MpDtls::setNotificationDispatcher(OsMsgDispatcher* pDispatcher)
{
   OsLock lock(mLock);
   mpNotificationDispatcher = pDispatcher;
}

void MpDtls::setConnectionId(int connectionId)
{
   OsLock lock(mLock);
   mConnectionId = connectionId;
}

OsStatus MpDtls::setParams(const UtlString& remoteFingerprint,
                           const UtlString& hashAlgorithm,
                           DtlsRole role,
                           int numProfiles,
                           const SdpMediaLine::SdpCryptoSuiteType* profiles)
{
   // We don't technically need SRTP at this point, but it will be needed
   // eventually and this is one of first things a user will do to try to use
   // DTLS-SRTP, so we fail here if ENABLE_SRTP isn't defined.
#if defined(ENABLE_SRTP) && defined(HAVE_SSL)
   if (remoteFingerprint.isNull() || hashAlgorithm.isNull())
   {
      return OS_INVALID_ARGUMENT;
   }
   if (evpMdFromName(hashAlgorithm) == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::setParams: unsupported hash algorithm '%s'",
         hashAlgorithm.data());
      return OS_INVALID_ARGUMENT;
   }

   UtlString profileList = buildOpenSslProfileList(numProfiles, profiles);
   if (profileList.isNull())
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::setParams: no usable SRTP profiles in list of %d",
         numProfiles);
      return OS_INVALID_ARGUMENT;
   }

   {
      OsLock lock(mLock);
      mRemoteFingerprint = remoteFingerprint;
      mHashAlgorithm     = hashAlgorithm;
      mRole              = role;
      mProfileList       = profileList;
      mParamsSet         = TRUE;

      if (mState == DTLS_STATE_IDLE)
      {
         updateStatus(DTLS_STATE_ARMED, DTLS_FAIL_NONE,
                      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      }
   }

   OsSysLog::add(FAC_MP, PRI_INFO,
      "MpDtls::setParams: role=%s, hash=%s, profiles=%s",
      (role == DTLS_ROLE_CLIENT) ? "client" : "server",
      hashAlgorithm.data(),
      profileList.data());

   // If destination is already set, kick the handshake. startHandshake
   // is internally guarded against missing prerequisites, so calling
   // here regardless is safe.
   startHandshake();

   // If a peer's first DTLS record arrived before we were ready,
   // process it now that the engine has been armed/initialised.
   replayPendingInboundRecord();

   return OS_SUCCESS;
#else
   return OS_FAILED;
#endif
}

OsStatus MpDtls::setDestination(OsSocket* rtpSocket,
                                const UtlString& remoteAddress,
                                int remotePort)
{
   // We don't technically need SRTP at this point, but it will be needed
   // eventually and this is one of first things a user will do to try to use
   // DTLS-SRTP, so we fail here if ENABLE_SRTP isn't defined.
#if defined(ENABLE_SRTP) && defined(HAVE_SSL)
   if (rtpSocket == NULL || remoteAddress.isNull() || remotePort <= 0)
   {
      return OS_INVALID_ARGUMENT;
   }

   {
      OsLock lock(mLock);
      mpRtpSocket     = rtpSocket;
      mRemoteAddress  = remoteAddress;
      mRemotePort     = remotePort;
      mDestinationSet = TRUE;

      if (mState == DTLS_STATE_IDLE)
      {
         updateStatus(DTLS_STATE_ARMED, DTLS_FAIL_NONE,
                      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      }
   }

   // If params are already set, kick the handshake. startHandshake
   // is internally guarded against missing prerequisites, so calling
   // here regardless is safe.
   startHandshake();

   // If a peer's first DTLS record arrived before we were ready,
   // process it now that the engine has been armed/initialised.
   replayPendingInboundRecord();

   return OS_SUCCESS;
#else
   return OS_FAILED;
#endif
}

OsStatus MpDtls::startHandshake()
{
#ifdef HAVE_SSL
   OsLock lock(mLock);

   if (!mParamsSet || !mDestinationSet)
   {
      // Not yet armed; nothing to do.
      return OS_SUCCESS;
   }
   if (mState != DTLS_STATE_ARMED)
   {
      // Already started, finished, or failed.
      return OS_SUCCESS;
   }

   if (mpSslCtx == NULL)
   {
      OsStatus status = initSsl();
      if (status != OS_SUCCESS)
      {
         updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_INTERNAL,
                      SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
         return status;
      }
   }

   if (mRole == DTLS_ROLE_CLIENT)
   {
      // Kick off the handshake: SSL_do_handshake will produce ClientHello
      // bytes into mpOutBio, which driveHandshake() then drains to the wire.
      updateStatus(DTLS_STATE_HANDSHAKING, DTLS_FAIL_NONE,
                   SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      armHandshakeTimeout();
      return driveHandshake();
   }
   else
   {
      // Server: stay ARMED until the peer's ClientHello arrives in
      // processIncomingPacket().
      return OS_SUCCESS;
   }
#else
   return OS_FAILED;
#endif
}

OsStatus MpDtls::processIncomingPacket(const char* data, int dataLen)
{
#ifdef HAVE_SSL
   if (data == NULL || dataLen <= 0)
   {
      return OS_INVALID_ARGUMENT;
   }

   OsLock lock(mLock);

   // Ignore packets after we've failed.
   if (mState == DTLS_STATE_FAILED)
   {
      return OS_SUCCESS;
   }

   // If we're not yet ready to process DTLS records (state IDLE, or
   // state ARMED but missing setParams/setDestination), buffer the
   // FIRST such packet so we can replay it once the engine is armed
   // and SSL is initialised. Subsequent early-arrivals are dropped --
   // the peer's DTLS retransmit will deliver them again if we miss
   // them. This handles the common race where the peer's ClientHello
   // arrives in the microseconds between the two sides completing
   // their setDtlsSrtpParams/setConnectionDestination plumbing,
   // avoiding a 1-second OpenSSL retransmit-timer wait.
   if (mState == DTLS_STATE_IDLE ||
       (mState == DTLS_STATE_ARMED && (!mParamsSet || !mDestinationSet)))
   {
      if (mPendingInboundRecord.length() == 0)
      {
         mPendingInboundRecord.append(data, dataLen);
      }
      return OS_SUCCESS;
   }

   // Server-role: first inbound packet (the ClientHello) transitions us
   // out of ARMED and starts the handshake.
   if (mState == DTLS_STATE_ARMED)
   {
      if (mpSslCtx == NULL)
      {
         OsStatus status = initSsl();
         if (status != OS_SUCCESS)
         {
            updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_INTERNAL,
                         SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
            return status;
         }
      }
      updateStatus(DTLS_STATE_HANDSHAKING, DTLS_FAIL_NONE,
                   SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      armHandshakeTimeout();
   }

   // Push the inbound bytes into OpenSSL's read BIO.
   int written = BIO_write(mpInBio, data, dataLen);
   if (written != dataLen)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::processIncomingPacket: BIO_write wrote %d of %d bytes",
         written, dataLen);
      // Continue anyway -- partial writes shouldn't happen for memory BIOs
      // sized to default but log if they do.
   }

   if (mState == DTLS_STATE_HANDSHAKING)
   {
      return driveHandshake();
   }
   else
   {
      // ACTIVE: post-handshake DTLS traffic (close_notify, etc.). Drain
      // any response OpenSSL produces.
      return drainOutgoing();
   }
#else
   return OS_FAILED;
#endif
}

/* ============================ ACCESSORS ================================= */

OsStatus MpDtls::getStatus(DtlsState& state,
                       FailureReason& failureReason,
                       SdpMediaLine::SdpCryptoSuiteType& negotiatedSuite,
                       UtlBoolean& fingerprintVerified) const
{
#if defined(ENABLE_SRTP) && defined(HAVE_SSL)
   OsLock lock(mStatusLock);
   state               = mState;
   failureReason       = mFailureReason;
   negotiatedSuite     = mNegotiatedSuite;
   fingerprintVerified = mFingerprintVerified;
   return OS_SUCCESS;
#else
   return OS_FAILED;
#endif
}

UtlBoolean MpDtls::isActive() const
{
   OsLock lock(mStatusLock);
   return (mState == DTLS_STATE_ACTIVE) ? TRUE : FALSE;
}

const char* MpDtls::dtlsRoleName(DtlsRole role)
{
   switch (role)
   {
      case DTLS_ROLE_CLIENT:
         return "Client";
      case DTLS_ROLE_SERVER:
         return "Server";
      default:
         return NULL;
   }
}

const char* MpDtls::dtlsStateName(DtlsState state)
{
   switch (state)
   {
      case DTLS_STATE_IDLE:
         return "IDLE";
      case DTLS_STATE_ARMED:
         return "ARMED";
      case DTLS_STATE_HANDSHAKING:
         return "HANDSHAKING";
      case DTLS_STATE_ACTIVE:
         return "ACTIVE";
      case DTLS_STATE_FAILED:
         return "FAILED";
      default:
         return NULL;
   }
}

const char* MpDtls::failureReasonName(FailureReason reason)
{
   switch (reason)
   {
      case DTLS_FAIL_NONE:
         return "None";
      case DTLS_FAIL_FINGERPRINT_MISMATCH:
         return "FingerprintMismatch";
      case DTLS_FAIL_PROTOCOL_ERROR:
         return "ProtocolError";
      case DTLS_FAIL_TIMEOUT:
         return "Timeout";
      case DTLS_FAIL_INTERNAL:
         return "InternalError";
      default:
         return NULL;
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

#ifdef HAVE_SSL

OsStatus MpDtls::initSsl()
{
   // Caller holds mLock.

   MpDtlsIdentity* identity = MpDtlsIdentity::getInstance();
   if (identity == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpDtls::initSsl: no identity available");
      return OS_FAILED;
   }
   X509*     cert = identity->getCertificate();
   EVP_PKEY* key  = identity->getPrivateKey();
   if (cert == NULL || key == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::initSsl: identity missing cert or key");
      return OS_FAILED;
   }

   // DTLS_method() picks the highest mutually supported DTLS version
   // automatically. Available since OpenSSL 1.0.2.
   const SSL_METHOD* method = (mRole == DTLS_ROLE_CLIENT)
                              ? DTLS_client_method()
                              : DTLS_server_method();
   mpSslCtx = SSL_CTX_new(method);
   if (mpSslCtx == NULL)
   {
      logOpenSslError("MpDtls::initSsl (SSL_CTX_new)");
      return OS_FAILED;
   }

   // Cert + key.
   if (SSL_CTX_use_certificate(mpSslCtx, cert) != 1)
   {
      logOpenSslError("MpDtls::initSsl (use_certificate)");
      teardownSsl();
      return OS_FAILED;
   }
   if (SSL_CTX_use_PrivateKey(mpSslCtx, key) != 1)
   {
      logOpenSslError("MpDtls::initSsl (use_PrivateKey)");
      teardownSsl();
      return OS_FAILED;
   }

   // We do fingerprint-based trust, not chain-based. Force the peer to
   // present a cert (DTLS-SRTP requires it), but accept whatever it is;
   // we'll check the fingerprint after the handshake.
   SSL_CTX_set_verify(mpSslCtx,
                      SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                      dtlsVerifyCallbackAcceptAll);

   // SRTP profile negotiation (RFC 5764 §4.1.2).
   if (SSL_CTX_set_tlsext_use_srtp(mpSslCtx, mProfileList.data()) != 0)
   {
      logOpenSslError("MpDtls::initSsl (set_tlsext_use_srtp)");
      teardownSsl();
      return OS_FAILED;
   }

   // Read-ahead lets DTLS pull all available bytes per call -- needed
   // for memory BIOs since we dump whole records in at once.
   SSL_CTX_set_read_ahead(mpSslCtx, 1);

   // Build the SSL object.
   mpSsl = SSL_new(mpSslCtx);
   if (mpSsl == NULL)
   {
      logOpenSslError("MpDtls::initSsl (SSL_new)");
      teardownSsl();
      return OS_FAILED;
   }

   // Memory BIOs. OpenSSL will read inbound bytes from mpInBio (which
   // we feed via BIO_write), and write outbound bytes into mpOutBio
   // (which we drain via BIO_read).
   mpInBio  = BIO_new(BIO_s_mem());
   mpOutBio = BIO_new(BIO_s_mem());
   if (mpInBio == NULL || mpOutBio == NULL)
   {
      logOpenSslError("MpDtls::initSsl (BIO_new mem)");
      teardownSsl();
      return OS_FAILED;
   }
   // BIO_set_mem_eof_return(-1) makes the BIO behave as a non-blocking
   // stream that "needs more data" rather than reporting EOF, which
   // is exactly the semantics SSL_do_handshake expects on memory BIOs.
   BIO_set_mem_eof_return(mpInBio,  -1);
   BIO_set_mem_eof_return(mpOutBio, -1);

   // SSL_set_bio takes ownership of both BIOs -- they will be freed
   // when SSL_free is called. Do not call BIO_free on them yourself.
   SSL_set_bio(mpSsl, mpInBio, mpOutBio);

   if (mRole == DTLS_ROLE_CLIENT)
   {
      SSL_set_connect_state(mpSsl);
   }
   else
   {
      SSL_set_accept_state(mpSsl);
   }

   return OS_SUCCESS;
}

void MpDtls::teardownSsl()
{
   // Caller holds mLock.
   if (mpSsl != NULL)
   {
      SSL_free(mpSsl);   // also frees mpInBio and mpOutBio
      mpSsl    = NULL;
      mpInBio  = NULL;
      mpOutBio = NULL;
   }
   if (mpSslCtx != NULL)
   {
      SSL_CTX_free(mpSslCtx);
      mpSslCtx = NULL;
   }
}

OsStatus MpDtls::driveHandshake()
{
   // Caller holds mLock. Caller has already pushed any inbound bytes
   // into mpInBio (or this is the initial client-side kick).

   int rc = SSL_do_handshake(mpSsl);
   int err = SSL_get_error(mpSsl, rc);

   // Drain whatever bytes OpenSSL produced first -- even on error we
   // may have alerts to send.
   OsStatus drainStatus = drainOutgoing();
   if (drainStatus != OS_SUCCESS)
   {
      // Best-effort: log and continue evaluating handshake state below.
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "MpDtls::driveHandshake: drainOutgoing failed (status=%d)",
         drainStatus);
   }

   if (rc == 1)
   {
      // Handshake done. onHandshakeComplete() transitions state to
      // ACTIVE or FAILED, which disarms timers via updateStatus.
      return onHandshakeComplete();
   }

   // rc <= 0: either need-more-data or a real error.
   if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
   {
      // Normal: waiting for the next inbound flight. Stay in HANDSHAKING.
      // (Re-)arm the retransmit timer so we resend if the peer goes silent.
      ERR_clear_error();
      armRetransmitTimer();
      return OS_SUCCESS;
   }

   // Real error.
   logOpenSslError("MpDtls::driveHandshake (SSL_do_handshake)");
   updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_PROTOCOL_ERROR,
                SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
   return OS_FAILED;
}

OsStatus MpDtls::drainOutgoing()
{
   // Caller holds mLock.
   if (mpOutBio == NULL || mpRtpSocket == NULL)
   {
      return OS_SUCCESS;
   }

   // Loop pulling DTLS records out of the BIO. Each BIO_read on a
   // memory BIO returns up to one buffer's worth; for DTLS that
   // typically means one record (handshake flight) per call, but we
   // loop until there's nothing left.
   //
   // 1500 is a safe MTU upper bound for DTLS records on UDP.
   char    buf[1500];
   int     pending;
   while ((pending = (int)BIO_ctrl_pending(mpOutBio)) > 0)
   {
      int toRead = (pending > (int)sizeof(buf)) ? (int)sizeof(buf) : pending;
      int n = BIO_read(mpOutBio, buf, toRead);
      if (n <= 0)
      {
         break;
      }
      int sent = mpRtpSocket->write(buf, n,
                                    mRemoteAddress.data(),
                                    mRemotePort);
      if (sent != n)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
            "MpDtls::drainOutgoing: socket write returned %d of %d bytes "
            "(dest=%s:%d)",
            sent, n, mRemoteAddress.data(), mRemotePort);
         // Don't fail the whole handshake on a partial UDP send;
         // OpenSSL's DTLS retransmit will recover.
      }
   }
   return OS_SUCCESS;
}

void MpDtls::replayPendingInboundRecord()
{
   // Snapshot the buffer under the lock, then drop the lock and call
   // processIncomingPacket with the snapshot. This avoids re-entering
   // the lock from within itself (OsMutex is non-recursive on at least
   // some platforms) and keeps the snapshot stable across the call.
   UtlString snapshot;
   {
      OsLock lock(mLock);
      if (!mParamsSet || !mDestinationSet || mPendingInboundRecord.length() == 0)
      {
         // Not fully armed yet or nothing pending; nothing to do.
         return;
      }
      snapshot = mPendingInboundRecord;
      mPendingInboundRecord.remove(0);  // clear
   }
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MpDtls::replayPendingInboundRecord: replaying %d-byte buffered DTLS record",
      (int)snapshot.length());
   processIncomingPacket(snapshot.data(), (int)snapshot.length());
}

OsStatus MpDtls::onHandshakeComplete()
{
   // Caller holds mLock.

   // 1. Verify peer fingerprint.
   if (!verifyPeerFingerprint())
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::onHandshakeComplete: peer fingerprint mismatch");
      updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_FINGERPRINT_MISMATCH,
                   SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      return OS_FAILED;
   }

   // 2. Determine negotiated SRTP profile.
   SRTP_PROTECTION_PROFILE* profile = SSL_get_selected_srtp_profile(mpSsl);
   if (profile == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::onHandshakeComplete: no SRTP profile selected");
      updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_PROTOCOL_ERROR,
                   SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      return OS_FAILED;
   }
   SdpMediaLine::SdpCryptoSuiteType suite = srtpProfileFromOpenSslId(profile->id);
   if (suite == SdpMediaLine::CRYPTO_SUITE_TYPE_NONE)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::onHandshakeComplete: unmappable OpenSSL SRTP profile id=%d (%s)",
         profile->id, profile->name ? profile->name : "?");
      updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_PROTOCOL_ERROR,
                   SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      return OS_FAILED;
   }

   // 3. Export keying material per RFC 5764 §4.2.
   //
   //    block layout:
   //       client_write_SRTP_master_key   (keyLen)
   //       server_write_SRTP_master_key   (keyLen)
   //       client_write_SRTP_master_salt  (saltLen)
   //       server_write_SRTP_master_salt  (saltLen)
   //
   const int keyLen  = srtpMasterKeyLen(suite);
   const int saltLen = srtpMasterSaltLen(suite);
   if (keyLen == 0 || saltLen == 0)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::onHandshakeComplete: no key/salt sizes for suite %d",
         suite);
      updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_INTERNAL,
                   SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      return OS_FAILED;
   }
   const int blockLen = 2 * (keyLen + saltLen);

   unsigned char block[2 * (32 + 14)];   // generous upper bound
   if (blockLen > (int)sizeof(block))
   {
      // Defensive: should not happen with currently supported suites.
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::onHandshakeComplete: keying block too large (%d)", blockLen);
      updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_INTERNAL,
                   SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      return OS_FAILED;
   }
   if (SSL_export_keying_material(mpSsl, block, blockLen,
                                  DTLS_SRTP_EXPORTER_LABEL,
                                  DTLS_SRTP_EXPORTER_LABEL_LEN,
                                  NULL, 0, 0) != 1)
   {
      logOpenSslError("MpDtls::onHandshakeComplete (export_keying_material)");
      updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_INTERNAL,
                   SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      return OS_FAILED;
   }

   // Slice into client/server key+salt.
   const unsigned char* clientKey  = block;
   const unsigned char* serverKey  = clientKey  + keyLen;
   const unsigned char* clientSalt = serverKey  + keyLen;
   const unsigned char* serverSalt = clientSalt + saltLen;

   // libsrtp expects key||salt as a single contiguous buffer per direction.
   // Build a UtlString containing the raw bytes; MpSrtp::setSrtpParams
   // already handles this format (length is checked against the libsrtp
   // policy's cipher_key_len).
   //
   // If we are the DTLS client, our outbound (protect) uses client_*,
   // and our inbound (unprotect) uses server_*. Server reverses these.
   UtlString outboundKey;   // for MprToNet (protect)
   UtlString inboundKey;    // for MprFromNet (unprotect)
   if (mRole == DTLS_ROLE_CLIENT)
   {
      outboundKey.append((const char*)clientKey,  keyLen);
      outboundKey.append((const char*)clientSalt, saltLen);
      inboundKey.append((const char*)serverKey,   keyLen);
      inboundKey.append((const char*)serverSalt,  saltLen);
   }
   else
   {
      outboundKey.append((const char*)serverKey,  keyLen);
      outboundKey.append((const char*)serverSalt, saltLen);
      inboundKey.append((const char*)clientKey,   keyLen);
      inboundKey.append((const char*)clientSalt,  saltLen);
   }

   // Wipe the export block ASAP.
   memset(block, 0, sizeof(block));

   // 4. Install in the existing SDES SRTP key path. These messages
   //    are consumed by MpRtp(Input/Output)Connection::handleMessage
   //    and forwarded to Mpr(From/To)Net::setSrtpParams -- exactly
   //    the same path SDES uses today.
   if (mpFlowgraphQueue == NULL ||
       mFromNetResourceName.isNull() || mToNetResourceName.isNull())
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::onHandshakeComplete: SRTP install targets not registered");
      updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_INTERNAL, suite, FALSE);
      return OS_FAILED;
   }

   OsStatus status = MpSrtp::setSrtpParams(mFromNetResourceName, *mpFlowgraphQueue, suite, inboundKey);
   if (status != OS_SUCCESS)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::onHandshakeComplete: send MpSetSrtpParamsMsg for unprotect to %s(FromNet) failed (%d)", mFromNetResourceName, status);
      updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_INTERNAL, suite, FALSE);
      return OS_FAILED;
   }

   status = MpSrtp::setSrtpParams(mToNetResourceName, *mpFlowgraphQueue, suite, outboundKey);
   if (status != OS_SUCCESS)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::onHandshakeComplete: send MpSetSrtpParamsMsg for protect to %s(ToNet) failed (%d)", mToNetResourceName, status);
      updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_INTERNAL, suite, FALSE);
      return OS_FAILED;
   }

   updateStatus(DTLS_STATE_ACTIVE, DTLS_FAIL_NONE, suite, TRUE);
   OsSysLog::add(FAC_MP, PRI_INFO,
      "MpDtls::onHandshakeComplete: SRTP active, suite=%s, role=%s",
      SdpMediaLine::SdpCryptoSuiteTypeString[suite],
      (mRole == DTLS_ROLE_CLIENT) ? "client" : "server");
   return OS_SUCCESS;
}

UtlBoolean MpDtls::verifyPeerFingerprint()
{
   // Caller holds mLock.
   X509* peerCert = SSL_get_peer_certificate(mpSsl);
   if (peerCert == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::verifyPeerFingerprint: peer presented no certificate");
      return FALSE;
   }

   const EVP_MD* md = evpMdFromName(mHashAlgorithm);
   if (md == NULL)
   {
      // Already validated in setParams(), so this would be a logic error.
      X509_free(peerCert);
      return FALSE;
   }

   unsigned char raw[EVP_MAX_MD_SIZE];
   unsigned int  rawLen = 0;
   int rc = X509_digest(peerCert, md, raw, &rawLen);
   X509_free(peerCert);
   if (rc != 1)
   {
      logOpenSslError("MpDtls::verifyPeerFingerprint (X509_digest)");
      return FALSE;
   }

   UtlString actual;
   formatFingerprint(raw, rawLen, actual);

   if (actual.compareTo(mRemoteFingerprint, UtlString::ignoreCase) != 0)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtls::verifyPeerFingerprint: mismatch. expected=%s actual=%s",
         mRemoteFingerprint.data(), actual.data());
      return FALSE;
   }
   return TRUE;
}

void MpDtls::updateStatus(DtlsState state,
                          FailureReason failureReason,
                          SdpMediaLine::SdpCryptoSuiteType negotiatedSuite,
                          UtlBoolean fingerprintVerified)
{
   // Note: using SdpMediaLine::SdpCryptoSuiteTypeString as opposed to openSslSrtpProfileName, since openSslSrtpProfileName doesn't handle CRYPTO_SUITE_TYPE_NONE
   OsSysLog::add(FAC_MP, PRI_DEBUG, "MpDtls::updateStatus: %s [%s] -> [%s], connectionId=%d, failureReason=%s, negotiatedSuite=%s, fingerprintVerified=%s, remote=%s:%d",
      mRole == DTLS_ROLE_CLIENT ? "Client" : "Server", dtlsStateName(mState), dtlsStateName(state), mConnectionId,
      failureReasonName(failureReason), SdpMediaLine::SdpCryptoSuiteTypeString[negotiatedSuite], (fingerprintVerified ? "True" : "False"),
      mRemoteAddress.data(), mRemotePort);

   // Detect terminal-state *transition* (not just re-entry).
   bool enteringActive = (state == DTLS_STATE_ACTIVE) &&
                         (mState != DTLS_STATE_ACTIVE);
   bool enteringFailed = (state == DTLS_STATE_FAILED) &&
                         (mState != DTLS_STATE_FAILED);

   // Update both the OpenSSL-thread-side state (mState) and the
   // cross-thread-visible mirror under mStatusLock.
   mState = state;
   {
      OsLock lock(mStatusLock);
      mFailureReason       = failureReason;
      mNegotiatedSuite     = negotiatedSuite;
      mFingerprintVerified = fingerprintVerified;
   }

   // Centralized timer cleanup on terminal-state transitions. Caller
   // already holds mLock (all updateStatus call sites do).
   if (state == DTLS_STATE_ACTIVE || state == DTLS_STATE_FAILED)
   {
      disarmRetransmitTimer();
      disarmHandshakeTimeout();
   }

   // Notification dispatch on terminal-state *transitions* only, so we
   // never double-fire if updateStatus is called twice with the same
   // state. Caller already holds mLock.
   if (enteringActive)
   {
      postCompleteNotification();
   }
   else if (enteringFailed)
   {
      postFailedNotification(failureReason);
   }
}

SdpMediaLine::SdpCryptoSuiteType
MpDtls::srtpProfileFromOpenSslId(int sslProfileId)
{
   // Map OpenSSL's SRTP_* profile constants (from <openssl/srtp.h>) to
   // our SdpCryptoSuiteType. Only the suites with both a use_srtp
   // registration AND a libsrtp policy are supported here.
   switch (sslProfileId)
   {
   case SRTP_AES128_CM_SHA1_80:
      return SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80;
   case SRTP_AES128_CM_SHA1_32:
      return SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32;
   case SRTP_AEAD_AES_128_GCM:
      return SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM;
   case SRTP_AEAD_AES_256_GCM:
      return SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM;
   default:
      return SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;
   }
}

#endif

const char* MpDtls::openSslSrtpProfileName(SdpMediaLine::SdpCryptoSuiteType suite)
{
   switch (suite)
   {
   case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80:
      return "SRTP_AES128_CM_SHA1_80";
   case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32:
      return "SRTP_AES128_CM_SHA1_32";
   case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM:
      return "SRTP_AEAD_AES_128_GCM";
   case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM:
      return "SRTP_AEAD_AES_256_GCM";
   default:
      return NULL;
   }
}

UtlBoolean MpDtls::isSrtpCryptoSuiteDtlsEligible(SdpMediaLine::SdpCryptoSuiteType suite)
{
   // Two hurdles: must have an RFC 5764 use_srtp registration, AND the
   // linked-in libsrtp must have the cipher.
   if (openSslSrtpProfileName(suite) == NULL)
   {
      return FALSE;
   }
   return MpSrtp::isCryptoSuiteSupported(suite) ? TRUE : FALSE;
}

#ifdef HAVE_SSL
UtlString MpDtls::buildOpenSslProfileList(int numProfiles,
                                          const SdpMediaLine::SdpCryptoSuiteType* profiles)
{
   // OpenSSL wants a colon-separated list of registered profile names.
   //
   // Precedence:
   //   1. Caller's per-connection list, if non-empty.
   //   2. Process-wide default set via setDefaultProfiles, if non-empty.
   //   3. Engine built-in default.
   //
   // Note: we silently skip suites that have no DTLS-SRTP equivalent
   // (e.g. AES-192, F8) rather than failing the whole list, since the
   // caller may legitimately have an SDES-oriented preference list.
   //
   // We also silently skip suites whose underlying cipher is missing
   // from the linked-in libsrtp build (typically GCM on a libsrtp built
   // with no external crypto backend). MpSrtp::isCryptoSuiteSupported()
   // is populated at MpSrtp::globalInitialize() time. The built-in
   // default list flows through this filter too, so on a non-GCM
   // libsrtp build the engine default naturally degrades to the two
   // CM_128 SHA1 suites.
   UtlString out;

   const SdpMediaLine::SdpCryptoSuiteType  builtInProfiles[] = {
      SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM,
      SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM,
      SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80,
      SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32
   };
   const int builtInCount = (int)(sizeof(builtInProfiles) / sizeof(builtInProfiles[0]));

   // Snapshot the factory default under lock so we don't race with
   // setDefaultProfiles().
   SdpMediaLine::SdpCryptoSuiteType factoryDefault[MAX_DEFAULT_PROFILES];
   int                              factoryDefaultCount = 0;
   {
      OsLock lock(sDefaultProfilesLock);
      factoryDefaultCount = sDefaultProfileCount;
      for (int i = 0; i < factoryDefaultCount; i++)
      {
         factoryDefault[i] = sDefaultProfiles[i];
      }
   }

   const SdpMediaLine::SdpCryptoSuiteType* list  = profiles;
   int                                     count = numProfiles;
   if (count <= 0 || list == NULL)
   {
      if (factoryDefaultCount > 0)
      {
         list  = factoryDefault;
         count = factoryDefaultCount;
      }
      else
      {
         list  = builtInProfiles;
         count = builtInCount;
      }
   }

   for (int i = 0; i < count; i++)
   {
      const char* name = openSslSrtpProfileName(list[i]);
      if (name == NULL)
      {
         // No DTLS-SRTP use_srtp registration -- skip silently.
         continue;
      }
      if (!MpSrtp::isCryptoSuiteSupported(list[i]))
      {
         // libsrtp wasn't built with the crypto for this suite -- skip
         // silently. Negotiating it would just fail at SRTP key install
         // time after a successful DTLS handshake.
         continue;
      }
      if (!out.isNull())
      {
         out.append(":");
      }
      out.append(name);
   }
   return out;
}

void MpDtls::armRetransmitTimer()
{
   // Caller holds mLock.
   if (mpSsl == NULL)
   {
      return;
   }

   struct timeval tv;
   tv.tv_sec = 0;
   tv.tv_usec = 0;
   int rc = DTLSv1_get_timeout(mpSsl, &tv);
   if (rc != 1)
   {
      // No timeout requested at this stage of the handshake.
      disarmRetransmitTimer();
      return;
   }

   // Build the timer + callback lazily.
   if (mpRetransmitCallback == NULL)
   {
      mpRetransmitCallback = new OsCallback((intptr_t)this, retransmitTimerCallback);
   }
   if (mpRetransmitTimer == NULL)
   {
      mpRetransmitTimer = new OsTimer(*mpRetransmitCallback);
   }

   // OsTimer::oneshotAfter cancels any prior pending fire and rearms.
   OsTime when((unsigned long)tv.tv_sec, (unsigned long)tv.tv_usec);
   mpRetransmitTimer->oneshotAfter(when);
}

void MpDtls::disarmRetransmitTimer()
{
   // Caller holds mLock.
   if (mpRetransmitTimer != NULL)
   {
      mpRetransmitTimer->stop();
   }
}

void MpDtls::armHandshakeTimeout()
{
   // Caller holds mLock.
   if (mpHandshakeTimeoutCallback == NULL)
   {
      mpHandshakeTimeoutCallback =
         new OsCallback((intptr_t)this, handshakeTimeoutCallback);
   }
   if (mpHandshakeTimeoutTimer == NULL)
   {
      mpHandshakeTimeoutTimer = new OsTimer(*mpHandshakeTimeoutCallback);
   }

   OsTime when((unsigned long)sHandshakeTimeoutSeconds, 0UL);
   mpHandshakeTimeoutTimer->oneshotAfter(when);
}

void MpDtls::disarmHandshakeTimeout()
{
   // Caller holds mLock.
   if (mpHandshakeTimeoutTimer != NULL)
   {
      mpHandshakeTimeoutTimer->stop();
   }
}

#else  // !HAVE_SSL

void MpDtls::armRetransmitTimer()      {}
void MpDtls::disarmRetransmitTimer()   {}
void MpDtls::armHandshakeTimeout()     {}
void MpDtls::disarmHandshakeTimeout()  {}

#endif  // HAVE_SSL

/* ============================ TIMER ENTRY POINTS ======================== */
//
// The methods below are unconditional (compile regardless of ENABLE_SRTP).
// They run on the media thread (handle*) or the timer thread (callbacks).
// All they do is marshal between threads via the flowgraph queue and call
// into the engine; the engine itself stubs out cleanly when ENABLE_SRTP
// is not defined.
//

void MpDtls::handleRetransmit()
{
#ifdef HAVE_SSL
   OsLock lock(mLock);
   if (mpSsl == NULL || mState != DTLS_STATE_HANDSHAKING)
   {
      return;
   }

   int rc = DTLSv1_handle_timeout(mpSsl);
   if (rc < 0)
   {
      logOpenSslError("MpDtls::handleRetransmit (DTLSv1_handle_timeout)");
      updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_PROTOCOL_ERROR,
                   SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
      return;
   }
   // rc == 1: retransmit produced bytes; rc == 0: nothing to retransmit
   // right now (handshake state advanced concurrently). Either way,
   // drain any pending outbound bytes and re-arm.
   drainOutgoing();
   armRetransmitTimer();
#endif
}

void MpDtls::handleHandshakeTimeout()
{
#ifdef HAVE_SSL
   OsLock lock(mLock);
   if (mState != DTLS_STATE_HANDSHAKING)
   {
      // Already done one way or another. Stale fire; ignore.
      return;
   }

   OsSysLog::add(FAC_MP, PRI_ERR,
      "MpDtls::handleHandshakeTimeout: handshake did not complete within %d seconds",
      sHandshakeTimeoutSeconds);
   updateStatus(DTLS_STATE_FAILED, DTLS_FAIL_TIMEOUT,
                SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, FALSE);
#endif
}

void MpDtls::postTimerMessage(int msgType)
{
   // Runs on the timer thread. Acquire mLock to read mpFlowgraphQueue
   // and mFromNetResourceName safely.
   OsMsgQ*   queue = NULL;
   UtlString resourceName;
   {
      OsLock lock(mLock);
      queue = mpFlowgraphQueue;
      resourceName = mFromNetResourceName;
   }
   if (queue == NULL || resourceName.isNull())
   {
      return;
   }
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)msgType, resourceName);
   queue->send(msg, OsTime::OS_INFINITY);
}

void MpDtls::postCompleteNotification()
{
   // Caller holds mLock.
   if (mpNotificationDispatcher == NULL)
   {
      return;
   }
   // Use the FromNet resource name as the originator -- it's the
   // resource that hosted the handshake state machine on this side.
   MpResNotificationMsg msg(MpResNotificationMsg::MPRNM_DTLS_HANDSHAKE_COMPLETE,
                            mFromNetResourceName,
                            mConnectionId);
   mpNotificationDispatcher->post(msg);
}

void MpDtls::postFailedNotification(FailureReason reason)
{
   // Caller holds mLock.
   if (mpNotificationDispatcher == NULL)
   {
      return;
   }
   MprnIntMsg msg(MpResNotificationMsg::MPRNM_DTLS_HANDSHAKE_FAILED,
                  mFromNetResourceName,
                  (int)reason,
                  mConnectionId);
   mpNotificationDispatcher->post(msg);
}

void MpDtls::retransmitTimerCallback(const intptr_t userData,
                                     const intptr_t /*eventData*/)
{
   MpDtls* self = (MpDtls*)userData;
   if (self != NULL)
   {
      self->postTimerMessage(MpResourceMsg::MPRM_DTLS_RETRANSMIT);
   }
}

void MpDtls::handshakeTimeoutCallback(const intptr_t userData,
                                      const intptr_t /*eventData*/)
{
   MpDtls* self = (MpDtls*)userData;
   if (self != NULL)
   {
      self->postTimerMessage(MpResourceMsg::MPRM_DTLS_HANDSHAKE_TIMEOUT);
   }
}
