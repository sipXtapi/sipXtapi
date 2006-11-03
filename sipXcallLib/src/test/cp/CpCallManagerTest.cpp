// 
// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <cp/CpCallManager.h>
#include <utl/UtlString.h>

/**
 * Unit tests for CpCallManager.
 */
class CpCallManangerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(CpCallManangerTest);

    CPPUNIT_TEST(testCallId);

    CPPUNIT_TEST_SUITE_END();

public:

    void testCallId()
    {
#define CASES 10
#define MIN_DIFFS 4
       UtlString output[CASES];

       // Generate some call-Ids.
       for (int i = 0; i < sizeof (output) / sizeof (output[0]); i++)
       {
          CpCallManager::getNewCallId("t", &output[i]);
          // Enable this statement if you want to see some sample values
          // from getNewCallId.
          #if 0
            fprintf(stderr, "%s\n", output[i].data());
          #endif
       }

       // Compare that they're different enough.
       for (int i = 0; i < sizeof (output) / sizeof (output[0]); i++)
       {
          for (int j = i+1; j < sizeof (output) / sizeof (output[0]); j++)
          {
             UtlString* s1 = &output[i];
             UtlString* s2 = &output[j];
             
             int differences = 0;
             for (int k = 0; k < s1->length() && k < s1->length(); k++)
             {
                if ((*s1)(k) != (*s2)(k))
                {
                   differences++;
                }
             }
             if (differences < MIN_DIFFS)
             {
                char msg[200];
                sprintf(msg,
                        "Call-IDs '%s' and '%s' have %d different characters, "
                        "which is less than the minimum, %d",
                        s1->data(), s2->data(), differences, MIN_DIFFS);
                CPPUNIT_ASSERT_MESSAGE(msg, FALSE);
             }
          }
       }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CpCallManangerTest);
