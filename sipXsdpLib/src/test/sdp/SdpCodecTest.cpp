//
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
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
    CPPUNIT_TEST(testFmtpParsing);
    CPPUNIT_TEST(testH264SameTest);
    CPPUNIT_TEST(testMimeCase);
    CPPUNIT_TEST_SUITE_END();

public:

    void testFmtpParsing()
    {
        const char* fmtpData = "profile-level-id=428016; max-mbps=216000; max-fs=3600; max-br=5120; sar=13";
        UtlString value;
        SdpCodec::getFmtpParameter(fmtpData, "profile-level-id", value);
        CPPUNIT_ASSERT_EQUAL("428016", value);

        SdpCodec::getFmtpParameter(fmtpData, "max-mbps", value);
        CPPUNIT_ASSERT_EQUAL("216000", value);

        SdpCodec::getFmtpParameter(fmtpData, "max-fs", value);
        CPPUNIT_ASSERT_EQUAL("3600", value);

        SdpCodec::getFmtpParameter(fmtpData, "max-br", value);
        CPPUNIT_ASSERT_EQUAL("5120", value);

        SdpCodec::getFmtpParameter(fmtpData, "sar", value);
        CPPUNIT_ASSERT_EQUAL("13", value);

        // G.722.1 fmtp test
        const char* g7221Fmtp48 = "bitrate=48000";
        SdpCodec::getFmtpParameter(g7221Fmtp48, "bitrate", value);
        CPPUNIT_ASSERT_EQUAL("48000", value);
        int intValue;
        SdpCodec::getFmtpParameter(g7221Fmtp48, "bitrate", intValue);
        CPPUNIT_ASSERT_EQUAL(48000, intValue);

        const char* g7221Fmtp32 = "bitrate=32000";
        int compares;
        CPPUNIT_ASSERT_EQUAL(TRUE, SdpCodec::compareFmtp(MIME_TYPE_AUDIO, MIME_SUBTYPE_G7221, g7221Fmtp48, g7221Fmtp48, compares));
        CPPUNIT_ASSERT_EQUAL(1, compares);
        CPPUNIT_ASSERT_EQUAL(FALSE, SdpCodec::compareFmtp(MIME_TYPE_AUDIO, MIME_SUBTYPE_G7221, g7221Fmtp32, g7221Fmtp48, compares));

        // This format for size was found in SdpBodyTest.  Not sure if that
        // was a bug in the unit test or if this is actually found in real SDP from
        // some source.
        //  Most RFCs use "=" as the name/value separator for fmtp parameters.
        // Note: size":" not size"="
        fmtpData = "size:QCIF\r\n";
        SdpCodec::getFmtpParameter(fmtpData, "size", value, ':');
        CPPUNIT_ASSERT_EQUAL("QCIF", value);
        int videoSizes[100];
        int numSizes;
        OsStatus status = SdpCodec::getVideoSizes(fmtpData, 100, numSizes, videoSizes);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);
        CPPUNIT_ASSERT_EQUAL(1, numSizes);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_QCIF, videoSizes[0]);

        fmtpData = "size:QVGA/SQCIF/QCIF\r\n";
        status = SdpCodec::getVideoSizes(fmtpData, 2, numSizes, videoSizes);
        CPPUNIT_ASSERT_EQUAL(OS_LIMIT_REACHED, status);
        CPPUNIT_ASSERT_EQUAL(2, numSizes);

        status = SdpCodec::getVideoSizes(fmtpData, 100, numSizes, videoSizes);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);
        CPPUNIT_ASSERT_EQUAL(3, numSizes);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_QVGA, videoSizes[0]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_SQCIF, videoSizes[1]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_QCIF, videoSizes[2]);

        // This size format is used in SdpDefaultCodecFactory, not sure if it is used
        // out in the wild.  Could not find an RFC for this.
        fmtpData = "size=QCIF/CIF";
        status = SdpCodec::getVideoSizes(fmtpData, 10, numSizes, videoSizes);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);
        CPPUNIT_ASSERT_EQUAL(2, numSizes);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_QCIF, videoSizes[0]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_CIF, videoSizes[1]);

        fmtpData = "size=SQCIF/QCIF/CIF/QVGA/VGA/CIF4/CIF16";
        status = SdpCodec::getVideoSizes(fmtpData, 10, numSizes, videoSizes);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);
        CPPUNIT_ASSERT_EQUAL(7, numSizes);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_SQCIF, videoSizes[0]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_QCIF, videoSizes[1]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_CIF, videoSizes[2]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_QVGA, videoSizes[3]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_VGA, videoSizes[4]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_4CIF, videoSizes[5]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_16CIF, videoSizes[6]);

        // RFC 4629 size format for H.261 and H.263
        fmtpData = "CIF4=2;CIF=1;QCIF=1;SQCIF=1;CUSTOM=352,240,1;CUSTOM=704,480,2;J;T";
        status = SdpCodec::getVideoSizes(fmtpData, 10, numSizes, videoSizes);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);
        CPPUNIT_ASSERT_EQUAL(4, numSizes);
        // It is not clear if order should matter here
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_4CIF, videoSizes[3]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_CIF, videoSizes[2]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_QCIF, videoSizes[1]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_SQCIF, videoSizes[0]);

        // RFC 4629 size format for H.261 and H.263
        fmtpData = "VGA=2;QVGA;CIF;SQCIF=1;CIF16;CUSTOM=352,240,1;CUSTOM=704,480,2;J;T";
        status = SdpCodec::getVideoSizes(fmtpData, 10, numSizes, videoSizes);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);
        CPPUNIT_ASSERT_EQUAL(5, numSizes);
        // It is not clear if order should matter here
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_16CIF, videoSizes[4]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_VGA, videoSizes[3]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_QVGA, videoSizes[2]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_CIF, videoSizes[1]);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_SQCIF, videoSizes[0]);

        // This size format was found in SdpBodyTest, not sure it is valid
        // Note: size":" as opposed to size"="
        //fmtpData = "size:QVGA/SQCIF/QCIF";

        // Not sure the source of this size format
        // Possibly iChat???
        fmtpData = "imagesize 0 rules 20:640:480:640:480:20:";
        status = SdpCodec::getVideoSizes(fmtpData, 10, numSizes, videoSizes);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);
        CPPUNIT_ASSERT_EQUAL(1, numSizes);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_QCIF, videoSizes[0]);

        // Not sure the source of this size format
        // Possibly iChat???
        fmtpData = "imagesize 1 rules 30:352:288";
        status = SdpCodec::getVideoSizes(fmtpData, 10, numSizes, videoSizes);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);
        CPPUNIT_ASSERT_EQUAL(1, numSizes);
        CPPUNIT_ASSERT_EQUAL(SDP_VIDEO_FORMAT_CIF, videoSizes[0]);

        // TODO: test getFmtpParameter for iLBC mode parameter
    }

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

    void testMimeCase()
    {
        SdpCodec codec(SdpCodec::SDP_CODEC_PCMU,
                     SdpCodec::SDP_CODEC_PCMU,
                     MIME_TYPE_AUDIO,
                     MIME_SUBTYPE_PCMU,
                     8000,
                     20000,
                     1,
                     "",
                     SdpCodec::SDP_CODEC_CPU_LOW,
                     SDP_CODEC_BANDWIDTH_NORMAL);

        UtlString mimeType(MIME_TYPE_AUDIO);
        UtlString mimeSubtype(MIME_SUBTYPE_PCMU);

        // Put token in canonical case
        mimeType.SDP_MIME_TO_CASE();
        mimeSubtype.SDP_MIME_SUBTYPE_TO_CASE();

        UtlString gotMimeTime;
        UtlString gotMimeSubtype;

        codec.getMediaType(gotMimeTime);
        codec.getEncodingName(gotMimeSubtype);

        // NOTE: case sensative compare
        CPPUNIT_ASSERT_EQUAL(gotMimeTime, mimeType);
        CPPUNIT_ASSERT_EQUAL(gotMimeSubtype, mimeSubtype);

        SdpCodec codecMixedCase(SdpCodec::SDP_CODEC_PCMU,
                     SdpCodec::SDP_CODEC_PCMU,
                     "AuDiO", //MIME_TYPE_AUDIO,
                     "pCmU", //MIME_SUBTYPE_PCMU,
                     8000,
                     20000,
                     1,
                     "",
                     SdpCodec::SDP_CODEC_CPU_LOW,
                     SDP_CODEC_BANDWIDTH_NORMAL);

        codecMixedCase.getMediaType(gotMimeTime);
        codecMixedCase.getEncodingName(gotMimeSubtype);

        // NOTE: case sensative compare
        CPPUNIT_ASSERT_EQUAL(gotMimeTime, mimeType);
        CPPUNIT_ASSERT_EQUAL(gotMimeSubtype, mimeSubtype);

        mimeType = "aUdIo";
        mimeSubtype = "PcMu";
        mimeType.SDP_MIME_TO_CASE();
        mimeSubtype.SDP_MIME_SUBTYPE_TO_CASE();

        // NOTE: case sensative compare
        CPPUNIT_ASSERT_EQUAL(gotMimeTime, mimeType);
        CPPUNIT_ASSERT_EQUAL(gotMimeSubtype, mimeSubtype);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SdpCodecTest);

