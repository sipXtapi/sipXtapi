//
//
// Copyright (C) 2010-2014 SIPez LLC All rights reserved.
//
// Copyright (C) 2010 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////


#include <SipxPortUnitTestEnvironment.h>

void usage(const char* name)
{
    printf("Usage:\n"
           "\t%s [--help | testclassname]\n"
           "Where:\n"
           "\ttestclassname is the name of a unit test class\n",
           name);
}

int main(int argc, char* argv[])
{
    if(argc > 0 && strcmp(argv[1], "--help") == 0)
    {
        usage(argv[0]);
        return(-1);
    }

    int result = 0;

    SipxPortUnitTestEnvironment::runTests(argc > 0 ? argv[1] : "");

    SipxPortUnitTestEnvironment::reportResults();

    result = SipxPortUnitTestEnvironment::getTestPointFailureCount();
    result += SipxPortUnitTestEnvironment::getTestAbortCount();

    return(result);
}
