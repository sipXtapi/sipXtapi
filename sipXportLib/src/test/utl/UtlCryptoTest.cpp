//  
// Copyright (C) 2008-2010 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>
#ifdef HAVE_SSL

#include <os/OsIntTypes.h>
#include <sipxunittests.h>
#include <utl/UtlCryptoKey.h>
#include <utl/UtlCryptoKeyRsa.h>
#include <utl/UtlCryptoKeySym.h>

#include <os/OsSocketCrypto.h>
#include <os/OsSocket.h>

#include <os/OsTask.h>

// NOTE Test packet set to PCMU payload

static const unsigned char test_frame[] = 
{
   //RTP header
   0x80, 0,  0x01, 0x00,   0x02, 0x00, 0x00, 0x00,  0x03, 0x00, 0x00, 0x00,

   //Payload
   0x52,0x49,0x46,0x46,0x6C,0x71,0x02,0x00,0x57,0x41,0x56,0x45,
   0x66,0x6D,0x74,0x20,0x10,0x00,0x00,0x00,0x01,0x00,0x01,0x00,
   0x40,0x1F,0x00,0x00,0x80,0x3E,0x00,0x00,0x02,0x00,0x10,0x00,
   0x64,0x61,0x74,0x61,0xFE,0x70,0x02,0x00,0x00,0x00,0xB7,0x33,
   0xAB,0x5E,0x56,0x79,0x6A,0x7F,0x99,0x6F,0xD7,0x4C,0xE2,0x1C,
   0x05,0xE8,0x45,0xB7,0xE6,0x92,0x39,0x81,0x21,0x85,0x15,0x9E,
   0xB7,0xC7,0xFA,0xFA,0x17,0x2F,0x2A,0x5B,0xAD,0x77,0xBC,0x7F,
   0xFC,0x71,0xC6,0x50,0xC2,0x21,0xFA,0xEC,0x75,0xBB,0x9F,0x95,
   0xFD,0x81,0xD3,0x83,0xED,0x9A,0x3E,0xC3,0xF6,0xF5,0x66,0x2A,
   0x8F,0x57,0xD7,0x75,0xE4,0x7F,0x39,0x74,0x96,0x54,0x98,0x26

};



class UtlCryptoTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(UtlCryptoTest);
   CPPUNIT_TEST(testCreate);
   CPPUNIT_TEST(testSymmetricSocket);
   CPPUNIT_TEST_SUITE_END();




   void testCreate()
   {
      UtlCryptoKeyRsa a;
      UtlCryptoKeySym b;
      
   }

   void testSymmetricSocket()
   {
      char buffer[1000];
      char tempMemory[1000];


      UtlCryptoKeySym symKey;
      symKey.generateKey();

      symKey.getBinaryKey(tempMemory, 1000);

      printf("KeyDump:\n%s\n", symKey.output().data());

      int port1 = 10001;
      int port2 = 10011;
      int port3 = 10021;
      int port4 = 10031;

      OsNatDatagramSocketCrypto s1(port2, "127.0.0.1", port1, "127.0.0.1", NULL, tempMemory, 1000, tempMemory, 1000);
      OsNatDatagramSocketCrypto s2(port1, "127.0.0.1", port2, "127.0.0.1", NULL, tempMemory, 1000, tempMemory, 1000);

      OsNatDatagramSocketCrypto s3(port4, "127.0.0.1", port3, "127.0.0.1", NULL, tempMemory, 1000, tempMemory, 1000);
      OsNatDatagramSocket       s4(port3, "127.0.0.1", port4, "127.0.0.1", NULL);

      CPPUNIT_ASSERT_EQUAL(TRUE, s1.isOk());
      CPPUNIT_ASSERT_EQUAL(TRUE, s2.isOk());
      CPPUNIT_ASSERT_EQUAL(TRUE, s3.isOk());
      CPPUNIT_ASSERT_EQUAL(TRUE, s4.isOk());

      s1.write((const char*)test_frame, sizeof(test_frame));

      OsTask::delay(10);

      int i;
      UtlString str;
      int port = 0;
      int len = s2.read((char*)buffer, 1000, &str, &port );
      
      CPPUNIT_ASSERT_EQUAL(len, (int)sizeof(test_frame));
      for (i = 0; i < len; i++)
         CPPUNIT_ASSERT_EQUAL(buffer[i], (char)test_frame[i]);

      s3.write((const char*)test_frame, sizeof(test_frame));
      OsTask::delay(10);

      UtlBoolean different = FALSE;
      len = s4.read((char*)buffer, 1000, &str, &port );
      CPPUNIT_ASSERT(len > 0);

      for (i = 0; i < len; i++) {
         if (buffer[i] != (char)test_frame[i])
         {
            different = TRUE;
            break;
         }
      }
      CPPUNIT_ASSERT_EQUAL(TRUE, different);


   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(UtlCryptoTest);

#endif
