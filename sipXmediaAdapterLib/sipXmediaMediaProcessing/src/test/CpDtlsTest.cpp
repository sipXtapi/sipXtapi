//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////


#if defined(ENABLE_SRTP) && defined(HAVE_SSL)

#include <sipxunittests.h>
#include "mi/CpMediaInterfaceFactory.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"
#include "CpTopologyGraphInterface.h"
#include "mi/CpMediaInterface.h"
#include "mi/MiNotification.h"
#include "mi/MiIntNotf.h"

#include <mp/MpDtls.h>
#include <mp/MpDtlsIdentity.h>

#include <sdp/SdpMediaLine.h>
#include <sdp/SdpCodecList.h>

#include <os/OsTask.h>
#include <os/OsSocket.h>
#include <os/OsNatDatagramSocket.h>
#include <os/OsMsgDispatcher.h>
#include <os/OsSysLog.h>

#include <utl/UtlSList.h>


// Defaults for Media Interface Factory and Media Interface initialization.
#define FRAME_SIZE_MS       0
#define MAX_SAMPLE_RATE     0
#define DEFAULT_SAMPLE_RATE 0

// Test ports. Picked to not collide with the existing CpCryptoTest range.
#define DTLS_TEST_PORT_A    16000
#define DTLS_TEST_PORT_B    17000

// How long to wait for a handshake to complete in tests where it should
// succeed quickly (loopback).
#define HANDSHAKE_SUCCESS_TIMEOUT_MS  3000

// Polling interval while waiting for state transitions.
#define POLL_INTERVAL_MS  25

#ifdef WIN32
#include <string>
static std::string getExecutableDir()
{
   char buf[MAX_PATH];
   memset(buf, 0, sizeof(buf));
   DWORD len = GetModuleFileNameA(GetModuleHandle(NULL), buf, sizeof(buf) - 1);
   // Remove executable name
   for (char* p = buf + len; p > buf; p--) {
      if (*p == '\\') {
         *p = 0;
         break;
      }
   }
   return std::string(buf);
}
#endif

/// Simple synchronous notification queue used to verify that DTLS
/// notifications are actually dispatched to the application.
class TestNotificationDispatcher : public OsMsgDispatcher
{
public:
   TestNotificationDispatcher() : mLock(OsMutex::Q_FIFO) {}
   virtual ~TestNotificationDispatcher() {}

   virtual OsStatus post(const OsMsg& msg)
   {
      OsLock lock(mLock);
      OsMsg* copy = msg.createCopy();
      if (copy != NULL)
      {
         mMessages.append(new UtlVoidPtr(copy));
      }
      return OS_SUCCESS;
   }

   virtual OsStatus receive(OsMsg*& rpMsg, const OsTime& rTimeout = OsTime::OS_INFINITY)
   {
      return OS_NOT_SUPPORTED;
   }

   /// Return the number of messages currently queued.
   int count()
   {
      OsLock lock(mLock);
      return (int)mMessages.entries();
   }

   /// Pop the front message (caller takes ownership). NULL if empty.
   OsMsg* pop()
   {
      OsLock lock(mLock);
      UtlVoidPtr* slot = (UtlVoidPtr*)mMessages.get();
      if (slot == NULL) return NULL;
      OsMsg* msg = (OsMsg*)slot->getValue();
      delete slot;
      return msg;
   }

   /// Wait up to timeoutMs for at least one queued message of the
   /// given MiNotification subtype. Returns the message (caller owns)
   /// or NULL on timeout. Non-matching messages are popped and freed.
   MiNotification* waitFor(MiNotification::NotfType notfType, int timeoutMs)
   {
      int elapsed = 0;
      while (elapsed < timeoutMs)
      {
         OsMsg* m = pop();
         if (m != NULL)
         {
            MiNotification* mi = dynamic_cast<MiNotification*>(m);
            if (mi != NULL && mi->getType() == notfType)
            {
               return mi;
            }
            delete m;
            continue;     // try next without sleeping
         }
         OsTask::delay(POLL_INTERVAL_MS);
         elapsed += POLL_INTERVAL_MS;
      }
      return NULL;
   }

private:
   OsMutex   mLock;
   UtlSList  mMessages;
};


class CpDtlsTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(CpDtlsTest);
   CPPUNIT_TEST(testFingerprintAutoGeneration);
   CPPUNIT_TEST(testFingerprintHashAlgorithms);
   CPPUNIT_TEST(testParamValidation);
   CPPUNIT_TEST(testHandshakeSucceedsLoopback);
   CPPUNIT_TEST(testHandshakeTimeout);
   CPPUNIT_TEST(testHandshakeCompleteNotification);
   CPPUNIT_TEST(testFingerprintMismatchFailsAndNotifies);
   CPPUNIT_TEST(testProfileNegotiationIntersection);
   CPPUNIT_TEST(testHandshakeAndAudioFlow);
   CPPUNIT_TEST_SUITE_END();

