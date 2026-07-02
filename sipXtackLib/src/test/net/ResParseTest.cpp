//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////

#include <sipxunittests.h>

#include <string.h>

#include <os/OsDefs.h>
#include "resparse/rr.h"

// These tests drive res_parse() directly with hand-built, wire-format DNS
// responses.  Unlike SipSrvLookupTest (which needs a live 'named' and only
// exercises well-formed responses through the resolver), these run on every
// platform with no external dependencies and cover the buffer-bounds handling
// added to guard against crafted responses -- inflated record counts and
// rdata lengths that would otherwise drive the parser past the end of the
// receive buffer (CWE-125).

/**
 * Unit test for the DNS response parser (res_parse / parse_rr).
 */
class ResParseTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(ResParseTest);
   CPPUNIT_TEST(testValidARecord);
   CPPUNIT_TEST(testValidSrvRecordCompressedTarget);
   CPPUNIT_TEST(testTruncatedHeader);
   CPPUNIT_TEST(testInflatedAnswerCount);
   CPPUNIT_TEST(testOversizedRdlength);
   CPPUNIT_TEST(testTruncatedRdata);
   CPPUNIT_TEST_SUITE_END();

public:

   // The parser assumes the on-the-wire 12-byte DNS header layout, and the
   // hand-built buffers below place the question immediately after it.  Guard
   // that assumption so a surprising HEADER layout fails loudly rather than
   // silently misparsing.
   void setUp()
   {
      CPPUNIT_ASSERT_EQUAL((size_t) 12, sizeof(HEADER));
   }

   // A well-formed response carrying a single A record for "test" -> 1.2.3.4.
   // Regression coverage: the bounds checks must not disturb normal parsing.
   void testValidARecord()
   {
      unsigned char msg[] = {
         // Header
         0x12, 0x34,             // ID
         0x81, 0x80,             // flags: response, recursion available
         0x00, 0x01,             // QDCOUNT = 1
         0x00, 0x01,             // ANCOUNT = 1
         0x00, 0x00,             // NSCOUNT = 0
         0x00, 0x00,             // ARCOUNT = 0
         // Question: "test" IN A
         0x04, 't', 'e', 's', 't', 0x00,
         0x00, 0x01,             // QTYPE  = A
         0x00, 0x01,             // QCLASS = IN
         // Answer: "test" IN A, ttl 3600, 1.2.3.4
         0x04, 't', 'e', 's', 't', 0x00,
         0x00, 0x01,             // TYPE  = A
         0x00, 0x01,             // CLASS = IN
         0x00, 0x00, 0x0E, 0x10, // TTL   = 3600
         0x00, 0x04,             // RDLENGTH = 4
         0x01, 0x02, 0x03, 0x04  // RDATA = 1.2.3.4
      };

      res_response* resp = res_parse((char*) msg, (int) sizeof(msg));
      CPPUNIT_ASSERT(resp != NULL);
      CPPUNIT_ASSERT_EQUAL(1, (int) resp->header.ancount);
      CPPUNIT_ASSERT(resp->answer != NULL);
      CPPUNIT_ASSERT(resp->answer[0] != NULL);
      CPPUNIT_ASSERT_EQUAL((int) T_A, (int) resp->answer[0]->type);

      const unsigned char* addr =
         (const unsigned char*) &resp->answer[0]->rdata.address;
      CPPUNIT_ASSERT(addr[0] == 1 && addr[1] == 2 &&
                     addr[2] == 3 && addr[3] == 4);

      res_free(resp);
   }

   // A well-formed SRV record whose target is a compression pointer back to
   // the question name.  Exercises the _getshort reads and the
   // compression-pointer-following name expansion (dn_expand) inside parse_rr,
   // all bounded by the end of the message.
   void testValidSrvRecordCompressedTarget()
   {
      unsigned char msg[] = {
         // Header
         0x12, 0x34,             // ID
         0x81, 0x80,             // flags
         0x00, 0x01,             // QDCOUNT = 1
         0x00, 0x01,             // ANCOUNT = 1
         0x00, 0x00,             // NSCOUNT = 0
         0x00, 0x00,             // ARCOUNT = 0
         // Question at offset 12: "test" IN SRV
         0x04, 't', 'e', 's', 't', 0x00,
         0x00, 0x21,             // QTYPE  = SRV (33)
         0x00, 0x01,             // QCLASS = IN
         // Answer: name is a pointer to the question name at offset 12
         0xC0, 0x0C,             // NAME -> offset 12
         0x00, 0x21,             // TYPE  = SRV
         0x00, 0x01,             // CLASS = IN
         0x00, 0x00, 0x0E, 0x10, // TTL   = 3600
         0x00, 0x08,             // RDLENGTH = 8
         0x00, 0x0A,             // priority = 10
         0x00, 0x05,             // weight   = 5
         0x13, 0x88,             // port     = 5000
         0xC0, 0x0C              // target -> offset 12 ("test")
      };

      res_response* resp = res_parse((char*) msg, (int) sizeof(msg));
      CPPUNIT_ASSERT(resp != NULL);
      CPPUNIT_ASSERT_EQUAL(1, (int) resp->header.ancount);
      CPPUNIT_ASSERT(resp->answer[0] != NULL);
      CPPUNIT_ASSERT_EQUAL((int) T_SRV, (int) resp->answer[0]->type);
      CPPUNIT_ASSERT_EQUAL(10,   (int) resp->answer[0]->rdata.srv.priority);
      CPPUNIT_ASSERT_EQUAL(5,    (int) resp->answer[0]->rdata.srv.weight);
      CPPUNIT_ASSERT_EQUAL(5000, (int) resp->answer[0]->rdata.srv.port);
      CPPUNIT_ASSERT(resp->answer[0]->rdata.srv.target != NULL);
      CPPUNIT_ASSERT(strncmp(resp->answer[0]->rdata.srv.target, "test", 4) == 0);

      res_free(resp);
   }

   // A message shorter than the fixed 12-byte header must be rejected rather
   // than memcpy'd out of bounds.
   void testTruncatedHeader()
   {
      unsigned char msg[] = { 0x12, 0x34, 0x81, 0x80, 0x00, 0x01, 0x00, 0x00 };
      CPPUNIT_ASSERT(res_parse((char*) msg, (int) sizeof(msg)) == NULL);
   }

   // ANCOUNT claims far more answer records than the buffer contains.  The
   // parser must stop at the end of the message and fail, not read past it.
   void testInflatedAnswerCount()
   {
      unsigned char msg[] = {
         0x12, 0x34,
         0x81, 0x80,
         0x00, 0x01,             // QDCOUNT = 1
         0x00, 0x10,             // ANCOUNT = 16 (only one record present)
         0x00, 0x00,
         0x00, 0x00,
         0x04, 't', 'e', 's', 't', 0x00,
         0x00, 0x01,
         0x00, 0x01,
         0x04, 't', 'e', 's', 't', 0x00,
         0x00, 0x01,
         0x00, 0x01,
         0x00, 0x00, 0x0E, 0x10,
         0x00, 0x04,
         0x01, 0x02, 0x03, 0x04
      };
      CPPUNIT_ASSERT(res_parse((char*) msg, (int) sizeof(msg)) == NULL);
   }

   // RDLENGTH claims more rdata than remains in the message.  parse_rr must
   // reject the record rather than let a handler read past the buffer.
   void testOversizedRdlength()
   {
      unsigned char msg[] = {
         0x12, 0x34,
         0x81, 0x80,
         0x00, 0x01,
         0x00, 0x01,
         0x00, 0x00,
         0x00, 0x00,
         0x04, 't', 'e', 's', 't', 0x00,
         0x00, 0x01,
         0x00, 0x01,
         0x04, 't', 'e', 's', 't', 0x00,
         0x00, 0x01,
         0x00, 0x01,
         0x00, 0x00, 0x0E, 0x10,
         0x00, 0x40,             // RDLENGTH = 64, but only 4 rdata bytes follow
         0x01, 0x02, 0x03, 0x04
      };
      CPPUNIT_ASSERT(res_parse((char*) msg, (int) sizeof(msg)) == NULL);
   }

   // The message is truncated in the middle of an A record's rdata: RDLENGTH
   // is a valid 4 but only 2 rdata bytes are actually present (the declared
   // length runs past the end of the message).
   void testTruncatedRdata()
   {
      unsigned char msg[] = {
         0x12, 0x34,
         0x81, 0x80,
         0x00, 0x01,
         0x00, 0x01,
         0x00, 0x00,
         0x00, 0x00,
         0x04, 't', 'e', 's', 't', 0x00,
         0x00, 0x01,
         0x00, 0x01,
         0x04, 't', 'e', 's', 't', 0x00,
         0x00, 0x01,
         0x00, 0x01,
         0x00, 0x00, 0x0E, 0x10,
         0x00, 0x04,             // RDLENGTH = 4
         0x01, 0x02              // ... but only 2 bytes present
      };
      CPPUNIT_ASSERT(res_parse((char*) msg, (int) sizeof(msg)) == NULL);
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ResParseTest);
