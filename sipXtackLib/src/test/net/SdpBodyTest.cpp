// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/HttpMessage.h>
#include <net/SdpBody.h>


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
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdpBodyTest);
