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

#include "utl/UtlRandom.h"
#include "os/OsDateTime.h"
#include "os/OsMsgDispatcher.h"
#include "os/OsSysLog.h"

/**
 * Unittest for OsMsgDispatcher
 */
class OsMsgDispatcherTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(OsMsgDispatcherTest);
   CPPUNIT_TEST(testSingleMessage);
   CPPUNIT_TEST(testFullDispatcher);
   CPPUNIT_TEST_SUITE_END();


public:
   void setUp() 
   {
      OsTime t;
      OsDateTime::getCurTime(t);
      mpRandGenerator = new UtlRandom((int)t.cvtToMsecs());
   }
   void tearDown() 
   {
      delete mpRandGenerator;
      mpRandGenerator = NULL;
   }

   void testSingleMessage()
   {
      OsMsg msg(1, (char)mpRandGenerator->rand());
      OsMsg* pReceivedMsg = NULL;

      OsMsgDispatcher dispatcher;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, dispatcher.post(msg));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, dispatcher.receive(pReceivedMsg));
      CPPUNIT_ASSERT(pReceivedMsg != NULL);
      //CPPUNIT_ASSERT_EQUAL(&msg, pReceivedMsg); // This may not be true, as queue may make copy.
      CPPUNIT_ASSERT_EQUAL(msg.getMsgSubType(), pReceivedMsg->getMsgSubType());

      delete pReceivedMsg;
   }

   void testFullDispatcher()
   {
      OsMsgDispatcher dispatcher;
      OsMsg** pMsgArray = new OsMsg*[dispatcher.maxMsgs()+1];
      int i;
      for(i = 0; i < dispatcher.maxMsgs() + 1; i++)
      {
         pMsgArray[i] = new OsMsg(1, (char)mpRandGenerator->rand());
      }

      // Fill the dispatcher till it is just full, no more.
      for(i = 0; i < dispatcher.maxMsgs(); i++)
      {
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, dispatcher.post(*pMsgArray[i]));
      }

      // Now, the prime assertion for this test -- the next message pushed
      // should fail having reached the limit of the queue.
      CPPUNIT_ASSERT_EQUAL(OS_LIMIT_REACHED, 
                           dispatcher.post(*pMsgArray[dispatcher.maxMsgs()]));

      // Check all the messages in the queue to make sure that they start with
      // the contents of pMsgArray[1], all up through pMsgArray[maxMsgs-1], not
      // including that last one that was tried to be pushed.
      OsMsg* pCurRMsg = NULL;
      for(i = 0; i < dispatcher.maxMsgs(); i++)
      {
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, dispatcher.receive(pCurRMsg));
         CPPUNIT_ASSERT(pCurRMsg != NULL);
         CPPUNIT_ASSERT_EQUAL(pMsgArray[i]->getMsgSubType(), pCurRMsg->getMsgSubType());
         delete pCurRMsg;
         pCurRMsg = NULL;
      }

      // Now, if we try to receive one more, with a timeout, it is expected to 
      // timeout and return without the message.
      CPPUNIT_ASSERT_EQUAL(OS_WAIT_TIMEOUT, dispatcher.receive(pCurRMsg, OsTime(50)));

      // Cleanup dynamic array.
      for(i = 0; i < dispatcher.maxMsgs() + 1; i++)
      {
         delete pMsgArray[i];
      }
      delete[] pMsgArray;
   }

protected:
   UtlRandom* mpRandGenerator;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsMsgDispatcherTest);
