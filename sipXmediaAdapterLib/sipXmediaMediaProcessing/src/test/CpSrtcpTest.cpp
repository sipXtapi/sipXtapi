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

#include <string.h>

#include <sipxunittests.h>
#include "mi/CpMediaInterfaceFactory.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"
#include "CpTopologyGraphInterface.h"
#include "mi/CpMediaInterface.h"

#include <mp/MpSrtp.h>

#include <sdp/SdpMediaLine.h>
#include <sdp/SdpCodecList.h>

#include <os/OsTask.h>
#include <os/OsSocket.h>
#include <os/OsDatagramSocket.h>
#include <os/OsSysLog.h>

// Test hook exported by RTCPConnection.cpp under DEBUGGING_RTCP_REPORTS.  The
// production reporting cadence is 5 seconds, which would make this suite
// intolerably slow.  The period is sampled when a connection is constructed,
// so it must be set before createConnection().
extern "C" { extern int adjustRtcpPeriod(int x); }

// Defaults for Media Interface Factory and Media Interface initialization.
#define FRAME_SIZE_MS       0
#define MAX_SAMPLE_RATE     0
#define DEFAULT_SAMPLE_RATE 0

// Ports, chosen to avoid CpCryptoTest (6000) and CpDtlsTest (16000/17000).
//
// SRTCP_TEST_RTP_SINK_PORT must differ from SRTCP_TEST_COLLECTOR_PORT: the
// endpoint really does emit RTP once startRtpSend() is called and an audio
// source is live, and pointing it at the collector makes the capture race
// between a 172 byte PCMU packet and the report we are actually after.
#define SRTCP_TEST_ENDPOINT_PORT   18000
#define SRTCP_TEST_COLLECTOR_PORT  18100
#define SRTCP_TEST_RTP_SINK_PORT   18200

// Shortened RTCP reporting period.  Correctness no longer depends on this
// being generous: startRtpSendImpl() latches the renderer closed before
// starting it, so a report cannot precede the key install however tight the
// cadence.
//
// Note this suite does NOT exercise that latch.  Measured on a loopback build,
// these tests pass at 50ms with the latch removed -- the media thread drains
// the key message far faster than the first reporting alarm, so the race the
// latch guards is simply not reachable here.  The latch is defence against a
// media thread that is NOT draining promptly (flowgraph not yet ticking, a
// stalled frame, a debugger break), which a test like this cannot stage.  Its
// behaviour is asserted directly by
// CpDtlsTest::testRtcpWithheldWhileHandshakePending, where the keys genuinely
// never arrive.
#define SRTCP_TEST_REPORT_PERIOD_MS  50

// How long to wait for reports to arrive before giving up.
#define SRTCP_TEST_COLLECT_TIMEOUT_MS  6000

