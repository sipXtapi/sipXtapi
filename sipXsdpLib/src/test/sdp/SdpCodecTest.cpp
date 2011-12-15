//
// Copyright (C) 2006-2011 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <sipxunittests.h>

#include <os/OsDefs.h>
#include <sdp/SdpCodec.h>
#include <sdp/SdpDefaultCodecFactory.h>

/// Unit test for SdpCodec class
class SdpCodecTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(SdpCodecTest);
    CPPUNIT_TEST(testH264SameTest);
    CPPUNIT_TEST_SUITE_END();

public:

    void testH264SameTest()
    {
        SdpCodec sipXH264CodecMode0 = SdpDefaultCodecFactory::getCodec(SdpDefaultCodecFactory::getCodecType("H264_HD1088_4096"));
        CPPUNIT_ASSERT_EQUAL(sipXH264CodecMode0.getCodecType(), SdpCodec::SDP_CODEC_H264_HD1088_4096);
        SdpCodec sipXH264CodecMode1 = SdpDefaultCodecFactory::getCodec(SdpDefaultCodecFactory::getCodecType("H264_PM1_HD1088_4096"));
        CPPUNIT_ASSERT_EQUAL(sipXH264CodecMode1.getCodecType(), SdpCodec::SDP_CODEC_H264_PM1_HD1088_4096);

        SdpCodec h264Codec1(SdpCodec::SDP_CODEC_UNKNOWN, SdpCodec::SDP_CODEC_UNKNOWN, MIME_TYPE_VIDEO, "H264", 90000, 20000, -1, "profile-level-id=428016; max-mbps=216000; max-fs=3600; max-br=5120; sar=13");
        SdpCodec h264Codec2(SdpCodec::SDP_CODEC_UNKNOWN, SdpCodec::SDP_CODEC_UNKNOWN, MIME_TYPE_VIDEO, "H264", 90000, 20000, -1, "profile-level-id=428016; packetization-mode=1; max-mbps=216000; max-fs=3600; max-br=5120; sar=13");
        SdpCodec h264Codec3(SdpCodec::SDP_CODEC_UNKNOWN, SdpCodec::SDP_CODEC_UNKNOWN, MIME_TYPE_VIDEO, "H264", 90000, 20000, -1, "profile-level-id=640016; packetization-mode=1; max-mbps=216000; max-fs=3600; max-br=5120; sar=13");

        CPPUNIT_ASSERT(sipXH264CodecMode0.isSameDefinition(sipXH264CodecMode0));
        CPPUNIT_ASSERT(sipXH264CodecMode0.isSameDefinition(h264Codec1));
        CPPUNIT_ASSERT(!sipXH264CodecMode0.isSameDefinition(h264Codec2));
        CPPUNIT_ASSERT(!sipXH264CodecMode0.isSameDefinition(h264Codec3));

        CPPUNIT_ASSERT(!sipXH264CodecMode1.isSameDefinition(h264Codec1));
        CPPUNIT_ASSERT(sipXH264CodecMode1.isSameDefinition(h264Codec2));
        CPPUNIT_ASSERT(!sipXH264CodecMode1.isSameDefinition(h264Codec3));
        CPPUNIT_ASSERT(!h264Codec2.isSameDefinition(h264Codec3));

        CPPUNIT_ASSERT(sipXH264CodecMode0.isFmtpParameterSame(h264Codec1, "packetization-mode", "0"));
        CPPUNIT_ASSERT(!sipXH264CodecMode1.isFmtpParameterSame(h264Codec1, "packetization-mode", "0"));
        CPPUNIT_ASSERT(!sipXH264CodecMode0.isFmtpParameterSame(h264Codec2, "packetization-mode", "0"));
        CPPUNIT_ASSERT(sipXH264CodecMode1.isFmtpParameterSame(h264Codec2, "packetization-mode", "0"));
        CPPUNIT_ASSERT(!sipXH264CodecMode0.isFmtpParameterSame(h264Codec3, "packetization-mode", "0"));
        CPPUNIT_ASSERT(sipXH264CodecMode1.isFmtpParameterSame(h264Codec3, "packetization-mode", "0"));
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SdpCodecTest);

