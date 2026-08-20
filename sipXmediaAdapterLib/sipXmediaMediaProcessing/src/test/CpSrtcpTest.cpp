//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// This suite exists to cover the combination that used to be forbidden: SRTP
// and RTCP enabled at the same time.  Without RTCP there is no SRTCP to check,
// and without SRTP every MpSrtp call is a pass-through stub.
#if defined(ENABLE_SRTP) && !defined(EXCLUDE_RTCP)

#include "CpRtcpTestSupport.h"

#include <mp/MpSrtp.h>

using namespace CpRtcpTestSupport;


/**
 * End-to-end check that outbound RTCP really is SRTCP on the wire.
 *
 * MpSrtpTest covers the crypto in isolation.  What it cannot cover is whether
 * the RTCP subsystem is actually wired to it: CRTCPRender generates reports on
 * the CRTCManager thread and writes them through CNetworkChannel, a path that
 * bypasses MprToNet entirely and had no SRTP at all until SRTCP support was
 * added.
 *
 * So this suite points a real media interface's RTCP destination at a plain
 * UDP socket it owns, and inspects the datagrams that land there.  The
 * decisive assertion is that the captured bytes fail to be plain RTCP and DO
 * decrypt under the negotiated key -- MpSrtpTest::testSrtcpRejectsForeignKey
 * and ::testSrtcpRejectsTamperedPacket establish that a successful unprotect
 * is not something that happens by accident.
 *
 * Only the SDES key path is exercised here.  DTLS-SRTP additionally requires
 * HAVE_SSL and a live handshake; its RTCP behaviour is covered in CpDtlsTest.
 */
class CpSrtcpTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(CpSrtcpTest);

   CPPUNIT_TEST(testOutboundRtcpIsProtectedWithSdesKeys);
   CPPUNIT_TEST(testMuxedRtcpIsProtectedWithSdesKeys);
   CPPUNIT_TEST(testMuxedRtcpIsReceivedOnRtpPort);

   CPPUNIT_TEST_SUITE_END();

