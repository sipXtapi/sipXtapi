//
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/SdpBody.h>
#include <net/SdpHelper.h>
#include <sdp/SdpCodec.h>
#include <sdp/Sdp.h>
#include <sdp/SdpMediaLine.h>


/**
* Unit test for SdpHelper
*/
class SdpHelperTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(SdpHelperTest);
    CPPUNIT_TEST(testHelper);
    CPPUNIT_TEST_SUITE_END();

private:
    UtlString mSdpInitConfigStr;
public:
    void setUp()
    {
        mSdpInitConfigStr = "v=0\r\n"  
            "o=- 333525334858460 333525334858460 IN IP4 192.168.0.156\r\n"
            "s=test123\r\n"
            "e=unknown@invalid.net\r\n"
            "p=+972 683 1000\r\n"
            "c=IN IP4 127.0.0.1\r\n"
            "b=RR:0\r\n"
            "b=RS:0\r\n"
            "b=CT:10000\r\n"
            "t=4058038202 0\r\n"
            "k=base64:base64key\r\n"
            "a=tool:ResipParserTester\r\n"
            "a=inactive\r\n"
            "m=audio 41466/6 RTP/AVP 0 101\r\n"
            "i=Audio Stream\r\n"
            "c=IN IP4 192.168.0.156/100/3\r\n"
            "c=IN IP6 FF15::101/3\r\n"
            "k=clear:base64clearkey\r\n"
            "a=fmtp:101 0-11\r\n"
            "a=ptime:20\r\n"
            "a=fmtp:0 annexb=no\r\n"
            "a=maxptime:40\r\n"
            "a=setup:active\r\n"
            "a=sendrecv\r\n"
            "a=rtpmap:101 telephone-event/8000\r\n"
            "a=crypto:1 F8_128_HMAC_SHA1_80 inline:MTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5QUJjZGVm|2^20|1:4;inline:QUJjZGVmMTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5|2^20|2:4 FEC_ORDER=FEC_SRTP\r\n"
            "m=video 21234 RTP/AVP 140\r\n"
            "b=RR:1\r\n"
            "b=RS:0\r\n"
            "a=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:QUJjZGVmMTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5|2:18;inline:QUJjZGVmMTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5|21|3:4 KDR=23 FEC_ORDER=SRTP_FEC UNENCRYPTED_SRTP\r\n"
            "a=crypto:2 AES_CM_128_HMAC_SHA1_32 inline:QUJjZGVmMTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5|2^20 FEC_KEY=inline:QUJjZGVmMTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5|2^20|2:4 WSH=60\r\n"
            "a=fingerprint:sha-1 0123456789\r\n"
            "a=key-mgmt:mikey thisissomebase64data\r\n"
            "a=curr:qos e2e sendrecv\r\n"
            "a=curr:qos local send\r\n"
            "a=des:qos mandatory e2e sendrecv\r\n"
            "a=des:qos optional local send\r\n"
            "a=conf:qos e2e none\r\n"
            "a=conf:qos remote recv\r\n"
            "a=remote-candidates:1 192.168.0.1 5060 2 192.168.0.1 5061\r\n"
            "a=remote-candidates:3 192.168.0.2 5063\r\n"
            "a=candidate:foundation1 1 udp 100000 127.0.0.1 21234 typ host raddr 127.0.0.8 rport 6667 name value name2 value2\r\n"
            "a=candidate:foundation2 2 udp 100001 192.168.0.1 6667 raddr 127.0.0.9 rport 6668 name value name2 value2\r\n"
            "a=candidate:foundation3 3 udp 100002 192.168.0.2 6668 raddr 127.0.0.9 name value name2 value2\r\n"
            "a=candidate:foundation3 3 udp 100002 123.123.123.124 127 name value name2 value2\r\n"
            "a=candidate:foundation3 3 udp 100002 192.168.0.2 6668 typ relay\r\n"
            "a=rtcp:127 IN IP4 123.123.123.124/60\r\n"
            "a=rtpmap:140 vp71/144000\r\n"
            "a=fmtp:140 CIF=1 QCIF=2 SQCIF\r\n";
    }

    void testHelper()
    {
        SdpBody sdpBody(mSdpInitConfigStr, mSdpInitConfigStr.length());
        UtlString sdpString;
        Sdp* convSdp = SdpHelper::createSdpFromSdpBody(sdpBody);
        
        if(convSdp)
        {
            // Ensure string builder does not crash
            printf("\n\nsipX Sdp Helper Test:\n");
            convSdp->toString(sdpString);
            printf("%s\n", sdpString.data());

            // Perform some random assertions
            // Note:  The SdpBody class implementation is far from complete 
            //         - therefor the SdpHelper for sipX is incomplete 
            //         - therefor some assertions are commented out

            //assert(convSdp->getOriginatorUnicastAddress() == "192.168.0.156");
            //assert(convSdp->getSessionName() == "test123");
            //assert(convSdp->getEmailAddresses().entries() == 1);
            //assert(convSdp->getPhoneNumbers().entries() == 1);
            //assert(convSdp->getBandwidths().entries() == 2);
            //assert(convSdp->getTimes().entries() == 1);
            //assert(convSdp->getToolNameAndVersion() == "ResipParserTester");
            assert(convSdp->getMediaLines().entries() == 2);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getMediaType() == SdpMediaLine::MEDIA_TYPE_AUDIO);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getMediaType() == SdpMediaLine::MEDIA_TYPE_VIDEO);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getTransportProtocolType() == SdpMediaLine::PROTOCOL_TYPE_RTP_AVP);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().entries() == 2);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCodecs().entries() == 1);
            assert(((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().at(0))->getCodecType() == SdpCodec::SDP_CODEC_GIPS_PCMU);
            assert(((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().at(1))->getCodecType() == SdpCodec::SDP_CODEC_TONES);
            assert(((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCodecs().at(0))->getCodecType() == SdpCodec::SDP_CODEC_VP71_CIF);
            UtlString fmtp;
            ((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().at(1))->getSdpFmtpField(fmtp);
            assert(fmtp == "0-11");
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getConnections().entries() == 6);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getConnections().entries() == 1);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getRtcpConnections().entries() == 0);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getRtcpConnections().entries() == 1);
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getConnections().at(0))->getAddress() == "192.168.0.156");
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getConnections().at(0))->getPort() == 41466);
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)convSdp->getMediaLines().at(1))->getRtcpConnections().at(0))->getPort() == 127);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getBandwidths().entries() == 0);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getBandwidths().entries() == 2);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getEncryptionKey() == "base64clearkey");
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getDirection() == SdpMediaLine::DIRECTION_TYPE_SENDRECV);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getDirection() == SdpMediaLine::DIRECTION_TYPE_INACTIVE);  // !slg! SdpBody only reads first occurance of a= direction keyword
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getPacketTime() == 20);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getTcpConnectionAttribute() == SdpMediaLine::TCP_CONNECTION_ATTRIBUTE_NONE);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getTcpSetupAttribute() == SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCryptos().entries() == 1);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCryptos().entries() == 2);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCandidates().entries() == 0);   // !slg! candidate code in SdpBody is for old draft and doesn't work for this test
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCandidates().entries() == 5); 
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getRemoteCandidates().entries() == 3);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getPreConditionCurrentStatus().entries() == 2);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getPreConditionConfirmStatus().entries() == 2);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getPreConditionDesiredStatus().entries() == 2);
        }
        delete convSdp;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdpHelperTest);
