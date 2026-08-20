//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// Deliberately gated on RTCP alone, NOT on ENABLE_SRTP.
//
// rtcp-mux (RFC 5761) is a demultiplexing rule, not a security feature: it
// tells RTP and RTCP apart by packet type, which has nothing to do with
// encryption. The implementation reflects that -- no part of the mux path is
// behind ENABLE_SRTP -- so it works on a plain build, and these cases belong
// where they can actually run there.
//
// That matters because the default build (and CI) define no ENABLE_SRTP, so
// CpSrtcpTest compiles to nothing. Without this suite, rtcp-mux and plain
// RTCP would be functional but entirely untested in the configuration most
// builds ship.
#ifndef EXCLUDE_RTCP

#include "CpRtcpTestSupport.h"

using namespace CpRtcpTestSupport;

/**
 * Plain RTCP and rtcp-mux, with no encryption anywhere.
 *
 * Covers both directions:
 *   - send:    reports leave on the RTP port when multiplexing is enabled,
 *              and on the RTCP port when it is not.
 *   - receive: RTCP arriving on the muxed RTP port is recognised as RTCP,
 *              and RTP arriving there is not.
 *
 * The peer is a plain UDP socket rather than a second media interface, so the
 * test controls exactly which bytes reach the port and when.
 */
class CpRtcpMuxTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(CpRtcpMuxTest);

   CPPUNIT_TEST(testPlainRtcpUsesTheRtcpPort);
   CPPUNIT_TEST(testMuxedRtcpArrivesOnRtpPort);
   CPPUNIT_TEST(testMuxedRtcpIsReceivedOnRtpPort);
   CPPUNIT_TEST(testMuxedRtpIsNotCountedAsRtcp);

   CPPUNIT_TEST_SUITE_END();

