//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <utl/UtlDefs.h>
#include <os/OsCallback.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

/** Flag that callback function was called */
UtlBoolean gCallbackCalled;

void setCallbackFlag(const int userData, const int eventData)
{
    gCallbackCalled = TRUE;
}

class OsCallbackTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsCallbackTest);
    CPPUNIT_TEST(testCallback);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCallback()
    {
        OsCallback* pCallback;

        pCallback = new OsCallback(12345, setCallbackFlag);
        gCallbackCalled = FALSE;
        pCallback->signal(67890);
        CPPUNIT_ASSERT(gCallbackCalled);
        delete pCallback;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsCallbackTest);

