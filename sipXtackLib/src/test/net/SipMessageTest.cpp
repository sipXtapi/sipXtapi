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
#include <net/SipMessage.h>


/**
 * Unittest for SipMessage
 */
class SipMessageTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipMessageTest);
      CPPUNIT_TEST(testGetVia);
      CPPUNIT_TEST(testGetAddrVia);
      CPPUNIT_TEST(testGetNoBranchVia);
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


};

CPPUNIT_TEST_SUITE_REGISTRATION(SipMessageTest);
