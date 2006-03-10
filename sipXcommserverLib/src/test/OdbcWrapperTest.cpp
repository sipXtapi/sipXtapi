// 
// Copyright (C) 2006 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "odbc/OdbcWrapper.h"

// DEFINES
#define ODBC_LOGGING
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class OdbcWrapperTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(OdbcWrapperTest);
   CPPUNIT_TEST(testOdbcConnect);
   CPPUNIT_TEST(testOdbcExecute);
   CPPUNIT_TEST(testOdbcValidateData);
   CPPUNIT_TEST_SUITE_END();

public:

   void setUp()
      {
#ifdef ODBC_LOGGING         
         OsSysLog::initialize(0, "odbc");
         OsSysLog::setOutputFile(0, "odbcTest.log");
         OsSysLog::setLoggingPriority(PRI_DEBUG);   
#endif
      }
      
   void tearDown()
      {
#ifdef ODBC_LOGGING         
         OsSysLog::flush();
#endif         
      }
      
   void testOdbcConnect()
      {
         OdbcHandle handle = NULL;
       
         CPPUNIT_ASSERT((handle=odbcConnect("SIPXCDR",
                                            "localhost",
                                            "postgres",
                                            "{PostgreSQL}"))!=NULL);
         
         if (handle)
         {
            CPPUNIT_ASSERT(odbcDisconnect(handle));
         }
      }
      
   void testOdbcExecute()
      {
         OdbcHandle handle = NULL;
         char sqlStatement[256];
        
         CPPUNIT_ASSERT((handle=odbcConnect("SIPXCDR",
                                            "localhost",
                                            "postgres",
                                            "{PostgreSQL}"))!=NULL);
         
         if (handle)
         {
            // Clear database
            sprintf(sqlStatement, "DELETE FROM call_state_events *;");
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));
            sprintf(sqlStatement, "DELETE FROM call_state_observer_events *;");
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));            
            
            sprintf(sqlStatement,
                    "INSERT INTO call_state_observer_events VALUES (DEFAULT,"
                    "\'10.1.20.3:5060\',"
                    "0,"
                    "timestamp \'2006-03-10 12:59:00.666\',"
                    "\'R\',"
                    "101,"
                    "\'AuthProxyCseObserver\',"
                    "\'http://www.sipfoundry.org/sipX/scgema/xml/cse-01-00\');");
                 
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));
       
            sprintf(sqlStatement,
                    "INSERT INTO call_state_events VALUES (DEFAULT,"
                    "\'10.1.20.3:5060\',"
                    "1,"
                    "timestamp \'2006-03-10 13:00:00.123\',"
                    "\'R\',"
                    "\'call-111111\',"
                    "\'12345\',"
                    "\'67890\',"
                    "\'sip:153@example.com\',"
                    "\'sip:202@example.com\',"
                    "\'10.1.1.71\',"
                    "\'refer-header\',"
                    "0,"
                    "\'No Reason\');");
                 
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));
            
            sprintf(sqlStatement,
                    "INSERT INTO call_state_events VALUES (DEFAULT,"
                    "\'10.1.20.3:5060\',"
                    "2,"
                    "timestamp \'2006-03-10 13:00:10.573\',"
                    "\'S\',"
                    "\'call-111111\',"
                    "\'12345\',"
                    "\'67890\',"
                    "\'sip:153@example.com\',"
                    "\'sip:202@example.com\',"
                    "\'10.1.1.71\',"
                    "\'refer-header\',"
                    "0,"
                    "\'No Reason\');");
                 
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));            
       
            CPPUNIT_ASSERT(odbcDisconnect(handle));
         }
      }
   
   void testOdbcValidateData()
      {
         OdbcHandle handle = NULL;
         char sqlStatement[256];
      
         CPPUNIT_ASSERT((handle=odbcConnect("SIPXCDR",
                                            "localhost",
                                            "postgres",
                                            "{PostgreSQL}"))!=NULL);
         
         if (handle)
         {
            int cols;            
            
            // Get data from call_state_observer_events
            sprintf(sqlStatement, "SELECT * FROM call_state_observer_events;");
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));
            
            CPPUNIT_ASSERT((cols=odbcResultColumns(handle)) == 8);
            
            CPPUNIT_ASSERT(odbcGetNextRow(handle));
            
            char buffer[256];
            
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 2, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "10.1.20.3:5060") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 3, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "0") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 4, buffer, 256));
            // When returning a timestamp as string data the fractional component
            // is truncated - this is expected behavior
            CPPUNIT_ASSERT(strcmp(buffer, "2006-03-10 12:59:00") == 0);            
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 5, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "R") == 0);            
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 6, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "101") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 7, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "AuthProxyCseObserver") == 0);    
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 8, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, 
                           "http://www.sipfoundry.org/sipX/scgema/xml/cse-01-00") == 0); 
                          
            // End of rows 
            CPPUNIT_ASSERT(!odbcGetNextRow(handle));
            
            CPPUNIT_ASSERT(odbcClearResultSet(handle));
            
            // Get data from call_state_events
            sprintf(sqlStatement, "SELECT * FROM call_state_events;");
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));
            
            CPPUNIT_ASSERT((cols=odbcResultColumns(handle)) == 14);            
            
            CPPUNIT_ASSERT(odbcDisconnect(handle));     
         }
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OdbcWrapperTest);

