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
#include "os/OsFileSystem.h"

/// constructor
TestOsSysLogListener::TestOsSysLogListener()
{
};

UtlString TestOsSysLogListener::getLogFilename(UtlString testName)
{
   std::string tn(testName.data());
   
   size_t colon_pos = 0;
   while (std::string::npos != (colon_pos = tn.find(":", colon_pos)))
   {
      tn.replace(colon_pos, 1, "_");
   }
   UtlString fn(tn.c_str());
   fn.append(".log");
   return fn;
}

void TestOsSysLogListener::startTest( CPPUNIT_NS::Test *test ) 
{ 
   UtlString testLogFile = getLogFilename(test->getName().c_str());
   
   OsSysLog::initialize(0,"UnitTest");
   OsSysLog::setLoggingPriority(PRI_DEBUG);

   OsFileSystem::remove(testLogFile, FALSE, TRUE);
   OsSysLog::setOutputFile(0,testLogFile);
}  

void TestOsSysLogListener::endTest( CPPUNIT_NS::Test *test ) 
{ 
   OsSysLog::flush();

   // Remove the log file if it is an empty one.
   UtlString testLogFile = getLogFilename(test->getName().c_str());
   OsPath testLogFilePath(testLogFile);
   OsFileInfo tLogFInfo;
   OsStatus stat = OsFileSystem::getFileInfo(testLogFilePath, tLogFInfo);
   if (stat == OS_SUCCESS)
   { 
      unsigned long logSz = 0;
      stat = tLogFInfo.getSize(logSz);
      if (stat == OS_SUCCESS && (logSz == 0))
      {
         stat = OsFileSystem::remove(testLogFile, FALSE, TRUE);
         if (stat != OS_SUCCESS)
         {
            printf("Failed to remove file %s\n", testLogFile.data());
         }
      }
   }
}  

/// destructor
TestOsSysLogListener::~TestOsSysLogListener()
{
};
