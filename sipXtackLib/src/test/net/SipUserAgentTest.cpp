//
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Scott Zuk
//         szuk AT telusplanet DOT net
//////////////////////////////////////////////////////////////////////////////

#include <sipxunittests.h>

#include <os/OsDefs.h>
#include <os/OsTimerTask.h>
#include <os/OsProcess.h>
#include <os/OsNatAgentTask.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipLineMgr.h>
#include <net/SipRefreshMgr.h>
#include <net/SipMessageEvent.h>

#define SHUTDOWN_TEST_ITERATIONS 3

/**
 * Unittest for SipUserAgent
 */
class SipUserAgentTest : public SIPX_UNIT_BASE_CLASS
{
      CPPUNIT_TEST_SUITE(SipUserAgentTest);
      CPPUNIT_TEST(testRefreshMgrTimeouts);
      CPPUNIT_TEST(testShutdownBlocking);
      CPPUNIT_TEST(testShutdownNonBlocking);
      CPPUNIT_TEST_SUITE_END();

public:

   // OsProcess doesn't provide any thread info so this method returns
   // the number of threads running under the process given by PID.
   // FIXME: Only implemented for linux, always returns 1 otherwise.
   int getNumThreads( int PID )
   {
       int numThreads = 1;

#ifdef __linux__
       // /proc parsing stolen from OsProcessIteratorLinux.cpp
       char pidString[20];
       snprintf(pidString, 20, "%d", PID);

       OsPath fullProcName = "/proc/";
       fullProcName += pidString;
       fullProcName += "/status";
       OsFileLinux procFile(fullProcName);
       if (procFile.open(OsFile::READ_ONLY) == OS_SUCCESS)
       {
           long len = 5000; //since the length is always 0 for these files, lets try to read 5k
           char *buffer = new char[len+1];
           if (buffer)
           {
               unsigned long bytesRead;
               procFile.read((void *)buffer,(unsigned long)len,bytesRead);

               if (bytesRead)
               {
                   procFile.close();
                   //null-terminate the string
                   buffer[bytesRead] = 0;
                   //now parse the info we need
                   char *ptr = strtok(buffer,"\n");
                   while(ptr)
                   {
                       if (memcmp(ptr,"Threads:",8) == 0)
                       {
                           numThreads = atoi(ptr+8);
                       }

                       ptr = strtok(NULL,"\n");
                   }
               }
               else
                   osPrintf("Couldn't read bytes in readProcFile\n");

               delete [] buffer;
           }

           procFile.close();
       }
#endif
       return numThreads;
   }

