//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _TestOutputter_h_
#define _TestOutputter_h_

#include <iostream>
#include <cppunit/Outputter.h>
#include <cppunit/TextOutputter.h>

class CppUnit::TestResultCollector;

/**
 * Handle actually printing to stdout, stderr and report file
 */
class TestOutputter : public CppUnit::TextOutputter
{
 public:

    TestOutputter(CppUnit::TestResultCollector *collector);

    virtual ~TestOutputter();

 protected:

    void printFailures();

    void printFailureWarning();

 private:
    
    CppUnit::TextOutputter *m_err;
};

#endif
