//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Gated on RTCP alone, NOT on ENABLE_SRTP.
//
// MpSrtp::isRtcpPacket() is a pure predicate over two header bytes and is
// compiled unconditionally -- verified by the fact that it is still emitted as
// a real symbol when MpSrtp.cpp is built without ENABLE_SRTP, while the
// libsrtp calls around it are stubbed out. rtcp-mux demultiplexing therefore
// works on a plain build, and its tests belong somewhere they can run there.
#ifndef EXCLUDE_RTCP

#include <string.h>

#include <os/OsIntTypes.h>

#include <sipxunittests.h>
#include "mp/MpSrtp.h"

/**
 * The RFC 5761 demultiplexing rule.
 *
 * With rtcp-mux both kinds of packet share a port, so MprFromNet can no longer
 * tell them apart by which socket delivered them and classifies by packet type
 * instead. These tests pin that rule down directly.
 *
 * They earn their place: the end-to-end muxed tests mostly drive the send
 * path, and removing the classification from MprFromNet::pushPacket does not
 * make those fail. Only CpRtcpMuxTest::testMuxedRtcpIsReceivedOnRtpPort and
 * these do -- and these pin the rule down directly, with no flowgraph or
 * socket in the way.
 */
class MpRtcpMuxTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(MpRtcpMuxTest);

    CPPUNIT_TEST(testClassifiesRtcpByPacketType);
    CPPUNIT_TEST(testLeavesStunAndDtlsAlone);

    CPPUNIT_TEST_SUITE_END();

public:

    // RFC 5761 section 4: RTCP packet types occupy 192-223 and no RTP payload
    // type collides with them.
    void testClassifiesRtcpByPacketType()
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
        // with and without the marker bit set -- the bit that puts the second
        // byte above 127 and is the reason the 64-95 range is unusable.
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
    void testLeavesStunAndDtlsAlone()
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
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpRtcpMuxTest);

#endif  // !EXCLUDE_RTCP