#ifdef WIN32
#include <string>
static std::string getSrtcpTestExecutableDir()
{
   char buf[MAX_PATH];
   memset(buf, 0, sizeof(buf));
   DWORD len = GetModuleFileNameA(GetModuleHandle(NULL), buf, sizeof(buf) - 1);
   for (char* p = buf + len; p > buf; p--)
   {
      if (*p == '\\')
      {
         *p = 0;
         break;
      }
   }
   return std::string(buf);
}
#endif


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

   CPPUNIT_TEST(testPlainRtcpIsUnprotectedWithoutSrtp);
   CPPUNIT_TEST(testOutboundRtcpIsProtectedWithSdesKeys);
   CPPUNIT_TEST(testMuxedRtcpArrivesOnRtpPort);
   CPPUNIT_TEST(testMuxedRtcpIsProtectedWithSdesKeys);

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
         getSrtcpTestExecutableDir().c_str(),
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

      // Speed up reporting, remembering the previous value so the rest of the
      // executable is not left with this suite's RTCP cadence.
      mSavedRtcpPeriod = adjustRtcpPeriod(SRTCP_TEST_REPORT_PERIOD_MS);
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

   /// Does this look like a plain (unprotected) RTCP packet?  Version 2 and a
   /// packet type in the RTCP range.  SRTCP leaves both of those fields in the
   /// clear, so this alone cannot distinguish the two -- it is used only to
   /// confirm the capture really is RTCP-shaped, never as proof of protection.
   static bool looksLikeRtcp(const unsigned char* buf, int len)
   {
      if (len < 8) return false;
      if ((buf[0] & 0xC0) != 0x80) return false;
      return (buf[1] >= 200 && buf[1] <= 204);
   }

   /// Sum of the length fields across a compound RTCP packet, in bytes,
   /// stopping at the first sub-packet that does not parse.
   ///
   /// For plain RTCP this accounts for the whole datagram exactly.  It cannot
   /// do so for SRTCP: RFC 3711 encrypts everything past the first 8 bytes,
   /// including the headers of the second and later sub-packets, so the walk
   /// runs into ciphertext and stops early with a meaningless offset.  That
   /// asymmetry is used only as a "this is not plaintext RTCP" signal; the
   /// real evidence of protection is that the bytes decrypt (see below).
   static int rtcpDeclaredLength(const unsigned char* buf, int len)
   {
      int offset = 0;
      while (offset + 4 <= len)
      {
         if ((buf[offset] & 0xC0) != 0x80) break;
         int words = (buf[offset + 2] << 8) | buf[offset + 3];
         int bytes = (words + 1) * 4;
         if (bytes <= 0 || offset + bytes > len) break;
         offset += bytes;
      }
      return offset;
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

   /// Block until an RTCP datagram lands on the collector socket, or the
   /// timeout expires.  Returns the byte count, or 0 on timeout.
   ///
   /// The endpoint's sockets are OsNatDatagramSockets, so the RTCP port also
   /// carries STUN, and a misrouted media packet would land here too.
   /// Datagrams are demultiplexed the way RFC 7983 and MprFromNet::pushPacket
   /// do it -- 0-3 is STUN, 128-191 is RTP/RTCP -- and then narrowed to RTCP
   /// by packet type, per the RFC 5761 section 4 rule that RTP payload types
   /// never collide with the RTCP packet types 192-223.
   ///
   /// Filtering on the type rather than just the range matters: RTP shares the
   /// 128-191 first-byte range, so the looser test would happily hand back a
   /// media packet and fail downstream with a confusing "not RTCP shaped".
   /// SRTCP leaves both of these header bytes in the clear, so the
   /// classification holds for protected and unprotected traffic alike.
   static int collectOneReport(OsDatagramSocket& collector,
                               unsigned char* buf,
                               int bufSize,
                               int timeoutMs)
   {
      int elapsed = 0;
      const int slice = 50;
      while (elapsed < timeoutMs)
      {
         if (collector.isReadyToRead(slice))
         {
            int n = collector.read((char*)buf, bufSize);
            if (n > 0)
            {
               if (n >= 2 && buf[0] >= 128 && buf[0] <= 191 &&
                   buf[1] >= 192 && buf[1] <= 223)
               {
                  return n;
               }
               // STUN, or RTP that found its way here. Keep waiting.
               continue;
            }
         }
         elapsed += slice;
      }
      return 0;
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
                                 SRTCP_TEST_COLLECTOR_PORT, localAddress.data());
      CPPUNIT_ASSERT_MESSAGE("could not bind RTCP collector socket",
                             collector.isOk());

      CpTopologyGraphInterface* ti;
      int connId;
      buildEndpoint(SRTCP_TEST_ENDPOINT_PORT, localAddress, ti, connId);

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
                                      rtcpMux ? SRTCP_TEST_COLLECTOR_PORT
                                              : SRTCP_TEST_RTP_SINK_PORT,
                                      rtcpMux ? SRTCP_TEST_RTP_SINK_PORT
                                              : SRTCP_TEST_COLLECTOR_PORT,
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
                                      SRTCP_TEST_COLLECT_TIMEOUT_MS);

      ti->deleteConnection(connId);
      ti->release();

      for (int i = 0; i < numCodecs; i++)
      {
         delete codecs[i];
      }
      delete[] codecs;

      return captured;
   }

   /* ============== Control: no SRTP, plain RTCP ==================== */

   // Establishes the baseline the protected case is compared against, and
   // guards the other direction of the change: a connection with no crypto
   // suite must still emit ordinary, unpadded RTCP.
   void testPlainRtcpIsUnprotectedWithoutSrtp()
   {
      unsigned char report[1500];
      int len = runEndpointAndCaptureReport(SdpMediaLine::CRYPTO_SUITE_TYPE_NONE,
                                           UtlString(), report, sizeof(report));

      CPPUNIT_ASSERT_MESSAGE("no RTCP report arrived within the timeout",
                             len > 0);
      CPPUNIT_ASSERT_MESSAGE("capture is not RTCP shaped",
                             looksLikeRtcp(report, len));

      // Plain RTCP: the compound's declared length accounts for every byte in
      // the datagram, with no trailer.
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "unprotected RTCP datagram has unexpected trailing bytes",
         len, rtcpDeclaredLength(report, len));
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

   /* ============== rtcp-mux, unencrypted =========================== */

   // The point of RFC 5761: reports leave on the RTP port, not a second one.
   // The collector here is bound to what the endpoint was told is the peer's
   // RTP port, and the RTCP port it was given points elsewhere -- so a capture
   // proves the renderer switched sockets rather than merely still working.
   void testMuxedRtcpArrivesOnRtpPort()
   {
      unsigned char report[1500];
      int len = runEndpointAndCaptureReport(SdpMediaLine::CRYPTO_SUITE_TYPE_NONE,
                                           UtlString(), report, sizeof(report),
                                           TRUE /* rtcpMux */);

      CPPUNIT_ASSERT_MESSAGE("no RTCP arrived on the RTP port with rtcp-mux",
                             len > 0);
      CPPUNIT_ASSERT_MESSAGE("capture is not RTCP shaped",
                             looksLikeRtcp(report, len));

      // Unprotected, so the compound accounts for the whole datagram.
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
         "muxed plain RTCP datagram has unexpected trailing bytes",
         len, rtcpDeclaredLength(report, len));
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
