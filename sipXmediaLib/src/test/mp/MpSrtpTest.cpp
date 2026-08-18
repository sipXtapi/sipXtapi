//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Without ENABLE_SRTP every MpSrtp method is a pass-through stub, so there is
// nothing here to test.  The one exception -- the static packet validators --
// is not worth compiling a whole fixture for on its own.
#ifdef ENABLE_SRTP

#include <string.h>

#include <os/OsIntTypes.h>
#include <os/OsSysLog.h>

#include <sipxunittests.h>
#include "mp/MpSrtp.h"

/**
 * Unittest for the SRTCP half of MpSrtp.
 *
 * SRTCP protection is what lets RTCP and SRTP be enabled at the same time.
 * The arrangement it depends on is asymmetric, and that asymmetry is the thing
 * these tests pin down:
 *
 *   - The SENDER uses two MpSrtp instances against one master key: MprToNet's,
 *     which only ever protects RTP, and CRTCPRender's, which only ever protects
 *     RTCP.  They are split because RTCP is rendered on the CRTCManager thread
 *     rather than the media thread.
 *   - The RECEIVER uses a single MprFromNet instance for both, because both
 *     arrive on the NetInTask thread.
 *
 * That is only sound because RFC 3711 derives the SRTP and SRTCP session keys
 * from the master key with different labels, and because the SRTCP index lives
 * in its own per-stream counter rather than being derived from the RTP
 * sequence number.  testSplitSenderMatchesSharedReceiver() is the direct
 * check of it; the rest cover the surrounding behaviour.
 */
class MpSrtpTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(MpSrtpTest);

    CPPUNIT_TEST(testUnkeyedSessionIsPassThrough);
    CPPUNIT_TEST(testSrtcpRoundTrip);
    CPPUNIT_TEST(testSrtcpIndexAdvances);
    CPPUNIT_TEST(testSplitSenderMatchesSharedReceiver);
    CPPUNIT_TEST(testBareReceiverReportSurvives);
    CPPUNIT_TEST(testSrtcpRejectsTamperedPacket);
    CPPUNIT_TEST(testSrtcpRejectsForeignKey);

    CPPUNIT_TEST(testMuxClassifiesRtcpByPacketType);
    CPPUNIT_TEST(testMuxLeavesStunAndDtlsAlone);
    CPPUNIT_TEST(testMuxClassifiesProtectedTrafficToo);

    CPPUNIT_TEST_SUITE_END();