public:

   CpMediaInterfaceFactory* mpMediaFactory;
   int                      mSavedRtcpPeriod;

   // AES_CM_128_HMAC_SHA1_80 wants 16 bytes of master key followed by 14
   // bytes of master salt.  The media API takes this as raw bytes; base64 is
   // the SDP layer's concern, not this one's.
   enum { CM_128_KEY_LEN = 30 };

   virtual void setUp()
   {
      enableConsoleOutput(0);

      UtlString codecPaths[] = {
#ifdef WIN32
         "..\\sipXmediaLib\\bin",
         "..\\..\\sipXmediaLib\\bin",
         cpRtcpTestExecutableDir().c_str(),
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
                                               CP_RTCP_TEST_FRAME_SIZE_MS,
                                               CP_RTCP_TEST_MAX_SAMPLE_RATE,
                                               CP_RTCP_TEST_DEFAULT_SAMPLE_RATE,
                                               TRUE);
      CPPUNIT_ASSERT(mpMediaFactory != NULL);

      // Speed up reporting, remembering the previous value so the rest of the
      // executable is not left with this suite's RTCP cadence.
      mSavedRtcpPeriod = adjustRtcpPeriod(CP_RTCP_TEST_REPORT_PERIOD_MS);
   }

   virtual void tearDown()
   {
      adjustRtcpPeriod(mSavedRtcpPeriod);
      sipxDestroyMediaFactoryFactory();
      CpMediaInterfaceFactory::clearCodecPaths();
      mpMediaFactory = NULL;
   }

   /* --- Helpers --------------------------------------------------------- */

   static UtlString makeKey(unsigned char seed)
   {
      char raw[CM_128_KEY_LEN];
      for (int i = 0; i < CM_128_KEY_LEN; i++)
      {
         raw[i] = (char)(seed + i * 7);
      }
      return UtlString(raw, CM_128_KEY_LEN);
   }

   /// Build a media interface with one connection bound to localPort.
   void buildEndpoint(int localPort,
                      const UtlString& localAddress,
                      CpTopologyGraphInterface*& outInterface,
                      int& outConnectionId)
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
                           ti->createConnection(connectionId, localAddress, localPort));
      CPPUNIT_ASSERT(connectionId > 0);

      outInterface    = ti;
      outConnectionId = connectionId;
   }

   /// Drive one connection that sends RTCP to the collector socket, with the
   /// given crypto suite and key (NONE / empty for plain RTCP).  Fills buf
   /// with the first report captured and returns its length, 0 on timeout.
   ///
   /// With rtcpMux set, the collector stands in for the peer's RTP port and the
   /// endpoint is told multiplexing was negotiated, so reports must show up
   /// there rather than on the separate RTCP port.
   int runEndpointAndCaptureReport(SdpMediaLine::SdpCryptoSuiteType cryptoSuite,
                                   const UtlString& cryptoKey,
                                   unsigned char* buf,
                                   int bufSize,
                                   UtlBoolean rtcpMux = FALSE)
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      // Our stand-in for the far end's RTCP port.  Bound before the endpoint
      // starts sending so nothing is missed.
      OsDatagramSocket collector(0, NULL,
                                 CP_RTCP_TEST_COLLECTOR_PORT, localAddress.data());
      CPPUNIT_ASSERT_MESSAGE("could not bind RTCP collector socket",
                             collector.isOk());

      CpTopologyGraphInterface* ti;
      int connId;
      buildEndpoint(CP_RTCP_TEST_ENDPOINT_PORT, localAddress, ti, connId);

      // Must be set before startRtpSend(): that call starts the RTCP renderer,
      // which needs to know which socket to write reports to.
      if (rtcpMux)
      {
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
            ti->setRtcpMux(connId, CpMediaInterface::AUDIO_STREAM, TRUE));
      }

      SdpCodecList      codecList;
      int               numCodecs = 0;
      SdpCodec**        codecs = NULL;
      UtlString         capAddr;
      int               capRtpPort = 0, capRtcpPort = 0;
      int               capVideoRtp = 0, capVideoRtcp = 0;
      SdpSrtpParameters capSrtp;
      int               capVideoBw = 0, capVideoFr = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         ti->getCapabilities(connId, capAddr, capRtpPort, capRtcpPort,
                             capVideoRtp, capVideoRtcp, codecList, capSrtp,
                             /*bandWidth=*/0, capVideoBw, capVideoFr));
      codecList.getCodecs(numCodecs, codecs);

      // RTP goes to a port nobody is listening on; only the RTCP destination
      // matters here.  RTCP reports are emitted whether or not media flows --
      // a Receiver Report plus SDES goes out every period regardless.
      // Muxed: RTP destination IS the collector, and the RTCP port argument is
      // ignored -- deliberately pointed somewhere else to prove it is unused.
      // Unmuxed: RTP goes to a dead port and RTCP to the collector.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         ti->setConnectionDestination(connId, localAddress.data(),
                                      rtcpMux ? CP_RTCP_TEST_COLLECTOR_PORT
                                              : CP_RTCP_TEST_RTP_SINK_PORT,
                                      rtcpMux ? CP_RTCP_TEST_RTP_SINK_PORT
                                              : CP_RTCP_TEST_COLLECTOR_PORT,
                                      0, 0));

      // startRtpSend is what starts the RTCP renderer (via setSockets) and,
      // on the crypto overload, what installs the outbound SRTP key.
      if (cryptoSuite == SdpMediaLine::CRYPTO_SUITE_TYPE_NONE)
      {
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
            ti->startRtpSend(connId, numCodecs, codecs));
      }
      else
      {
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
            ti->startRtpSend(connId, numCodecs, codecs, cryptoSuite, cryptoKey));
      }

      int captured = collectOneReport(collector, buf, bufSize,
                                      CP_RTCP_TEST_COLLECT_TIMEOUT_MS);

      ti->deleteConnection(connId);
      ti->release();

      for (int i = 0; i < numCodecs; i++)
      {
         delete codecs[i];
      }
      delete[] codecs;

      return captured;
   }

   /* ============== SDES-keyed connection emits SRTCP =============== */

   void testOutboundRtcpIsProtectedWithSdesKeys()
   {
      const SdpMediaLine::SdpCryptoSuiteType suite =
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80;

      // GCM is absent from libsrtp builds without an external crypto backend;
      // AES-CM is always present.  Fail loudly rather than silently skipping.
      CPPUNIT_ASSERT_MESSAGE("libsrtp lacks AES_CM_128_HMAC_SHA1_80",
                             MpSrtp::isCryptoSuiteSupported(suite));

      const UtlString key = makeKey(0x5a);

      unsigned char report[1500];
      int len = runEndpointAndCaptureReport(suite, key, report, sizeof(report));

      CPPUNIT_ASSERT_MESSAGE("no RTCP report arrived within the timeout",
                             len > 0);

      // The first header stays in the clear, so the datagram is still RTCP
      // shaped at a glance -- which is exactly why "looks like RTCP" is not
      // sufficient evidence either way.
      CPPUNIT_ASSERT_MESSAGE("capture is not RTCP shaped",
                             looksLikeRtcp(report, len));

      // First, the cheap discriminator: a plaintext compound accounts for
      // every byte of its datagram (the control test above asserts exactly
      // that).  This one cannot, because the later sub-packet headers are
      // encrypted.
      CPPUNIT_ASSERT_MESSAGE("RTCP report was sent unprotected",
                             rtcpDeclaredLength(report, len) != len);

      // Then the real assertion: it decrypts and authenticates under the key
      // handed to startRtpSend.  A receiver keyed with anything else, or a
      // packet altered in flight, is rejected -- MpSrtpTest establishes that,
      // which is what makes a successful unprotect here meaningful.
      MpSrtp receiver;
      CPPUNIT_ASSERT(receiver.setSrtpParams(suite, key, TRUE /* forUnprotect */));

      int size = len;
      CPPUNIT_ASSERT_MESSAGE("captured report did not authenticate under the "
                             "negotiated SDES key",
         receiver.srtpUnprotectIfNeeded(report, &size, TRUE /* rtcp */));

      // SRTCP overhead for an HMAC_SHA1_80 suite: a 4 byte E-flag/index word
      // plus a 10 byte authentication tag.
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "unexpected SRTCP trailer size (want 4 byte index + 10 byte tag)",
         14, len - size);

      // And what comes out is a well formed RTCP compound, so the renderer
      // protected a real report rather than something malformed.
      CPPUNIT_ASSERT_MESSAGE("decrypted payload is not RTCP",
                             looksLikeRtcp(report, size));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("decrypted compound is malformed",
                                   size, rtcpDeclaredLength(report, size));
   }

   /// Stand up a muxed, SDES-keyed endpoint that is RECEIVING, and hand back
   /// its connection plus the port everything should be sent to.
   ///
   /// The peer here is a plain socket rather than a second media interface:
   /// that way the test controls exactly which bytes hit the port and when,
   /// which is what makes the assertions about classification unambiguous.
   void buildReceivingMuxedEndpoint(const UtlString& localAddress,
                                    SdpMediaLine::SdpCryptoSuiteType suite,
                                    const UtlString& key,
                                    CpTopologyGraphInterface*& outTi,
                                    int& outConnId,
                                    int& outNumCodecs,
                                    SdpCodec**& outCodecs)
   {
      buildEndpoint(CP_RTCP_TEST_ENDPOINT_PORT, localAddress, outTi, outConnId);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         outTi->setRtcpMux(outConnId, CpMediaInterface::AUDIO_STREAM, TRUE));

      SdpCodecList      codecList;
      UtlString         capAddr;
      int               capRtpPort = 0, capRtcpPort = 0;
      int               capVideoRtp = 0, capVideoRtcp = 0;
      SdpSrtpParameters capSrtp;
      int               capVideoBw = 0, capVideoFr = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         outTi->getCapabilities(outConnId, capAddr, capRtpPort, capRtcpPort,
                                capVideoRtp, capVideoRtcp, codecList, capSrtp,
                                /*bandWidth=*/0, capVideoBw, capVideoFr));
      outNumCodecs = 0;
      outCodecs = NULL;
      codecList.getCodecs(outNumCodecs, outCodecs);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         outTi->setConnectionDestination(outConnId, localAddress.data(),
                                         CP_RTCP_TEST_RTP_SINK_PORT,
                                         CP_RTCP_TEST_RTP_SINK_PORT, 0, 0));

      if (suite == SdpMediaLine::CRYPTO_SUITE_TYPE_NONE)
      {
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
            outTi->startRtpReceive(outConnId, outNumCodecs, outCodecs));
      }
      else
      {
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
            outTi->startRtpReceive(outConnId, outNumCodecs, outCodecs, suite, key));
      }
   }

   /* ============== rtcp-mux, end-to-end RECEIVE ==================== */

   // The send-side muxed tests prove reports leave on the RTP port. This is
   // the other half: that a report ARRIVING on the RTP port is recognised as
   // RTCP, unprotected with the SRTCP context, and accepted.
   //
   // Worth having because it is genuinely uncovered otherwise: with the
   // classification in MprFromNet::pushPacket removed entirely, every other
   // test in this suite and in CpDtlsTest still passes.
   void testMuxedRtcpIsReceivedOnRtpPort()
   {
      const SdpMediaLine::SdpCryptoSuiteType suite =
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80;
      CPPUNIT_ASSERT_MESSAGE("libsrtp lacks AES_CM_128_HMAC_SHA1_80",
                             MpSrtp::isCryptoSuiteSupported(suite));

      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      const UtlString key = makeKey(0x2e);

      CpTopologyGraphInterface* ti;
      int connId, numCodecs;
      SdpCodec** codecs;
      buildReceivingMuxedEndpoint(localAddress, suite, key,
                                  ti, connId, numCodecs, codecs);

      int rtpBefore = 0, rtcpBefore = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         ti->getRtpPacketCounts(connId, CpMediaInterface::AUDIO_STREAM,
                                rtpBefore, rtcpBefore));

      // Protect a Sender Report with the same key the endpoint was given for
      // its receive direction, and send it to the endpoint's RTP port.
      MpSrtp sender;
      CPPUNIT_ASSERT(sender.setSrtpParams(suite, key, FALSE /* forUnprotect */));

      unsigned char report[256];
      int len = buildSenderReport(report, 0x51ee7a11);
      int size = len;
      CPPUNIT_ASSERT(sender.srtpProtectIfNeeded(report, &size, TRUE /* rtcp */,
                                                sizeof(report)));
      CPPUNIT_ASSERT_MESSAGE("test packet is not SRTCP", size > len);

      OsDatagramSocket peer(CP_RTCP_TEST_ENDPOINT_PORT, localAddress.data(),
                            CP_RTCP_TEST_RTP_SINK_PORT, localAddress.data());
      CPPUNIT_ASSERT_MESSAGE("could not bind peer socket", peer.isOk());

      // A few copies: the SRTCP replay window accepts increasing indices, and
      // sending more than one removes any dependence on a single datagram
      // surviving the loopback.
      for (int i = 0; i < 3; i++)
      {
         unsigned char copy[256];
         int copyLen = buildSenderReport(copy, 0x51ee7a11);
         int copySize = copyLen;
         CPPUNIT_ASSERT(sender.srtpProtectIfNeeded(copy, &copySize, TRUE,
                                                   sizeof(copy)));
         CPPUNIT_ASSERT_EQUAL(copySize,
            peer.write((const char*)copy, copySize));
         OsTask::delay(20);
      }

      int rtcpAfter = waitForRtcpCount(ti, connId, rtcpBefore + 1, 3000);

      CPPUNIT_ASSERT_MESSAGE("SRTCP sent to the muxed RTP port was never "
                             "accepted -- classified as RTP, gated, or failed "
                             "to unprotect",
                             rtcpAfter > rtcpBefore);

      ti->deleteConnection(connId);
      ti->release();
      for (int i = 0; i < numCodecs; i++) delete codecs[i];
      delete[] codecs;
   }

   /* ============== rtcp-mux, SDES keyed ============================ */

   // Both concerns at once: reports go to the RTP port AND are still SRTCP.
   // With multiplexing the receiver can no longer tell RTP from RTCP by
   // socket, so getting the protection right depends on the packet-type
   // classification rather than on which port delivered the packet.
   void testMuxedRtcpIsProtectedWithSdesKeys()
   {
      const SdpMediaLine::SdpCryptoSuiteType suite =
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80;
      CPPUNIT_ASSERT_MESSAGE("libsrtp lacks AES_CM_128_HMAC_SHA1_80",
                             MpSrtp::isCryptoSuiteSupported(suite));

      const UtlString key = makeKey(0x7b);

      unsigned char report[1500];
      int len = runEndpointAndCaptureReport(suite, key, report, sizeof(report),
                                           TRUE /* rtcpMux */);

      CPPUNIT_ASSERT_MESSAGE("no RTCP arrived on the RTP port with rtcp-mux",
                             len > 0);
      CPPUNIT_ASSERT_MESSAGE("capture is not RTCP shaped",
                             looksLikeRtcp(report, len));
      CPPUNIT_ASSERT_MESSAGE("muxed RTCP was sent unprotected",
                             rtcpDeclaredLength(report, len) != len);

      MpSrtp receiver;
      CPPUNIT_ASSERT(receiver.setSrtpParams(suite, key, TRUE /* forUnprotect */));

      int size = len;
      CPPUNIT_ASSERT_MESSAGE("captured muxed report did not authenticate under "
                             "the negotiated SDES key",
         receiver.srtpUnprotectIfNeeded(report, &size, TRUE /* rtcp */));

      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "unexpected SRTCP trailer size (want 4 byte index + 10 byte tag)",
         14, len - size);
      CPPUNIT_ASSERT_MESSAGE("decrypted payload is not RTCP",
                             looksLikeRtcp(report, size));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("decrypted compound is malformed",
                                   size, rtcpDeclaredLength(report, size));
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CpSrtcpTest);

#endif  // ENABLE_SRTP && !EXCLUDE_RTCP
