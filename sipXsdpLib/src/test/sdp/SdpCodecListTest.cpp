//
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
//
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <sipxunittests.h>

#include <os/OsDefs.h>
#include <sdp/SdpCodec.h>
#include <sdp/SdpCodecList.h>
#include <sdp/SdpDefaultCodecFactory.h>

/// Unit test for SdpCodec class
class SdpCodecListTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(SdpCodecListTest);
    CPPUNIT_TEST(testAddGetCodec);
    CPPUNIT_TEST(testH264Codecs)
    CPPUNIT_TEST_SUITE_END();

public:

    void testAddGetCodec()
    {
        SdpCodecList sdpFactory;
        SdpCodec* pPcmuCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMU,
                                            SdpCodec::SDP_CODEC_UNKNOWN,
                                            MIME_TYPE_AUDIO,
                                            MIME_SUBTYPE_PCMU,
                                            8000,
                                            20000); // ptime
        sdpFactory.addCodec(*pPcmuCodec);
        CPPUNIT_ASSERT_EQUAL(sdpFactory.getCodecCount(), 1);

        int compares;
        CPPUNIT_ASSERT_EQUAL(pPcmuCodec->compareFmtp("", compares), TRUE);
        CPPUNIT_ASSERT_EQUAL(compares, 0);

        const SdpCodec* matchCodec = sdpFactory.getCodec(MIME_TYPE_AUDIO, "pcmu", 8000, 1, "");
        CPPUNIT_ASSERT(matchCodec);

        SdpCodec* pPcmaCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMA,
                                            SdpCodec::SDP_CODEC_UNKNOWN,
                                            MIME_TYPE_AUDIO,
                                            MIME_SUBTYPE_PCMA,
                                            8000,
                                            20000); // ptime
        sdpFactory.addCodec(*pPcmaCodec);
        CPPUNIT_ASSERT_EQUAL(sdpFactory.getCodecCount(), 2);

        matchCodec = sdpFactory.getCodec(MIME_TYPE_AUDIO, "pcma", 8000, 1, "");
        CPPUNIT_ASSERT(matchCodec);

        SdpCodec* pSuperCodec = new SdpCodec((SdpCodec::SdpCodecTypes)333,
                                            SdpCodec::SDP_CODEC_UNKNOWN,
                                            MIME_TYPE_AUDIO,
                                            "superaudio",
                                            8000,
                                            20000); // ptime
        sdpFactory.addCodec(*pSuperCodec);
        CPPUNIT_ASSERT_EQUAL(sdpFactory.getCodecCount(), 3);

        matchCodec = sdpFactory.getCodec(MIME_TYPE_AUDIO, "superaudio", 8000, 1, "");
        CPPUNIT_ASSERT(matchCodec);

        // Mixed case match
        matchCodec = sdpFactory.getCodec("AudIO" /*MIME_TYPE_AUDIO*/, "sUperAUDIO", 8000, 1, "");
        CPPUNIT_ASSERT(matchCodec);

        SdpCodec* pSuperDuperCodec = new SdpCodec((SdpCodec::SdpCodecTypes)334,
                                            SdpCodec::SDP_CODEC_UNKNOWN,
                                            MIME_TYPE_AUDIO,
                                            "superduperaudio",
                                            8000,
                                            20000); // ptime
        sdpFactory.addCodec(*pSuperDuperCodec);
        CPPUNIT_ASSERT_EQUAL(sdpFactory.getCodecCount(), 4);

        SdpCodec* pQvgaCodec = new SdpCodec(SdpCodec::SDP_CODEC_VP71_QVGA,
                                            SdpCodec::SDP_CODEC_UNKNOWN,
                                            MIME_TYPE_VIDEO,
                                            "vp71", // MIME subtype
                                            9000,
                                            20000,
                                            1,
                                            "",
                                            SdpCodec::SDP_CODEC_CPU_LOW,
                                            SDP_CODEC_BANDWIDTH_NORMAL,
                                            SDP_VIDEO_FORMAT_QCIF);
        sdpFactory.addCodec(*pQvgaCodec);
        CPPUNIT_ASSERT_EQUAL(5, sdpFactory.getCodecCount());

        matchCodec = sdpFactory.getCodec(MIME_TYPE_VIDEO, "vp71", 9000, 1, "size:QCIF/SQCIF");
        CPPUNIT_ASSERT(matchCodec);
    }

    void testH264Codecs()
    {
        SdpCodecList codecList;
        codecList.addCodecs("G722 AAC_LC_32000 PCMU H264_CIF_256 H264_PM1_CIF_256");

        CPPUNIT_ASSERT_EQUAL(codecList.getCodecCount(), 5);

        int numVideoCodecs;
        SdpCodec** videoCodecs = NULL;
        codecList.getCodecs(numVideoCodecs, videoCodecs, MIME_TYPE_VIDEO);
        CPPUNIT_ASSERT_EQUAL(numVideoCodecs, 2);
        CPPUNIT_ASSERT(videoCodecs);
        SdpCodecList::freeArray(numVideoCodecs, videoCodecs);

        // Default fmtp should be packetization-mode=0
        const SdpCodec* matchCodec = codecList.getCodec(MIME_TYPE_VIDEO, MIME_SUBTYPE_H264, 90000, 1, "");
        CPPUNIT_ASSERT_EQUAL(matchCodec, NULL);

        matchCodec = codecList.getCodec(MIME_TYPE_VIDEO, MIME_SUBTYPE_H264, 90000, 1, "profile-level-id=42800D;");
        CPPUNIT_ASSERT_EQUAL(matchCodec->getCodecType(), SdpCodec::SDP_CODEC_H264_CIF_256);

        matchCodec = codecList.getCodec(MIME_TYPE_VIDEO, MIME_SUBTYPE_H264, 90000, 1, "profile-level-id=42800D;packetization-mode=0");
        CPPUNIT_ASSERT(matchCodec);
        CPPUNIT_ASSERT_EQUAL(matchCodec->getCodecType(), SdpCodec::SDP_CODEC_H264_CIF_256);

        matchCodec = codecList.getCodec(MIME_TYPE_VIDEO, MIME_SUBTYPE_H264, 90000, 1, "packetization-mode=1;profile-level-id=42800D;");
        CPPUNIT_ASSERT(matchCodec);
        CPPUNIT_ASSERT_EQUAL(matchCodec->getCodecType(), SdpCodec::SDP_CODEC_H264_PM1_CIF_256);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SdpCodecListTest);

