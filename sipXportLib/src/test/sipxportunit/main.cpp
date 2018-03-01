//
//
// Copyright (C) 2010-2018 SIPez LLC All rights reserved.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////


#include <SipxPortUnitTestEnvironment.h>

void usage(const char* name)
{
    printf("Usage:\n"
           "\t%s [--help | --list | [--nocatchsignals] testclassname[::testmethodname]]\n"
           "Where:\n"
           "\t--help provides this usage help\n"
           "\t--nocatchsignals indicates to abort when signals are thrown.  Default\n"
           "\t\tbehavior is to catch signals (e.g. SEGV) and try to continue with next test.\n"
           "\ttestclassname is the name of the unit test class to run\n"
           "\ttestmethodname is the name of the method to run (member of testclassname)\n",
           name);
}

int main(int argc, char* argv[])
{
    int testNameArgIndex = 1;
    int result = 0;
    if(argc > 1)
    {
       if(strcmp(argv[1], "--help") == 0)
        {
            usage(argv[0]);
            result = -1;
        }

       else if(strcmp(argv[1], "--list") == 0)
        {
            SipxPortUnitTestEnvironment::listTests(argc > 2 ? argv[2] : "");
            result = -2;
        }

        // Default is to catch signals
        else if(strcmp(argv[1], "--nocatchsignals") == 0)
        {
            SipxPortUnitTestEnvironment::setCatchSignals(false);
            testNameArgIndex++;
        }

        else if(argv[1][0] == '-')
        {
            printf("Unknown argument: %s\n", argv[1]);
            usage(argv[0]);
            result = -1;
        }

        //printf("argv[1]: %s\n", argv[1]);
    }


    if(result == 0)
    {
        SipxPortUnitTestEnvironment::runTests(argc > testNameArgIndex ? argv[testNameArgIndex] : "");

        SipxPortUnitTestEnvironment::reportResults();

        result = SipxPortUnitTestEnvironment::getTestPointFailureCount();
        result += SipxPortUnitTestEnvironment::getTestAbortCount();
    }

    return(result);
}
