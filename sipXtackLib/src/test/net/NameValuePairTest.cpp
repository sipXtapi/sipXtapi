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

#include <net/NameValuePair.h>

class TestTemplate : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(TestTemplate);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST_SUITE_END();


public:
    void testAccessors()
    {
        const char* name = "xxx";
        const char* value = "xxx-value";
        const char* valueRef;

        NameValuePair* nv = new NameValuePair(name);

        valueRef = nv->getValue();
        CPPUNIT_ASSERT_MESSAGE("value should be null", NULL == valueRef);

        nv->setValue(value);
        valueRef = nv->getValue();
        ASSERT_STR_EQUAL_MESSAGE("incorrect value retrieved", value, valueRef);

        nv->setValue(name);
        valueRef = nv->getValue();
        ASSERT_STR_EQUAL_MESSAGE("incorrect value retrieved", valueRef, name);

        delete nv;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTemplate);
