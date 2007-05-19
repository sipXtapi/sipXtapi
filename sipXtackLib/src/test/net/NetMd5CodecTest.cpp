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

#include <os/OsDefs.h>
#include <net/NetMd5Codec.h>

/**
 * Unittest for NetMd5Codec
 */
class NetMd5CodecTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(NetMd5CodecTest);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST_SUITE_END();


public:
    void testManipulators()
    {
        const char* a1 = "john.salesman:sales@www/example.com:5+5=10";
        const char* a2 = "GET:/private/prices.html";
        const char* a1Encoded = "806d252e3788478d0cebb3c079f515bc";
        const char* a2Encoded = "254bd53db6966fa1387fa1973bb5e53c";

        UtlString a1EncodedString;
        UtlString a2EncodedString;
                                                                                
        NetMd5Codec::encode(a1, a1EncodedString);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("md5 encode test 1", 
            0, a1EncodedString.compareTo(a1Encoded));
                                                                                
        NetMd5Codec::encode(a2, a2EncodedString);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("md5 encode test 2", 
            0, a2EncodedString.compareTo(a2Encoded));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetMd5CodecTest);
