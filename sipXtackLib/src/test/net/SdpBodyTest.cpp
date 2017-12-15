//
// Copyright (C) 2005-2017 SIPez LLC.  All rights reserved.
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <sipxunittests.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <utl/UtlHashBag.h>
#include <net/HttpMessage.h>
#include <net/SdpBody.h>
#include <sdp/SdpCodecList.h>
#include <sdp/SdpMediaLine.h>
#include <sdp/SdpDefaultCodecFactory.h>
#include <net/NetBase64Codec.h>

const char* sBadCodecs[] = 
{
    "ISAC",
    "IPCMWB",
    "EG711A",
    "EG711U",
    "G729A-FOR-CISCO-7960",
    "AVT", // duplicate of TELEPHONE-EVENT
    "AVT-TONES", // duplicate of TELEPHONE-EVENT
    "AUDIO/TELEPHONE-EVENT" // duplicate of TELEPHONE-EVENT
};


/**
* Unit test for SdpBody
*/
class SdpBodyTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(SdpBodyTest);
    CPPUNIT_TEST(testParser);
    CPPUNIT_TEST(testIndexAccessor);
    CPPUNIT_TEST(testNewMessage);
    CPPUNIT_TEST(testTimeHeaders);
    CPPUNIT_TEST(testGetMediaSetCount);
    CPPUNIT_TEST(testGetMediaAddress);
    CPPUNIT_TEST(testCandidateParsing);
    CPPUNIT_TEST(testRtcpPortParsing);
    CPPUNIT_TEST(testVideoCodecSelection);
    CPPUNIT_TEST(testH264CodecCandidate);
    CPPUNIT_TEST(testPtime);
    CPPUNIT_TEST(testGetCodecsInCommon);
    CPPUNIT_TEST(testDirectionAttribute);
    CPPUNIT_TEST(testUseRemotePayloadIds);
    CPPUNIT_TEST(testMlineOrder);
    CPPUNIT_TEST(testVideoInCommon1);
    CPPUNIT_TEST(testVideoInCommon2);
    CPPUNIT_TEST(testCryptoParser);
    CPPUNIT_TEST(test3Mlines);
    CPPUNIT_TEST(test5Mlines);
    CPPUNIT_TEST(testGetCodecsInCommonFull);
    CPPUNIT_TEST_SUITE_END();

    UtlHashBag mCodecsToIgnore;