public:

    // AES_CM_128_HMAC_SHA1_* take a 16 byte master key followed by a 14 byte
    // master salt.  MpSrtp::setSrtpParams checks the length against the
    // libsrtp policy, so these must be exactly 30 bytes.
    enum { CM_128_KEY_LEN = 30 };

    // Generous working buffers.  srtpProtectIfNeeded refuses to run unless the
    // buffer has 20 bytes of headroom beyond the current packet size.
    enum { BUF_SIZE = 256 };

    virtual void setUp()
    {
        // srtp_init() is process global and the capability probe behind it is
        // one-shot, so this is done once and never torn down: calling
        // globalShutdown() in tearDown() would pull the crypto kernel out from
        // under any other SRTP suite in the same executable.
        static bool sInitialized = false;
        if (!sInitialized)
        {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, MpSrtp::globalInitialize());
            sInitialized = true;
        }

        // Every suite these tests use must be present in the linked-in
        // libsrtp.  AES-CM is built in unconditionally; GCM would not be, which
        // is why nothing here uses it.
        CPPUNIT_ASSERT_MESSAGE("libsrtp lacks AES_CM_128_HMAC_SHA1_80",
            MpSrtp::isCryptoSuiteSupported(
                SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80));
        CPPUNIT_ASSERT_MESSAGE("libsrtp lacks AES_CM_128_HMAC_SHA1_32",
            MpSrtp::isCryptoSuiteSupported(
                SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32));
    }

    /* --- Helpers --------------------------------------------------------- */

    /// A deterministic 30 byte master key || salt.  The seed lets a test build
    /// a second, unrelated key to prove that authentication actually depends
    /// on the key material.
    static UtlString makeKey(unsigned char seed)
    {
        char raw[CM_128_KEY_LEN];
        for (int i = 0; i < CM_128_KEY_LEN; i++)
        {
            raw[i] = (char)(seed + i * 7);
        }
        return UtlString(raw, CM_128_KEY_LEN);
    }

    /// Build a minimal but well formed RTCP Receiver Report: version 2,
    /// reception report count 0, PT 201, length 1 (two 32-bit words), followed
    /// by the sender SSRC.  Eight bytes total, which is the smallest legal
    /// RTCP packet and therefore the tightest case for the SRTCP length
    /// checks.
    static int buildReceiverReport(unsigned char* buf, uint32_t ssrc)
    {
        buf[0] = 0x80;                       // V=2, P=0, RC=0
        buf[1] = 201;                        // PT = RR
        buf[2] = 0x00; buf[3] = 0x01;        // length = 1 (8 bytes)
        buf[4] = (unsigned char)(ssrc >> 24);
        buf[5] = (unsigned char)(ssrc >> 16);
        buf[6] = (unsigned char)(ssrc >> 8);
        buf[7] = (unsigned char)(ssrc);
        return 8;
    }

    /// Build an RTCP Sender Report (28 bytes) followed by the given trailing
    /// payload bytes, which stands in for the SR+SDES compound CRTCPRender
    /// actually emits.
    static int buildSenderReport(unsigned char* buf, uint32_t ssrc)
    {
        memset(buf, 0, 28);
        buf[0] = 0x80;                       // V=2, P=0, RC=0
        buf[1] = 200;                        // PT = SR
        buf[2] = 0x00; buf[3] = 0x06;        // length = 6 (28 bytes)
        buf[4] = (unsigned char)(ssrc >> 24);
        buf[5] = (unsigned char)(ssrc >> 16);
        buf[6] = (unsigned char)(ssrc >> 8);
        buf[7] = (unsigned char)(ssrc);
        buf[8]  = 0xc7;                      // NTP timestamp, msw
        buf[12] = 0x40;                      // NTP timestamp, lsw
        buf[19] = 0xe8;                      // RTP timestamp
        buf[23] = 0x64;                      // sender's packet count
        buf[26] = 0x27; buf[27] = 0x10;      // sender's octet count
        return 28;
    }

    /// Build an RTP packet: 12 byte header plus a short payload.
    static int buildRtpPacket(unsigned char* buf, uint32_t ssrc, uint16_t seq)
    {
        memset(buf, 0, 16);
        buf[0] = 0x80;                       // V=2, P=0, X=0, CC=0
        buf[1] = 0;                          // M=0, PT=0 (PCMU)
        buf[2] = (unsigned char)(seq >> 8);
        buf[3] = (unsigned char)(seq);
        buf[7] = 0x64;                       // timestamp
        buf[8]  = (unsigned char)(ssrc >> 24);
        buf[9]  = (unsigned char)(ssrc >> 16);
        buf[10] = (unsigned char)(ssrc >> 8);
        buf[11] = (unsigned char)(ssrc);
        buf[12] = 0xde; buf[13] = 0xad;      // payload
        buf[14] = 0xbe; buf[15] = 0xef;
        return 16;
    }

    /* ============== Plain RTCP is left alone ======================= */

    // A connection that never negotiated a crypto suite must behave exactly as
    // it did before SRTCP existed.  This is what keeps RTCP working on the
    // default build, where ENABLE_SRTP is compiled in but no key is ever set.
    void testUnkeyedSessionIsPassThrough()
    {
        MpSrtp srtp;
        CPPUNIT_ASSERT(!srtp.isSessionCreated());

        unsigned char packet[BUF_SIZE];
        unsigned char original[BUF_SIZE];
        int len = buildSenderReport(packet, 0x1fcd53e2);
        memcpy(original, packet, len);
        int size = len;

        CPPUNIT_ASSERT(srtp.srtpProtectIfNeeded(packet, &size, TRUE, BUF_SIZE));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("unkeyed protect changed the length",
                                     len, size);
        CPPUNIT_ASSERT_MESSAGE("unkeyed protect changed the bytes",
                               memcmp(original, packet, len) == 0);

        CPPUNIT_ASSERT(srtp.srtpUnprotectIfNeeded(packet, &size, TRUE));
        CPPUNIT_ASSERT_EQUAL(len, size);
        CPPUNIT_ASSERT(memcmp(original, packet, len) == 0);
    }

    /* ============== Basic SRTCP round trip ========================= */

    void testSrtcpRoundTrip()
    {
        const UtlString key = makeKey(0x11);

        MpSrtp sender;
        MpSrtp receiver;
        CPPUNIT_ASSERT(sender.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80,
            key, FALSE /* forUnprotect */));
        CPPUNIT_ASSERT(receiver.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80,
            key, TRUE /* forUnprotect */));
        CPPUNIT_ASSERT(sender.isSessionCreated());
        CPPUNIT_ASSERT(receiver.isSessionCreated());

        unsigned char packet[BUF_SIZE];
        unsigned char original[BUF_SIZE];
        int len = buildSenderReport(packet, 0x1fcd53e2);
        memcpy(original, packet, len);

        int size = len;
        CPPUNIT_ASSERT(sender.srtpProtectIfNeeded(packet, &size, TRUE, BUF_SIZE));

        // SRTCP appends a 4 byte E-flag/index word and, for HMAC_SHA1_80, a
        // 10 byte authentication tag.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected SRTCP overhead",
                                     len + 14, size);

        // The first 8 bytes (header + SSRC) stay in the clear per RFC 3711;
        // everything after must have been encrypted.
        CPPUNIT_ASSERT_MESSAGE("SRTCP header should not be encrypted",
                               memcmp(original, packet, 8) == 0);
        CPPUNIT_ASSERT_MESSAGE("SRTCP payload was not encrypted",
                               memcmp(original + 8, packet + 8, len - 8) != 0);

        CPPUNIT_ASSERT(receiver.srtpUnprotectIfNeeded(packet, &size, TRUE));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("unprotect did not restore the length",
                                     len, size);
        CPPUNIT_ASSERT_MESSAGE("unprotect did not restore the bytes",
                               memcmp(original, packet, len) == 0);
    }

    /* ============== The SRTCP index is its own counter ============= */

    // The dedicated RTCP session has to maintain an SRTCP index that advances
    // per RTCP packet, independently of any RTP sequence number.  If it did
    // not, two identical reports would encrypt to identical ciphertext -- a
    // keystream reuse break.
    void testSrtcpIndexAdvances()
    {
        const UtlString key = makeKey(0x22);

        MpSrtp sender;
        MpSrtp receiver;
        CPPUNIT_ASSERT(sender.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80, key, FALSE));
        CPPUNIT_ASSERT(receiver.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80, key, TRUE));

        unsigned char first[BUF_SIZE], second[BUF_SIZE], original[BUF_SIZE];
        int len = buildSenderReport(original, 0x1fcd53e2);

        memcpy(first, original, len);
        int firstSize = len;
        CPPUNIT_ASSERT(sender.srtpProtectIfNeeded(first, &firstSize, TRUE, BUF_SIZE));

        memcpy(second, original, len);
        int secondSize = len;
        CPPUNIT_ASSERT(sender.srtpProtectIfNeeded(second, &secondSize, TRUE, BUF_SIZE));

        CPPUNIT_ASSERT_EQUAL(firstSize, secondSize);

        // Identical plaintext, same key, consecutive indices -> the protected
        // bytes must differ.
        CPPUNIT_ASSERT_MESSAGE("identical reports encrypted identically",
                               memcmp(first, second, firstSize) != 0);

        // And specifically the index word, which sits immediately after the
        // encrypted payload, must have advanced.
        CPPUNIT_ASSERT_MESSAGE("SRTCP index did not advance",
                               memcmp(first + len, second + len, 4) != 0);

        // Both still authenticate and decrypt back to the original.  Taking
        // them in order also exercises the receiver's SRTCP replay window.
        CPPUNIT_ASSERT(receiver.srtpUnprotectIfNeeded(first, &firstSize, TRUE));
        CPPUNIT_ASSERT_EQUAL(len, firstSize);
        CPPUNIT_ASSERT(memcmp(original, first, len) == 0);

        CPPUNIT_ASSERT(receiver.srtpUnprotectIfNeeded(second, &secondSize, TRUE));
        CPPUNIT_ASSERT_EQUAL(len, secondSize);
        CPPUNIT_ASSERT(memcmp(original, second, len) == 0);
    }

    /* ============== The load-bearing test ========================== */

    // Reproduces the exact instance topology the media stack uses: an RTP-only
    // sender context and an RTCP-only sender context sharing one master key and
    // one SSRC, against a single receiver context that handles both.
    //
    // If SRTP and SRTCP shared key material or index space, splitting the
    // sender across two libsrtp sessions would corrupt one or both streams.
    // Interleaving them here is what would expose that.
    void testSplitSenderMatchesSharedReceiver()
    {
        const UtlString key   = makeKey(0x33);
        const uint32_t  ssrc  = 0x1fcd53e2;

        // Sender side: two contexts, mirroring MprToNet and CRTCPRender.
        MpSrtp senderRtp;
        MpSrtp senderRtcp;
        CPPUNIT_ASSERT(senderRtp.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80, key, FALSE));
        CPPUNIT_ASSERT(senderRtcp.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80, key, FALSE));

        // Receiver side: one context for both, mirroring MprFromNet.
        MpSrtp receiver;
        CPPUNIT_ASSERT(receiver.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80, key, TRUE));

        for (int round = 0; round < 8; round++)
        {
            // --- RTP through the RTP-only sender context ---
            unsigned char rtp[BUF_SIZE], rtpOriginal[BUF_SIZE];
            int rtpLen = buildRtpPacket(rtp, ssrc, (uint16_t)(1000 + round));
            memcpy(rtpOriginal, rtp, rtpLen);
            int rtpSize = rtpLen;

            CPPUNIT_ASSERT_MESSAGE("RTP protect failed",
                senderRtp.srtpProtectIfNeeded(rtp, &rtpSize, FALSE, BUF_SIZE));
            CPPUNIT_ASSERT_MESSAGE("RTP was not expanded by the auth tag",
                                   rtpSize > rtpLen);

            // --- RTCP through the RTCP-only sender context ---
            unsigned char rtcp[BUF_SIZE], rtcpOriginal[BUF_SIZE];
            int rtcpLen = buildSenderReport(rtcp, ssrc);
            memcpy(rtcpOriginal, rtcp, rtcpLen);
            int rtcpSize = rtcpLen;

            CPPUNIT_ASSERT_MESSAGE("RTCP protect failed",
                senderRtcp.srtpProtectIfNeeded(rtcp, &rtcpSize, TRUE, BUF_SIZE));
            CPPUNIT_ASSERT_MESSAGE("RTCP was not expanded by index + tag",
                                   rtcpSize > rtcpLen);

            // --- Both unprotect through the one receiver context ---
            CPPUNIT_ASSERT_MESSAGE("RTP unprotect failed",
                receiver.srtpUnprotectIfNeeded(rtp, &rtpSize, FALSE));
            CPPUNIT_ASSERT_EQUAL(rtpLen, rtpSize);
            CPPUNIT_ASSERT_MESSAGE("RTP did not round trip",
                                   memcmp(rtpOriginal, rtp, rtpLen) == 0);

            CPPUNIT_ASSERT_MESSAGE("RTCP unprotect failed",
                receiver.srtpUnprotectIfNeeded(rtcp, &rtcpSize, TRUE));
            CPPUNIT_ASSERT_EQUAL(rtcpLen, rtcpSize);
            CPPUNIT_ASSERT_MESSAGE("RTCP did not round trip",
                                   memcmp(rtcpOriginal, rtcp, rtcpLen) == 0);
        }
    }

    /* ============== Smallest legal report =========================== */

    // Regression test.  MpSrtp::isValidSrtcp used to require 20 bytes, though
    // its own comment derives the floor as 8 (header + SSRC) + 4 (index) +
    // 4 (HMAC_SHA1_32 tag) = 16.  A bare Receiver Report with no report blocks
    // under a _32 suite lands on exactly 16 bytes and was silently discarded
    // before it ever reached srtp_unprotect_rtcp.  Nothing caught it while
    // SRTCP was unreachable.
    void testBareReceiverReportSurvives()
    {
        const UtlString key = makeKey(0x44);

        MpSrtp sender;
        MpSrtp receiver;
        CPPUNIT_ASSERT(sender.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32, key, FALSE));
        CPPUNIT_ASSERT(receiver.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32, key, TRUE));

        unsigned char packet[BUF_SIZE], original[BUF_SIZE];
        int len = buildReceiverReport(packet, 0x1fcd53e2);
        CPPUNIT_ASSERT_EQUAL(8, len);
        memcpy(original, packet, len);

        int size = len;
        CPPUNIT_ASSERT(sender.srtpProtectIfNeeded(packet, &size, TRUE, BUF_SIZE));

        // 8 + 4 byte index + 4 byte truncated tag.  This is the number the
        // length floor has to accept.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("bare RR did not protect to 16 bytes",
                                     16, size);
        CPPUNIT_ASSERT_MESSAGE("16 byte SRTCP rejected by the length check",
                               MpSrtp::isValidSrtcp(packet, (size_t)size));

        CPPUNIT_ASSERT_MESSAGE("bare RR failed to unprotect",
            receiver.srtpUnprotectIfNeeded(packet, &size, TRUE));
        CPPUNIT_ASSERT_EQUAL(len, size);
        CPPUNIT_ASSERT(memcmp(original, packet, len) == 0);
    }

    /* ============== Authentication actually authenticates =========== */

    void testSrtcpRejectsTamperedPacket()
    {
        const UtlString key = makeKey(0x55);

        MpSrtp sender;
        MpSrtp receiver;
        CPPUNIT_ASSERT(sender.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80, key, FALSE));
        CPPUNIT_ASSERT(receiver.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80, key, TRUE));

        unsigned char packet[BUF_SIZE];
        int len = buildSenderReport(packet, 0x1fcd53e2);
        int size = len;
        CPPUNIT_ASSERT(sender.srtpProtectIfNeeded(packet, &size, TRUE, BUF_SIZE));

        // Flip a bit in the encrypted body, leaving the cleartext header (and
        // therefore the isValidSrtcp fast path) intact so the packet really
        // does reach srtp_unprotect_rtcp.
        packet[12] ^= 0x01;

        CPPUNIT_ASSERT_MESSAGE("tampered SRTCP was accepted",
            !receiver.srtpUnprotectIfNeeded(packet, &size, TRUE));
    }

    // A receiver keyed with unrelated material must reject the report rather
    // than hand back garbage.  Together with the tamper test this is what
    // establishes that a successful unprotect in the end-to-end tests is
    // real evidence the sender used the negotiated key.
    void testSrtcpRejectsForeignKey()
    {
        MpSrtp sender;
        MpSrtp receiver;
        CPPUNIT_ASSERT(sender.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80,
            makeKey(0x66), FALSE));
        CPPUNIT_ASSERT(receiver.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80,
            makeKey(0x77), TRUE));

        unsigned char packet[BUF_SIZE];
        int len = buildSenderReport(packet, 0x1fcd53e2);
        int size = len;
        CPPUNIT_ASSERT(sender.srtpProtectIfNeeded(packet, &size, TRUE, BUF_SIZE));

        CPPUNIT_ASSERT_MESSAGE("SRTCP under a foreign key was accepted",
            !receiver.srtpUnprotectIfNeeded(packet, &size, TRUE));
    }

    /* ============== rtcp-mux demultiplexing ========================= */
    //
    // With RFC 5761 both kinds of packet share a port, so MprFromNet can no
    // longer tell them apart by which socket delivered them and classifies by
    // packet type instead.  These pin that rule down directly: the end-to-end
    // muxed tests in CpSrtcpTest drive the SEND side, and removing this
    // classification does not make them fail, so without these the receive
    // side would be uncovered.

    // RFC 5761 section 4: RTCP packet types occupy 192-223 and no RTP payload
    // type collides with them.
    void testMuxClassifiesRtcpByPacketType()
    {
        unsigned char buf[64];

        // Every RTCP type in the range classifies as RTCP.
        for (int pt = 192; pt <= 223; pt++)
        {
            buf[0] = 0x80;
            buf[1] = (unsigned char)pt;
            CPPUNIT_ASSERT_MESSAGE("RTCP packet type not recognised",
                MpSrtp::isRtcpPacket(buf, sizeof(buf)));
        }

        // sipXtapi's static and dynamic RTP payload types classify as RTP,
        // with and without the marker bit set (which is what puts the second
        // byte above 127 and is the reason the 64-95 range is unusable).
        static const int rtpPayloadTypes[] = { 0, 3, 8, 9, 18, 34, 96, 97, 110, 127 };
        for (int i = 0; i < (int)(sizeof(rtpPayloadTypes)/sizeof(rtpPayloadTypes[0])); i++)
        {
            buf[0] = 0x80;
            buf[1] = (unsigned char)rtpPayloadTypes[i];
            CPPUNIT_ASSERT_MESSAGE("RTP misclassified as RTCP",
                !MpSrtp::isRtcpPacket(buf, sizeof(buf)));

            buf[1] = (unsigned char)(rtpPayloadTypes[i] | 0x80);   // marker set
            CPPUNIT_ASSERT_MESSAGE("marked RTP misclassified as RTCP",
                !MpSrtp::isRtcpPacket(buf, sizeof(buf)));
        }
    }

    // STUN and DTLS share the port too; they must fall through to the demux
    // that handles them rather than being mistaken for control traffic.
    void testMuxLeavesStunAndDtlsAlone()
    {
        unsigned char buf[64];
        memset(buf, 0, sizeof(buf));

        // STUN: first byte 0-3 (RFC 7983).
        for (int b0 = 0; b0 <= 3; b0++)
        {
            buf[0] = (unsigned char)b0;
            buf[1] = 0xC8;      // would be an RTCP type if we only looked here
            CPPUNIT_ASSERT_MESSAGE("STUN misclassified as RTCP",
                !MpSrtp::isRtcpPacket(buf, sizeof(buf)));
        }

        // DTLS records: first byte 20-63.
        for (int b0 = 20; b0 <= 63; b0++)
        {
            buf[0] = (unsigned char)b0;
            buf[1] = 0xC8;
            CPPUNIT_ASSERT_MESSAGE("DTLS record misclassified as RTCP",
                !MpSrtp::isRtcpPacket(buf, sizeof(buf)));
        }

        // Runt datagrams cannot be classified at all.
        buf[0] = 0x80;
        CPPUNIT_ASSERT(!MpSrtp::isRtcpPacket(buf, 1));
        CPPUNIT_ASSERT(!MpSrtp::isRtcpPacket(buf, 0));
        CPPUNIT_ASSERT(!MpSrtp::isRtcpPacket(NULL, 64));
    }

    // The classification has to work on protected traffic, because the
    // receiver has to decide which unprotect context to use BEFORE it can
    // decrypt anything.  RFC 3711 leaves the first two bytes of both SRTP and
    // SRTCP in the clear, which is what makes that possible -- so a real
    // protected packet must classify the same as its plaintext.
    void testMuxClassifiesProtectedTrafficToo()
    {
        const UtlString key = makeKey(0x5c);

        MpSrtp senderRtp;
        MpSrtp senderRtcp;
        CPPUNIT_ASSERT(senderRtp.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80, key, FALSE));
        CPPUNIT_ASSERT(senderRtcp.setSrtpParams(
            SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80, key, FALSE));

        unsigned char rtp[BUF_SIZE];
        int rtpLen = buildRtpPacket(rtp, 0x1fcd53e2, 4242);
        int rtpSize = rtpLen;
        CPPUNIT_ASSERT(senderRtp.srtpProtectIfNeeded(rtp, &rtpSize, FALSE, BUF_SIZE));
        CPPUNIT_ASSERT_MESSAGE("protected RTP misclassified as RTCP",
            !MpSrtp::isRtcpPacket(rtp, rtpSize));

        unsigned char rtcp[BUF_SIZE];
        int rtcpLen = buildSenderReport(rtcp, 0x1fcd53e2);
        int rtcpSize = rtcpLen;
        CPPUNIT_ASSERT(senderRtcp.srtpProtectIfNeeded(rtcp, &rtcpSize, TRUE, BUF_SIZE));
        CPPUNIT_ASSERT_MESSAGE("protected RTCP not recognised as RTCP",
            MpSrtp::isRtcpPacket(rtcp, rtcpSize));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpSrtpTest);

#endif  // ENABLE_SRTP