   void testRefreshMgrTimeouts()
   {
      int myPID = OsProcess::getCurrentPID();
      int startingThreads;

      // Stop TimerTask and NatAgentTask before counting threads.
      // Some tests do not bother stopping them, so they may come started.
      OsTimerTask::destroyTimerTask();
      OsNatAgentTask::releaseInstance();

      // Count number of threads now.
      startingThreads = getNumThreads(myPID);

      for(int i = 0; i < 1; ++i)
      {
         // Limit life time of lineMgr and refreshMgr. They should be freed
         // before releasing OsNatAgentTask instance, or we will crash.
         {
            SipLineMgr regLineMgr;
            regLineMgr.StartLineMgr();
            SipUserAgent sipRegistrar( 5099
                                      ,5099
                                      ,5098
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
                                      ,&regLineMgr     // No lineMgr needed for registrar
                                      );
            sipRegistrar.start();
            OsMsgQ messageQueue;
            sipRegistrar.addMessageObserver(messageQueue);

            SipLineMgr lineMgr;
            SipRefreshMgr refreshMgr;

            lineMgr.StartLineMgr();
            lineMgr.initializeRefreshMgr( &refreshMgr );

            SipUserAgent sipUA( 5090
                              ,5090
                              ,5091
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
                              ,&lineMgr
                              );

            sipUA.start();
            refreshMgr.init(&sipUA);

            // Wait and give time for SIP UA and other tasks to start
            refreshMgr.StartRefreshMgr();

            // Set a short refresh period so we can run the test faster
            refreshMgr.setRegistryPeriod(15); // seconds

            // Add a line and have it register
            const char* realm = "sipXtackUnitTest";
            const char* userId = "foo";
            const char* uriString = "sip:foo@127.0.0.1:5099";
            SipLine line(uriString, uriString, userId);
            line.addCredentials(realm, userId, "password", HTTP_DIGEST_AUTHENTICATION);
            lineMgr.addLine(line,
                             FALSE); // add disabled
            lineMgr.setStateForLine(uriString, SipLine::LINE_STATE_PROVISIONED);
            lineMgr.enableLine(uriString);
            

            // Wait for the first registration
            OsTime regTimeout(5, 0);
            OsMsg* appMessagePtr = NULL;
            printf("waiting for initial reg\n");
            OsStatus messageStatus = messageQueue.receive(appMessagePtr,
                                                          regTimeout);
            printf("got reg\n");
            CPPUNIT_ASSERT_EQUAL(messageStatus, OS_SUCCESS);
            CPPUNIT_ASSERT(messageStatus);
            CPPUNIT_ASSERT_EQUAL(appMessagePtr->getMsgType(), OsMsg::PHONE_APP);
            SipMessage* sipMessage = (SipMessage*)((SipMessageEvent*)appMessagePtr)->getMessage();
            CPPUNIT_ASSERT(sipMessage);
            CPPUNIT_ASSERT(!sipMessage->isResponse());
            {
                SipMessage response;
                response.setRequestUnauthorized(sipMessage,
                                                HTTP_DIGEST_AUTHENTICATION, // scheme
                                                realm,
                                                "111wwwsipx", // nonce
                                                ""); // opaque

                sipRegistrar.send(response);
                printf("sent reg\n");
            }

            printf("waiting for reg with auth\n");
            messageStatus = messageQueue.receive(appMessagePtr,
                                                          regTimeout);
            printf("got re-reg\n");
            CPPUNIT_ASSERT_EQUAL(messageStatus, OS_SUCCESS);
            CPPUNIT_ASSERT(messageStatus);
            CPPUNIT_ASSERT_EQUAL(appMessagePtr->getMsgType(), OsMsg::PHONE_APP);
            sipMessage = (SipMessage*)((SipMessageEvent*)appMessagePtr)->getMessage();
            CPPUNIT_ASSERT(sipMessage);
            CPPUNIT_ASSERT(!sipMessage->isResponse());
            {
                SipMessage response;
                response.setOkResponseData(sipMessage);

                sipRegistrar.send(response);
                printf("sent reg\n");
            }
            // Wait a bit to be sure response was sent
            OsTask::delay(200);

            // Now shutdown so the rest of the REGISTER refreshes don't get received.
            printf("shutting down reg\n");
            sipRegistrar.removeMessageObserver(messageQueue);
            sipRegistrar.shutdown(TRUE);
            regLineMgr.requestShutdown();
            printf("reg shutdown\n");

            // Wait long enough for several REGISTER timeouts/retansmits and refreshes to occur
            OsTask::delay(100000); // 100 seconds

            // Shut down the tasks in reverse order.
            refreshMgr.requestShutdown();
            sipUA.shutdown(TRUE);
            lineMgr.requestShutdown();

            CPPUNIT_ASSERT(sipUA.isShutdownDone());
            CPPUNIT_ASSERT(sipRegistrar.isShutdownDone());
         }

         // Stop TimerTask and NatAgentTask again before counting threads.
         // They were started while testing.
         OsTimerTask::destroyTimerTask();
         OsNatAgentTask::releaseInstance();

         // Test to see that all the threads created by the above operations
         // get properly shut down.
         // Since the threads do not shut down synchronously with the above
         // calls, we have to wait before we know they will be cleared.
         OsTask::delay(1000);   // 1 second
         int numThreads = getNumThreads(myPID);
         CPPUNIT_ASSERT_EQUAL(startingThreads,numThreads);
      }
   };

