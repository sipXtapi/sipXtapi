//
// Copyright (C) 2006 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/HttpMessage.h>
#include <net/SdpBody.h>
#include <utl/UtlString.h>

// Extract the contents of an HttpBody, with the Content-Type header prepended,
// and the boundary string replaced with "[boundary]".
UtlString extract_contents(HttpBody* body);

/**
 * Unittest for HttpBody
 */
class HttpBodyTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(HttpBodyTest);
   CPPUNIT_TEST(testCreateMultipart);
   CPPUNIT_TEST_SUITE_END();

public:

   void testCreateMultipart()
      {
         HttpBodyMultipart body("multipart/related");

         ASSERT_STR_EQUAL_MESSAGE("Zero body parts",
                                  "Content-Type: multipart/related;"
                                      "boundary=\"[boundary]\"\r\n"
                                  "\r\n"
                                  "--[boundary]--\r\n",
                                  extract_contents(&body).data());
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(HttpBodyTest);

// Extract the contents of an HttpBody.
UtlString extract_contents(HttpBody* body)
{
   UtlString res;

   res.append("Content-Type: ");
   res.append(body->getContentType());
   res.append("\r\n\r\n");
   res.append(body->getBytes());

   // Replace the boundary string with "[boundary]".
   const char* boundary_string = body->getMultipartBoundary();
   size_t location;
   while ((location = res.index(boundary_string)) != UTL_NOT_FOUND)
   {
      res.replace(location, strlen(boundary_string),
                  "[boundary]");
   }

   return res;
}
