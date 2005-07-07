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
#include <utl/UtlHashMap.h>

#include <os/OsDefs.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>

/**
 * Unittest for SipMessage
 */
class SipMessageTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipMessageTest);
      CPPUNIT_TEST(testGetVia);
      CPPUNIT_TEST(testGetAddrVia);
      CPPUNIT_TEST(testGetNoBranchVia);
      CPPUNIT_TEST(testGetEventField);
      CPPUNIT_TEST(testProbPort);
   // :TODO: After the sipxtapi merge, which is supposed to clean up a lot
   // of how SipUserAgent is shut down, reactivate this test.
#if 0
      CPPUNIT_TEST(testCodecError);
#endif
      CPPUNIT_TEST_SUITE_END();

      public:

   void testGetVia()
      {
         const char* SimpleMessage =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

         UtlString viaAddress;
         int viaPort;
         UtlString protocol;
         int recievedPort;
         UtlBoolean receivedSet;
         UtlBoolean maddrSet;
         UtlBoolean receivePortSet;
         
         testMsg.getLastVia(&viaAddress,
                            &viaPort,
                            &protocol,
                            &recievedPort,
                            &receivedSet,
                            &maddrSet,
                            &receivePortSet);

         ASSERT_STR_EQUAL("sipx.local",viaAddress.data());
         CPPUNIT_ASSERT_EQUAL(33855, viaPort);
         ASSERT_STR_EQUAL("TCP",protocol.data());

      };

   void testGetAddrVia()
      {
         const char* SimpleMessage =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

         UtlString viaAddress;
         int viaPort;
         UtlString protocol;
         int recievedPort;
         UtlBoolean receivedSet;
         UtlBoolean maddrSet;
         UtlBoolean receivePortSet;
         
         testMsg.getLastVia(&viaAddress,
                            &viaPort,
                            &protocol,
                            &recievedPort,
                            &receivedSet,
                            &maddrSet,
                            &receivePortSet);

         ASSERT_STR_EQUAL("10.1.1.3",viaAddress.data());
         CPPUNIT_ASSERT_EQUAL(33855, viaPort);
         ASSERT_STR_EQUAL("TCP",protocol.data());

      };

   void testGetNoBranchVia()
      {
         const char* SimpleMessage =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

         UtlString viaAddress;
         int viaPort;
         UtlString protocol;
         int recievedPort;
         UtlBoolean receivedSet;
         UtlBoolean maddrSet;
         UtlBoolean receivePortSet;
         
         testMsg.getLastVia(&viaAddress,
                            &viaPort,
                            &protocol,
                            &recievedPort,
                            &receivedSet,
                            &maddrSet,
                            &receivePortSet);

         ASSERT_STR_EQUAL("10.1.1.3",viaAddress.data());
         CPPUNIT_ASSERT_EQUAL(33855, viaPort);
         ASSERT_STR_EQUAL("TCP",protocol.data());

      };

   void testGetEventField()
      {
         UtlString fullEventField;
         UtlString package;
         UtlString id;
         UtlHashMap params;
         UtlString* paramValue;

         const char* SubscribeMessage =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SubscribeMessage, strlen( SubscribeMessage ) );

         // use the raw interface to get the full field value
         CPPUNIT_ASSERT(testMsg.getEventField(fullEventField));
         ASSERT_STR_EQUAL("the-package",fullEventField.data());

         // use the parsing interface, but don't ask for the parameters
         CPPUNIT_ASSERT(testMsg.getEventField(&package));
         ASSERT_STR_EQUAL("the-package",package.data());

         // use the parsing interface and get the parameters (which should be empty)
         CPPUNIT_ASSERT(testMsg.getEventField(&package, &id, &params));
         ASSERT_STR_EQUAL("the-package",package.data());
         CPPUNIT_ASSERT(id.isNull());
         CPPUNIT_ASSERT(params.isEmpty());

         fullEventField.remove(0);
         package.remove(0);

         const char* SubscribeMessageWithId =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package;id=45wwrt2\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsgWithId( SubscribeMessageWithId, strlen( SubscribeMessageWithId ) );

         // use the raw interface to get the full field value
         CPPUNIT_ASSERT(testMsgWithId.getEventField(fullEventField));
         ASSERT_STR_EQUAL("the-package;id=45wwrt2",fullEventField.data());

         // use the parsing interface, but don't ask for the parameters
         CPPUNIT_ASSERT(testMsgWithId.getEventField(&package));
         ASSERT_STR_EQUAL("the-package",package.data());

         // use the parsing interface and get the parameters (which should be empty)
         CPPUNIT_ASSERT(testMsgWithId.getEventField(&package, &id, &params));
         ASSERT_STR_EQUAL("the-package",package.data());
         ASSERT_STR_EQUAL("45wwrt2",id.data());
         CPPUNIT_ASSERT(params.isEmpty());

         fullEventField.remove(0);
         package.remove(0);
         id.remove(0);

         const char* SubscribeMessageWithParams =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package;p1=one;id=45wwrt2;p2=two\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsgWithParams( SubscribeMessageWithParams, strlen( SubscribeMessageWithParams ) );

         // use the raw interface to get the full field value
         CPPUNIT_ASSERT(testMsgWithParams.getEventField(fullEventField));
         ASSERT_STR_EQUAL("the-package;p1=one;id=45wwrt2;p2=two",fullEventField.data());

         // use the parsing interface, but don't ask for the parameters
         CPPUNIT_ASSERT(testMsgWithParams.getEventField(&package));
         ASSERT_STR_EQUAL("the-package",package.data());

         // use the parsing interface and get the parameters (which should have values)
         CPPUNIT_ASSERT(testMsgWithParams.getEventField(&package, &id, &params));
         ASSERT_STR_EQUAL("the-package",package.data());
         ASSERT_STR_EQUAL("45wwrt2",id.data());
         CPPUNIT_ASSERT(params.entries()==2);

         UtlString paramName1("p1");
         CPPUNIT_ASSERT(NULL != (paramValue = dynamic_cast<UtlString*>(params.findValue(&paramName1))));
         ASSERT_STR_EQUAL("one",paramValue->data());

         UtlString paramName2("p2");
         CPPUNIT_ASSERT(NULL != (paramValue = dynamic_cast<UtlString*>(params.findValue(&paramName2))));
         ASSERT_STR_EQUAL("two",paramValue->data());
      };

   void testProbPort()
      {
         const char *szUrl = "\"Display@Name\"<sip:username@sipserver;transport=tcp>;tag=1234-2345";

         UtlString address;
         int       port;
         UtlString protocol;
         UtlString user;
         UtlString userLabel;
         UtlString tag;
            
         SipMessage::parseAddressFromUri(szUrl,
                                         &address,
                                         &port,
                                         &protocol,
                                         &user,
                                         &userLabel,
                                         &tag);
         
         CPPUNIT_ASSERT_EQUAL(0, port);

         ASSERT_STR_EQUAL("username", user);
         ASSERT_STR_EQUAL("\"Display@Name\"", userLabel);
         ASSERT_STR_EQUAL("1234-2345", tag);
         ASSERT_STR_EQUAL("sipserver", address);

         OsSocket::SocketProtocolTypes protoNumber;
         SipMessage::convertProtocolStringToEnum(protocol.data(), protoNumber);

         CPPUNIT_ASSERT_EQUAL(OsSocket::TCP, protoNumber);
      }

   void testCodecError()
      {
         const char* message =
            "INVITE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         // Construct a message.
         SipMessage testMsg(message, strlen(message));

         // Construct a SipUserAgent to provide the "agent name" for
         // the Warning header.
         SipUserAgent user_agent(0,
                                 0,
                                 -1,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 "PING",
                                 NULL,
                                 SIP_DEFAULT_RTT,
                                 TRUE,
                                 -1,
                                 OsServerTask::DEF_MAX_MSGS);

         // Construct error response for insufficient codeecs.
         SipMessage response;
         response.setInviteBadCodecs(&testMsg, &user_agent);

         // Check that the response code is 488.
         // Note this code is hard-coded here, because it is fixed by the
         // standard.  Using the #define would leave this test vulnerable
         // to mistakes in the #define!
         CPPUNIT_ASSERT_EQUAL(488, response.getResponseStatusCode());

         // Check that the Warning header is correct.
         const char* warning_header = response.getHeaderValue(0, "Warning");
         CPPUNIT_ASSERT(warning_header != NULL);

         // Parse the Warning header value.
         int code;
         char agent[128], text[128];
         int r =
            sscanf(warning_header, " %d %s \"%[^\"]\"", &code, agent, text);
         // Ensure that all three fields could be found.
         CPPUNIT_ASSERT_EQUAL(3, r);
         // Check the warning code.
         // Note this code is hard-coded here, because it is fixed by the
         // standard.  Using the #define would leave this test vulnerable
         // to mistakes in the #define!
         CPPUNIT_ASSERT_EQUAL(305, code);
         // Figure out what the agent value should be.
         UtlString address;
         int port;
         user_agent.getViaInfo(OsSocket::UDP, address, port);
         char agent_expected[128];
         strcpy(agent_expected, address.data());
         if (port != 5060)      // PORT_NONE
         {
            sprintf(&agent_expected[strlen(agent_expected)], ":%d", port);
         }
         // Check the agent value.
         ASSERT_STR_EQUAL(agent_expected, agent);

         // Print the warning data.
         printf("Warning code: %d\n", code);
         printf("Warning agent: '%s'\n", agent);
         printf("Warning text: '%s'\n", text);
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipMessageTest);
