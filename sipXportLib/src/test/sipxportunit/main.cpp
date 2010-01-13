//
//
// Copyright (C) 2010 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2010 SIPez LLC All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////


#include <SipxPortUnitTestEnvironment.h>

int main(int arc, char* argv[])
{
    int result = 0;

    SipxPortUnitTestEnvironment::runTests();

    SipxPortUnitTestEnvironment::reportResults();

    result = SipxPortUnitTestEnvironment::getTestPointFailureCount();
    result += SipxPortUnitTestEnvironment::getTestAbortCount();

    return(result);
}
