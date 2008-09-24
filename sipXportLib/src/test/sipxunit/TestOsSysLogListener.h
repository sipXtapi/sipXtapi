// 
// Copyright (C) 2007 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2007 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _TESTOSSYSLOGLISTENER_H_
#define _TESTOSSYSLOGLISTENER_H_

// SYSTEM INCLUDES
#include <utl/UtlString.h>
#include <cppunit/Portability.h>
#include <cppunit/TestListener.h>

// APPLICATION INCLUDES

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Test;

/// One line description
/**
 * Long description
 */
class TestOsSysLogListener : public CPPUNIT_NS::TestListener 
{
  public:
   /*! Constructs a TextTestProgressListener object.
    */
   TestOsSysLogListener();

   /// Destructor.
   virtual ~TestOsSysLogListener();

   static UtlString getLogFilename(const UtlString& testName);

   virtual void startTest( CPPUNIT_NS::Test *test );

   virtual void endTest( CPPUNIT_NS::Test *test );

  private:

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   TestOsSysLogListener(const TestOsSysLogListener& nocopyconstructor);

   /// There is no assignment operator.
   TestOsSysLogListener& operator=(const TestOsSysLogListener& noassignmentoperator);
   // @endcond     
};

#endif // _TESTOSSYSLOGLISTENER_H_
