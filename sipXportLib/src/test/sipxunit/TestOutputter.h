//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

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

    void TestOutputter::printFailures();

    void TestOutputter::printFailureWarning();

 private:
    
    CppUnit::TextOutputter *m_err;
};

#endif
