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
           "\t%s [--help | --nocatchsignals | testclassname]\n"
           "Where:\n"
           "\t--help provides this usage help\n"
           "\t--nocatchsignals indicates to abort when signals are thrown.  Default\n"
           "\t\tbehavior is to catch signals (e.g. SEGV) and try to continue with next test.\n"
           "\ttestclassname is the name of a unit test class\n",
           name);
}

int main(int argc, char* argv[])
{
    int testNameArgIndex = 1;
    if(argc > 1 && strcmp(argv[1], "--help") == 0)
    {
        usage(argv[0]);
        return(-1);
    }

    // Default is to catch signals
    if(argc > 1 && strcmp(argv[1], "--nocatchsignals") == 0)
    {
        SipxPortUnitTestEnvironment::setCatchSignals(false);
        testNameArgIndex++;
    }

    int result = 0;

    SipxPortUnitTestEnvironment::runTests(argc > testNameArgIndex ? argv[testNameArgIndex] : "");

    SipxPortUnitTestEnvironment::reportResults();

    result = SipxPortUnitTestEnvironment::getTestPointFailureCount();
    result += SipxPortUnitTestEnvironment::getTestAbortCount();

    return(result);
}
