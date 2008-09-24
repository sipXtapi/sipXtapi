//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cppunit/Test.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/Message.h>
#include <cppunit/Exception.h>
#include <cppunit/TestFailure.h>
#include <sipxunit/TestUtilities.h>
#include <sipxunit/TestMonitor.h>

TestMonitor::TestMonitor()
    : CppUnit::TestResultCollector()
{
    m_wasSuccessful = true;
    m_verbose = true;
}

TestMonitor::~TestMonitor()
{
}

void TestMonitor::startTest(CppUnit::Test *test)
{
    // set static context, assumes tests run in serial
    TestUtilities::resetKnownBugTesting();
    if (m_verbose)
    {
        std::cout << "[TEST]:" << test->getName() << std::endl;
    }

    TestResultCollector::startTest(test);
}

void TestMonitor::addFailure(const CppUnit::TestFailure &failure)
{
    // query static context, assumes tests run in serial
    if (!TestUtilities::testingKnownBug())
    {
        m_wasSuccessful = false;
    }

    // Add bug details to final message
    if (strlen(TestUtilities::getKnownBugMessage()) != 0)
    {
        CppUnit::Message msg = failure.thrownException()->message();
        msg.addDetail(TestUtilities::getKnownBugMessage());
        failure.thrownException()->setMessage(msg);
    }

    TestResultCollector::addFailure(failure);
}

bool TestMonitor::wasSuccessful() const
{
    return m_wasSuccessful;
}

