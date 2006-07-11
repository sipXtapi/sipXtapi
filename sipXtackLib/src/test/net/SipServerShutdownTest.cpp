//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <os/OsTimerTask.h>
#include <os/OsProcess.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipLineMgr.h>
#include <net/SipRefreshMgr.h>
#include <net/SipTcpServer.h>

/**
 * Unittest for server shutdown testing
 */
class SipServerShutdownTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipServerShutdownTest);
      CPPUNIT_TEST(testTcpShutdown);
      CPPUNIT_TEST_SUITE_END();

public:

   void testTcpShutdown()
   {
      SipUserAgent* sipUA = new SipUserAgent( PORT_NONE
                                             ,PORT_NONE
                                             ,PORT_NONE
                                             ,NULL     // default publicAddress
                                             ,NULL     // default defaultUser
                                             ,"127.0.0.1"     // default defaultSipAddress
                                             ,NULL     // default sipProxyServers
                                             ,NULL     // default sipDirectoryServers
                                             ,NULL     // default sipRegistryServers
                                             ,NULL     // default authenticationScheme
                                             ,NULL     // default authenicateRealm
                                             ,NULL     // default authenticateDb
                                             ,NULL     // default authorizeUserIds
                                             ,NULL     // default authorizePasswords
                                             ,NULL     // default natPingUrl
                                             ,0        // default natPingFrequency
                                             ,"PING"   // natPingMethod
         );

      for (int i=0; i<10; ++i)
      {
         SipTcpServer* pSipTcpServer = new SipTcpServer(5090, sipUA, SIP_TRANSPORT_TCP, 
                                                       "SipTcpServer-%d", false);
         pSipTcpServer->startListener();

         OsTask::delay(1000);

         pSipTcpServer->shutdownListener();
         delete pSipTcpServer;
         pSipTcpServer = NULL;
      }

   };

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipServerShutdownTest);