   void testShutdownBlocking()
   {
      int myPID = OsProcess::getCurrentPID();
      int startingThreads;

      // Stop TimerTask and NatAgentTask before counting threads.
      // Some tests do not bother stopping them, so they may come started.
      OsTimerTask::destroyTimerTask();
      OsNatAgentTask::releaseInstance();

      // Count number of threads now.
      startingThreads = getNumThreads(myPID);

      // Simple invite message from siptest/src/siptest/invite.txt
      const char* SimpleMessage = 
          "INVITE sip:1@192.168.0.6 SIP/2.0\r\n"
          "Route: <sip:foo@192.168.0.4:5064;lr>\r\n"
          "From: <sip:888@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 8\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

      for(int i = 0; i < SHUTDOWN_TEST_ITERATIONS; ++i)
      {
         // Limit life time of lineMgr and refreshMgr. They should be freed
         // before releasing OsNatAgentTask instance, or we will crash.
         {
            SipLineMgr    lineMgr;
            SipRefreshMgr refreshMgr;

            lineMgr.StartLineMgr();
            lineMgr.initializeRefreshMgr( &refreshMgr );

            SipUserAgent sipUA( 5090
                              ,5090
                              ,5091
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
                              ,&lineMgr
                              );

            sipUA.start();
            refreshMgr.init(&sipUA);

            sipUA.send(testMsg);

            // Wait long enough for some stack timeouts/retansmits to occur
            OsTask::delay(10000); // 10 seconds

            // Shut down the tasks in reverse order.
            refreshMgr.requestShutdown();
            sipUA.shutdown(TRUE);
            lineMgr.requestShutdown();

            CPPUNIT_ASSERT(sipUA.isShutdownDone());
         }

         // Stop TimerTask and NatAgentTask again before counting threads.
         // They were started while testing.
         OsTimerTask::destroyTimerTask();
         OsNatAgentTask::releaseInstance();

         // Test to see that all the threads created by the above operations
         // get properly shut down.
         // Since the threads do not shut down synchronously with the above
         // calls, we have to wait before we know they will be cleared.
         OsTask::delay(1000);   // 1 second
         int numThreads = getNumThreads(myPID);
         CPPUNIT_ASSERT_EQUAL(startingThreads,numThreads);
      }
   };

   void testShutdownNonBlocking()
   {
      int myPID = OsProcess::getCurrentPID();
      int startingThreads;

      // Stop TimerTask and NatAgentTask before counting threads.
      // Some tests do not bother stopping them, so they may come started.
      OsTimerTask::destroyTimerTask();
      OsNatAgentTask::releaseInstance();

      // Count number of threads now.
      startingThreads = getNumThreads(myPID);

      // Simple invite message from siptest/src/siptest/invite.txt
      const char* SimpleMessage = 
          "INVITE sip:1@192.168.0.6 SIP/2.0\r\n"
          "Route: <sip:foo@192.168.0.4:5064;lr>\r\n"
          "From: <sip:888@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 8\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

      for(int i = 0; i < SHUTDOWN_TEST_ITERATIONS; ++i)
      {
         // Limit life time of lineMgr and refreshMgr. They should be freed
         // before releasing OsNatAgentTask instance, or we will crash.
         {
            SipLineMgr    lineMgr;
            SipRefreshMgr refreshMgr;

            lineMgr.StartLineMgr();
            lineMgr.initializeRefreshMgr( &refreshMgr );

            SipUserAgent sipUA( 5090
                              ,5090
                              ,5091
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
                              ,&lineMgr
                              );

            sipUA.start();
            refreshMgr.init(&sipUA);

            sipUA.send(testMsg);

            // Wait long enough for some stack timeouts/retransmits to occur
            OsTask::delay(10000); // 10 seconds

            sipUA.shutdown(FALSE);
            lineMgr.requestShutdown();
            refreshMgr.requestShutdown();

            while(!sipUA.isShutdownDone())
            {
               ;
            }
            CPPUNIT_ASSERT(sipUA.isShutdownDone());
         }

         // Stop TimerTask and NatAgentTask again before counting threads.
         // They were started while testing.
         OsTimerTask::destroyTimerTask();
         OsNatAgentTask::releaseInstance();

         // Test to see that all the threads created by the above operations
         // get properly shut down.
         // Since the threads do not shut down synchronously with the above
         // calls, we have to wait before we know they will be cleared.
         OsTask::delay(1000);   // 1 second
         int numThreads = getNumThreads(myPID);
         CPPUNIT_ASSERT_EQUAL(startingThreads,numThreads);
      }
   };

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipUserAgentTest);
