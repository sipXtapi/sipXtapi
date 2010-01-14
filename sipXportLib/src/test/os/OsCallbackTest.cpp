//
// Copyright (C) 2007-2010 SIPez LLC  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
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
#include <sipxunittests.h>


/** Flag that callback function was called */
UtlBoolean gCallbackCalled;

void setCallbackFlag(const intptr_t userData, const intptr_t eventData)
{
    gCallbackCalled = TRUE;
}

class OsCallbackTest : public SIPX_UNIT_BASE_CLASS
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

