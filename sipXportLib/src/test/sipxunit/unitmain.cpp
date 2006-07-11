//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <sipxunit/TestRunner.h>

/**
 * include this in your project and it will find all Suite's that have been
 * registered with the CppUnit TextFactoryRegistry and run them
 */
int main( int argc, char* argv[] )
{
    TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    bool wasSucessful = runner.run();

    return !wasSucessful;
}


