//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

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


