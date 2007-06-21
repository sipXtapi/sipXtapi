//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpMMTimer.h>
#include <os/OsDateTime.h>
#include <os/OsSysLog.h>

#ifdef WIN32
#include <windows.h>
#include <mp/MpMMTimerWnt.h>
#endif

/**
 * Unittest for MpMMTimer and its successors
 */
class MpMMTimerTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpMMTimerTest);
   CPPUNIT_TEST(testLinearTimer);
   CPPUNIT_TEST_SUITE_END();


public:
   void setUp()
   {
   }

   void tearDown()
   {
   }

   void testLinearTimer()
   {
      MpMMTimer* pMMTimer = NULL;

#ifndef WIN32
      // Right now MMTimers are only implemented for win32.
      // as other platforms are implemented, change this.
      printf("MMTimer not implemented for this platform.  Test disabled.\n");
      return;
#endif

#ifdef WIN32
      MpMMTimerWnt mmTimerWnt(MpMMTimer::Linear);
      pMMTimer = &mmTimerWnt;
#else
      printf("MpMMTimerTest::testLinearTimer Not implemented on this platform!");
      return;
#endif

      OsTime t;
      unsigned resolution;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->getResolution(resolution));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->run(resolution));
#ifdef WIN32
      LARGE_INTEGER perfFreqPerSec;
      CPPUNIT_ASSERT(QueryPerformanceFrequency(&perfFreqPerSec) > 0);
      //printf("Performance frequency is %I64d ticks per sec\n", 
      //       perfFreqPerSec.QuadPart);
      // Convert it to per usec instead of per sec.
      double perfFreqPerUSec = double(perfFreqPerSec.QuadPart) / double(1000000.0);
      //printf("Performance frequency is %f ticks per usec\n", 
      //       perfFreqPerUSec);
      LARGE_INTEGER perfCount[50];
#endif
      int i;
      for(i = 0; i < 50; i++)
      {
#ifdef WIN32
         CPPUNIT_ASSERT(QueryPerformanceCounter(&perfCount[i]) > 0);
#endif
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->waitForNextTick());
      }

#ifdef WIN32
      __int64 delta;
      for(i = 0; i < 50; i = i+2)
      {
         delta = __int64(perfCount[i+1].QuadPart / perfFreqPerUSec) - 
                 __int64(perfCount[i].QuadPart / perfFreqPerUSec);

         printf("fireDeltaUSecs (%d-%d) == %I64d usec\n", 
                i, i+1, delta);
      }
#endif

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMMTimer->stop());
   }

protected:

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpMMTimerTest);
