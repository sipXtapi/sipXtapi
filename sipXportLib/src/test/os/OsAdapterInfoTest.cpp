//
// Copyright (C) 2007-2017 SIPez LLC  All rights reserved.
//
// Copyright (C) 2004-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2009 SIPez LLC  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <utl/UtlDefs.h>
#include <os/linux/AdapterInfo.h>
#include <os/HostAdapterAddress.h>
#include <sipxunittests.h>

#define MAX_A	64
class OsAdapterInfoTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(OsAdapterInfoTest);
    CPPUNIT_TEST(testCount);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCount()
    {
	const HostAdapterAddress *adapters[MAX_A];
	int numAddrs = MAX_A;
	bool ret = getAllLocalHostIps(adapters, numAddrs);
	CPPUNIT_ASSERT(ret == true);
	CPPUNIT_ASSERT(numAddrs > 0);
	
	printf ("Found %d interface(s)\n", numAddrs);
	for (int i = 0; i < numAddrs; i++)
	{
	    printf("[%d] %s: %s\n", i, adapters[i]->mAdapter.data(), adapters[i]->mAddress.data()); 
	}
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsAdapterInfoTest);