public:
    void setUp()
    {
        // In case there is garbage from prior runs, delete contents
        mCodecsToIgnore.destroyAll();

        for(size_t badCodecIndex = 0; badCodecIndex < sizeof(sBadCodecs)/sizeof(const char*); badCodecIndex++)
        {
            mCodecsToIgnore.insert(new UtlString(sBadCodecs[badCodecIndex]));
        }
    }

    void testParser()
    {
        const char *sdp = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "a=ptime: 30\r\n"
            "m=audio 59170 TCP/RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=video 61372 TCP/RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

        SdpBody body(sdp);

        UtlString bodyString;
        int bodyLength;

        body.getBytes(&bodyString, &bodyLength);

        ASSERT_STR_EQUAL( bodyString.data(), sdp);

        int otherLength = body.getLength();
        CPPUNIT_ASSERT( bodyLength == otherLength );

        const char *junk = "IRRELEVENT JUNK THAT IS NOT SDP";

        UtlString embeddedSdp(sdp);
        embeddedSdp.append(junk);

        SdpBody embeddedBody(embeddedSdp.data(), strlen(sdp));

        UtlString extractedString;
        int extractedStringLength;

        embeddedBody.getBytes(&extractedString, &extractedStringLength);

        ASSERT_STR_EQUAL( bodyString.data(), extractedString.data());
    } 

    void testIndexAccessor()
    {
        const char *sdp = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=audio 59170 TCP/RTP/AVP 0\r\n"
            "a=ptime:22\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=video 61372 TCP/RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

        SdpBody body(sdp);

        /* Why are there not accessors for version or other standard fields !?! */

        int fields = body.SdpBody::getFieldCount();
        CPPUNIT_ASSERT(fields == 16);

        UtlString name;
        UtlString value;

        CPPUNIT_ASSERT(body.getValue(0, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "v");
        ASSERT_STR_EQUAL(value.data(), "0");

        CPPUNIT_ASSERT(body.getValue(1, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "o");
        ASSERT_STR_EQUAL(value.data(), "mhandley 2890844526 2890842807 IN IP4 126.16.64.4");

        CPPUNIT_ASSERT(body.getValue(2, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "s");
        ASSERT_STR_EQUAL(value.data(), "SDP Seminar");

        CPPUNIT_ASSERT(body.getValue(3, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "i");
        ASSERT_STR_EQUAL(value.data(), "A Seminar on the session description protocol");

        CPPUNIT_ASSERT(body.getValue(4, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "u");
        ASSERT_STR_EQUAL(value.data(), "http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps");

        CPPUNIT_ASSERT(body.getValue(5, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "e");
        ASSERT_STR_EQUAL(value.data(), "mjh@isi.edu (Mark Handley)");

        CPPUNIT_ASSERT(body.getValue(6, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "c");
        ASSERT_STR_EQUAL(value.data(), "IN IP4 224.2.17.12/127");

        CPPUNIT_ASSERT(body.getValue(7, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "t");
        ASSERT_STR_EQUAL(value.data(), "2873397496 2873404696");

        CPPUNIT_ASSERT(body.getValue(8, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "a");
        ASSERT_STR_EQUAL(value.data(), "recvonly");

        CPPUNIT_ASSERT(body.getValue(9, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "m");
        ASSERT_STR_EQUAL(value.data(), "audio 49170 RTP/AVP 0");

        CPPUNIT_ASSERT(body.getValue(10, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "m");
        ASSERT_STR_EQUAL(value.data(), "audio 59170 TCP/RTP/AVP 0");

        CPPUNIT_ASSERT(body.getValue(11, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "a");
        ASSERT_STR_EQUAL(value.data(), "ptime:22");

        CPPUNIT_ASSERT(body.getValue(12, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "m");
        ASSERT_STR_EQUAL(value.data(), "video 51372 RTP/AVP 31");

        CPPUNIT_ASSERT(body.getValue(13, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "m");
        ASSERT_STR_EQUAL(value.data(), "video 61372 TCP/RTP/AVP 31");

        CPPUNIT_ASSERT(body.getValue(14, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "m");
        ASSERT_STR_EQUAL(value.data(), "application 32416 udp wb");

        CPPUNIT_ASSERT(body.getValue(15, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "a");
        ASSERT_STR_EQUAL(value.data(), "orient:portrait");

        CPPUNIT_ASSERT(!body.getValue(16, &name, &value));
        CPPUNIT_ASSERT(name.isNull());
        CPPUNIT_ASSERT(value.isNull());

        name = "foo";
        value = "notempty";

        CPPUNIT_ASSERT(!body.getValue(16, &name, &value));
        CPPUNIT_ASSERT(name.isNull());
        CPPUNIT_ASSERT(value.isNull());

        name = "foo";
        value = "notempty";

        CPPUNIT_ASSERT(!body.getValue(-2, &name, &value));
        CPPUNIT_ASSERT(value.isNull());
        CPPUNIT_ASSERT(name.isNull());
    }

    void testNewMessage()
    {
        UtlString sdpMsg;
        UtlString sdpExpected;
        int sdpLength;

        SdpBody newSdp;

        const char *expectedStdVersion = 
            "v=0\r\n"
            ;

        const char *defaultOriginator = // provided automatically
            "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
            ;

        const char *expectedEmptySessionName = // not optional, so will be there but empty
            "s=\r\n"
            ;
        newSdp.getBytes(&sdpMsg, &sdpLength);

        sdpExpected.remove(0);
        sdpExpected.append(expectedStdVersion);
        sdpExpected.append(defaultOriginator);
        sdpExpected.append(expectedEmptySessionName);

        ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());

        newSdp.setStandardHeaderFields("my session name",
            "me@example.org",
            "+1-8005551212",
            "10.1.1.1"
            );

        newSdp.getBytes(&sdpMsg, &sdpLength);

        const char *expectedStdHead = 
            "o=sipX 5 5 IN IP4 10.1.1.1\r\n"
            "s=my session name\r\n"
            "e=me@example.org\r\n"
            "p=+1-8005551212\r\n"
            ;

        sdpExpected.remove(0);
        sdpExpected.append(expectedStdVersion);
        sdpExpected.append(expectedStdHead);

        ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());

        newSdp.setPhoneNumberField("+1-7819385306");

        newSdp.getBytes(&sdpMsg, &sdpLength);

        const char *expectedPhone = 
            "o=sipX 5 5 IN IP4 10.1.1.1\r\n"
            "s=my session name\r\n"
            "e=me@example.org\r\n"
            "p=+1-7819385306\r\n"
            ;

        sdpExpected.remove(0);
        sdpExpected.append(expectedStdVersion);
        sdpExpected.append(expectedPhone);

        ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());

        newSdp.setEmailAddressField("you@example.com");

        newSdp.getBytes(&sdpMsg, &sdpLength);

        const char *expectedEmail = 
            "o=sipX 5 5 IN IP4 10.1.1.1\r\n"
            "s=my session name\r\n"
            "e=you@example.com\r\n"
            "p=+1-7819385306\r\n"
            ;

        sdpExpected.remove(0);
        sdpExpected.append(expectedStdVersion);
        sdpExpected.append(expectedEmail);

        ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());

        newSdp.setSessionNameField("your session name");

        newSdp.getBytes(&sdpMsg, &sdpLength);

        const char *expectedName = 
            "o=sipX 5 5 IN IP4 10.1.1.1\r\n"
            "s=your session name\r\n"
            "e=you@example.com\r\n"
            "p=+1-7819385306\r\n"
            ;

        sdpExpected.remove(0);
        sdpExpected.append(expectedStdVersion);
        sdpExpected.append(expectedName);

        ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());

        newSdp.setOriginator("myStream", 10, 20, "10.2.2.2");

        newSdp.getBytes(&sdpMsg, &sdpLength);

        const char *expectedOrigin = 
            "o=myStream 10 20 IN IP4 10.2.2.2\r\n"
            "s=your session name\r\n"
            "e=you@example.com\r\n"
            "p=+1-7819385306\r\n"
            ;

        sdpExpected.remove(0);
        sdpExpected.append(expectedStdVersion);
        sdpExpected.append(expectedOrigin);

        ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());
    }

    void testTimeHeaders()
    {
        const char *sdp = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=audio 59170 TCP/RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=video 61372 TCP/RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

        SdpBody body(sdp);

        body.addNtpTime( 10000000, 20000000 );

        UtlString sdpMsg;
        int sdpLength;

        body.getBytes(&sdpMsg, &sdpLength);

        const char *expectedSdp = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "t=10000000 20000000\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=audio 59170 TCP/RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=video 61372 TCP/RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

        ASSERT_STR_EQUAL(expectedSdp, sdpMsg.data());

        const char *sdp2 = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=audio 59170 TCP/RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=video 61372 TCP/RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

        SdpBody body2(sdp2);

        body2.addEpochTime( 0, 1 );

        body2.getBytes(&sdpMsg, &sdpLength);

        const char *expectedSdp2 = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "t=2208988800 2208988801\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=audio 59170 TCP/RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=video 61372 TCP/RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

        ASSERT_STR_EQUAL(expectedSdp2, sdpMsg.data());
    }

    void testGetMediaSetCount()
    {
        const char *sdp1 = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=audio 59170 TCP/RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=video 61372 TCP/RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;
        SdpBody body1(sdp1);

        UtlString sdpBodyString;
        int bodlen;
        body1.getBytes(&sdpBodyString, &bodlen);
        CPPUNIT_ASSERT(body1.getMediaSetCount() == 5);

        const char *sdp2 = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "a=orient:portrait\r\n"
            ;
        SdpBody body2(sdp2);

        CPPUNIT_ASSERT(body2.getMediaSetCount() == 0);

        const char *sdp3 = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "i=Silly Title\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "c=IN IP4 224.2.17.15/127\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;
        SdpBody body3(sdp3);

        CPPUNIT_ASSERT(body3.getMediaSetCount() == 3);
    }

    void testGetMediaAddress()
    {
        UtlString address;

        const char *sdp1 = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=audio 59170 TCP/RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=video 61372 TCP/RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;
        SdpBody body1(sdp1);

        CPPUNIT_ASSERT(body1.getMediaAddress(0, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.12");
        address.remove(0);

        CPPUNIT_ASSERT(body1.getMediaAddress(1, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.12");
        address.remove(0);

        CPPUNIT_ASSERT(body1.getMediaAddress(2, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.12");

        CPPUNIT_ASSERT(body1.getMediaAddress(3, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.12");

        CPPUNIT_ASSERT(body1.getMediaAddress(4, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.12");

        address = "wrong";

        CPPUNIT_ASSERT(!body1.getMediaAddress(5, &address));
        CPPUNIT_ASSERT(address.isNull());

        address = "wrong";

        CPPUNIT_ASSERT(!body1.getMediaAddress(-1, &address));
        CPPUNIT_ASSERT(address.isNull());

        const char *sdp2 = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=audio 59170 TCP/RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "c=IN IP4 224.2.17.15/127\r\n"
            "m=video 61372 TCP/RTP/AVP 31\r\n"
            "c=IN IP4 224.2.17.16/127\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;
        SdpBody body2(sdp2);

        CPPUNIT_ASSERT(body2.getMediaAddress(0, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.12");

        CPPUNIT_ASSERT(body2.getMediaAddress(1, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.12");


        CPPUNIT_ASSERT(body2.getMediaAddress(2, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.15");

        CPPUNIT_ASSERT(body2.getMediaAddress(3, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.16");
        
        CPPUNIT_ASSERT(body2.getMediaAddress(4, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.12");
        
    }

    void testCandidateParsing()
    {
        int candidateId;
        UtlString transportId;
        UtlString transportType;
        uint64_t qValue;
        UtlString candidateIp;
        int candidatePort;
        UtlBoolean bRC ;
        int i;

        /*
        * First test: Verify that we don't blow up when no candidate
        * attributes are found.
        */
        const char* sdp1 = 
            "v=0\r\n"
            "o=Pingtel 5 5 IN IP4 192.168.1.102\r\n"
            "s=phone-call\r\n"
            "c=IN IP4 192.168.1.102\r\n"
            "t=0 0\r\n"
            "m=audio 8778 RTP/AVP 0 8 96\r\n"
            "a=rtpmap:0 pcmu/8000/1\r\n"
            "a=rtpmap:8 pcma/8000/1\r\n"
            "a=rtpmap:96 telephone-event/8000/1\r\n" ;

        SdpBody body1(sdp1) ;

        for (i=-1; i<4; i++)
        {
            bRC = body1.getCandidateAttribute(0, -1, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;

            CPPUNIT_ASSERT(bRC == FALSE) ;
        }

        /*
        * Second test: Verify that we can parse expected data
        */
        const char* sdp2 = 
            "v=0\r\n"
            "o=Pingtel 5 5 IN IP4 192.168.1.102\r\n"
            "s=phone-call\r\n"
            "c=IN IP4 192.168.1.102\r\n"
            "t=0 0\r\n"
            "m=audio 8778 RTP/AVP 0 8 96\r\n"
            "a=ptime: 33\r\n"
            "a=rtpmap:0 pcmu/8000/1\r\n"
            "a=rtpmap:8 pcma/8000/1\r\n"
            "a=rtpmap:96 telephone-event/8000/1\r\n" 
            "a=candidate:0 tid1 UDP 10000100004 10.1.1.102 9999\r\n"
            "a=candidate:1 tid2 UDP 10000100005 10.1.1.102 10000\r\n"
            "a=candidate:2 tid3 UDP 10000100006 10.1.1.103 9999\r\n" 
            "m=audio 9778 TCP/RTP/AVP 0 8 96\r\n"
            "a=rtpmap:0 pcmu/8000/1\r\n"
            "a=rtpmap:8 pcma/8000/1\r\n"
            "a=rtpmap:96 telephone-event/8000/1\r\n" 
            "a=candidate:0 tid1 TCP 10000100004 10.1.1.104 9999\r\n"
            "a=candidate:1 tid2 TCP 10000100005 10.1.1.104 10000\r\n"
            "a=candidate:2 tid3 TCP 10000100006 10.1.1.105 9999\r\n" 
            "a=ptime:22\r\n"
            "m=video 1234 RTP/AVP 0 8 96\r\n"
            "a=rtpmap:0 pcmu/8000/1\r\n"
            "a=rtpmap:8 pcma/8000/1\r\n"
            "a=rtpmap:96 telephone-event/8000/1\r\n" 
            "a=candidate:0 tid1b UDP 10000100004 10.1.1.102 9999\r\n"
            "a=candidate:1 tid2b UDP 10000100005 10.1.1.102 10000\r\n"
            "a=candidate:2 tid3b UDP 10000100006 10.1.1.103 9999\r\n" ;


        SdpBody body2(sdp2) ;

        bRC = body2.getCandidateAttribute(0, -1, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT_EQUAL(bRC, FALSE);

        bRC = body2.getCandidateAttribute(0, 0, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT_EQUAL(bRC, TRUE);
        CPPUNIT_ASSERT(candidateId == 0) ;
        ASSERT_STR_EQUAL("tid1", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 10000100004) ;
        ASSERT_STR_EQUAL("10.1.1.102", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;

        bRC = body2.getCandidateAttribute(0, 1, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 1) ;
        ASSERT_STR_EQUAL("tid2", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 10000100005) ;
        ASSERT_STR_EQUAL("10.1.1.102", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 10000) ;

        bRC = body2.getCandidateAttribute(0, 2, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 2) ;
        ASSERT_STR_EQUAL("tid3", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 10000100006) ;
        ASSERT_STR_EQUAL("10.1.1.103", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;

        bRC = body2.getCandidateAttribute(0, 3, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == FALSE) ;

        bRC = body2.getCandidateAttribute(1, 0, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 0) ;
        ASSERT_STR_EQUAL("tid1", transportId.data()) ;
        ASSERT_STR_EQUAL("TCP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 10000100004) ;
        ASSERT_STR_EQUAL("10.1.1.104", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;

        bRC = body2.getCandidateAttribute(1, 1, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 1) ;
        ASSERT_STR_EQUAL("tid2", transportId.data()) ;
        ASSERT_STR_EQUAL("TCP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 10000100005) ;
        ASSERT_STR_EQUAL("10.1.1.104", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 10000) ;

        bRC = body2.getCandidateAttribute(1, 2, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 2) ;
        ASSERT_STR_EQUAL("tid3", transportId.data()) ;
        ASSERT_STR_EQUAL("TCP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 10000100006) ;
        ASSERT_STR_EQUAL("10.1.1.105", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;

        bRC = body2.getCandidateAttribute(1, 3, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == FALSE) ;

        bRC = body2.getCandidateAttribute(2, -1, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == FALSE) ;

        bRC = body2.getCandidateAttribute(2, 0, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 0) ;
        ASSERT_STR_EQUAL("tid1b", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 10000100004) ;
        ASSERT_STR_EQUAL("10.1.1.102", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;

        bRC = body2.getCandidateAttribute(2, 1, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 1) ;
        ASSERT_STR_EQUAL("tid2b", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 10000100005) ;
        ASSERT_STR_EQUAL("10.1.1.102", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 10000) ;

        bRC = body2.getCandidateAttribute(2, 2, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 2) ;
        ASSERT_STR_EQUAL("tid3b", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 10000100006) ;
        ASSERT_STR_EQUAL("10.1.1.103", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;        

        bRC = body2.getCandidateAttribute(2, 3, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == FALSE) ;


        SdpBody testBody;
        const char* testBodyExpected = 
            "v=0\r\n"
            "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
            "s=\r\n"
            "a=candidate:0 tid1 UDP 1000104 10.1.1.102 9999\r\n"
            "a=candidate:0 tid2 UDP 1000104 10.1.1.102 10000\r\n" ;

        testBody.addCandidateAttribute(0, "tid1", "UDP", 1000104, "10.1.1.102", 9999) ;
        testBody.addCandidateAttribute(0, "tid2", "UDP", 1000104, "10.1.1.102", 10000) ;
        UtlString strBody ;
        int nBody = 0;
        testBody.getBytes(&strBody, &nBody) ;

        ASSERT_STR_EQUAL(testBodyExpected, strBody.data()) ;
    }

    void testRtcpPortParsing()
    {
        SdpBody testBody ;
        SdpSrtpParameters testSrtp;
        UtlString strBody ;
        int nBody ;
        UtlString hostAddresses[1];
        int rtpAudioPorts[1];
        int rtcpAudioPorts[1];
        int rtpVideoPorts[1];
        int rtcpVideoPorts[1];
        RTP_TRANSPORT transportTypes[1];
        

        testSrtp.securityLevel = 0;

        SdpCodec* pAudioCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMU, 99, "audio", "superaudio") ;
        SdpCodec* pVideoCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMU, 100, "video", "supervideo") ;
        SdpCodec* pAppCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMU, 101, "app", "superapp") ;

        // This test case isn't exactly valid, but allows us to walk the m lines.
        testBody.setSessionNameField("foo") ;
        hostAddresses[0] = "10.1.1.30";
        rtpAudioPorts[0] = 8700;
        rtcpAudioPorts[0] = 8701;
        rtpVideoPorts[0] = 0;
        rtcpVideoPorts[0] = 0;
        transportTypes[0] = RTP_TRANSPORT_UDP;
        testBody.addCodecsOffer(1, hostAddresses, rtpAudioPorts, rtcpAudioPorts, 
                                rtpVideoPorts, rtcpVideoPorts, transportTypes,
                                1, &pAudioCodec, testSrtp, 0, 0, RTP_TRANSPORT_UDP) ;
                                
                                
        hostAddresses[0] = "10.1.1.30";
        rtpAudioPorts[0] = 18700;
        rtcpAudioPorts[0] = 18701;
        rtpVideoPorts[0] = 0;
        rtcpVideoPorts[0] = 0;
        transportTypes[0] = RTP_TRANSPORT_TCP;
        testBody.addCodecsOffer(1, hostAddresses, rtpAudioPorts, rtcpAudioPorts, 
                                rtpVideoPorts, rtcpVideoPorts, transportTypes,
                                1, &pAudioCodec, testSrtp, 0, 0, RTP_TRANSPORT_TCP) ;
                                
        hostAddresses[0] = "10.1.1.31";
        rtpAudioPorts[0] = 0;
        rtcpAudioPorts[0] = 0;
        rtpVideoPorts[0] = 8801;
        rtcpVideoPorts[0] = 8802;
        testBody.addCodecsOffer(1, hostAddresses, rtpAudioPorts, rtcpAudioPorts, 
                                rtpVideoPorts, rtcpVideoPorts, transportTypes,
                                1, &pVideoCodec, testSrtp, 0, 0, RTP_TRANSPORT_TCP) ;
        hostAddresses[0] = "10.1.1.32";
        rtpAudioPorts[0] = 8900;
        rtcpAudioPorts[0] = 8999;
        rtpVideoPorts[0] = 0;
        rtcpVideoPorts[0] = 0;
        testBody.addCodecsOffer(1, hostAddresses, rtpAudioPorts, rtcpAudioPorts, 
                                rtpVideoPorts, rtcpVideoPorts, transportTypes,
                                1, &pAppCodec, testSrtp, 0, 0, RTP_TRANSPORT_TCP) ;

        // TODO: there is a bug in addCodecsOffer.  The last m line (the one
        // generated for pAppCodec) should be of app media type.  In the SDB
        // string below the last m line is of audio media type.

        // SDP Mime Subtype canonical case is lower case
        // Depends upon SDP_MIME_SUBTYPE_TO_CASE
#if 0
        const char* testBodyExpected = 
            "v=0\r\n"
            "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
            "s=foo\r\n"
            "c=IN IP4 10.1.1.30\r\n"
            "t=0 0\r\n"
            "m=audio 8700 RTP/AVP 99\r\n"
            "a=control:trackID=1\r\n"
            "a=rtpmap:99 superaudio/8000/1\r\n"
            "a=ptime:20\r\n"
            "m=audio 18700 TCP/RTP/AVP 99\r\n"
            "a=control:trackID=1\r\n"
            "c=IN IP4 10.1.1.30\r\n"
            "a=rtpmap:99 superaudio/8000/1\r\n"
            "a=ptime:20\r\n"
            "m=video 8801 TCP/RTP/AVP 100\r\n"
            "a=control:trackID=2\r\n"
            "c=IN IP4 10.1.1.31\r\n"
            "a=rtcp:8802\r\n"
            "a=rtpmap:100 supervideo/8000/1\r\n"
            "a=fmtp:100 size:QCIF\r\n"
            "m=audio 8900 TCP/RTP/AVP 101\r\n"
            "a=control:trackID=1\r\n"
            "c=IN IP4 10.1.1.32\r\n" 
            "a=rtcp:8999\r\n"
            "a=rtpmap:101 superapp/8000/1\r\n";


        // SDP Mime Subtype canonical case is upper case
        // Depends upon SDP_MIME_SUBTYPE_TO_CASE
#else
        const char* testBodyExpected = 
            "v=0\r\n"
            "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
            "s=foo\r\n"
            "c=IN IP4 10.1.1.30\r\n"
            "t=0 0\r\n"
            "m=audio 8700 RTP/AVP 99\r\n"
            "a=control:trackID=1\r\n"
            "a=rtpmap:99 SUPERAUDIO/8000/1\r\n"
            "a=ptime:20\r\n"
            "m=audio 18700 TCP/RTP/AVP 99\r\n"
            "a=control:trackID=1\r\n"
            "c=IN IP4 10.1.1.30\r\n"
            "a=rtpmap:99 SUPERAUDIO/8000/1\r\n"
            "a=ptime:20\r\n"
            "m=video 8801 TCP/RTP/AVP 100\r\n"
            "a=control:trackID=2\r\n"
            "c=IN IP4 10.1.1.31\r\n"
            "a=rtcp:8802\r\n"
            "a=rtpmap:100 SUPERVIDEO/8000/1\r\n"
            "a=fmtp:100 size:QCIF\r\n"
            "m=audio 8900 TCP/RTP/AVP 101\r\n"
            "a=control:trackID=1\r\n"
            "c=IN IP4 10.1.1.32\r\n" 
            "a=rtcp:8999\r\n"
            "a=rtpmap:101 SUPERAPP/8000/1\r\n";

#endif
        UtlString address ;
        int port ;

        testBody.getBytes(&strBody, &nBody) ;
        ASSERT_STR_EQUAL(testBodyExpected, strBody.data()) ;


        SdpBody bodyCheck(testBodyExpected) ;

        CPPUNIT_ASSERT(bodyCheck.getMediaSetCount() == 4) ;

        bodyCheck.getMediaAddress(0, &address) ;
        ASSERT_STR_EQUAL("10.1.1.30", address.data()) ;
        bodyCheck.getMediaPort(0, &port) ; 
        CPPUNIT_ASSERT(port == 8700) ;
        bodyCheck.getMediaRtcpPort(0, &port) ;
        CPPUNIT_ASSERT(port == 8701) ;

        bodyCheck.getMediaAddress(1, &address) ;
        ASSERT_STR_EQUAL("10.1.1.30", address.data()) ;
        bodyCheck.getMediaPort(1, &port) ; 
        CPPUNIT_ASSERT(port == 18700) ;
        bodyCheck.getMediaRtcpPort(1, &port) ;
        CPPUNIT_ASSERT(port == 18701) ;

        bodyCheck.getMediaAddress(2, &address) ;
        ASSERT_STR_EQUAL("10.1.1.31", address.data()) ;
        bodyCheck.getMediaPort(2, &port) ; 
        CPPUNIT_ASSERT(port == 8801) ;
        bodyCheck.getMediaRtcpPort(2, &port) ;
        CPPUNIT_ASSERT(port == 8802) ;

        bodyCheck.getMediaAddress(3, &address) ;
        ASSERT_STR_EQUAL("10.1.1.32", address.data()) ;
        bodyCheck.getMediaPort(3, &port) ; 
        CPPUNIT_ASSERT(port == 8900) ;
        bodyCheck.getMediaRtcpPort(3, &port) ;
        CPPUNIT_ASSERT(port == 8999) ;
    }

    void testVideoCodecSelection()
    {
        SdpCodecList fac;

        SdpCodec* pQvgaCodec = new SdpCodec(SdpCodec::SDP_CODEC_VP71_QVGA,
                                            99,
                                            MIME_TYPE_VIDEO,
                                            "vp71",
                                            9000,
                                            20000,
                                            1,
                                            "",
                                            SdpCodec::SDP_CODEC_CPU_LOW,
                                            SDP_CODEC_BANDWIDTH_NORMAL,
                                            SDP_VIDEO_FORMAT_QVGA);
        SdpCodec* pSqcifCodec = new SdpCodec(SdpCodec::SDP_CODEC_VP71_SQCIF,
                                             100,
                                             MIME_TYPE_VIDEO,
                                             "vp71",
                                             9000,
                                             20000,
                                             1,
                                             "",
                                             SdpCodec::SDP_CODEC_CPU_LOW,
                                             SDP_CODEC_BANDWIDTH_NORMAL,
                                             SDP_VIDEO_FORMAT_SQCIF);
        SdpCodec* pQcifCodec = new SdpCodec(SdpCodec::SDP_CODEC_VP71_QCIF,
                                            101,
                                            MIME_TYPE_VIDEO,
                                            "vp71",
                                            9000,
                                            20000,
                                            1,
                                            "",
                                            SdpCodec::SDP_CODEC_CPU_LOW,
                                            SDP_CODEC_BANDWIDTH_NORMAL,
                                            SDP_VIDEO_FORMAT_QCIF);

        fac.addCodec(*pQvgaCodec);
        fac.addCodec(*pSqcifCodec);
        fac.addCodec(*pQcifCodec);

        const char* remoteBody = 
                "v=0\r\n"
                "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
                "s=foo\r\n"
                "c=IN IP4 10.1.1.30\r\n"
                "t=0 0\r\n"
                "m=audio 8700 RTP/AVP 99\r\n"
                "a=rtpmap:99 superaudio/8000/1\r\n"
                "m=video 8801 RTP/AVP 100\r\n"
                "a=rtpmap:100 vp71/9000/1\r\n"
                "a=fmtp:100 size:QCIF/SQCIF\r\n"
                "c=IN IP4 10.1.1.31\r\n";

        const char* localBody = 
                "v=0\r\n"
                "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
                "s=foo\r\n"
                "c=IN IP4 10.1.1.30\r\n"
                "t=0 0\r\n"
                "m=audio 8700 RTP/AVP 99\r\n"
                "a=rtpmap:99 superaudio/8000/1\r\n"
                "m=video 8801 RTP/AVP 100\r\n"
                "a=rtpmap:100 vp71/9000/1\r\n"
                "a=fmtp:100 size:QVGA/SQCIF/QCIF\r\n"
                "c=IN IP4 10.1.1.31\r\n";

        SdpBody local(localBody);
        SdpBody remote(remoteBody);

        UtlString fmtp;
        int numVideoSizes;
        int videoSizes[10];

        CPPUNIT_ASSERT_EQUAL(local.getPayloadFormat(1, 100, fmtp), TRUE);
        CPPUNIT_ASSERT_EQUAL(fmtp, "size:QVGA/SQCIF/QCIF");
        CPPUNIT_ASSERT_EQUAL(SdpCodec::getVideoSizes(fmtp, 10, numVideoSizes, videoSizes), OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(numVideoSizes, 3);
        CPPUNIT_ASSERT_EQUAL(videoSizes[0], SDP_VIDEO_FORMAT_QVGA);
        CPPUNIT_ASSERT_EQUAL(videoSizes[1], SDP_VIDEO_FORMAT_SQCIF);
        CPPUNIT_ASSERT_EQUAL(videoSizes[2], SDP_VIDEO_FORMAT_QCIF);

        CPPUNIT_ASSERT_EQUAL(remote.getPayloadFormat(1, 100, fmtp), TRUE);
        CPPUNIT_ASSERT_EQUAL(fmtp, "size:QCIF/SQCIF");
        CPPUNIT_ASSERT_EQUAL(SdpCodec::getVideoSizes(fmtp, 10, numVideoSizes, videoSizes), OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(numVideoSizes, 2);
        CPPUNIT_ASSERT_EQUAL(videoSizes[0], SDP_VIDEO_FORMAT_QCIF);
        CPPUNIT_ASSERT_EQUAL(videoSizes[1], SDP_VIDEO_FORMAT_SQCIF);

        int audioPayloads[5] = {99,0,0,0,0};
        int videoPayloads[5] = {100,0,0,0,0};
        int numInCommon;
        SdpCodec* codecsInCommonForEncoder[5] = {NULL, NULL, NULL, NULL, NULL};
        SdpCodec* codecsInCommonForDecoder[5] = {NULL, NULL, NULL, NULL, NULL};

        int audioMediaIndex = 0;
        int videoMediaIndex = 1;
        remote.getCodecsInCommon(audioMediaIndex, videoMediaIndex, 1, 1, audioPayloads, videoPayloads, 8801, fac,
                     numInCommon, codecsInCommonForEncoder, codecsInCommonForDecoder);
        CPPUNIT_ASSERT_EQUAL(numInCommon, 2);

        CPPUNIT_ASSERT_EQUAL(codecsInCommonForEncoder[0] ? codecsInCommonForEncoder[0]->getVideoFormat() : -1 , SDP_VIDEO_FORMAT_QCIF);
        CPPUNIT_ASSERT_EQUAL(codecsInCommonForEncoder[1] ? codecsInCommonForEncoder[1]->getVideoFormat() : -1 , SDP_VIDEO_FORMAT_SQCIF);
        CPPUNIT_ASSERT_EQUAL(codecsInCommonForDecoder[0] ? codecsInCommonForDecoder[0]->getVideoFormat() : -1 , SDP_VIDEO_FORMAT_QCIF);
        CPPUNIT_ASSERT_EQUAL(codecsInCommonForDecoder[1] ? codecsInCommonForDecoder[1]->getVideoFormat() : -1 , SDP_VIDEO_FORMAT_SQCIF);
    };

    void testH264CodecCandidate()
    {
        const char* videoSdpOfferBytes = 
            "v=0\r\n"
            "o=polycomdme 1493339645 0 IN IP4 172.22.2.119\r\n"
            "s=-\r\n"
            "c=IN IP4 172.22.2.119\r\n"
            "b=AS:512\r\n"
            "t=0 0\r\n"
            "m=audio 49160 RTP/AVP 115 102 9 15 0 8 18 101\r\n"
            "a=rtpmap:115 G7221/32000\r\n"
            "a=fmtp:115 bitrate=48000\r\n"
            "a=rtpmap:102 G7221/16000\r\n"
            "a=fmtp:102 bitrate=32000\r\n"
            "a=rtpmap:9 G722/8000\r\n"
            "a=rtpmap:15 G728/8000\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:8 PCMA/8000\r\n"
            "a=rtpmap:18 G729/8000\r\n"
            "a=fmtp:18 annexb=no\r\n"
            "a=rtpmap:101 telephone-event/8000\r\n"
            "a=fmtp:101 0-15\r\n"
            "a=sendrecv\r\n"
            "m=video 49162 RTP/AVP 109 110 111 112 96 34 31\r\n"
            "b=TIAS:512000\r\n"
            "a=rtpmap:109 H264/90000\r\n"
            "a=fmtp:109 profile-level-id=428016; max-mbps=216000; max-fs=3600; max-br=5120; sar=13\r\n"
            "a=rtpmap:110 H264/90000\r\n"
            "a=fmtp:110 profile-level-id=42E016; packetization-mode=1; max-mbps=216000; max-fs=3600; max-br=5120; sar=13\r\n"
            "a=rtpmap:111 H264/90000\r\n"
            "a=fmtp:111 profile-level-id=640016; packetization-mode=1; max-mbps=216000; max-fs=3600; max-br=5120; sar=13\r\n"
            "a=rtpmap:112 H264/90000\r\n"
            "a=fmtp:112 profile-level-id=44E016; packetization-mode=1; max-mbps=216000; max-fs=3600; max-br=5120; sar=13\r\n"
            "a=rtpmap:96 H263-1998/90000\r\n"
            "a=fmtp:96 CIF4=2;CIF=1;QCIF=1;SQCIF=1;CUSTOM=352,240,1;CUSTOM=704,480,2;J;T\r\n"
            "a=rtpmap:34 H263/90000\r\n"
            "a=fmtp:34 CIF4=2;CIF=1;QCIF=1;SQCIF=1\r\n"
            "a=rtpmap:31 H261/90000\r\n"
            "a=fmtp:31 CIF=1;QCIF=1\r\n"
            "a=sendrecv\r\n"
            "a=rtcp-fb:* ccm fir tmmbr\r\n"
            "m=application 49164 RTP/AVP 10\r\n"
            "a=rtpmap:100 H224/4800\r\n"
            "a=sendrecv\r\n";

        SdpBody offerBody(videoSdpOfferBytes);

        CPPUNIT_ASSERT_EQUAL(offerBody.findMediaType(SDP_AUDIO_MEDIA_TYPE, 0), 0);
        CPPUNIT_ASSERT_EQUAL(offerBody.findMediaType(SDP_VIDEO_MEDIA_TYPE, 0), 1);
        CPPUNIT_ASSERT_EQUAL(offerBody.findMediaType(MIME_TYPE_APPLICATION, 0), 2);
        CPPUNIT_ASSERT_EQUAL(offerBody.findMediaType(SDP_AUDIO_MEDIA_TYPE, 1), -1);
        CPPUNIT_ASSERT_EQUAL(offerBody.findMediaType(SDP_AUDIO_MEDIA_TYPE, 2), -1);
        CPPUNIT_ASSERT_EQUAL(offerBody.findMediaType(MIME_TYPE_APPLICATION, 3), -1);

        int mediaPort;
        CPPUNIT_ASSERT_EQUAL(offerBody.getMediaPort(0, &mediaPort), TRUE);
        CPPUNIT_ASSERT_EQUAL(mediaPort, 49160);
        CPPUNIT_ASSERT_EQUAL(offerBody.getMediaPort(1, &mediaPort), TRUE);
        CPPUNIT_ASSERT_EQUAL(mediaPort, 49162);
        CPPUNIT_ASSERT_EQUAL(offerBody.getMediaPort(2, &mediaPort), TRUE);
        CPPUNIT_ASSERT_EQUAL(mediaPort, 49164);
        CPPUNIT_ASSERT_EQUAL(offerBody.getMediaPort(3, &mediaPort), FALSE);

        SdpCodecList codecList;
        codecList.addCodecs("G722 PCMU H264_PM1_HD1088_4096 H264_HD1088_4096");

        CPPUNIT_ASSERT_EQUAL(codecList.getCodecCount(), 4);

        int numCodecsInCommon = 0;
        SdpCodec** commonCodecsForEncoder = NULL;
        UtlString rtpAddress;
        int rtpPort = 0;
        int rtcpPort = 0;
        int videoRtpPort = 0;
        int videoRtcpPort = 0;
        SdpSrtpParameters localSrtpParams;
        SdpSrtpParameters matchingSrtpParams;
        int localBandwidth = 0;
        int matchingBandwidth = 0;
        int localVideoFramerate = 0;
        int matchingVideoFramerate = 0;

        memset(&localSrtpParams, 0, sizeof(SdpSrtpParameters));
        memset(&matchingSrtpParams, 0, sizeof(SdpSrtpParameters));

        offerBody.getBestAudioCodecs(codecList, numCodecsInCommon, commonCodecsForEncoder,
            rtpAddress, rtpPort, rtcpPort, videoRtpPort, videoRtcpPort, 
            localSrtpParams, matchingSrtpParams,
            localBandwidth, matchingBandwidth, localVideoFramerate, matchingVideoFramerate);

        int expectedCodecCount = 4;
        CPPUNIT_ASSERT_EQUAL(numCodecsInCommon, expectedCodecCount);
        CPPUNIT_ASSERT_EQUAL(codecList.getCodecCount(), expectedCodecCount);

        SdpCodec* encodec = NULL;
        UtlString mimeSubtype;

        int encodePayloadIds[] = {9, 0, 109, 110};
        int codecIds[] = {9, 0, 175, 184};
        const char* mimeSubtypes[] = {"g722", "pcmu", "h264", "h264"};

/*
encode codec[0] payload: 9 internal ID: 9 MIME subtype: g722
encode codec[1] payload: 0 internal ID: 0 MIME subtype: pcmu
encode codec[2] payload: 109 internal ID: 175 MIME subtype: h264
encode codec[3] payload: 110 internal ID: 184 MIME subtype: h264
*/

        for(int codecIndex = 0; codecIndex < numCodecsInCommon; codecIndex++)
        {
            encodec = commonCodecsForEncoder[codecIndex];
            if(encodec)
            {
                encodec->getEncodingName(mimeSubtype);
//#ifdef TEST_PRINT
                printf("encode codec[%d] payload: %d internal ID: %d MIME subtype: %s\n",
                    codecIndex, encodec->getCodecPayloadFormat(), encodec->getCodecType(), mimeSubtype.data());
//#endif
                UtlString message;
                message.appendFormat("encode index: %d", codecIndex);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(message.data(), encodec->getCodecPayloadFormat(), encodePayloadIds[codecIndex]);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(message.data(), encodec->getCodecType(), codecIds[codecIndex]);
                UtlString mimeSubtypeString(mimeSubtypes[codecIndex]);
                mimeSubtypeString.SDP_MIME_SUBTYPE_TO_CASE();
                CPPUNIT_ASSERT_EQUAL_MESSAGE(message.data(), mimeSubtype, mimeSubtypeString);
            }
        }
    };

    void testPtime()
    {
        // Body with no ptime
        const char* noPtimeBodyString =
                "v=0\r\n"
                "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
                "s=foo\r\n"
                "c=IN IP4 10.1.1.30\r\n"
                "t=0 0\r\n"
                "m=audio 8700 RTP/AVP 99\r\n"
                "a=rtpmap:99 superaudio/8000/1\r\n"
                "m=video 8801 RTP/AVP 100\r\n"
                "a=rtpmap:100 vp71/9000/1\r\n"
                "a=fmtp:100 size:QCIF/SQCIF\r\n"
                "c=IN IP4 10.1.1.31\r\n";
        SdpBody noPtimeSdpBody(noPtimeBodyString);

        int ptimeValue = -1;
        // ptime for first m line
        CPPUNIT_ASSERT(!noPtimeSdpBody.getPtime(0, ptimeValue));
        CPPUNIT_ASSERT_EQUAL(0, ptimeValue);

        ptimeValue = -1;
        // ptime for second m line
        CPPUNIT_ASSERT(!noPtimeSdpBody.getPtime(1, ptimeValue));
        CPPUNIT_ASSERT_EQUAL(0, ptimeValue);

        ptimeValue = -1;
        // ptime for third m line (m line does not exist)
        CPPUNIT_ASSERT(!noPtimeSdpBody.getPtime(2, ptimeValue));
        CPPUNIT_ASSERT_EQUAL(0, ptimeValue);

        const char* sloppyPtimeBodyString =
                "v=0\r\n"
                "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
                "s=foo\r\n"
                "c=IN IP4 10.1.1.30\r\n"
                "t=0 0\r\n"
                "m=audio 8700 RTP/AVP 96 97\r\n"
                "a=ptime : 11\r\n"
                "a=rtpmap:96 pcmu/8000/1\r\n"
                "a=rtpmap:97 pcma/8000/1\r\n"
                "a=ptime:12\r\n"   // second ptime for media set should be ignored
                "m=audio 8710 RTP/AVP 98 99\r\n"
                "a=rtpmap:98 superaudio/8000/1\r\n"
                "a= ptime: 22 \r\n"
                "a=rtpmap:99 superduperaudio/8000/1\r\n"
                "m=video 8801 RTP/AVP 100\r\n"
                "a=rtpmap:100 vp71/9000/1\r\n"
                "a=ptime:33\r\n"
                "a=fmtp:100 size:QCIF/SQCIF\r\n"
                "c=IN IP4 10.1.1.31\r\n";
        SdpBody sloppyPtimeSdpBody(sloppyPtimeBodyString);
    
        // ptime for first m line
        ptimeValue = -1;
        CPPUNIT_ASSERT(sloppyPtimeSdpBody.getPtime(0, ptimeValue));
        CPPUNIT_ASSERT_EQUAL(11, ptimeValue);
    
        // ptime for second m line
        ptimeValue = -1;
        CPPUNIT_ASSERT(sloppyPtimeSdpBody.getPtime(1, ptimeValue));
        CPPUNIT_ASSERT_EQUAL(22, ptimeValue);

        // ptime for third m line
        // Video does not usually have a ptime, by syntactically this should work
        ptimeValue = -1;
        CPPUNIT_ASSERT(sloppyPtimeSdpBody.getPtime(2, ptimeValue));
        CPPUNIT_ASSERT_EQUAL(33, ptimeValue);

        // The following test a lot of the utilities used in getCodecsInCommon
        UtlString mimeSubtype;
        int sampleRate;
        int numChannels;
        CPPUNIT_ASSERT_EQUAL(sloppyPtimeSdpBody.getPayloadRtpMap(0, 96, mimeSubtype, sampleRate, numChannels), TRUE);
        CPPUNIT_ASSERT_EQUAL(mimeSubtype, "pcmu");
        CPPUNIT_ASSERT_EQUAL(sampleRate, 8000);
        CPPUNIT_ASSERT_EQUAL(numChannels, 1);

        CPPUNIT_ASSERT_EQUAL(sloppyPtimeSdpBody.getPayloadRtpMap(0, 97, mimeSubtype, sampleRate, numChannels), TRUE);
        CPPUNIT_ASSERT_EQUAL(mimeSubtype, "pcma");
        CPPUNIT_ASSERT_EQUAL(sampleRate, 8000);
        CPPUNIT_ASSERT_EQUAL(numChannels, 1);

        CPPUNIT_ASSERT_EQUAL(sloppyPtimeSdpBody.getPayloadRtpMap(1, 98, mimeSubtype, sampleRate, numChannels), TRUE);
        CPPUNIT_ASSERT_EQUAL(mimeSubtype, "superaudio");
        CPPUNIT_ASSERT_EQUAL(sampleRate, 8000);
        CPPUNIT_ASSERT_EQUAL(numChannels, 1);

        CPPUNIT_ASSERT_EQUAL(sloppyPtimeSdpBody.getPayloadRtpMap(1, 99, mimeSubtype, sampleRate, numChannels), TRUE);
        CPPUNIT_ASSERT_EQUAL(mimeSubtype, "superduperaudio");
        CPPUNIT_ASSERT_EQUAL(sampleRate, 8000);
        CPPUNIT_ASSERT_EQUAL(numChannels, 1);

        CPPUNIT_ASSERT_EQUAL(sloppyPtimeSdpBody.getPayloadRtpMap(2, 100, mimeSubtype, sampleRate, numChannels), TRUE);
        CPPUNIT_ASSERT_EQUAL(mimeSubtype, "vp71");
        CPPUNIT_ASSERT_EQUAL(sampleRate, 9000);
        CPPUNIT_ASSERT_EQUAL(numChannels, 1);

        UtlString fmtp;
        int codecMode = 256;
        CPPUNIT_ASSERT_EQUAL(sloppyPtimeSdpBody.getPayloadFormat(0, 96, fmtp), FALSE);
        CPPUNIT_ASSERT_EQUAL(fmtp, "");
        CPPUNIT_ASSERT_EQUAL(SdpCodec::getFmtpParameter(fmtp, "mode", codecMode), FALSE);
        CPPUNIT_ASSERT_EQUAL(codecMode, -1);

        CPPUNIT_ASSERT_EQUAL(sloppyPtimeSdpBody.getPayloadFormat(2, 100, fmtp), TRUE);
        CPPUNIT_ASSERT_EQUAL(fmtp, "size:QCIF/SQCIF");
        CPPUNIT_ASSERT_EQUAL(SdpCodec::getFmtpParameter(fmtp, "mode", codecMode), FALSE);
        CPPUNIT_ASSERT_EQUAL(codecMode, -1);

        int numVideoSizes;
        int videoSizes[10];
        SdpCodec::getVideoSizes(fmtp, 10, numVideoSizes, videoSizes);
        CPPUNIT_ASSERT_EQUAL(numVideoSizes, 2);

        SdpCodecList sdpFactory;
        SdpCodec* pPcmuCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMU, 
                                            SdpCodec::SDP_CODEC_UNKNOWN, 
                                            MIME_TYPE_AUDIO, 
                                            MIME_SUBTYPE_PCMU,
                                            8000, 
                                            20000); // ptime
        sdpFactory.addCodec(*pPcmuCodec);

        const SdpCodec* matchCodec = sdpFactory.getCodec(MIME_TYPE_AUDIO, "pcmu", 8000, 1, "");
        CPPUNIT_ASSERT(matchCodec);

        SdpCodec* pPcmaCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMA, 
                                            SdpCodec::SDP_CODEC_UNKNOWN, 
                                            MIME_TYPE_AUDIO, 
                                            MIME_SUBTYPE_PCMA,
                                            8000, 
                                            20000); // ptime
        sdpFactory.addCodec(*pPcmaCodec);

        matchCodec = sdpFactory.getCodec(MIME_TYPE_AUDIO, "pcma", 8000, 1, "");
        CPPUNIT_ASSERT(matchCodec);

        SdpCodec* pSuperCodec = new SdpCodec((SdpCodec::SdpCodecTypes)333, 
                                            SdpCodec::SDP_CODEC_UNKNOWN, 
                                            MIME_TYPE_AUDIO, 
                                            "superaudio",
                                            8000, 
                                            20000); // ptime
        sdpFactory.addCodec(*pSuperCodec);

        matchCodec = sdpFactory.getCodec(MIME_TYPE_AUDIO, "superaudio", 8000, 1, "");
        CPPUNIT_ASSERT(matchCodec);

        SdpCodec* pSuperDuperCodec = new SdpCodec((SdpCodec::SdpCodecTypes)334, 
                                            SdpCodec::SDP_CODEC_UNKNOWN, 
                                            MIME_TYPE_AUDIO, 
                                            "superduperaudio",
                                            8000, 
                                            20000); // ptime
        sdpFactory.addCodec(*pSuperDuperCodec);

        matchCodec = sdpFactory.getCodec(MIME_TYPE_AUDIO, "superduperaudio", 8000, 1, "");
        CPPUNIT_ASSERT(matchCodec);

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

        SdpCodec* codecArrayForEncoder[5];
        SdpCodec* codecArrayForDecoder[5];
        int numCodecsInCommon = 0;
        int videoRtpPort = -1;
        // This is insanely stupid.  I need to get the payload types from
        // the body so that I can get the codecs which contain the payloads
        int audioPayloads[4] = {96, 97, 98, 99};
        int videoPayloads[1] = {100};
        int audioMediaIndex = 0;
        int videoMediaIndex = 2;
        sloppyPtimeSdpBody.getCodecsInCommon(audioMediaIndex, videoMediaIndex, 4, 1, audioPayloads, videoPayloads,
            videoRtpPort, sdpFactory, numCodecsInCommon, codecArrayForEncoder,
            codecArrayForDecoder);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Known issue with getCodecsInCommon broken for multiple audio medialines", 5, numCodecsInCommon);

        int codecIndex;
        int encoderPayloadId;
        int decoderPayloadId;
        for(codecIndex = 0; codecIndex < numCodecsInCommon; codecIndex++)
        {
            encoderPayloadId = codecArrayForEncoder[codecIndex]->getCodecPayloadFormat();
            decoderPayloadId = codecArrayForDecoder[codecIndex]->getCodecPayloadFormat();

            // We are providing the answer in the SDP offer/answer case.  So we now do the
            // friendly thing of using the payload ID of the remote side.
            UtlString assertMsg;
            assertMsg.appendFormat("codecIndex = %d", codecIndex);
            printf("codec[%d] encoderPayloadId: %d decoderPayloadId: %d\n",
                   codecIndex, encoderPayloadId, decoderPayloadId);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(assertMsg.data(), encoderPayloadId, decoderPayloadId);
            if(encoderPayloadId != decoderPayloadId)
            {
                UtlString codec1Dump;
                UtlString codec2Dump;
                codecArrayForEncoder[codecIndex]->toString(codec1Dump);
                codecArrayForDecoder[codecIndex]->toString(codec2Dump);
                printf("Encoder codec:\n%s\nDecoder codec:\n%s\n",
                        codec1Dump.data(), codec2Dump.data());
            }

            switch(encoderPayloadId)
            {
            case 98:
            case 99:
                // Because of the broken nature of how media sets are treated
                // in the SdpBody (codecs are considered global accross all
                // media sets), the first ptime in the first media set becomes
                // global accross all media sets
                //CPPUNIT_ASSERT_EQUAL(codecArray[codecIndex]->getPacketLength(),
                //                     22000);
                //break;

            case 96:
            case 97:
                CPPUNIT_ASSERT_EQUAL(11000,
                                     codecArrayForEncoder[codecIndex]->getPacketLength());
                CPPUNIT_ASSERT_EQUAL(11000,
                                     codecArrayForDecoder[codecIndex]->getPacketLength());
                break;

            case 100:
                // currently video ptime is ignored in SdpBody
                // Should be 33000
                CPPUNIT_ASSERT_EQUAL(20000, 
                                     codecArrayForEncoder[codecIndex]->getPacketLength());
                CPPUNIT_ASSERT_EQUAL(20000, 
                                     codecArrayForDecoder[codecIndex]->getPacketLength());
                break;

            default:
                CPPUNIT_ASSERT_EQUAL(-2, encoderPayloadId);
                break;
            }

        }
     }


     void testGetCodecsInCommon()
     {
        const char* sloppyPtimeBodyString =
           "v=0\r\n"
           "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
           "s=foo\r\n"
           "c=IN IP4 10.1.1.30\r\n"
           "t=0 0\r\n"
           "m=audio 8700 RTP/AVP 96 97\r\n"
           "a=rtpmap:96 pcmu/8000/1\r\n"
           "a=rtpmap:97 pcma/8000/1\r\n"
           "m=audio 8710 RTP/AVP 98 99\r\n"
           "a=rtpmap:98 superaudio/8000/1\r\n"
           "a=rtpmap:99 superduperaudio/8000/1\r\n"
           "m=video 8801 RTP/AVP 100\r\n"
           "a=rtpmap:100 vp71/9000/1\r\n"
           "a=fmtp:100 size:QCIF/SQCIF\r\n"
           "c=IN IP4 10.1.1.31\r\n";
        SdpBody sloppyPtimeSdpBody(sloppyPtimeBodyString);

        SdpCodecList sdpFactory;
        SdpCodec* pPcmuCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMU, 
           SdpCodec::SDP_CODEC_PCMU, 
           MIME_TYPE_AUDIO, 
           MIME_SUBTYPE_PCMU,
           8000, 
           20000); // ptime
        sdpFactory.addCodec(*pPcmuCodec);

        SdpCodec* pPcmaCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMA, 
           SdpCodec::SDP_CODEC_PCMA, 
           MIME_TYPE_AUDIO, 
           MIME_SUBTYPE_PCMA,
           8000, 
           20000); // ptime
        sdpFactory.addCodec(*pPcmaCodec);

        SdpCodec* pSuperCodec = new SdpCodec((SdpCodec::SdpCodecTypes)102, 
           (SdpCodec::SdpCodecTypes)102, 
           MIME_TYPE_AUDIO, 
           "superaudio",
           8000, 
           20000); // ptime
        sdpFactory.addCodec(*pSuperCodec);

        SdpCodec* pSuperDuperCodec = new SdpCodec((SdpCodec::SdpCodecTypes)103, 
           (SdpCodec::SdpCodecTypes)103, 
           MIME_TYPE_AUDIO, 
           "superduperaudio",
           8000, 
           20000); // ptime
        sdpFactory.addCodec(*pSuperDuperCodec);

        SdpCodec* pQvgaCodec = new SdpCodec(SdpCodec::SDP_CODEC_VP71_QVGA, 
           (SdpCodec::SdpCodecTypes)104, 
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

        SdpCodec* codecArrayForEncoder[5];
        SdpCodec* codecArrayForDecoder[5];
        int numCodecsInCommon = 0;
        int videoRtpPort = -1;
        // This is insanely stupid.  I need to get the payload types from
        // the body so that I can get the codecs which contain the payloads
        int audioPayloads[4] = {96, 97, 98, 99};
        int videoPayloads[1] = {100};
        int audioMediaIndex = 0;
        int videoMediaIndex = 2;
        sloppyPtimeSdpBody.getCodecsInCommon(audioMediaIndex, videoMediaIndex, 4, 1, audioPayloads, videoPayloads,
           videoRtpPort, sdpFactory, numCodecsInCommon, codecArrayForEncoder,
           codecArrayForDecoder);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Known issue with getCodecsInCommon broken for multiple audio medialines", 5, numCodecsInCommon);

        int codecIndex;
        int encoderPayloadId;
        int decoderPayloadId;
        for(codecIndex = 0; codecIndex < numCodecsInCommon; codecIndex++)
        {
           encoderPayloadId = codecArrayForEncoder[codecIndex]->getCodecPayloadFormat();
           decoderPayloadId = codecArrayForDecoder[codecIndex]->getCodecPayloadFormat();
           printf("codec[%d] encoderPayloadId: %d decoderPayloadId: %d \n",
                   codecIndex, encoderPayloadId, decoderPayloadId);
           // decoder codecs keep the payload Id of the factory

           switch(encoderPayloadId)
           {
           case 96:
              CPPUNIT_ASSERT_EQUAL(20000,
                 codecArrayForEncoder[codecIndex]->getPacketLength());
              CPPUNIT_ASSERT_EQUAL(20000,
                 codecArrayForDecoder[codecIndex]->getPacketLength());
              // We are providing the answer in the SDP offer/answer case.  So we now do the
              // friendly thing of using the payload ID of the remote side.
              //CPPUNIT_ASSERT_EQUAL((int)SdpCodec::SDP_CODEC_PCMU, decoderPayloadId);
              CPPUNIT_ASSERT_EQUAL(96, decoderPayloadId);
              break;
           case 97:
              CPPUNIT_ASSERT_EQUAL(20000,
                 codecArrayForEncoder[codecIndex]->getPacketLength());
              CPPUNIT_ASSERT_EQUAL(20000,
                 codecArrayForDecoder[codecIndex]->getPacketLength());
              // We are providing the answer in the SDP offer/answer case.  So we now do the
              // friendly thing of using the payload ID of the remote side.
              //CPPUNIT_ASSERT_EQUAL((int)SdpCodec::SDP_CODEC_PCMA, decoderPayloadId);
              CPPUNIT_ASSERT_EQUAL(97, decoderPayloadId);
              break;
           case 98:
              CPPUNIT_ASSERT_EQUAL(20000,
                 codecArrayForEncoder[codecIndex]->getPacketLength());
              CPPUNIT_ASSERT_EQUAL(20000,
                 codecArrayForDecoder[codecIndex]->getPacketLength());
              // We are providing the answer in the SDP offer/answer case.  So we now do the
              // friendly thing of using the payload ID of the remote side.
              //CPPUNIT_ASSERT_EQUAL(102, decoderPayloadId);
              CPPUNIT_ASSERT_EQUAL(98, decoderPayloadId);
              break;
           case 99:
              CPPUNIT_ASSERT_EQUAL(20000,
                 codecArrayForEncoder[codecIndex]->getPacketLength());
              CPPUNIT_ASSERT_EQUAL(20000,
                 codecArrayForDecoder[codecIndex]->getPacketLength());
              // We are providing the answer in the SDP offer/answer case.  So we now do the
              // friendly thing of using the payload ID of the remote side.
              //CPPUNIT_ASSERT_EQUAL(103, decoderPayloadId);
              CPPUNIT_ASSERT_EQUAL(99, decoderPayloadId);
              break;

           case 100:
              // currently video ptime is ignored in SdpBody
              // Should be 33000
              CPPUNIT_ASSERT_EQUAL(20000, 
                 codecArrayForEncoder[codecIndex]->getPacketLength());
              CPPUNIT_ASSERT_EQUAL(20000, 
                 codecArrayForDecoder[codecIndex]->getPacketLength());
              CPPUNIT_ASSERT_EQUAL(104, decoderPayloadId);
              break;

           default:
              CPPUNIT_ASSERT_EQUAL(-2, encoderPayloadId);
              break;
           }

        }
    }

    void testDirectionAttribute()
    {
        const char* videoSdpOfferBytes = 
            "v=0\r\n"
            "o=polycomdme 1493339645 0 IN IP4 172.22.2.119\r\n"
            "s=-\r\n"
            "c=IN IP4 172.22.2.119\r\n"
            "b=AS:512\r\n"
            "t=0 0\r\n"
            "m=audio 49160 RTP/AVP 115 102 9 15 0 8 18 101\r\n"
            "a=rtpmap:115 G7221/32000\r\n"
            "a=fmtp:115 bitrate=48000\r\n"
            "a=rtpmap:102 G7221/16000\r\n"
            "a=fmtp:102 bitrate=32000\r\n"
            "a=rtpmap:9 G722/8000\r\n"
            "a=rtpmap:15 G728/8000\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:8 PCMA/8000\r\n"
            "a=rtpmap:18 G729/8000\r\n"
            "a=fmtp:18 annexb=no\r\n"
            "a=rtpmap:101 telephone-event/8000\r\n"
            "a=fmtp:101 0-15\r\n"
            "a=sendrecv\r\n"
            "m=audio 49160 RTP/AVP 9\r\n"
            "a=rtpmap:9 G722/8000\r\n"
            "a=inactive\r\n"
            "m=video 49162 RTP/AVP 109 110 111 112 96 34 31\r\n"
            "b=TIAS:512000\r\n"
            "a=rtpmap:109 H264/90000\r\n"
            "a=fmtp:109 profile-level-id=428016; max-mbps=216000; max-fs=3600; max-br=5120; sar=13\r\n"
            "a=rtpmap:110 H264/90000\r\n"
            "a=fmtp:110 profile-level-id=42E016; packetization-mode=1; max-mbps=216000; max-fs=3600; max-br=5120; sar=13\r\n"
            "a=rtpmap:111 H264/90000\r\n"
            "a=fmtp:111 profile-level-id=640016; packetization-mode=1; max-mbps=216000; max-fs=3600; max-br=5120; sar=13\r\n"
            "a=rtpmap:112 H264/90000\r\n"
            "a=fmtp:112 profile-level-id=44E016; packetization-mode=1; max-mbps=216000; max-fs=3600; max-br=5120; sar=13\r\n"
            "a=rtpmap:96 H263-1998/90000\r\n"
            "a=fmtp:96 CIF4=2;CIF=1;QCIF=1;SQCIF=1;CUSTOM=352,240,1;CUSTOM=704,480,2;J;T\r\n"
            "a=rtpmap:34 H263/90000\r\n"
            "a=fmtp:34 CIF4=2;CIF=1;QCIF=1;SQCIF=1\r\n"
            "a=rtpmap:31 H261/90000\r\n"
            "a=fmtp:31 CIF=1;QCIF=1\r\n"
            "a=rtcp-fb:* ccm fir tmmbr\r\n"
            "a=sendonly\r\n"
            "m=application 49164 RTP/AVP 100\r\n"
            "a=rtpmap:100 H224/4800\r\n"
            "a=sendrecv\r\n"
            "m=foo 12345 RTP/AVP 111\r\n"
            "a=recvonly\r\n"
            "m=foo 54321 RTP/AVP 111\r\n";

        SdpBody body(videoSdpOfferBytes);

        SdpBody::SessionDirection direction;
        body.getMediaStreamDirection(0, direction);
        CPPUNIT_ASSERT_EQUAL(SdpBody::SendRecv, direction);

        body.getMediaStreamDirection(1, direction);
        CPPUNIT_ASSERT_EQUAL(SdpBody::Inactive, direction);

        body.getMediaStreamDirection(2, direction);
        CPPUNIT_ASSERT_EQUAL(SdpBody::SendOnly, direction);

        body.getMediaStreamDirection(3, direction);
        CPPUNIT_ASSERT_EQUAL(SdpBody::SendRecv, direction);

        body.getMediaStreamDirection(4, direction);
        CPPUNIT_ASSERT_EQUAL(SdpBody::RecvOnly, direction);

        body.getMediaStreamDirection(5, direction);
        CPPUNIT_ASSERT_EQUAL(SdpBody::Unknown, direction);
    }

    void testUseRemotePayloadIds()
    {
        const char* sdpBytes =
            "v=0\r\n"
            "o=- 1335371328 1 IN IP4 22.88.66.11\r\n"
            "s=-\r\n"
            "c=IN IP4 22.88.66.11\r\n"
            "b=CT:1000\r\n"
            "t=0 0\r\n"
            "m=audio 60802 RTP/AVP 0 18 101\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:18 G729/8000\r\n"
            "a=fmtp:18 annexb=no\r\n"
            "a=rtpmap:101 telephone-event/8000\r\n"
            "m=video 60804 RTP/AVP 96 97 34\r\n"
            "b=TIAS:910000\r\n"
            "a=rtpmap:96 H264/90000\r\n"
            "a=fmtp:96 profile-level-id=42801f;max-mbps=108000;max-fs=3840\r\n"
            "a=rtpmap:97 H263-1998/90000\r\n"
            "a=fmtp:97 CIF4=1;CIF=1;QCIF=1\r\n"
            "a=rtpmap:34 H263/90000\r\n"
            "a=fmtp:34 CIF4=1;CIF=1;QCIF=1\r\n";
        SdpBody sdpOffer(sdpBytes);

        // Setup some codecs
        SdpCodecList codecList;
        codecList.addCodecs("G722 AAC_LC_32000 PCMU telephone-event");
        // TODO: test if video codecs get filtered out
        //codecList.addCodecs("G722 AAC_LC_32000 PCMU H264_CIF_256 H264_PM1_CIF_256 telephone-event");
        codecList.bindPayloadTypes();

        // Test if we read the mediaLine out correctly
        SdpMediaLine testMediaLine;
        CPPUNIT_ASSERT_EQUAL(sdpOffer.getMediaLine(0, testMediaLine, &codecList), TRUE);
        SdpCodecList* mediaLineCodecList = (SdpCodecList*)testMediaLine.getCodecs();
        CPPUNIT_ASSERT_EQUAL(mediaLineCodecList->getCodecCount(), 2);
        const SdpCodec* mediaLineCodec = 
            mediaLineCodecList->getCodec(SDP_AUDIO_MEDIA_TYPE, MIME_SUBTYPE_DTMF_TONES, 8000, 1, "");
        CPPUNIT_ASSERT(mediaLineCodec);
        CPPUNIT_ASSERT_EQUAL(mediaLineCodec->getCodecPayloadFormat(), 101);
        SdpMediaLine::SdpConnection* connection = 
            (SdpMediaLine::SdpConnection*)testMediaLine.getConnections().first();
        CPPUNIT_ASSERT(connection);
        CPPUNIT_ASSERT_EQUAL(connection->getAddress(), "22.88.66.11");
        CPPUNIT_ASSERT_EQUAL(connection->getPort(), 60802);

        // Build local mediaLine
        const char* localHost = "44.33.22.11";
        int localPort = 4444;
        SdpMediaLine localMediaLine;
        localMediaLine.setMediaType(SdpMediaLine::MEDIA_TYPE_AUDIO);
        localMediaLine.setTransportProtocolType(SdpMediaLine::PROTOCOL_TYPE_RTP_AVP);
        localMediaLine.addConnection(Sdp::NET_TYPE_IN, Sdp::ADDRESS_TYPE_IP4, localHost, localPort);
        localMediaLine.addRtcpConnection(Sdp::NET_TYPE_IN, Sdp::ADDRESS_TYPE_IP4, localHost, localPort + 1);
        localMediaLine.setCodecs(codecList);


        SdpMediaLine remoteMediaLine;
        SdpCodecList localDecodeCodecs;
        // Match remote mediaLine and extract SdpBody data into mediaLine container
        sdpOffer.getCodecsInCommon(localMediaLine,
                                   0, // index of remote mediaLine
                                   remoteMediaLine,
                                   localDecodeCodecs);

        SdpCodecList* remoteCodecs = (SdpCodecList*) remoteMediaLine.getCodecs();
        CPPUNIT_ASSERT_EQUAL(remoteCodecs->getCodecCount(), 2);
        CPPUNIT_ASSERT_EQUAL(localDecodeCodecs.getCodecCount(), 4);

        const SdpCodec* encodeTonesCodec =
            remoteCodecs->getCodec(SDP_AUDIO_MEDIA_TYPE, MIME_SUBTYPE_DTMF_TONES, 8000, 1, "");
        CPPUNIT_ASSERT(encodeTonesCodec);
        CPPUNIT_ASSERT_EQUAL(encodeTonesCodec->getCodecPayloadFormat(), 101);
        const SdpCodec* decodeTonesCodec = 
            localDecodeCodecs.getCodec(SDP_AUDIO_MEDIA_TYPE, MIME_SUBTYPE_DTMF_TONES, 8000, 1, "");
        CPPUNIT_ASSERT(decodeTonesCodec);
        CPPUNIT_ASSERT_EQUAL(decodeTonesCodec->getCodecPayloadFormat(), 101);
    }

    void testVideoInCommon1()
    {
        // Polycom Group Series 500
        const char* sdpBytes =
            "v=0\r\n"
            "o=GroupSeries 88424642 0 IN IP4 132.177.252.157\r\n"
            "s=MRD=MRE MRC-V=1.0.0\r\n"
            "c=IN IP4 132.177.252.157\r\n"
            "b=AS:1024\r\n"
            "t=0 0\r\n"
            "m=audio 49242 RTP/AVP 115 102 9 15 0 8 18 101\r\n"
            "a=rtpmap:115 G7221/32000\r\n"
            "a=fmtp:115 bitrate=48000\r\n"
            "a=rtpmap:102 G7221/16000\r\n"
            "a=fmtp:102 bitrate=32000\r\n"
            "a=rtpmap:9 G722/8000\r\n"
            "a=rtpmap:15 G728/8000\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:8 PCMA/8000\r\n"
            "a=rtpmap:18 G729/8000\r\n"
            "a=fmtp:18 annexb=no\r\n"
            "a=rtpmap:101 telephone-event/8000\r\n"
            "a=fmtp:101 0-15\r\n"
            "a=sendrecv\r\n"
            "m=video 49244 RTP/AVP 116 109 110 111 96 34 31\r\n"
            "b=TIAS:1024000\r\n"
            "a=rtpmap:116 vnd.polycom.lpr/9000\r\n"
            "a=fmtp:116 V=1;minPP=0;PP=150;RS=52;RP=10;PS=1400\r\n"
            "a=rtpmap:109 H264/90000\r\n"
            "a=fmtp:109 profile-level-id=428020; max-mbps=245000; max-fs=8192; sar-supported=13; sar=13\r\n"
            "a=rtpmap:110 H264/90000\r\n"
            "a=fmtp:110 profile-level-id=428020; packetization-mode=1; max-mbps=245000; max-fs=8192; sar-supported=13; sar=13\r\n"
            "a=rtpmap:111 H264/90000\r\n"
            "a=fmtp:111 profile-level-id=640020; packetization-mode=1;\r\n"
            "max-mbps=245000; max-fs=8192; sar-supported=13; sar=13\r\n"
            "a=rtpmap:96 H263-1998/90000 \r\n"
            "a=fmtp:96 CIF4=1;CIF=1;QCIF=1;SQCIF=1;CUSTOM=352,240,1;CUST\r\n"
            "OM=704,480,1;CUSTOM=848,480,1;CUSTOM=640,368,1;CUSTOM=432,240,1\r\n"
            "a=rtpmap:34 H263/90000\r\n"
            "a=fmtp:34 CIF4=1;CIF=1;QCIF=1;SQCIF=1\r\n"
            "a=rtpmap:31 H26\r\n"
            "1/90000\r\n"
            "a=fmtp:31 CIF=1;QCIF=1\r\n"
            "a=sendrecv\r\n"
            "a=rtcp-fb:* ccm fir\r\n"
            "m=application 49248 RTP/AVP 100\r\n"
            "a=rtpmap:100 H224/4800\r\n"
            "a=sendrecv\r\n"
            "m=application 38660 UDP/BFCP *\r\n"
            "a=floorctrl:c-s\r\n"
            "a=setup:actpass\r\n"
            "a=connection:new\r\n";

        SdpBody sdpOffer(sdpBytes);

        // Setup some codecs
        SdpCodecList codecList;
        codecList.addCodecs("G722 AAC_LC_32000 PCMU telephone-event H264_HD1088_4096");
        CPPUNIT_ASSERT_EQUAL(codecList.getCodecCount(), 5);
        // TODO: test if video codecs get filtered out
        //codecList.addCodecs("G722 AAC_LC_32000 PCMU H264_CIF_256 H264_PM1_CIF_256 telephone-event");
        codecList.bindPayloadTypes();
        int numRtpCodecs = 0;
        SdpCodec** localRtpCodecs = NULL;
        codecList.getCodecs(numRtpCodecs, localRtpCodecs);
        CPPUNIT_ASSERT_EQUAL(numRtpCodecs, 5);

        // Test if we read the mediaLine out correctly
        SdpMediaLine testVideoMediaLine;
        CPPUNIT_ASSERT_EQUAL(sdpOffer.getMediaLine(1, testVideoMediaLine, &codecList), TRUE);
        CPPUNIT_ASSERT_EQUAL(testVideoMediaLine.getMediaType(), SdpMediaLine::MEDIA_TYPE_VIDEO);
        const SdpCodecList* mediaLineCodecList = testVideoMediaLine.getCodecs();
        CPPUNIT_ASSERT_EQUAL(mediaLineCodecList->getCodecCount(), 1);
        int numVideoCodecs = 0;
        SdpCodec** videoCodecArray = NULL;
        mediaLineCodecList->getCodecs(numVideoCodecs, videoCodecArray, SDP_VIDEO_MEDIA_TYPE, MIME_SUBTYPE_H264);
        CPPUNIT_ASSERT(videoCodecArray);
        CPPUNIT_ASSERT_EQUAL(numVideoCodecs, 1);

        // Build local mediaLine
        UtlString localHost("44.33.22.11");
        int localRtpPort = 4444;
        int localRtcpPort = localRtpPort + 1;
        SdpMediaLine localMediaLine;
        SdpBody::buildMediaLine(SdpMediaLine::MEDIA_TYPE_VIDEO,
                               0,
                               1,
                               &localHost,
                               &localRtpPort,
                               &localRtcpPort,
                               numRtpCodecs,
                               localRtpCodecs,
                               localMediaLine);

        printf("testVideo line: %d\n", __LINE__);

        SdpMediaLine remoteMediaLine;
        SdpCodecList localDecodeCodecs;
        // Match remote mediaLine and extract SdpBody data into mediaLine container
        sdpOffer.getCodecsInCommon(localMediaLine,
                                   1, // index of remote mediaLine in sdpOffer
                                   remoteMediaLine,
                                   localDecodeCodecs);

        const SdpCodecList* remoteCodecs = remoteMediaLine.getCodecs();
        CPPUNIT_ASSERT_EQUAL(remoteCodecs->getCodecCount(), 1);
        CPPUNIT_ASSERT_EQUAL(localDecodeCodecs.getCodecCount(), 1);

        int numEncodeH264Codecs = 0;
        SdpCodec** encodeH264CodecArray = NULL;
        remoteCodecs->getCodecs(numEncodeH264Codecs, encodeH264CodecArray, SDP_VIDEO_MEDIA_TYPE, MIME_SUBTYPE_H264);
        CPPUNIT_ASSERT_EQUAL(numEncodeH264Codecs, 1);
        CPPUNIT_ASSERT(encodeH264CodecArray);
        CPPUNIT_ASSERT_EQUAL(encodeH264CodecArray[0]->getCodecPayloadFormat(), 109);
    }

    void testVideoInCommon2()
    {
        // Teluu/Pjsip
        const char* sdpBytes =
            "v=0\r\n"
            "o=- 3570486035 3570486035 IN IP4 132.177.252.51\r\n"
            "s=pjmedia\r\n"
            "b=AS:352\r\n"
            "t=0 0\r\n"
            "a=X-nat:0\r\n"
            "m=audio 40000 RTP/AVP 98 97 99 104 3 0 8 9 105 106 18 4 110 2 15 100 102 96\r\n"
            "c=IN IP4 132.177.252.51\r\n"
            "b=TIAS:64000\r\n"
            "a=rtcp:40001 IN IP4 132.177.252.51\r\n"
            "a=sendrecv\r\n"
            "a=rtpmap:98 speex/16000\r\n"
            "a=rtpmap:97 speex/8000\r\n"
            "a=rtpmap:99 speex/32000\r\n"
            "a=rtpmap:104 iLBC/8000\r\n"
            "a=fmtp:104 mode=30\r\n"
            "a=rtpmap:3 GSM/8000\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:8 PCMA/8000\r\n"
            "a=rtpmap:9 G722/8000\r\n"
            "a=rtpmap:105 AMR/8000\r\n"
            "a=fmtp:105 octet-align=1\r\n"
            "a=rtpmap:106 AMR-WB/16000\r\n"
            "a=fmtp:106 octet-align=1\r\n"
            "a=rtpmap:18 G729/8000\r\n"
            "a=rtpmap:4 G723/8000\r\n"
            "a=rtpmap:110 G726-32/8000\r\n"
            "a=rtpmap:2 G721/8000\r\n"
            "a=rtpmap:15 G728/8000\r\n"
            "a=rtpmap:100 SILK/8000\r\n"
            "a=fmtp:100 useinbandfec=0\r\n"
            "a=rtpmap:102 SILK/16000\r\n"
            "a=fmtp:102 useinbandfec=0\r\n"
            "a=rtpmap:96 telephone-event/8000\r\n"
            "a=fmtp:96 0-15\r\n"
            "m=video 40002 RTP/AVP 97 96\r\n"
            "c=IN IP4 132.177.252.51\r\n"
            "b=TIAS:256000\r\n"
            "a=rtcp:40003 IN IP4 132.177.252.51\r\n"
            "a=sendrecv\r\n"
            "a=rtpmap:97 H264/90000\r\n"
            "a=fmtp:97 profile-level-id=42e01e; packetization-mode=1\r\n"
            "a=rtpmap:96 H263-1998/90000\r\n"
            "a=fmtp:96 CIF=1;QCIF=1\r\n";

        SdpBody sdpOffer(sdpBytes);

        // Setup some codecs
        SdpCodecList codecList;
        codecList.addCodecs("G722 AAC_LC_32000 PCMU telephone-event H264_PM1_HD1088_4096");
        CPPUNIT_ASSERT_EQUAL(codecList.getCodecCount(), 5);
        // TODO: test if video codecs get filtered out
        //codecList.addCodecs("G722 AAC_LC_32000 PCMU H264_CIF_256 H264_PM1_CIF_256 telephone-event");
        codecList.bindPayloadTypes();
        int numRtpCodecs = 0;
        SdpCodec** localRtpCodecs = NULL;
        codecList.getCodecs(numRtpCodecs, localRtpCodecs);
        CPPUNIT_ASSERT_EQUAL(numRtpCodecs, 5);

        // Test if we read the mediaLine out correctly
        SdpMediaLine testVideoMediaLine;
        CPPUNIT_ASSERT_EQUAL(sdpOffer.getMediaLine(1, testVideoMediaLine, NULL), TRUE);
        CPPUNIT_ASSERT_EQUAL(testVideoMediaLine.getMediaType(), SdpMediaLine::MEDIA_TYPE_VIDEO);
        const SdpCodecList* mediaLineCodecList = testVideoMediaLine.getCodecs();
        CPPUNIT_ASSERT_EQUAL(mediaLineCodecList->getCodecCount(), 2);
        int numVideoCodecs = 0;
        SdpCodec** videoCodecArray = NULL;
        mediaLineCodecList->getCodecs(numVideoCodecs, videoCodecArray);
        CPPUNIT_ASSERT_EQUAL(numVideoCodecs, 2);
        UtlString mimeSubtype;
        UtlString mimeType;
        videoCodecArray[0]->getEncodingName(mimeSubtype);
        CPPUNIT_ASSERT_EQUAL(mimeSubtype, MIME_SUBTYPE_H264);
        videoCodecArray[0]->getMediaType(mimeType);
        CPPUNIT_ASSERT_EQUAL(mimeType, SDP_VIDEO_MEDIA_TYPE);
        videoCodecArray[1]->getEncodingName(mimeSubtype);
        CPPUNIT_ASSERT_EQUAL(mimeSubtype, MIME_SUBTYPE_H263_1998);
        videoCodecArray[1]->getMediaType(mimeType);
        CPPUNIT_ASSERT_EQUAL(mimeType, SDP_VIDEO_MEDIA_TYPE);

        mediaLineCodecList->getCodecs(numVideoCodecs, videoCodecArray, SDP_VIDEO_MEDIA_TYPE, MIME_SUBTYPE_H264);
        CPPUNIT_ASSERT(videoCodecArray);
        CPPUNIT_ASSERT_EQUAL(numVideoCodecs, 1);

        // Build local mediaLine
        UtlString localHost("44.33.22.11");
        int localRtpPort = 4444;
        int localRtcpPort = localRtpPort + 1;
        SdpMediaLine localMediaLine;
        SdpBody::buildMediaLine(SdpMediaLine::MEDIA_TYPE_VIDEO,
                               0,
                               1,
                               &localHost,
                               &localRtpPort,
                               &localRtcpPort,
                               numRtpCodecs,
                               localRtpCodecs,
                               localMediaLine);

        printf("testVideo line: %d\n", __LINE__);

        SdpMediaLine remoteMediaLine;
        SdpCodecList localDecodeCodecs;
        // Match remote mediaLine and extract SdpBody data into mediaLine container
        sdpOffer.getCodecsInCommon(localMediaLine,
                                   1, // index of remote mediaLine in sdpOffer
                                   remoteMediaLine,
                                   localDecodeCodecs);

        const SdpCodecList* remoteCodecs = remoteMediaLine.getCodecs();
        CPPUNIT_ASSERT_EQUAL(remoteCodecs->getCodecCount(), 1);
        CPPUNIT_ASSERT_EQUAL(localDecodeCodecs.getCodecCount(), 1);

        int numEncodeH264Codecs = 0;
        SdpCodec** encodeH264CodecArray = NULL;
        remoteCodecs->getCodecs(numEncodeH264Codecs, encodeH264CodecArray, SDP_VIDEO_MEDIA_TYPE, MIME_SUBTYPE_H264);
        CPPUNIT_ASSERT_EQUAL(numEncodeH264Codecs, 1);
        CPPUNIT_ASSERT(encodeH264CodecArray);
        CPPUNIT_ASSERT_EQUAL(encodeH264CodecArray[0]->getCodecPayloadFormat(), 97);
    }

    void testMlineOrder()
    {
        const char* multiMediaLineSdpString =
            "v=0\r\n"
            "o=- 1335371328 1 IN IP4 205.168.62.28\r\n"
            "s=-\r\n"
            "c=IN IP4 205.168.62.106\r\n"
            "b=CT:1000\r\n"
            "t=0 0\r\n"
            "m=audio 60802 RTP/AVP 0 18 101\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:18 G729/8000\r\n"
            "a=fmtp:18 annexb=no\r\n"
            "a=rtpmap:101 telephone-event/8000\r\n"
            "m=video 60804 RTP/AVP 96 97 34\r\n"
            "b=TIAS:910000\r\n"
            "a=rtpmap:96 H264/90000\r\n"
            "a=fmtp:96 profile-level-id=42801f;max-mbps=108000;max-fs=3840\r\n"
            "a=rtpmap:97 H263-1998/90000\r\n"
            "a=fmtp:97 CIF4=1;CIF=1;QCIF=1\r\n"
            "a=rtpmap:34 H263/90000\r\n"
            "a=fmtp:34 CIF4=1;CIF=1;QCIF=1\r\n"
            "a=content:main\r\n"
            "a=rtcp-fb:* ccm fir\r\n"
            "a=rtcp-fb:* nack pli\r\n"
            "a=rtcp-fb:* ccm tmmbr\r\n"
            "m=application 60806 RTP/AVP 100\r\n"
            "a=rtpmap:100 H224/4800\r\n"
            "m=control 60808 RTP/AVP 96\r\n"
            "a=inactive\r\n"
            "a=rtpmap:96 H264/90000\r\n"
            "a=fmtp:96 profile-level-id=42801f;max-mbps=108000;max-fs=3840\r\n"
            "a=rtcp-fb:* ccm fir\r\n"
            "a=rtcp-fb:* nack pli\r\n"
            "a=rtcp-fb:* ccm tmmbr\r\n"
            "m=application 60812 TCP/BFCP *\r\n"
            "a=connection:new\r\n"
            "a=setup:active\r\n"
            "a=floorctrl:c-only\r\n";

        printf("Offer:\n%s", multiMediaLineSdpString);

        SdpBody sdpOffer(multiMediaLineSdpString);
        SdpBody sdpAnswer;
        UtlString hostAddress("12.34.56.78");
        int rtpAudioPort = 44066;
        int rtcpAudioPort = 44067;
        int rtpVideoPort = 44068;
        int rtcpVideoPort = 44071;
        RTP_TRANSPORT transportType = RTP_TRANSPORT_UDP;
        int numCodecs = 0;
        SdpCodec** codecArray = NULL;
        SdpSrtpParameters srtpParameters;
        memset(&srtpParameters, 0, sizeof(SdpSrtpParameters));
        int videoBandwidth = 0;
        int videoFramerate = 0;

        // Setup some codecs
        SdpCodecList codecList;
        codecList.addCodecs("G722 AAC_LC_32000 PCMU H264_CIF_256 H264_PM1_CIF_256 telephone-event");
        codecList.bindPayloadTypes();
        codecList.getCodecs(numCodecs, codecArray);

        sdpAnswer.setStandardHeaderFields("call", NULL, NULL, hostAddress);

        sdpAnswer.addCodecsAnswer(1,
                                  &hostAddress,
                                  &rtpAudioPort,
                                  &rtcpAudioPort,
                                  &rtpVideoPort,
                                  &rtcpVideoPort,
                                  &transportType,
                                  numCodecs,
                                  codecArray,
                                  srtpParameters,
                                  videoBandwidth,
                                  videoFramerate,
                                  &sdpOffer);

        UtlString answerString;
        int answerLength;
        sdpAnswer.getBytes(&answerString, &answerLength);
        printf("Answer:\n%s", answerString.data());

        CPPUNIT_ASSERT_EQUAL(5, sdpAnswer.getMediaSetCount());
        if(sdpAnswer.getMediaSetCount() != 5)
        {
            UtlString sdpBytes;
            int sdpBytesLength;
            sdpOffer.getBytes(&sdpBytes, &sdpBytesLength);
            printf("Offer:\n%s\n", sdpBytes.data());
            sdpAnswer.getBytes(&sdpBytes, &sdpBytesLength);
            printf("Answer:\n%s\n", sdpBytes.data());
        }

        int mediaPort;
        int rtcpPort;
        UtlString mediaType;
        int numMediaPortPairs;
        UtlString answerTransportType;
        const int maxPayloadTypes = 20;
        int numPayloads;
        int payloadIds[maxPayloadTypes];
        UtlString mimeSubtype;
        int sampleRate;
        int numChannels;

        sdpAnswer.getMediaData(0, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds);

        CPPUNIT_ASSERT_EQUAL("audio", mediaType);
        CPPUNIT_ASSERT_EQUAL(answerTransportType, "RTP/AVP");
        CPPUNIT_ASSERT_EQUAL(mediaPort,  rtpAudioPort);
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaRtcpPort(0, &rtcpPort), TRUE);
        CPPUNIT_ASSERT_EQUAL(rtcpPort, rtcpAudioPort);
        // Tones should assume the payload ID of the offer: 101
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getPayloadRtpMap(0, 101, mimeSubtype, sampleRate, numChannels), TRUE);
        mimeSubtype.SDP_MIME_SUBTYPE_TO_CASE();
        UtlString refMimeSubtype(MIME_SUBTYPE_DTMF_TONES);
        refMimeSubtype.SDP_MIME_SUBTYPE_TO_CASE();
        CPPUNIT_ASSERT_EQUAL(mimeSubtype, refMimeSubtype);

        sdpAnswer.getMediaData(1, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds);

        CPPUNIT_ASSERT_EQUAL("video", mediaType);
        CPPUNIT_ASSERT_EQUAL(answerTransportType, "RTP/AVP");
        CPPUNIT_ASSERT_EQUAL(mediaPort,  rtpVideoPort);
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaRtcpPort(1, &rtcpPort), TRUE);
        CPPUNIT_ASSERT_EQUAL(rtcpPort, rtcpVideoPort);

        sdpAnswer.getMediaData(2, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds);

        CPPUNIT_ASSERT_EQUAL("application", mediaType);
        CPPUNIT_ASSERT_EQUAL(mediaPort, 0);

        sdpAnswer.getMediaData(3, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds);

        CPPUNIT_ASSERT_EQUAL("control", mediaType);
        CPPUNIT_ASSERT_EQUAL(mediaPort, 0);

        sdpAnswer.getMediaData(4, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds);

        CPPUNIT_ASSERT_EQUAL("application", mediaType);
        CPPUNIT_ASSERT_EQUAL(mediaPort, 0);

    }

    void testCryptoParser()
    {
        const char* sdpBodyChars =
            "v=0\r\n"
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

        SdpBody body(sdpBodyChars);

        SdpSrtpParameters srtpParameters;
        // Audio stream
        CPPUNIT_ASSERT_EQUAL(body.getSrtpCryptoField(0, 0, srtpParameters), FALSE);

        CPPUNIT_ASSERT_EQUAL(body.getSrtpCryptoField(0, 1, srtpParameters), TRUE);
        CPPUNIT_ASSERT_EQUAL(srtpParameters.cipherType, F8_128_HMAC_SHA1_80);
        CPPUNIT_ASSERT_EQUAL(srtpParameters.securityLevel, SRTP_ENCRYPTION | SRTP_AUTHENTICATION);
        CPPUNIT_ASSERT_EQUAL((const char*)srtpParameters.masterKey, "123456789ABCDE0123456789ABcdef");

        CPPUNIT_ASSERT_EQUAL(body.getSrtpCryptoField(0, 2, srtpParameters), FALSE);

        // Video stream
        CPPUNIT_ASSERT_EQUAL(body.getSrtpCryptoField(1, 0, srtpParameters), FALSE);

        CPPUNIT_ASSERT_EQUAL(body.getSrtpCryptoField(1, 1, srtpParameters), TRUE);
        CPPUNIT_ASSERT_EQUAL(srtpParameters.cipherType, AES_CM_128_HMAC_SHA1_80);
        CPPUNIT_ASSERT_EQUAL(srtpParameters.securityLevel, SRTP_AUTHENTICATION);
        CPPUNIT_ASSERT_EQUAL((const char*)srtpParameters.masterKey, "ABcdef123456789ABCDE0123456789");

        CPPUNIT_ASSERT_EQUAL(body.getSrtpCryptoField(1, 2, srtpParameters), TRUE);
        CPPUNIT_ASSERT_EQUAL(srtpParameters.cipherType, AES_CM_128_HMAC_SHA1_32);
        CPPUNIT_ASSERT_EQUAL(srtpParameters.securityLevel, SRTP_ENCRYPTION | SRTP_AUTHENTICATION);
        CPPUNIT_ASSERT_EQUAL((const char*)srtpParameters.masterKey, "ABcdef123456789ABCDE0123456789");

        CPPUNIT_ASSERT_EQUAL(body.getSrtpCryptoField(1, 3, srtpParameters), FALSE);

    }

    void test3Mlines()
    {
        const char* offerString =
            "v=0\r\n"
            "o=alphapolycom 1272125284 0 IN IP4 172.22.2.35\r\n"
            "s=-\r\n"
            "c=IN IP4 172.22.2.35\r\n"
            "b=AS:1024\r\n"
            "t=0 0\r\n"
            "m=audio 49430 RTP/AVP 115 102 9 15 0 8 18 119\r\n"
            "a=rtpmap:115 G7221/32000\r\n"
            "a=fmtp:115 bitrate=48000\r\n"
            "a=rtpmap:102 G7221/16000\r\n"
            "a=fmtp:102 bitrate=32000\r\n"
            "a=rtpmap:9 G722/8000\r\n"
            "a=rtpmap:15 G728/8000\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:8 PCMA/8000\r\n"
            "a=rtpmap:18 G729/8000\r\n"
            "a=fmtp:18 annexb=no\r\n"
            "a=rtpmap:119 telephone-event/8000\r\n"
            "a=fmtp:119 0-15\r\n"
            "a=sendrecv\r\n"
            "m=video 49432 RTP/AVP 109 110 111 96 34 31\r\n"
            "b=TIAS:1024000\r\n"
            "a=rtpmap:109 H264/90000\r\n"
            "a=fmtp:109 profile-level-id=428016; max-mbps=244800; max-fs=8160; max-br=5120; sar=13\r\n"
            "a=rtpmap:110 H264/90000\r\n"
            "a=fmtp:110 profile-level-id=428016; packetization-mode=1; max-mbps=244800; max-fs=8160; max-br=5120; sar=13\r\n"
            "a=rtpmap:111 H264/90000\r\n"
            "a=fmtp:111 profile-level-id=640016; packetization-mode=1; max-mbps=244800; max-fs=8160; max-br=5120; sar=13\r\n"
            "a=rtpmap:96 H263-1998/90000\r\n"
            "a=fmtp:96 CIF4=2;CIF=1;QCIF=1;SQCIF=1;CUSTOM=352,240,1;CUSTOM=704,480,2\r\n"
            "a=rtpmap:34 H263/90000\r\n"
            "a=fmtp:34 CIF4=2;CIF=1;QCIF=1;SQCIF=1\r\n"
            "a=rtpmap:31 H261/90000\r\n"
            "a=fmtp:31 CIF=1;QCIF=1\r\n"
            "a=sendrecv\r\n"
            "a=rtcp-fb:* ccm fir tmmbr\r\n"
            "m=application 49434 RTP/AVP 100\r\n"
            "a=rtpmap:100 H224/4800\r\n"
            "a=sendrecv\r\n";

        printf("Offer:\n%s", offerString);
        SdpBody sdpOffer(offerString);

        SdpBody sdpAnswer;
        UtlString hostAddress("12.34.56.78");
        int rtpAudioPort = 44066;
        int rtcpAudioPort = 44067;
        int rtpVideoPort = 44068;
        int rtcpVideoPort = 44071;
        RTP_TRANSPORT transportType = RTP_TRANSPORT_UDP;
        int numCodecs = 0;
        SdpCodec** codecArray = NULL;
        SdpSrtpParameters srtpParameters;
        memset(&srtpParameters, 0, sizeof(SdpSrtpParameters));
        int videoBandwidth = 0;
        int videoFramerate = 0;

        // Setup some codecs
        SdpCodecList codecList;
        codecList.addCodecs("AAC_LC_32000 G722 PCMU MPEG4_GENERIC_AAC_LC_32000 MPEG4_GENERIC_AAC_LC_48000 MPEG4_GENERIC_AAC_LC_16000 telephone-event "
                            "H264_PM1_EDTV_512");
        codecList.bindPayloadTypes();
        codecList.getCodecs(numCodecs, codecArray);

        sdpAnswer.setStandardHeaderFields("call", NULL, NULL, hostAddress);

        sdpAnswer.addCodecsAnswer(1,
                                  &hostAddress,
                                  &rtpAudioPort,
                                  &rtcpAudioPort,
                                  &rtpVideoPort,
                                  &rtcpVideoPort,
                                  &transportType,
                                  numCodecs,
                                  codecArray,
                                  srtpParameters,
                                  videoBandwidth,
                                  videoFramerate,
                                  &sdpOffer);

        UtlString answerString;
        int answerLength;
        sdpAnswer.getBytes(&answerString, &answerLength);
        printf("Answer:\n%s", answerString.data());

        CPPUNIT_ASSERT_EQUAL(3, sdpAnswer.getMediaSetCount());

        int mediaPort;
        int rtcpPort;
        UtlString mediaType;
        int numMediaPortPairs;
        UtlString answerTransportType;
        const int maxPayloadTypes = 20;
        int numPayloads;
        int payloadIds[maxPayloadTypes];
        UtlString mimeSubtype;
        int sampleRate;
        int numChannels;

        sdpAnswer.getMediaData(0, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds);

        CPPUNIT_ASSERT_EQUAL("audio", mediaType);
        CPPUNIT_ASSERT_EQUAL(answerTransportType, "RTP/AVP");
        CPPUNIT_ASSERT_EQUAL(mediaPort,  rtpAudioPort);
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaRtcpPort(0, &rtcpPort), TRUE);
        CPPUNIT_ASSERT_EQUAL(rtcpPort, rtcpAudioPort);
        // Tones should assume the payload ID of the offer: 119
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getPayloadRtpMap(0, 119, mimeSubtype, sampleRate, numChannels), TRUE);
        UtlString refMimeSubtype(MIME_SUBTYPE_DTMF_TONES);
        refMimeSubtype.SDP_MIME_SUBTYPE_TO_CASE();
        CPPUNIT_ASSERT_EQUAL(mimeSubtype, refMimeSubtype);

        sdpAnswer.getMediaData(1, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds);

        CPPUNIT_ASSERT_EQUAL("video", mediaType);
        CPPUNIT_ASSERT_EQUAL(answerTransportType, "RTP/AVP");
        CPPUNIT_ASSERT_EQUAL(mediaPort,  rtpVideoPort);
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaRtcpPort(1, &rtcpPort), TRUE);
        CPPUNIT_ASSERT_EQUAL(rtcpPort, rtcpVideoPort);

        sdpAnswer.getMediaData(2, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds);

        CPPUNIT_ASSERT_EQUAL("application", mediaType);
        CPPUNIT_ASSERT_EQUAL(mediaPort, 0);

    }

    void test5Mlines()
    {
        const char* offerString =
            "v=0\r\n"
            "o=tandberg 210 3 IN IP4 172.17.2.59\r\n"
            "s=-\r\n"
            "c=IN IP4 172.17.2.59\r\n"
            "b=AS:4000\r\n"
            "t=0 0\r\n"
            "m=audio 2342 RTP/AVP 100 102 103 104 105 9 8 0 101\r\n"
            "b=TIAS:64000\r\n"
            "a=rtpmap:100 MP4A-LATM/90000\r\n"
            "a=fmtp:100 profile-level-id=24;object=23;bitrate=64000\r\n"
            "a=rtpmap:102 MP4A-LATM/90000\r\n"
            "a=fmtp:102 profile-level-id=24;object=23;bitrate=56000\r\n"
            "a=rtpmap:103 MP4A-LATM/90000\r\n"
            "a=fmtp:103 profile-level-id=24;object=23;bitrate=48000\r\n"
            "a=rtpmap:104 G7221/16000\r\n"
            "a=fmtp:104 bitrate=32000\r\n"
            "a=rtpmap:105 G7221/16000\r\n"
            "a=fmtp:105 bitrate=24000\r\n"
            "a=rtpmap:9 G722/8000\r\n"
            "a=rtpmap:8 PCMA/8000\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:101 telephone-event/8000\r\n"
            "a=fmtp:101 0-15\r\n"
            "a=sendrecv\r\n"
            "m=video 2344 RTP/AVP 97 98 99 34 31\r\n"
            "b=TIAS:4000000\r\n"
            "a=rtpmap:97 H264/90000\r\n"
            "a=fmtp:97 profile-level-id=428014;max-br=3333;max-mbps=108000;max-fs=3600;max-smbps=108000;max-fps=6000\r\n"
            "a=rtpmap:98 H264/90000\r\n"
            "a=fmtp:98 profile-level-id=428014;max-br=3333;max-mbps=108000;max-fs=3600;max-smbps=108000;packetization-mode=1;max-fps=6000\r\n"
            "a=rtpmap:99 H263-1998/90000\r\n"
            "a=fmtp:99 custom=1280,768,3;custom=1280,720,3;custom=1024,768,1;custom=1024,576,2;custom=800,600,1;cif4=1;custom=720,480,1;custom=640,480,1;custom=512,288,1;cif=1;custom=352,240,1;qcif=1;maxbr=20000\r\n"
            "a=rtpmap:34 H263/90000\r\n"
            "a=fmtp:34 cif4=1;cif=1;qcif=1;maxbr=20000\r\n"
            "a=rtpmap:31 H261/90000\r\n"
            "a=fmtp:31 cif=1;qcif=1;maxbr=20000\r\n"
            "a=rtcp-fb:* nack pli\r\n"
            "a=sendrecv\r\n"
            "a=content:main\r\n"
            "a=label:11\r\n"
            "a=answer:full\r\n"
            "m=application 5070 UDP/BFCP *\r\n"
            "a=floorctrl:c-s\r\n"
            "a=confid:1\r\n"
            "a=floorid:2 mstrm:12\r\n"
            "a=userid:210\r\n"
            "a=setup:actpass\r\n"
            "a=connection:new\r\n"
            "m=video 2346 RTP/AVP 97 98 99 34 31\r\n"
            "b=TIAS:4000000\r\n"
            "a=rtpmap:97 H264/90000\r\n"
            "a=fmtp:97 profile-level-id=428014;max-br=3333;max-mbps=58000;max-fs=3840;max-smbps=58000;max-fps=6000\r\n"
            "a=rtpmap:98 H264/90000\r\n"
            "a=fmtp:98 profile-level-id=428014;max-br=3333;max-mbps=58000;max-fs=3840;max-smbps=58000;packetization-mode=1;max-fps=6000\r\n"
            "a=rtpmap:99 H263-1998/90000\r\n"
            "a=fmtp:99 custom=1280,768,3;custom=1280,720,3;custom=1024,768,2;custom=1024,576,2;custom=800,600,1;cif4=1;custom=720,480,1;custom=640,480,1;custom=512,288,1;cif=1;custom=352,240,1;qcif=1;maxbr=20000\r\n"
            "a=rtpmap:34 H263/90000\r\n"
            "a=fmtp:34 cif4=1;cif=1;qcif=1;maxbr=20000\r\n"
            "a=rtpmap:31 H261/90000\r\n"
            "a=fmtp:31 cif=1;qcif=1;maxbr=20000\r\n"
            "a=rtcp-fb:* nack pli\r\n"
            "a=sendrecv\r\n"
            "a=content:slides\r\n"
            "a=label:12\r\n"
            "m=application 2348 RTP/AVP 106\r\n"
            "a=rtpmap:106 H224/4800\r\n"
            "a=sendrecv\r\n";


        printf("Offer:\n%s", offerString);
        SdpBody sdpOffer(offerString);

        SdpBody sdpAnswer;
        UtlString hostAddress("12.34.56.78");
        int rtpAudioPort = 2342;
        int rtcpAudioPort = 2343;
        int rtpVideoPort = 2344;
        int rtcpVideoPort = 2345;
        RTP_TRANSPORT transportType = RTP_TRANSPORT_UDP;
        int numCodecs = 0;
        SdpCodec** codecArray = NULL;
        SdpSrtpParameters srtpParameters;
        memset(&srtpParameters, 0, sizeof(SdpSrtpParameters));
        int videoBandwidth = 0;
        int videoFramerate = 0;

        // Setup some codecs
        SdpCodecList codecList;
        codecList.addCodecs("AAC_LC_32000 G722 PCMU telephone-event "
                            "H264_PM1_EDTV_512");
        codecList.bindPayloadTypes();
        codecList.getCodecs(numCodecs, codecArray);

        sdpAnswer.setStandardHeaderFields("call", NULL, NULL, hostAddress);

        sdpAnswer.addCodecsAnswer(1,
                                  &hostAddress,
                                  &rtpAudioPort,
                                  &rtcpAudioPort,
                                  &rtpVideoPort,
                                  &rtcpVideoPort,
                                  &transportType,
                                  numCodecs,
                                  codecArray,
                                  srtpParameters,
                                  videoBandwidth,
                                  videoFramerate,
                                  &sdpOffer);

        UtlString answerString;
        int answerLength;
        sdpAnswer.getBytes(&answerString, &answerLength);
        printf("Answer:\n%s", answerString.data());

        CPPUNIT_ASSERT_EQUAL(5, sdpAnswer.getMediaSetCount());

        int mediaPort;
        int rtcpPort;
        UtlString mediaType;
        int numMediaPortPairs;
        UtlString answerTransportType;
        const int maxPayloadTypes = 20;
        int numPayloads;
        int payloadIds[maxPayloadTypes];
        UtlString mimeSubtype;
        int sampleRate;
        int numChannels;
        UtlString localAddress;

        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaData(0, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds), TRUE);

        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaAddress(0, &localAddress), TRUE);
        CPPUNIT_ASSERT_EQUAL(localAddress, hostAddress);

        CPPUNIT_ASSERT_EQUAL("audio", mediaType);
        CPPUNIT_ASSERT_EQUAL(answerTransportType, "RTP/AVP");
        CPPUNIT_ASSERT_EQUAL(mediaPort,  rtpAudioPort);
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaRtcpPort(0, &rtcpPort), TRUE);
        CPPUNIT_ASSERT_EQUAL(rtcpPort, rtcpAudioPort);
        // Tones should assume the payload ID of the offer: 101
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getPayloadRtpMap(0, 101, mimeSubtype, sampleRate, numChannels), TRUE);
        UtlString refMimeSubtype(MIME_SUBTYPE_DTMF_TONES);
        refMimeSubtype.SDP_MIME_SUBTYPE_TO_CASE();
        CPPUNIT_ASSERT_EQUAL(mimeSubtype, refMimeSubtype);
        CPPUNIT_ASSERT_EQUAL(numPayloads, 4);
        CPPUNIT_ASSERT_EQUAL(payloadIds[0], 96); // AAC
        CPPUNIT_ASSERT_EQUAL(payloadIds[1], 9); // G722
        CPPUNIT_ASSERT_EQUAL(payloadIds[2], 0); // PCMU
        CPPUNIT_ASSERT_EQUAL(payloadIds[3], 101); //TONES

        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaData(1, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds), TRUE);

        CPPUNIT_ASSERT_EQUAL("video", mediaType);
        CPPUNIT_ASSERT_EQUAL(answerTransportType, "RTP/AVP");
        CPPUNIT_ASSERT_EQUAL(mediaPort,  rtpVideoPort);
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaRtcpPort(1, &rtcpPort), TRUE);
        CPPUNIT_ASSERT_EQUAL(rtcpPort, rtcpVideoPort);
        CPPUNIT_ASSERT_EQUAL(numPayloads, 1);
        CPPUNIT_ASSERT_EQUAL(payloadIds[0], 98); // Should use same payload ID as remote H.264 packetization-mode=1

        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaAddress(1, &localAddress), TRUE);
        CPPUNIT_ASSERT_EQUAL(localAddress, hostAddress);

        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaData(2, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds), TRUE);

        SdpBody::SessionDirection direction;

        CPPUNIT_ASSERT_EQUAL("application", mediaType);
        CPPUNIT_ASSERT_EQUAL(mediaPort, 0);
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaStreamDirection(2, direction), TRUE);
        CPPUNIT_ASSERT_EQUAL(direction, SdpBody::Inactive);

        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaData(3, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds), TRUE);

        CPPUNIT_ASSERT_EQUAL("video", mediaType);
        CPPUNIT_ASSERT_EQUAL(mediaPort, 0);
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaStreamDirection(3, direction), TRUE);
        CPPUNIT_ASSERT_EQUAL(direction, SdpBody::Inactive);

        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaData(4, &mediaType, &mediaPort, &numMediaPortPairs, &answerTransportType,
            maxPayloadTypes, &numPayloads, payloadIds), TRUE);

        CPPUNIT_ASSERT_EQUAL("application", mediaType);
        CPPUNIT_ASSERT_EQUAL(mediaPort, 0);
        CPPUNIT_ASSERT_EQUAL(sdpAnswer.getMediaStreamDirection(4, direction), TRUE);
        CPPUNIT_ASSERT_EQUAL(direction, SdpBody::Inactive);

    }
    void testGetCodecsInCommonFull()
    {
            // Intensionally no rtpmaps for 0 and 9 (PCMA and G722)
            // Payload IDs 122 and 97 intensionally in m line, but not defined in rtpmap
            // TODO: add G729 and G723 rtpmaps to SDP
        const char* sdpBodyText =
            "v=0\n"
            "o=sipX 5 5 IN IP4 192.168.0.101\n"
            "s=call\n"
            "t=0 0\n"
            "m=audio 9000 RTP/AVP 8 124 108 115 109 96 123 126 104 105 106 107 100 101 102 103 110 111 98 99 125 112 113 114 116 118 119 120 121 117 0 9 122 127 97 142 143 144 128 129 130 131 132 133 134 135 136 137 138 139 140 141i 145 146\n"
            "c=IN IP4 192.168.0.101\n"
            "a=rtpmap:8 PCMA/8000/1\n"
            "a=rtpmap:124 G726-24/8000/1\n"
            "a=rtpmap:108 OPUS/48000/2\n"
            "a=fmtp:108 stereo=0; sprop-stereo=0\n"
            "a=rtpmap:115 AMR/8000/1\n"
            "a=fmtp:115 octet-align=1\n"
            "a=rtpmap:136 AMR/8000/1\n"
            "a=fmtp:136 octet-align=0\n"
            "a=rtpmap:109 ILBC/8000/1\n"
            "a=rtpmap:96 ILBC/8000/1\n"
            "a=fmtp:96 mode=20\n"
            "a=rtpmap:123 G726-32/8000/1\n"
            "a=rtpmap:126 G726-16/8000/1\n"
            "a=rtpmap:104 SPEEX/32000/1\n"
            "a=rtpmap:105 SPEEX/32000/1\n"
            "a=fmtp:105 mode=1\n"
            "a=rtpmap:106 SPEEX/32000/1\n"
            "a=fmtp:106 mode=6\n"
            "a=rtpmap:107 SPEEX/32000/1\n"
            "a=fmtp:107 mode=10\n"
            "a=rtpmap:100 SPEEX/16000/1\n"
            "a=rtpmap:101 SPEEX/16000/1\n"
            "a=fmtp:101 mode=1\n"
            "a=rtpmap:102 SPEEX/16000/1\n"
            "a=fmtp:102 mode=6\n"
            "a=rtpmap:103 SPEEX/16000/1\n"
            "a=fmtp:103 mode=10\n"
            "a=rtpmap:110 SPEEX/8000/1\n"
            "a=rtpmap:111 SPEEX/8000/1\n"
            "a=fmtp:111 mode=2\n"
            "a=rtpmap:98 SPEEX/8000/1\n"
            "a=fmtp:98 mode=5\n"
            "a=rtpmap:99 SPEEX/8000/1\n"
            "a=fmtp:99 mode=7\n"
            "a=rtpmap:125 G726-40/8000/1\n"
            "a=rtpmap:112 L16/48000/1\n"
            "a=rtpmap:113 L16/44100/1\n"
            "a=rtpmap:114 L16/32000/1\n"
            "a=rtpmap:116 L16/24000/1\n"
            "a=rtpmap:118 L16/22050/1\n"
            "a=rtpmap:119 L16/16000/1\n"
            "a=rtpmap:120 L16/11025/1\n"
            "a=rtpmap:121 L16/8000/1\n"
            "a=rtpmap:127 L16/48000/2\n"
            "a=rtpmap:128 L16/44100/2\n"
            "a=rtpmap:129 L16/32000/2\n"
            "a=rtpmap:130 L16/24000/2\n"
            "a=rtpmap:131 L16/22050/2\n"
            "a=rtpmap:132 L16/16000/2\n"
            "a=rtpmap:133 L16/11025/2\n"
            "a=rtpmap:134 L16/8000/2\n"
            "a=rtpmap:135 gsm/8000/1\n"
            "a=rtpmap:117 AMR-WB/16000/1\n"
            "a=fmtp:117 octet-align=1\n"
            "a=rtpmap:142 G7221/32000\n"
            "a=fmtp:142 bitrate=48000\n"
            "a=rtpmap:143 G7221/32000\n"
            "a=fmtp:143 bitrate=32000\n"
            "a=rtpmap:144 G7221/32000\n"
            "a=fmtp:144 bitrate=24000\n"
            "a=rtpmap:137 AMR-WB/16000/1\n"
            "a=rtpmap:138 AAC_LC/32000/1\n"
            "a=rtpmap:139 MPEG4-GENERIC/32000/1\n"
            "a=fmtp:139 streamtype=5; profile-level-id=15; mode=AAC-hbr; config=1288; SizeLength=13; IndexLength=3; IndexDeltaLength=3; Profile=1\n"
            "a=rtpmap:140 MPEG4-GENERIC/16000/1\n"
            "a=fmtp:140 streamtype=5; profile-level-id=15; mode=AAC-hbr; config=1408; SizeLength=13; IndexLength=3; IndexDeltaLength=3; Profile=1\n"
            "a=rtpmap:141 MPEG4-GENERIC/48000/1\n"
            "a-fmtp:141 streamtype=5; profile-level-id=15; mode=AAC-hbr; config=1188; SizeLength=13; IndexLength=3; IndexDeltaLength=3; Profile=1\n"
            "a=ptime:30\n"
            "a=control:trackID=1\n"
            "a=rtpmap:145 G7221/16000\n"
            "a=fmtp:145 bitrate=32000\n"
            "a=rtpmap:146 G7221/16000\n"
            "a=fmtp:146 bitrate=24000\n";

        SdpBody sdpBody(sdpBodyText);
        UtlSList allCodecNames;
        int allCodecCount = SdpDefaultCodecFactory::getCodecNames(allCodecNames);
        CPPUNIT_ASSERT(100 < allCodecCount);
        CPPUNIT_ASSERT_EQUAL(allCodecCount, allCodecNames.entries());
        SdpCodecList enabledCodecs;

        // Number of rejected or unknown codecs in list
        CPPUNIT_ASSERT_EQUAL(0, enabledCodecs.addCodecs(allCodecNames));

        int numCommonCodecs = 0;
        SdpCodec** codecsInCommonArray = NULL;
        UtlString rtpAddress;
        int rtpPort = 0;
        int rtcpPort = 0;
        int videoRtpPort = 0;
        int videoRtcpPort = 0;
        SdpSrtpParameters localSrtpParameters;
        memset((void*)&localSrtpParameters, 0, sizeof(SdpSrtpParameters));
        SdpSrtpParameters matchingSrtpParameters;
        memset((void*)&matchingSrtpParameters, 0, sizeof(SdpSrtpParameters));
        int localBandwidth = 0;
        int matchingBandwidth = 0;
        int localVideoFramerate = 0;
        int matchingVideoFramerate = 0;

        sdpBody.getBestAudioCodecs(enabledCodecs,
                                   numCommonCodecs,
                                   codecsInCommonArray,
                                   //SdpCodec**& commonCodecsForDecoder,
                                   rtpAddress,
                                   rtpPort,
                                   rtcpPort,
                                   videoRtpPort,
                                   videoRtcpPort,
                                   localSrtpParameters,
                                   matchingSrtpParameters,
                                   localBandwidth,
                                   matchingBandwidth,
                                   localVideoFramerate,
                                   matchingVideoFramerate);


        CPPUNIT_ASSERT_EQUAL(numCommonCodecs, 52);
        if(numCommonCodecs != 52)
        {
            SdpCodecList codecsInCommonList(numCommonCodecs, codecsInCommonArray);
            UtlString codecsInCommonString;
            codecsInCommonList.toString(codecsInCommonString);
            printf("codecs in common: \n%s\n", codecsInCommonString.data());
            

        }

        // Non-static payload IDs should be initialized to -1
        int numEnabledCodecs = 0;
        SdpCodec** enabledCodecsArray = NULL;
        UtlString codecName;
        enabledCodecs.getCodecs(numEnabledCodecs, enabledCodecsArray);
        for(int enabledCodecIndex = 0; enabledCodecIndex < numEnabledCodecs; enabledCodecIndex++)
        {
            enabledCodecsArray[enabledCodecIndex]->getEncodingName(codecName);

            if(mCodecsToIgnore.contains(&codecName))
            {
                printf("Skipping payload id check for codec: %s\n", codecName.data());
                continue;
            }

            switch(enabledCodecsArray[enabledCodecIndex]->getCodecType())
            {
            // PCMA, PCMU, G722 are the only static codec payload IDs we support
            case SdpCodec::SDP_CODEC_PCMU:
            case SdpCodec::SDP_CODEC_PCMA:
            case SdpCodec::SDP_CODEC_GSM:
            case SdpCodec::SDP_CODEC_G722:
            case SdpCodec::SDP_CODEC_G723:
            case SdpCodec::SDP_CODEC_G729:
            case SdpCodec::SDP_CODEC_L16_44100_STEREO:
            case SdpCodec::SDP_CODEC_L16_44100_MONO:
            case SdpCodec::SDP_CODEC_H263:

                CPPUNIT_ASSERT_EQUAL_MESSAGE(codecName, 
                                             enabledCodecsArray[enabledCodecIndex]->getCodecType(), 
                                             enabledCodecsArray[enabledCodecIndex]->getCodecPayloadFormat());

            // This codecs have internal codec IDs different from their static payload id
            case SdpCodec::SDP_CODEC_H263_CIF:
            case SdpCodec::SDP_CODEC_H263_QCIF:
            case SdpCodec::SDP_CODEC_H263_SQCIF:
            case SdpCodec::SDP_CODEC_H263_QVGA:
            case SdpCodec::SDP_CODEC_G729A:
            case SdpCodec::SDP_CODEC_G729AB:

                CPPUNIT_ASSERT_MESSAGE(codecName, 
                                       SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC >=
                                       enabledCodecsArray[enabledCodecIndex]->getCodecPayloadFormat());
                break;

            // Dynamic codec payload types should not be bound yet (i.e. -1)
            default:
                CPPUNIT_ASSERT_EQUAL_MESSAGE(codecName, 
                                             -1, enabledCodecsArray[enabledCodecIndex]->getCodecPayloadFormat());
                break;
            }
        }
        SdpCodecList::freeArray(numEnabledCodecs, enabledCodecsArray);
        numEnabledCodecs = 0;
        enabledCodecsArray = NULL;

        // Bind the payload IDs in the enabled set of codecs to match those from
        // the SDP.
        enabledCodecs.copyPayloadTypes(numCommonCodecs, (const SdpCodec**)codecsInCommonArray);

        SdpCodecList::freeArray(numCommonCodecs, codecsInCommonArray);
        numCommonCodecs = 0;
        codecsInCommonArray = NULL;

        const UtlString* aSupportedCodec = NULL;
        UtlSListIterator iterator(allCodecNames);
        while((aSupportedCodec = (const UtlString*) iterator()))
        {
            if(mCodecsToIgnore.contains(aSupportedCodec))
            {
                printf("Skipping codec: %s\n", aSupportedCodec->data());
                continue;
            }
            enabledCodecs.clearCodecs();
            // Pretend we only support one codec
            enabledCodecs.addCodecs(*aSupportedCodec);

            // Find codec in common
            sdpBody.getBestAudioCodecs(enabledCodecs,
                                       numCommonCodecs,
                                       codecsInCommonArray,
                                       //SdpCodec**& commonCodecsForDecoder,
                                       rtpAddress,
                                       rtpPort,
                                       rtcpPort,
                                       videoRtpPort,
                                       videoRtcpPort,
                                       localSrtpParameters,
                                       matchingSrtpParameters,
                                       localBandwidth,
                                       matchingBandwidth,
                                       localVideoFramerate,
                                       matchingVideoFramerate);

            SdpCodec::SdpCodecTypes internalCodecId = SdpDefaultCodecFactory::getCodecType(*aSupportedCodec);
            CPPUNIT_ASSERT_MESSAGE(*aSupportedCodec, internalCodecId != SdpCodec::SDP_CODEC_UNKNOWN);

            const SdpCodec* localCodec = enabledCodecs.getCodec(internalCodecId);
            CPPUNIT_ASSERT_MESSAGE(*aSupportedCodec, localCodec);
            UtlString mediaType;
            if(localCodec)
            {
                localCodec->getMediaType(mediaType);
                mediaType.toUpper();
            }

            // There is only audio codecs in the SDP body
            if(mediaType == "AUDIO")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(*aSupportedCodec, 1, numCommonCodecs);
                if(numCommonCodecs > 1)
                {
                    printf("multiple matches (%d) found\n", numCommonCodecs);
                    for(int matchIndex = 0; matchIndex < numCommonCodecs; matchIndex++)
                    {
                        UtlString codecString;
                        codecsInCommonArray[matchIndex]->toString(codecString);
                        printf("\t%s", codecString.data());
                    }
                }
            }

            if(numCommonCodecs && localCodec)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(*aSupportedCodec, 
                                             codecsInCommonArray[0]->getCodecType(), internalCodecId);

                // This assume literal string blob to be the same on both sides.
                //  If the FMTP field has more than one parameter, and order does
                // not matter, this check may be too rigid.  We would have to iterate
                // through the parameters.
                UtlString localFmtp;
                UtlString remoteFmtp;
                localCodec->getSdpFmtpField(localFmtp);
                codecsInCommonArray[0]->getSdpFmtpField(remoteFmtp);
                if(*aSupportedCodec == "AMR" && localFmtp == "")
                {
                    // fmpt not specified (i.e. "") is the same as "octet-align=0"
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(*aSupportedCodec, "octet-align=0", remoteFmtp);
                }
                else
                {
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(*aSupportedCodec, localFmtp, remoteFmtp);
                }

                // Compare MIME sub type
                UtlString localSubtype;
                UtlString remoteSubtype;
                localCodec->getEncodingName(localSubtype);
                codecsInCommonArray[0]->getEncodingName(remoteSubtype);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(*aSupportedCodec, localSubtype, remoteSubtype);

                // Compare sample rates
                CPPUNIT_ASSERT_EQUAL_MESSAGE(*aSupportedCodec,
                                             localCodec->getSampleRate(), codecsInCommonArray[0]->getSampleRate());

                // Compare channels
                CPPUNIT_ASSERT_EQUAL_MESSAGE(*aSupportedCodec,
                                             localCodec->getNumChannels(), codecsInCommonArray[0]->getNumChannels());
            }
            else if(mediaType == "AUDIO")
            {
                printf("Codec %s not matched in SDP:\n", aSupportedCodec->data());
                if(localCodec)
                {
                    UtlString codecString;
                    localCodec->toString(codecString);
                    printf("\t%s\n", codecString.data());
                }
            }

            SdpCodecList::freeArray(numCommonCodecs, codecsInCommonArray);
            codecsInCommonArray = NULL;
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdpBodyTest);
