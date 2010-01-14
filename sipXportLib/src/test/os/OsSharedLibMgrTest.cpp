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
#include <os/OsStatus.h>
#include <os/OsSharedLibMgr.h>
#include <sipxunittests.h>

#if defined WIN32 && defined WINCE
#define LIB_NAME        "coredll.dll"
#elif defined WIN32
#define LIB_NAME        "kernel32.dll"
#elif defined __linux__
#define LIB_NAME        "libm.so.6"
#elif defined __APPLE__
#define LIB_NAME        "libm.dylib"
#elif defined ANDROID
#define LIB_NAME        "libm.so"
#else
#error Please define LIB_NAME for your platform
#endif

class OsSharedLibMgrTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(OsSharedLibMgrTest);
    CPPUNIT_TEST(testLoadUnload);
    CPPUNIT_TEST_SUITE_END();

public:

    void testLoadUnload()
    {
        OsStatus res;
        OsSharedLibMgrBase* pLibMgr = OsSharedLibMgr::getOsSharedLibMgr();
        CPPUNIT_ASSERT(pLibMgr != NULL);

        res = pLibMgr->loadSharedLib(LIB_NAME);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pLibMgr->loadSharedLib(LIB_NAME);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pLibMgr->unloadSharedLib(LIB_NAME);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pLibMgr->unloadSharedLib(LIB_NAME);
        CPPUNIT_ASSERT(res == OS_INVALID);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsSharedLibMgrTest);
