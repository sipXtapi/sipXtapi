//
// Copyright (C) 2004-2006 SIPfoundry Inc.
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
#include <net/HttpMessage.h>
#include <net/SdpBody.h>
#include <net/SdpCodecFactory.h>


/**
* Unit test for SdpBody
*/
class SdpBodyTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(SdpBodyTest);
    CPPUNIT_TEST(testParser);
    CPPUNIT_TEST(testIndexAccessor);
    CPPUNIT_TEST(testNewMessage);
    CPPUNIT_TEST(testTimeHeaders);
    CPPUNIT_TEST(testGetMediaSetCount);
    CPPUNIT_TEST(testGetMediaAddress);
    //CPPUNIT_TEST(testCandidateParsing);
    CPPUNIT_TEST(testRtcpPortParsing);
    CPPUNIT_TEST(testVideoCodecSelection);
    CPPUNIT_TEST_SUITE_END();

public:

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
            "m=video 51372 RTP/AVP 31\r\n"
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
            "m=video 51372 RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

        SdpBody body(sdp);

        /* Why are there not accessors for version or other standard fields !?! */

        int fields = body.SdpBody::getFieldCount();
        CPPUNIT_ASSERT(fields == 13);

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
        ASSERT_STR_EQUAL(value.data(), "video 51372 RTP/AVP 31");

        CPPUNIT_ASSERT(body.getValue(11, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "m");
        ASSERT_STR_EQUAL(value.data(), "application 32416 udp wb");

        CPPUNIT_ASSERT(body.getValue(12, &name, &value));
        ASSERT_STR_EQUAL(name.data(), "a");
        ASSERT_STR_EQUAL(value.data(), "orient:portrait");

        CPPUNIT_ASSERT(!body.getValue(13, &name, &value));
        CPPUNIT_ASSERT(name.isNull());
        CPPUNIT_ASSERT(value.isNull());

        name = "foo";
        value = "notempty";

        CPPUNIT_ASSERT(!body.getValue(15, &name, &value));
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
            "m=video 51372 RTP/AVP 31\r\n"
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
            "m=video 51372 RTP/AVP 31\r\n"
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
            "m=video 51372 RTP/AVP 31\r\n"
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
            "m=video 51372 RTP/AVP 31\r\n"
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
            "m=video 51372 RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;
        SdpBody body1(sdp1);

        CPPUNIT_ASSERT(body1.getMediaSetCount() == 3);

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

        CPPUNIT_ASSERT(body1.getMediaSetCount() == 3);
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
            "m=video 51372 RTP/AVP 31\r\n"
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

        address = "wrong";

        CPPUNIT_ASSERT(!body1.getMediaAddress(3, &address));
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
            "m=video 51372 RTP/AVP 31\r\n"
            "c=IN IP4 224.2.17.15/127\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;
        SdpBody body2(sdp2);

        CPPUNIT_ASSERT(body2.getMediaAddress(0, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.12");

        CPPUNIT_ASSERT(body2.getMediaAddress(1, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.15");

        CPPUNIT_ASSERT(body2.getMediaAddress(2, &address));
        ASSERT_STR_EQUAL( address.data(), "224.2.17.12");
    }

    void testCandidateParsing()
    {
        int candidateId;
        UtlString transportId;
        UtlString transportType;
        double qValue;
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
            "a=rtpmap:0 pcmu/8000/1\r\n"
            "a=rtpmap:8 pcma/8000/1\r\n"
            "a=rtpmap:96 telephone-event/8000/1\r\n" 
            "a=candidate:0 tid1 UDP 0.4 10.1.1.102 9999\r\n"
            "a=candidate:1 tid2 UDP 0.5 10.1.1.102 10000\r\n"
            "a=candidate:2 tid3 UDP 0.6 10.1.1.103 9999\r\n" 
            "m=video 1234 RTP/AVP 0 8 96\r\n"
            "a=rtpmap:0 pcmu/8000/1\r\n"
            "a=rtpmap:8 pcma/8000/1\r\n"
            "a=rtpmap:96 telephone-event/8000/1\r\n" 
            "a=candidate:0 tid1b UDP 0.4 10.1.1.102 9999\r\n"
            "a=candidate:1 tid2b UDP 0.5 10.1.1.102 10000\r\n"
            "a=candidate:2 tid3b UDP 0.6 10.1.1.103 9999\r\n" ;


        SdpBody body2(sdp2) ;

        bRC = body2.getCandidateAttribute(0, -1, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == FALSE) ;

        bRC = body2.getCandidateAttribute(0, 0, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 0) ;
        ASSERT_STR_EQUAL("tid1", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 0.4) ;
        ASSERT_STR_EQUAL("10.1.1.102", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;

        bRC = body2.getCandidateAttribute(0, 1, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 1) ;
        ASSERT_STR_EQUAL("tid2", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 0.5) ;
        ASSERT_STR_EQUAL("10.1.1.102", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 10000) ;

        bRC = body2.getCandidateAttribute(0, 2, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 2) ;
        ASSERT_STR_EQUAL("tid3", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 0.6) ;
        ASSERT_STR_EQUAL("10.1.1.103", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;

        bRC = body2.getCandidateAttribute(0, 3, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == FALSE) ;


        bRC = body2.getCandidateAttribute(1, -1, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == FALSE) ;

        bRC = body2.getCandidateAttribute(1, 0, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 0) ;
        ASSERT_STR_EQUAL("tid1b", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 0.4) ;
        ASSERT_STR_EQUAL("10.1.1.102", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;

        bRC = body2.getCandidateAttribute(1, 1, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 1) ;
        ASSERT_STR_EQUAL("tid2b", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 0.5) ;
        ASSERT_STR_EQUAL("10.1.1.102", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 10000) ;

        bRC = body2.getCandidateAttribute(1, 2, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        CPPUNIT_ASSERT(candidateId == 2) ;
        ASSERT_STR_EQUAL("tid3b", transportId.data()) ;
        ASSERT_STR_EQUAL("UDP", transportType.data()) ;
        CPPUNIT_ASSERT(qValue == 0.6) ;
        ASSERT_STR_EQUAL("10.1.1.103", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;        

        bRC = body2.getCandidateAttribute(1, 3, candidateId, transportId, 
                transportType, qValue, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == FALSE) ;


        SdpBody testBody;
        const char* testBodyExpected = 
            "v=0\r\n"
            "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
            "s=\r\n"
            "a=candidate:0 tid1 UDP 0.4 10.1.1.102 9999\r\n"
            "a=candidate:0 tid2 UDP 0.4 10.1.1.102 10000\r\n" ;

        testBody.addCandidateAttribute(0, "tid1", "UDP", 0.4, "10.1.1.102", 9999) ;
        testBody.addCandidateAttribute(0, "tid2", "UDP", 0.4, "10.1.1.102", 10000) ;
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
        testBody.addAudioCodecs(1, hostAddresses, rtpAudioPorts, rtcpAudioPorts, 
                                rtpVideoPorts, rtcpVideoPorts, 
                                1, &pAudioCodec, testSrtp, 0, 0) ;
        hostAddresses[0] = "10.1.1.31";
        rtpAudioPorts[0] = 0;
        rtcpAudioPorts[0] = 0;
        rtpVideoPorts[0] = 8801;
        rtcpVideoPorts[0] = 8802;
        testBody.addAudioCodecs(1, hostAddresses, rtpAudioPorts, rtcpAudioPorts, 
                                rtpVideoPorts, rtcpVideoPorts, 
                                1, &pVideoCodec, testSrtp, 0, 0) ;
        hostAddresses[0] = "10.1.1.32";
        rtpAudioPorts[0] = 8900;
        rtcpAudioPorts[0] = 8999;
        rtpVideoPorts[0] = 0;
        rtcpVideoPorts[0] = 0;
        testBody.addAudioCodecs(1, hostAddresses, rtpAudioPorts, rtcpAudioPorts, 
                                rtpVideoPorts, rtcpVideoPorts, 
                                1, &pAppCodec, testSrtp, 0, 0) ;

        const char* testBodyExpected = 
            "v=0\r\n"
            "o=sipX 5 5 IN IP4 127.0.0.1\r\n"
            "s=foo\r\n"
            "c=IN IP4 10.1.1.30\r\n"
            "t=0 0\r\n"
            "m=audio 8700 RTP/AVP 99\r\n"
            "a=rtpmap:99 superaudio/8000/1\r\n"
            "m=video 8801 RTP/AVP 100\r\n"
            "a=rtcp:8802\r\n"
            "a=rtpmap:100 supervideo/8000/1\r\n"
            "a=fmtp:100 size:QCIF\r\n"
            "c=IN IP4 10.1.1.31\r\n"
            "m=audio 8900 RTP/AVP 101\r\n"
            "a=rtcp:8999\r\n"
            "a=rtpmap:101 superapp/8000/1\r\n"
            "c=IN IP4 10.1.1.32\r\n" ;

        UtlString address ;
        int port ;

        testBody.getBytes(&strBody, &nBody) ;
        ASSERT_STR_EQUAL(testBodyExpected, strBody.data()) ;


        SdpBody bodyCheck(testBodyExpected) ;

        CPPUNIT_ASSERT(bodyCheck.getMediaSetCount() == 3) ;

        bodyCheck.getMediaAddress(0, &address) ;
        ASSERT_STR_EQUAL("10.1.1.30", address.data()) ;
        bodyCheck.getMediaPort(0, &port) ; 
        CPPUNIT_ASSERT(port == 8700) ;
        bodyCheck.getMediaRtcpPort(0, &port) ;
        CPPUNIT_ASSERT(port == 8701) ;

        bodyCheck.getMediaAddress(1, &address) ;
        ASSERT_STR_EQUAL("10.1.1.31", address.data()) ;
        bodyCheck.getMediaPort(1, &port) ; 
        CPPUNIT_ASSERT(port == 8801) ;
        bodyCheck.getMediaRtcpPort(1, &port) ;
        CPPUNIT_ASSERT(port == 8802) ;

        bodyCheck.getMediaAddress(2, &address) ;
        ASSERT_STR_EQUAL("10.1.1.32", address.data()) ;
        bodyCheck.getMediaPort(2, &port) ; 
        CPPUNIT_ASSERT(port == 8900) ;
        bodyCheck.getMediaRtcpPort(2, &port) ;
        CPPUNIT_ASSERT(port == 8999) ;
    }

    void testVideoCodecSelection()
    {
        SdpCodecFactory fac;

        SdpCodec* pQvgaCodec = new SdpCodec(SdpCodec::SDP_CODEC_VP71_QVGA, 99, "video", "vp71",
            9000, 20000, 1, "", 0, 2, SDP_VIDEO_FORMAT_QVGA) ;
        SdpCodec* pSqcifCodec = new SdpCodec(SdpCodec::SDP_CODEC_VP71_SQCIF, 100, "video", "vp71",
            9000, 20000, 1, "", 0, 2, SDP_VIDEO_FORMAT_SQCIF) ;
        SdpCodec* pQcifCodec = new SdpCodec(SdpCodec::SDP_CODEC_VP71_QCIF, 101, "video", "vp71",
            9000, 20000, 1, "", 0, 2, SDP_VIDEO_FORMAT_QCIF) ;

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
        int videoFmtp;
        int videoSizes[10];

        local.getPayloadFormat(100, fmtp, videoFmtp, numVideoSizes, videoSizes);
        CPPUNIT_ASSERT(numVideoSizes == 3);
        CPPUNIT_ASSERT(videoSizes[0] == SDP_VIDEO_FORMAT_QVGA);
        CPPUNIT_ASSERT(videoSizes[1] == SDP_VIDEO_FORMAT_SQCIF);
        CPPUNIT_ASSERT(videoSizes[2] == SDP_VIDEO_FORMAT_QCIF);

        remote.getPayloadFormat(100, fmtp, videoFmtp, numVideoSizes, videoSizes);
        CPPUNIT_ASSERT(numVideoSizes == 2);
        CPPUNIT_ASSERT(videoSizes[0] == SDP_VIDEO_FORMAT_QCIF);
        CPPUNIT_ASSERT(videoSizes[1] == SDP_VIDEO_FORMAT_SQCIF);

        int audioPayloads[5] = {99,0,0,0,0};
        int videoPayloads[5] = {100,0,0,0,0};
        int numInCommon;
        SdpCodec* codecsInCommon[5];

        remote.getCodecsInCommon(1, 1, audioPayloads, videoPayloads, 8801, fac, numInCommon, codecsInCommon);
        CPPUNIT_ASSERT(numInCommon == 2);

        CPPUNIT_ASSERT(codecsInCommon[0]->getVideoFormat() == SDP_VIDEO_FORMAT_QCIF);
        CPPUNIT_ASSERT(codecsInCommon[1]->getVideoFormat() == SDP_VIDEO_FORMAT_SQCIF);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdpBodyTest);