public:

   CpMediaInterfaceFactory* mpMediaFactory;

   /* --- Fixture setup/teardown ------------------------------------------ */

   virtual void setUp()
   {
      enableConsoleOutput(0);

      UtlString codecPaths[] = {
#ifdef WIN32
         "..\\sipXmediaLib\\bin",
         "..\\..\\sipXmediaLib\\bin",
         getExecutableDir().c_str(),
#elif __pingtel_on_posix__
         "../../../../../sipXmediaLib/bin",
         "../../../../sipXmediaLib/bin",
#else
#                                  error "Unknown platform"
#endif
         "."
      };
      int codecPathsNum = sizeof(codecPaths) / sizeof(codecPaths[0]);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpMediaInterfaceFactory::addCodecPaths(codecPathsNum, codecPaths));

      mpMediaFactory = sipXmediaFactoryFactory(NULL,
                                               FRAME_SIZE_MS,
                                               MAX_SAMPLE_RATE,
                                               DEFAULT_SAMPLE_RATE,
                                               TRUE);
      CPPUNIT_ASSERT(mpMediaFactory != NULL);
   }

   virtual void tearDown()
   {
      sipxDestroyMediaFactoryFactory();
      CpMediaInterfaceFactory::clearCodecPaths();
      mpMediaFactory = NULL;
   }

   /* --- Helpers --------------------------------------------------------- */

   /// Wait up to timeoutMs for the given engine to reach a terminal
   /// state (ACTIVE or FAILED). Returns TRUE on terminal state, FALSE
   /// on timeout.
   UtlBoolean waitForTerminalState(CpTopologyGraphInterface* ti,
                                   int                       connectionId,
                                   int                       timeoutMs,
                                   UtlBoolean&               outComplete,
                                   UtlBoolean&               outFingerprintVerified,
                                   SdpMediaLine::SdpCryptoSuiteType& outSuite)
   {
      int elapsed = 0;
      while (elapsed < timeoutMs)
      {
         OsStatus s = ti->getDtlsSrtpStatus(connectionId,
                                            CpMediaInterface::AUDIO_STREAM,
                                            outComplete,
                                            outFingerprintVerified,
                                            outSuite);
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, s);
         if (outComplete)
         {
            return TRUE;
         }
         OsTask::delay(POLL_INTERVAL_MS);
         elapsed += POLL_INTERVAL_MS;
      }
      return FALSE;
   }

   /// Build a CpTopologyGraphInterface bound to the given local port,
   /// with a connection created on a fresh socket pair. Returns the
   /// interface and connectionId by out-param. Caller must release()
   /// the interface.
   void buildEndpoint(int localPort,
                      const UtlString& localAddress,
                      CpTopologyGraphInterface*& outInterface,
                      int&                       outConnectionId)
   {
      CpMediaInterface* mi = mpMediaFactory->createMediaInterface(
                                NULL, localAddress, 0, NULL,
                                UtlString(), 0, UtlString(), 0, 25,
                                UtlString(), 0, UtlString(), UtlString(),
                                25, false);
      CPPUNIT_ASSERT(mi != NULL);
      CPPUNIT_ASSERT_EQUAL(UtlString("CpTopologyGraphInterface"), mi->getType());

      CpTopologyGraphInterface* ti = (CpTopologyGraphInterface*)mi;

      int connectionId = -1;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           ti->createConnection(connectionId,
                                                localAddress,
                                                localPort));
      CPPUNIT_ASSERT(connectionId > 0);

      outInterface     = ti;
      outConnectionId  = connectionId;
   }

   /* ============== Fingerprint auto-generation ==================== */

   void testFingerprintAutoGeneration()
   {
      // Without setIdentity, getLocalDtlsFingerprint should auto-generate
      // and return a non-empty fingerprint.
      UtlString fp1;
      OsStatus s = CpTopologyGraphInterface::getLocalDtlsFingerprint(fp1);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, s);
      CPPUNIT_ASSERT_MESSAGE("auto-gen fingerprint is empty", !fp1.isNull());

      // SHA-256 fingerprint is 32 bytes -> 64 hex chars + 31 colons = 95.
      CPPUNIT_ASSERT_EQUAL(95, (int)fp1.length());

      // Second call returns the same value (singleton identity).
      UtlString fp2;
      s = CpTopologyGraphInterface::getLocalDtlsFingerprint(fp2);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, s);
      CPPUNIT_ASSERT_EQUAL(fp1, fp2);
   }


   /* ============== Different hash algorithms ==================== */

   void testFingerprintHashAlgorithms()
   {
      // SHA-256: 32 bytes -> 95 chars
      UtlString fp;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp, "SHA-256"));
      CPPUNIT_ASSERT_EQUAL(95, (int)fp.length());

      // SHA-1: 20 bytes -> 40 hex + 19 colons = 59 chars
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp, "SHA-1"));
      CPPUNIT_ASSERT_EQUAL(59, (int)fp.length());

      // SHA-384: 48 bytes -> 96 hex + 47 colons = 143 chars
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp, "SHA-384"));
      CPPUNIT_ASSERT_EQUAL(143, (int)fp.length());

      // SHA-512: 64 bytes -> 128 hex + 63 colons = 191 chars
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp, "SHA-512"));
      CPPUNIT_ASSERT_EQUAL(191, (int)fp.length());

      // Bogus algorithm -> rejected.
      CPPUNIT_ASSERT_EQUAL(OS_INVALID_ARGUMENT,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp, "MD5"));
   }


   /* ============== API parameter validation ====================== */

   void testParamValidation()
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      CpTopologyGraphInterface* ti;
      int connId;
      buildEndpoint(DTLS_TEST_PORT_A, localAddress, ti, connId);

      UtlString fp;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp));

      // ACTPASS is only valid in SDP offers; SIP layer must resolve it.
      CPPUNIT_ASSERT_EQUAL(OS_INVALID_ARGUMENT,
         ti->setDtlsSrtpParams(connId, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
                               SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTPASS));

      // NONE is nonsensical.
      CPPUNIT_ASSERT_EQUAL(OS_INVALID_ARGUMENT,
         ti->setDtlsSrtpParams(connId, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
                               SdpMediaLine::TCP_SETUP_ATTRIBUTE_NONE));

      // HOLDCONN is nonsensical for media.
      CPPUNIT_ASSERT_EQUAL(OS_INVALID_ARGUMENT,
         ti->setDtlsSrtpParams(connId, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
                               SdpMediaLine::TCP_SETUP_ATTRIBUTE_HOLDCONN));

      // Bogus hash algorithm -> rejected.
      CPPUNIT_ASSERT_EQUAL(OS_INVALID_ARGUMENT,
         ti->setDtlsSrtpParams(connId, CpMediaInterface::AUDIO_STREAM, fp, "MD5",
                               SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE));

      // setDtlsSrtpProfiles with an SDES-only suite -> rejected.
      SdpMediaLine::SdpCryptoSuiteType bad[] = {
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_80
      };
      CPPUNIT_ASSERT_EQUAL(OS_INVALID_ARGUMENT,
         CpTopologyGraphInterface::setDtlsSrtpProfiles(1, bad));

      // setDtlsSrtpProfiles with a valid suite is accepted.
      SdpMediaLine::SdpCryptoSuiteType good[] = {
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80
      };
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::setDtlsSrtpProfiles(1, good));
      // Reset.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::setDtlsSrtpProfiles(0, NULL));

      // setDtlsHandshakeTimeout: zero / negative rejected.
      CPPUNIT_ASSERT_EQUAL(OS_INVALID_ARGUMENT,
         CpTopologyGraphInterface::setDtlsHandshakeTimeout(0));
      CPPUNIT_ASSERT_EQUAL(OS_INVALID_ARGUMENT,
         CpTopologyGraphInterface::setDtlsHandshakeTimeout(-5));

      // Reasonable value accepted; restore default for other tests.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::setDtlsHandshakeTimeout(20));

      ti->deleteConnection(connId);
      ti->release();
   }


   /* ============== Successful loopback handshake ================== */

   void testHandshakeSucceedsLoopback()
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      // Reset to factory defaults so this test isn't affected by leftovers
      // from testParamValidation.
      CpTopologyGraphInterface::setDtlsSrtpProfiles(0, NULL);
      CpTopologyGraphInterface::setDtlsHandshakeTimeout(20);

      // Both endpoints share the same MpDtlsIdentity (singleton in
      // process), so both end up advertising the same fingerprint.
      // Per RFC 5763 there's no requirement that peer fingerprints
      // differ -- only that each side's cert matches what they
      // advertised. Self-pairing is a legitimate configuration.
      UtlString fp;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp));
      CPPUNIT_ASSERT(!fp.isNull());

      // ---- Build endpoint A (DTLS client / "active") ----
      CpTopologyGraphInterface* tiA;
      int connIdA;
      buildEndpoint(DTLS_TEST_PORT_A, localAddress, tiA, connIdA);

      // ---- Build endpoint B (DTLS server / "passive") ----
      CpTopologyGraphInterface* tiB;
      int connIdB;
      buildEndpoint(DTLS_TEST_PORT_B, localAddress, tiB, connIdB);

      // Configure DTLS-SRTP params on both ends. Each side passes the
      // PEER's fingerprint as remoteFingerprint -- here that's the same
      // string since both share the singleton identity.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setDtlsSrtpParams(connIdA, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
                                SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->setDtlsSrtpParams(connIdB, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
                                SdpMediaLine::TCP_SETUP_ATTRIBUTE_PASSIVE));

      // Wire up destinations. A points at B's port, and vice versa.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setConnectionDestination(connIdA,
                                       localAddress.data(),
                                       DTLS_TEST_PORT_B,
                                       DTLS_TEST_PORT_B + 1,
                                       0, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->setConnectionDestination(connIdB,
                                       localAddress.data(),
                                       DTLS_TEST_PORT_A,
                                       DTLS_TEST_PORT_A + 1,
                                       0, 0));

      // Start RTP receive on both ends so MprFromNet is wired up to
      // demux DTLS records and feed them to MpDtls. We don't need
      // startRtpSend for the handshake to complete, since DTLS bytes
      // are written directly by MpDtls, not through MprToNet.
      SdpCodecList      codecList;
      int               numCodecs = 0;
      SdpCodec**        codecs = NULL;
      UtlString         capAddr;
      int               capRtpPort = 0, capRtcpPort = 0;
      int               capVideoRtp = 0, capVideoRtcp = 0;
      SdpSrtpParameters capSrtp;
      int               capVideoBw = 0, capVideoFr = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->getCapabilities(connIdA, capAddr, capRtpPort, capRtcpPort,
            capVideoRtp, capVideoRtcp,
            codecList, capSrtp,
            /*bandWidth=*/0, capVideoBw, capVideoFr));
      codecList.getCodecs(numCodecs, codecs);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->startRtpReceive(connIdA, numCodecs, codecs));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->startRtpReceive(connIdB, numCodecs, codecs));

      // Wait for both sides to reach a terminal state.
      UtlBoolean completeA = FALSE, verifiedA = FALSE;
      UtlBoolean completeB = FALSE, verifiedB = FALSE;
      SdpMediaLine::SdpCryptoSuiteType suiteA = SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;
      SdpMediaLine::SdpCryptoSuiteType suiteB = SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;

      UtlBoolean okA = waitForTerminalState(tiA, connIdA,
                                            HANDSHAKE_SUCCESS_TIMEOUT_MS,
                                            completeA, verifiedA, suiteA);
      UtlBoolean okB = waitForTerminalState(tiB, connIdB,
                                            HANDSHAKE_SUCCESS_TIMEOUT_MS,
                                            completeB, verifiedB, suiteB);

      CPPUNIT_ASSERT_MESSAGE("A did not reach terminal state in time", okA);
      CPPUNIT_ASSERT_MESSAGE("B did not reach terminal state in time", okB);
      CPPUNIT_ASSERT_MESSAGE("A handshake did not complete",        completeA);
      CPPUNIT_ASSERT_MESSAGE("B handshake did not complete",        completeB);
      CPPUNIT_ASSERT_MESSAGE("A fingerprint not verified",          verifiedA);
      CPPUNIT_ASSERT_MESSAGE("B fingerprint not verified",          verifiedB);
      CPPUNIT_ASSERT_MESSAGE("A negotiated suite is NONE",
         suiteA != SdpMediaLine::CRYPTO_SUITE_TYPE_NONE);
      CPPUNIT_ASSERT_MESSAGE("B negotiated suite is NONE",
         suiteB != SdpMediaLine::CRYPTO_SUITE_TYPE_NONE);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A and B negotiated different suites",
                                   suiteA, suiteB);

      // Cleanup.
      tiA->deleteConnection(connIdA);
      tiB->deleteConnection(connIdB);
      tiA->release();
      tiB->release();

      for (int i = 0; i < numCodecs; i++)
      {
         delete codecs[i];
      }
      delete[] codecs;
   }


   /* ============== Hard handshake timeout ========================= */

   void testHandshakeTimeout()
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      // Tighten the timeout for this test so we don't sit for 20s.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::setDtlsHandshakeTimeout(2));

      UtlString fp;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp));

      // Build endpoint A only -- no peer B. A's handshake will never
      // get a response and should time out after 2 seconds.
      CpTopologyGraphInterface* tiA;
      int connIdA;
      buildEndpoint(DTLS_TEST_PORT_A, localAddress, tiA, connIdA);

      // Configure A as DTLS client. Point at a port nobody is listening on.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setDtlsSrtpParams(connIdA, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
                                SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setConnectionDestination(connIdA,
                                       localAddress.data(),
                                       DTLS_TEST_PORT_B,
                                       DTLS_TEST_PORT_B + 1,
                                       0, 0));

      // We need MprFromNet wired up so the timer-fired message gets
      // dispatched to MpDtls. startRtpReceive does that.
      SdpCodecList      codecList;
      int               numCodecs = 0;
      SdpCodec** codecs = NULL;
      UtlString         capAddr;
      int               capRtpPort = 0, capRtcpPort = 0;
      int               capVideoRtp = 0, capVideoRtcp = 0;
      SdpSrtpParameters capSrtp;
      int               capVideoBw = 0, capVideoFr = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->getCapabilities(connIdA, capAddr, capRtpPort, capRtcpPort,
            capVideoRtp, capVideoRtcp,
            codecList, capSrtp,
            /*bandWidth=*/0, capVideoBw, capVideoFr));
      codecList.getCodecs(numCodecs, codecs);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->startRtpReceive(connIdA, numCodecs, codecs));

      // Wait up to 4 seconds for terminal state. Should fire at ~2s.
      UtlBoolean complete = FALSE, verified = FALSE;
      SdpMediaLine::SdpCryptoSuiteType suite = SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;
      UtlBoolean ok = waitForTerminalState(tiA, connIdA,
                                           4000,
                                           complete, verified, suite);
      CPPUNIT_ASSERT_MESSAGE("A did not reach terminal state in 4s", ok);

      // Should have completed (terminal-state-wise) but FAILED, not active.
      // We can't directly read mState/mFailureReason via getDtlsSrtpStatus
      // (it only exposes "complete + verified + suite"), but a complete
      // handshake with verified=FALSE is the failure signature.
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Timed-out handshake should not be 'verified'",
                                   FALSE, verified);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Timed-out handshake should have no suite",
                                   SdpMediaLine::CRYPTO_SUITE_TYPE_NONE, suite);

      // Cleanup, restore default timeout.
      tiA->deleteConnection(connIdA);
      tiA->release();

      CpTopologyGraphInterface::setDtlsHandshakeTimeout(20);

      for (int i = 0; i < numCodecs; i++)
      {
         delete codecs[i];
      }
      delete[] codecs;
   }


   /* ============== Handshake-complete notification ================ */

   void testHandshakeCompleteNotification()
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      CpTopologyGraphInterface::setDtlsSrtpProfiles(0, NULL);
      CpTopologyGraphInterface::setDtlsHandshakeTimeout(20);

      UtlString fp;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp));

      // Build both endpoints.
      CpTopologyGraphInterface* tiA;
      int connIdA;
      buildEndpoint(DTLS_TEST_PORT_A, localAddress, tiA, connIdA);

      CpTopologyGraphInterface* tiB;
      int connIdB;
      buildEndpoint(DTLS_TEST_PORT_B, localAddress, tiB, connIdB);

      // Install a notification dispatcher on each endpoint so we can
      // verify both ends actually receive the COMPLETE notification.
      TestNotificationDispatcher dispA;
      TestNotificationDispatcher dispB;
      tiA->setNotificationDispatcher(&dispA);
      tiB->setNotificationDispatcher(&dispB);

      // Configure DTLS-SRTP. Note dispatchers must be installed before
      // setDtlsSrtpParams since that's when MpDtls captures the pointer.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setDtlsSrtpParams(connIdA, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
                                SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->setDtlsSrtpParams(connIdB, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
                                SdpMediaLine::TCP_SETUP_ATTRIBUTE_PASSIVE));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setConnectionDestination(connIdA, localAddress.data(),
            DTLS_TEST_PORT_B, DTLS_TEST_PORT_B + 1, 0, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->setConnectionDestination(connIdB, localAddress.data(),
            DTLS_TEST_PORT_A, DTLS_TEST_PORT_A + 1, 0, 0));

      SdpCodecList      codecList;
      int               numCodecs = 0;
      SdpCodec**        codecs = NULL;
      UtlString         capAddr;
      int               capRtpPort = 0, capRtcpPort = 0;
      int               capVideoRtp = 0, capVideoRtcp = 0;
      SdpSrtpParameters capSrtp;
      int               capVideoBw = 0, capVideoFr = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->getCapabilities(connIdA, capAddr, capRtpPort, capRtcpPort,
            capVideoRtp, capVideoRtcp, codecList, capSrtp,
            /*bandWidth=*/0, capVideoBw, capVideoFr));
      codecList.getCodecs(numCodecs, codecs);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, tiA->startRtpReceive(connIdA, numCodecs, codecs));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, tiB->startRtpReceive(connIdB, numCodecs, codecs));

      // Wait for both COMPLETE notifications to arrive.
      MiNotification* notfA = dispA.waitFor(
         MiNotification::MI_NOTF_DTLS_HANDSHAKE_COMPLETE,
         HANDSHAKE_SUCCESS_TIMEOUT_MS);
      CPPUNIT_ASSERT_MESSAGE("A did not receive COMPLETE notification", notfA != NULL);

      MiNotification* notfB = dispB.waitFor(
         MiNotification::MI_NOTF_DTLS_HANDSHAKE_COMPLETE,
         HANDSHAKE_SUCCESS_TIMEOUT_MS);
      CPPUNIT_ASSERT_MESSAGE("B did not receive COMPLETE notification", notfB != NULL);

      // Verify the connection IDs match what we configured.
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A notification carries wrong connectionId",
                                   connIdA, notfA->getConnectionId());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("B notification carries wrong connectionId",
                                   connIdB, notfB->getConnectionId());

      delete notfA;
      delete notfB;

      // Cleanup. Detach dispatchers before tearing down so dangling
      // dispatcher pointers don't get used during destruction.
      tiA->setNotificationDispatcher(NULL);
      tiB->setNotificationDispatcher(NULL);
      tiA->deleteConnection(connIdA);
      tiB->deleteConnection(connIdB);
      tiA->release();
      tiB->release();

      for (int i = 0; i < numCodecs; i++)
      {
         delete codecs[i];
      }
      delete[] codecs;
   }


   /* ============== Fingerprint mismatch ========================== */

   void testFingerprintMismatchFailsAndNotifies()
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      CpTopologyGraphInterface::setDtlsSrtpProfiles(0, NULL);
      // Slightly tighter timeout so a hung handshake doesn't drag this
      // test out, but still enough headroom for normal handshake flights.
      CpTopologyGraphInterface::setDtlsHandshakeTimeout(5);

      UtlString correctFp;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(correctFp));

      // Build a bogus fingerprint of the same length (95 chars for SHA-256)
      // that is guaranteed not to match the real cert.
      UtlString bogusFp;
      for (int i = 0; i < 32; i++)
      {
         if (i > 0) bogusFp.append(":");
         bogusFp.append("AB");
      }

      CpTopologyGraphInterface* tiA;
      int connIdA;
      buildEndpoint(DTLS_TEST_PORT_A, localAddress, tiA, connIdA);

      CpTopologyGraphInterface* tiB;
      int connIdB;
      buildEndpoint(DTLS_TEST_PORT_B, localAddress, tiB, connIdB);

      TestNotificationDispatcher dispA;
      TestNotificationDispatcher dispB;
      tiA->setNotificationDispatcher(&dispA);
      tiB->setNotificationDispatcher(&dispB);

      // Endpoint A is told the WRONG fingerprint for B. Endpoint B has
      // the correct fingerprint for A.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setDtlsSrtpParams(connIdA, CpMediaInterface::AUDIO_STREAM, bogusFp, "SHA-256",
                                SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->setDtlsSrtpParams(connIdB, CpMediaInterface::AUDIO_STREAM, correctFp, "SHA-256",
                                SdpMediaLine::TCP_SETUP_ATTRIBUTE_PASSIVE));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setConnectionDestination(connIdA, localAddress.data(),
            DTLS_TEST_PORT_B, DTLS_TEST_PORT_B + 1, 0, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->setConnectionDestination(connIdB, localAddress.data(),
            DTLS_TEST_PORT_A, DTLS_TEST_PORT_A + 1, 0, 0));

      SdpCodecList      codecList;
      int               numCodecs = 0;
      SdpCodec**        codecs = NULL;
      UtlString         capAddr;
      int               capRtpPort = 0, capRtcpPort = 0;
      int               capVideoRtp = 0, capVideoRtcp = 0;
      SdpSrtpParameters capSrtp;
      int               capVideoBw = 0, capVideoFr = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->getCapabilities(connIdA, capAddr, capRtpPort, capRtcpPort,
            capVideoRtp, capVideoRtcp, codecList, capSrtp,
            /*bandWidth=*/0, capVideoBw, capVideoFr));
      codecList.getCodecs(numCodecs, codecs);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, tiA->startRtpReceive(connIdA, numCodecs, codecs));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, tiB->startRtpReceive(connIdB, numCodecs, codecs));

      // Endpoint A: handshake completes at the protocol level (peer
      // presents a cert, we accept), then post-handshake fingerprint
      // verification fails, and we transition to FAILED with reason
      // FINGERPRINT_MISMATCH. A FAILED notification fires.
      MiNotification* failNotf = dispA.waitFor(
         MiNotification::MI_NOTF_DTLS_HANDSHAKE_FAILED,
         HANDSHAKE_SUCCESS_TIMEOUT_MS);
      CPPUNIT_ASSERT_MESSAGE("A did not receive FAILED notification",
                             failNotf != NULL);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("FAILED notification has wrong connectionId",
                                   connIdA, failNotf->getConnectionId());

      // The failure reason rides as the int payload of MiIntNotf.
      MiIntNotf* intNotf = dynamic_cast<MiIntNotf*>(failNotf);
      CPPUNIT_ASSERT_MESSAGE("FAILED notification is not MiIntNotf",
                             intNotf != NULL);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("FAILED reason should be FINGERPRINT_MISMATCH",
                                   (int)MpDtls::DTLS_FAIL_FINGERPRINT_MISMATCH,
                                   intNotf->getValue());
      delete failNotf;

      // Endpoint A's getDtlsSrtpStatus reflects the failure: complete=TRUE,
      // verified=FALSE.
      UtlBoolean completeA = FALSE, verifiedA = FALSE;
      SdpMediaLine::SdpCryptoSuiteType suiteA = SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->getDtlsSrtpStatus(connIdA, CpMediaInterface::AUDIO_STREAM,
                                completeA, verifiedA, suiteA));
      CPPUNIT_ASSERT_MESSAGE("A complete should be TRUE",  completeA);
      CPPUNIT_ASSERT_MESSAGE("A should NOT be verified",  !verifiedA);

      // Endpoint B will see one of two outcomes depending on timing:
      //   - HANDSHAKE_COMPLETE if B's verification ran before A tore
      //     down the connection (B has the correct fingerprint for A).
      //   - HANDSHAKE_FAILED with TIMEOUT if A failed before B finished.
      // We only assert that B did NOT receive a FINGERPRINT_MISMATCH.
      OsTask::delay(500);
      while (dispB.count() > 0)
      {
         OsMsg* m = dispB.pop();
         MiIntNotf* in = dynamic_cast<MiIntNotf*>(m);
         if (in != NULL && in->getType() ==
                           MiNotification::MI_NOTF_DTLS_HANDSHAKE_FAILED)
         {
            CPPUNIT_ASSERT_MESSAGE("B should never see FINGERPRINT_MISMATCH",
                                   in->getValue() !=
                                   (int)MpDtls::DTLS_FAIL_FINGERPRINT_MISMATCH);
         }
         delete m;
      }

      // Cleanup.
      tiA->setNotificationDispatcher(NULL);
      tiB->setNotificationDispatcher(NULL);
      tiA->deleteConnection(connIdA);
      tiB->deleteConnection(connIdB);
      tiA->release();
      tiB->release();
      CpTopologyGraphInterface::setDtlsHandshakeTimeout(20);

      for (int i = 0; i < numCodecs; i++)
      {
         delete codecs[i];
      }
      delete[] codecs;
   }


   /* ============== SRTP profile negotiation ====================== */

   void testProfileNegotiationIntersection()
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      CpTopologyGraphInterface::setDtlsSrtpProfiles(0, NULL);
      CpTopologyGraphInterface::setDtlsHandshakeTimeout(20);

      UtlString fp;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp));

      CpTopologyGraphInterface* tiA;
      int connIdA;
      buildEndpoint(DTLS_TEST_PORT_A, localAddress, tiA, connIdA);

      CpTopologyGraphInterface* tiB;
      int connIdB;
      buildEndpoint(DTLS_TEST_PORT_B, localAddress, tiB, connIdB);

      // A offers ONLY HMAC_SHA1_80. B offers GCM-128 first, HMAC_SHA1_80
      // second. Intersection is HMAC_SHA1_80, which both must agree on.
      SdpMediaLine::SdpCryptoSuiteType profilesA[] = {
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80
      };
      SdpMediaLine::SdpCryptoSuiteType profilesB[] = {
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32,
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80
      };

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setDtlsSrtpParams(connIdA, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
                                SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE,
                                1, profilesA));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->setDtlsSrtpParams(connIdB, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
                                SdpMediaLine::TCP_SETUP_ATTRIBUTE_PASSIVE,
                                2, profilesB));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setConnectionDestination(connIdA, localAddress.data(),
            DTLS_TEST_PORT_B, DTLS_TEST_PORT_B + 1, 0, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->setConnectionDestination(connIdB, localAddress.data(),
            DTLS_TEST_PORT_A, DTLS_TEST_PORT_A + 1, 0, 0));

      SdpCodecList      codecList;
      int               numCodecs = 0;
      SdpCodec**        codecs = NULL;
      UtlString         capAddr;
      int               capRtpPort = 0, capRtcpPort = 0;
      int               capVideoRtp = 0, capVideoRtcp = 0;
      SdpSrtpParameters capSrtp;
      int               capVideoBw = 0, capVideoFr = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->getCapabilities(connIdA, capAddr, capRtpPort, capRtcpPort,
            capVideoRtp, capVideoRtcp, codecList, capSrtp,
            /*bandWidth=*/0, capVideoBw, capVideoFr));
      codecList.getCodecs(numCodecs, codecs);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, tiA->startRtpReceive(connIdA, numCodecs, codecs));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, tiB->startRtpReceive(connIdB, numCodecs, codecs));

      UtlBoolean completeA = FALSE, verifiedA = FALSE;
      UtlBoolean completeB = FALSE, verifiedB = FALSE;
      SdpMediaLine::SdpCryptoSuiteType suiteA = SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;
      SdpMediaLine::SdpCryptoSuiteType suiteB = SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;

      CPPUNIT_ASSERT(waitForTerminalState(tiA, connIdA, HANDSHAKE_SUCCESS_TIMEOUT_MS,
                                          completeA, verifiedA, suiteA));
      CPPUNIT_ASSERT(waitForTerminalState(tiB, connIdB, HANDSHAKE_SUCCESS_TIMEOUT_MS,
                                          completeB, verifiedB, suiteB));

      CPPUNIT_ASSERT_MESSAGE("A handshake should succeed", verifiedA);
      CPPUNIT_ASSERT_MESSAGE("B handshake should succeed", verifiedB);

      // Both sides must agree on the same suite.
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A and B should agree on negotiated suite",
                                   suiteA, suiteB);

      // The intersection -- and what we expect them to land on.
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Negotiated suite should be HMAC_SHA1_80",
                                   SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80,
                                   suiteA);

      tiA->deleteConnection(connIdA);
      tiB->deleteConnection(connIdB);
      tiA->release();
      tiB->release();

      for (int i = 0; i < numCodecs; i++)
      {
         delete codecs[i];
      }
      delete[] codecs;
   }


   /* ============== end-to-end with audio flow ============= */

   void testHandshakeAndAudioFlow()
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      CpTopologyGraphInterface::setDtlsSrtpProfiles(0, NULL);
      CpTopologyGraphInterface::setDtlsHandshakeTimeout(20);

      UtlString fp;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         CpTopologyGraphInterface::getLocalDtlsFingerprint(fp));

      CpTopologyGraphInterface* tiA;
      int connIdA;
      buildEndpoint(DTLS_TEST_PORT_A, localAddress, tiA, connIdA);

      CpTopologyGraphInterface* tiB;
      int connIdB;
      buildEndpoint(DTLS_TEST_PORT_B, localAddress, tiB, connIdB);

      // We are going to play a tone from tiA, we want to hear it on local 
      // speakers for the receiver tiB, so giveFocus
      tiB->giveFocus();

      // GCM not supported on all libSrtp builds, so force plain AES
      SdpMediaLine::SdpCryptoSuiteType aesOnly[] = {
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80
      };

      // Get Codec list for startRtpReceive and startRtpSend calls
      SdpCodecList      codecList;
      int               numCodecs = 0;
      SdpCodec**        codecs = NULL;
      UtlString         capAddr;
      int               capRtpPort = 0, capRtcpPort = 0;
      int               capVideoRtp = 0, capVideoRtcp = 0;
      SdpSrtpParameters capSrtp;
      int               capVideoBw = 0, capVideoFr = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->getCapabilities(connIdA, capAddr, capRtpPort, capRtcpPort,
            capVideoRtp, capVideoRtcp, codecList, capSrtp,
            /*bandWidth=*/0, capVideoBw, capVideoFr));
      codecList.getCodecs(numCodecs, codecs);

      // Setup receiver/server side (tiB) first, so it's ready to receive DTLS client Hello
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->setDtlsSrtpParams(connIdB, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
            SdpMediaLine::TCP_SETUP_ATTRIBUTE_PASSIVE,
            1, aesOnly));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiB->setConnectionDestination(connIdB, localAddress.data(),
            DTLS_TEST_PORT_A, DTLS_TEST_PORT_A + 1, 0, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, tiB->startRtpReceive(connIdB, numCodecs, codecs));

      // Setup sender/client side (tiA) next
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setDtlsSrtpParams(connIdA, CpMediaInterface::AUDIO_STREAM, fp, "SHA-256",
            SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE,
            1, aesOnly));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         tiA->setConnectionDestination(connIdA, localAddress.data(),
            DTLS_TEST_PORT_B, DTLS_TEST_PORT_B + 1, 0, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, tiA->startRtpReceive(connIdA, numCodecs, codecs));

      // Now start both sending so audio actually flows.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, tiA->startRtpSend(connIdA, numCodecs, codecs));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, tiB->startRtpSend(connIdB, numCodecs, codecs));

      // Wait for handshakes to complete.
      UtlBoolean completeA = FALSE, verifiedA = FALSE;
      UtlBoolean completeB = FALSE, verifiedB = FALSE;
      SdpMediaLine::SdpCryptoSuiteType suiteA = SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;
      SdpMediaLine::SdpCryptoSuiteType suiteB = SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;

      CPPUNIT_ASSERT(waitForTerminalState(tiA, connIdA, HANDSHAKE_SUCCESS_TIMEOUT_MS,
                                          completeA, verifiedA, suiteA));
      CPPUNIT_ASSERT(waitForTerminalState(tiB, connIdB, HANDSHAKE_SUCCESS_TIMEOUT_MS,
                                          completeB, verifiedB, suiteB));
      CPPUNIT_ASSERT_MESSAGE("A handshake failed", verifiedA);
      CPPUNIT_ASSERT_MESSAGE("B handshake failed", verifiedB);

      // Confirm GCM was negotiated.
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Suite must be AES_CM_128_HMAC_SHA1_80",
                                   SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80,
                                   suiteA);

      // Play tone out until connection is deleted - tiB has focus to tone should play on
      // default local speaker
      tiA->startTone(1, true, true);

      // Let audio flow for a couple of seconds. We don't validate the
      // audio content (the existing CpCryptoTest doesn't either) -- the
      // success criterion is "no crash, no assertion fires." If SRTP
      // protect/unprotect were broken we'd hit libsrtp errors and
      // probably flowgraph hangs.
      OsTask::delay(2000);

      tiA->deleteConnection(connIdA);
      tiB->deleteConnection(connIdB);
      tiA->release();
      tiB->release();

      for (int i = 0; i < numCodecs; i++)
      {
         delete codecs[i];
      }
      delete[] codecs;
   }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CpDtlsTest);

#endif  // ENABLE_SRTP
