//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _TestRunner_h_
#define _TestRunner_h_

#include <cppunit/TestRunner.h>
#include <sipxunit/TestOutputter.h>

class CppUnit::TestResult;
class TestMonitor;

/**
 * Instantiates nec. objects for running a test suite for main method
 */
class TestRunner : public CppUnit::TestRunner
{
 public:

    TestRunner();

    virtual ~TestRunner();

    bool run();

 private:

    TestMonitor *m_monitor;

    CppUnit::TestResult *m_result;

    TestOutputter *m_outputter;
};

#endif

