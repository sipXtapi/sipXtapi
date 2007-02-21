// 
// Copyright (C) 2007 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2007 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
//////////////////////////////////////////////////////////////////////////////

#include <string>

#ifdef __pingtel_on_posix__
#  include <unistd.h>
#endif

#include "TestOsSysLogListener.h"
#include <cppunit/Test.h>
#include "os/OsSysLog.h"

/// constructor
TestOsSysLogListener::TestOsSysLogListener()
{
};

void TestOsSysLogListener::startTest( CPPUNIT_NS::Test *test ) 
{ 
   std::string testName = test->getName();
   
   size_t colon_pos = 0;
   while (std::string::npos != (colon_pos = testName.find(":", colon_pos)))
   {
      testName.replace(colon_pos, 1, "_");
   }
   
   OsSysLog::initialize(0,"UnitTest");
   OsSysLog::setLoggingPriority(PRI_DEBUG);

   std::string testLogFile = testName + ".log";

   unlink(testLogFile.c_str());

   OsSysLog::setOutputFile(0,testLogFile.c_str());
}  

void TestOsSysLogListener::endTest( CPPUNIT_NS::Test *test ) 
{ 
   OsSysLog::flush();
}  

/// destructor
TestOsSysLogListener::~TestOsSysLogListener()
{
};