public:

   CpMediaInterfaceFactory* mpMediaFactory;
   int                      mSavedRtcpPeriod;

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

   /// Stand up a sending endpoint and capture its first report.
   ///
   /// Muxed: the collector stands in for the peer's RTP port, and the RTCP
   /// port argument is pointed elsewhere so a capture proves the renderer
   /// switched sockets rather than merely still working.
   /// Unmuxed: RTP goes to a dead port and RTCP to the collector.
   int runEndpointAndCaptureReport(unsigned char* buf, int bufSize,
                                   UtlBoolean rtcpMux)
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      OsDatagramSocket collector(0, NULL,
                                 CP_RTCP_TEST_COLLECTOR_PORT, localAddress.data());
      CPPUNIT_ASSERT_MESSAGE("could not bind RTCP collector socket",
                             collector.isOk());

      CpTopologyGraphInterface* ti;
      int connId;
      buildEndpoint(CP_RTCP_TEST_ENDPOINT_PORT, localAddress, ti, connId);

      // Must precede startRtpSend(): that starts the RTCP renderer, which
      // needs to know which socket to write to.
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

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         ti->setConnectionDestination(connId, localAddress.data(),
                                      rtcpMux ? CP_RTCP_TEST_COLLECTOR_PORT
                                              : CP_RTCP_TEST_RTP_SINK_PORT,
                                      rtcpMux ? CP_RTCP_TEST_RTP_SINK_PORT
                                              : CP_RTCP_TEST_COLLECTOR_PORT,
                                      0, 0));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         ti->startRtpSend(connId, numCodecs, codecs));

      int captured = collectOneReport(collector, buf, bufSize,
                                      CP_RTCP_TEST_COLLECT_TIMEOUT_MS);

      ti->deleteConnection(connId);
      ti->release();
      for (int i = 0; i < numCodecs; i++) delete codecs[i];
      delete[] codecs;

      return captured;
   }

   /// Stand up a muxed endpoint that is RECEIVING, and hand back its
   /// connection. Everything should be sent to CP_RTCP_TEST_ENDPOINT_PORT.
   void buildReceivingMuxedEndpoint(const UtlString& localAddress,
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

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         outTi->startRtpReceive(outConnId, outNumCodecs, outCodecs));
   }

   /* ============== Baseline: no mux, no encryption ================= */

   // The control the muxed cases are compared against, and a guard on the
   // other direction of the change: a connection that never enabled
   // multiplexing must still emit ordinary, unpadded RTCP on the RTCP port.
   void testPlainRtcpUsesTheRtcpPort()
   {
      unsigned char report[1500];
      int len = runEndpointAndCaptureReport(report, sizeof(report),
                                            FALSE /* rtcpMux */);

      CPPUNIT_ASSERT_MESSAGE("no RTCP report arrived within the timeout",
                             len > 0);
      CPPUNIT_ASSERT_MESSAGE("capture is not RTCP shaped",
                             looksLikeRtcp(report, len));

      // Unprotected: the compound accounts for every byte, no SRTCP trailer.
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "unprotected RTCP datagram has unexpected trailing bytes",
         len, rtcpDeclaredLength(report, len));
   }

   /* ============== rtcp-mux send ==================================== */

   // The point of RFC 5761: reports leave on the RTP port, not a second one.
   // The collector is bound to what the endpoint was told is the peer's RTP
   // port, and the RTCP port it was given points elsewhere -- so a capture
   // proves the renderer switched sockets.
   void testMuxedRtcpArrivesOnRtpPort()
   {
      unsigned char report[1500];
      int len = runEndpointAndCaptureReport(report, sizeof(report),
                                            TRUE /* rtcpMux */);

      CPPUNIT_ASSERT_MESSAGE("no RTCP arrived on the RTP port with rtcp-mux",
                             len > 0);
      CPPUNIT_ASSERT_MESSAGE("capture is not RTCP shaped",
                             looksLikeRtcp(report, len));
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "muxed plain RTCP datagram has unexpected trailing bytes",
         len, rtcpDeclaredLength(report, len));
   }

   /* ============== rtcp-mux receive ================================= */

   // The positive half: RTCP arriving on the muxed RTP port is recognised as
   // RTCP and accepted.
   //
   // This is the case that actually exercises the classification. Its negative
   // twin below would still pass with the classification removed -- everything
   // would simply be treated as RTP, which is what that test asserts anyway --
   // so without this one the receive path would look covered while not being.
   void testMuxedRtcpIsReceivedOnRtpPort()
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      CpTopologyGraphInterface* ti;
      int connId, numCodecs;
      SdpCodec** codecs;
      buildReceivingMuxedEndpoint(localAddress, ti, connId, numCodecs, codecs);

      int rtpBefore = 0, rtcpBefore = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         ti->getRtpPacketCounts(connId, CpMediaInterface::AUDIO_STREAM,
                                rtpBefore, rtcpBefore));

      OsDatagramSocket peer(CP_RTCP_TEST_ENDPOINT_PORT, localAddress.data(),
                            CP_RTCP_TEST_RTP_SINK_PORT, localAddress.data());
      CPPUNIT_ASSERT_MESSAGE("could not bind peer socket", peer.isOk());

      // Plain, unprotected Sender Reports straight at the RTP port.
      for (int i = 0; i < 3; i++)
      {
         unsigned char report[64];
         int len = buildSenderReport(report, 0x51ee7a11);
         CPPUNIT_ASSERT_EQUAL(len, peer.write((const char*)report, len));
         OsTask::delay(20);
      }

      int rtcpAfter = waitForRtcpCount(ti, connId, rtcpBefore + 1, 3000);

      CPPUNIT_ASSERT_MESSAGE("RTCP sent to the muxed RTP port was never "
                             "accepted -- it was classified as RTP",
                             rtcpAfter > rtcpBefore);

      ti->deleteConnection(connId);
      ti->release();
      for (int i = 0; i < numCodecs; i++) delete codecs[i];
      delete[] codecs;
   }

   // RTP arriving on a muxed port must not be counted as RTCP. Without
   // multiplexing this question cannot even be asked -- the socket answers it
   // -- so this is the case the packet-type classification exists for.
   //
   // Asserting the RTP count moved as well stops the test passing by simply
   // discarding everything.
   void testMuxedRtpIsNotCountedAsRtcp()
   {
      UtlString localAddress("127.0.0.1");
      OsSocket::getHostIp(&localAddress);

      CpTopologyGraphInterface* ti;
      int connId, numCodecs;
      SdpCodec** codecs;
      buildReceivingMuxedEndpoint(localAddress, ti, connId, numCodecs, codecs);

      int rtpBefore = 0, rtcpBefore = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         ti->getRtpPacketCounts(connId, CpMediaInterface::AUDIO_STREAM,
                                rtpBefore, rtcpBefore));

      OsDatagramSocket peer(CP_RTCP_TEST_ENDPOINT_PORT, localAddress.data(),
                            CP_RTCP_TEST_RTP_SINK_PORT, localAddress.data());
      CPPUNIT_ASSERT_MESSAGE("could not bind peer socket", peer.isOk());

      for (int i = 0; i < 5; i++)
      {
         unsigned char rtp[64];
         int rtpLen = buildRtpPacket(rtp, 0x51ee7a11, (uint16_t)(700 + i));
         CPPUNIT_ASSERT_EQUAL(rtpLen, peer.write((const char*)rtp, rtpLen));
         OsTask::delay(20);
      }

      // Give the receive path time to have got it wrong.
      OsTask::delay(500);

      int rtpAfter = 0, rtcpAfter = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         ti->getRtpPacketCounts(connId, CpMediaInterface::AUDIO_STREAM,
                                rtpAfter, rtcpAfter));

      CPPUNIT_ASSERT_EQUAL_MESSAGE("RTP on the muxed port was counted as RTCP",
                                   rtcpBefore, rtcpAfter);
      CPPUNIT_ASSERT_MESSAGE("RTP on the muxed port was not received at all",
                             rtpAfter > rtpBefore);

      ti->deleteConnection(connId);
      ti->release();
      for (int i = 0; i < numCodecs; i++) delete codecs[i];
      delete[] codecs;
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CpRtcpMuxTest);

#endif  // !EXCLUDE_RTCP
