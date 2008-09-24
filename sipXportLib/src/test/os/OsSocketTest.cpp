//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsFS.h>
#include <os/OsTask.h>
#include <sipxunit/TestUtilities.h>
#include <os/OsTestUtilities.h>
#include <os/OsDatagramSocket.h>
#include <os/OsMulticastSocket.h>
#include <os/OsConnectionSocket.h>
#include <os/OsServerSocket.h>
#include <string.h>

class SocketsTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(SocketsTest);
    CPPUNIT_TEST(testSocketUtils);
    CPPUNIT_TEST(testWriteMsg);
    CPPUNIT_TEST(testWriteAndAcceptMsg);
    CPPUNIT_TEST(testMulticast);
    CPPUNIT_TEST_SUITE_END();


public:
    static UtlString mLocalHost;
   
    void testSocketUtils()
    {
        OsSocket::getHostIp(&mLocalHost);

        if(mLocalHost.isNull())
        {
            // Otherwise all subsequent tests will fail
            mLocalHost = "127.0.0.1";
        }

        printf("testSocketUtils setting mLocalHost=\"%s\"\n", mLocalHost.data());
        CPPUNIT_ASSERT(!mLocalHost.isNull());
        CPPUNIT_ASSERT(OsSocket::isIp4Address(mLocalHost));
    }
    
    /**
     * Open datagram socket and send a few bytes.
     */
    void testWriteMsg()
    {
        OsSocket* s = new OsDatagramSocket(8020, mLocalHost);
        if(!s->isOk() || s->getSocketDescriptor() < 0)
        {
            printf("socket descripter not valid: %d\n", s->getSocketDescriptor());
        }

        CPPUNIT_ASSERT(s->getSocketDescriptor() >= 0);
        CPPUNIT_ASSERT(s->isOk());
        const char* msg = "hello\n";
        int len = (int)strlen(msg);
        int bytesWritten = s->write(msg, len);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("write correct number of bytes", 
            bytesWritten, len);
        s->close();
        delete s;
    }

    /**
     * Start a client and server and send 2 messages over TCP thru them
     */
    void testWriteAndAcceptMsg()
    {
        int port = 8021;
        long socketTimeout = 5000;
        OsServerSocket* server = new OsServerSocket(50, port, mLocalHost);
        printf("binding to %s:%d\n", mLocalHost.data(), port);
        // This test being single threaded stresses some platform specific issues
        // On some platforms you can do a blocking connect on the client and then accept on the
        // server side.  On others its the other way around.  So we do a non-blocking connect on
        // the client side.  We block latter after the accept to assure it connects.
        UtlBoolean block = FALSE;
        OsSocket* client = new OsConnectionSocket(port, mLocalHost, block);
        OsSocket* serverClient = NULL;
        /*int delayCount = 0;
        while(!server->isConnectionReady())
        {
            OsTask::delay(100);
            delayCount++;
            if(delayCount > 50) break;
        }
        if(server->isConnectionReady())
        {*/
            serverClient = server->accept();
            CPPUNIT_ASSERT(serverClient);
        /*}
        else
        {
            CPPUNIT_ASSERT_MESSAGE("server->isConnectionReady returned false", 0);
        }*/

        // Now make client connection blocking.
        // With non-blocking client server-to-client message test will fail
        // on fast machines (say on Core 2 Duo processors).
        client->makeBlocking();

        // Now make client connection blocking.
        // With non-blocking client server-to-client message test will fail
        // on fast machines (say on Core 2 Duo processors).
        client->makeBlocking();

        CPPUNIT_ASSERT_MESSAGE("socket server failed to accept connection", 
                               serverClient != NULL);

        const char* msg = "hello\n";
        int len = (int)strlen(msg) + 1; // +1 for NULL
        int bytesWritten = client->write(msg, len, socketTimeout);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("write correct number of bytes", 
                bytesWritten, len);

        char recvBuf[1024];
        int bytesRead = serverClient->read(recvBuf, sizeof(recvBuf) - 1, socketTimeout);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("read correct number of bytes", 
                len, bytesRead);
        ASSERT_STR_EQUAL_MESSAGE("message same as was sent", msg, recvBuf);

        const char *resp = "bye";
        len = (int)strlen(resp) + 1; // +1 for NULL
        bytesWritten = serverClient->write(resp, len, socketTimeout);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("write correct number of bytes on 2nd msg", 
            len, bytesWritten);

        bytesRead = client->read(recvBuf, sizeof(recvBuf) - 1, socketTimeout);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("read correct number of bytes on 2nd msg", 
            len, bytesRead);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("write correct number of bytes on 2nd msg", 
            len, bytesWritten);

        ASSERT_STR_EQUAL_MESSAGE("2nd message same as was sent", 
                resp, recvBuf);

        serverClient->close();
        client->close();
        server->close();

        delete client;
        delete server;
    }

    /**
     * Open two multicast sockets and enable local loopback.
     * Send a few bytes on the 1st & make sure we receive the data back on the 2nd.
     */
    void testMulticast()
    {
        OsMulticastSocket* writer = new OsMulticastSocket(8022, "224.10.11.12",
                                                          8022, "224.10.11.12");
        OsMulticastSocket* reader = new OsMulticastSocket(8022, "224.10.11.12",
                                                          8022, "224.10.11.12");
        writer->setHopCount(1);

        // Make sure we'll receive our own message.
        // * In Winsock, the IP_MULTICAST_LOOP option applies only to the receive path.
        // * In the UNIX version, the IP_MULTICAST_LOOP option applies to the send path.
#ifdef WIN32 // [
        reader->setLoopback(true);
#else // WIN32 ][
        writer->setLoopback(true);
#endif // WIN32 ]

        const char* msg = "hello\n";
        int len = strlen(msg) + 1; // +1 for NULL
        int bytesWritten = writer->write(msg, len);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("write correct number of bytes", 
                bytesWritten, len);

        char recvBuf[1024];
        int bytesRead = reader->OsSocket::read(recvBuf, sizeof(recvBuf) - 1, 100);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("read correct number of bytes", 
                len, bytesRead);
        ASSERT_STR_EQUAL_MESSAGE("message same as was sent", msg, recvBuf);

        writer->close();
        reader->close();
        delete writer;
        delete reader;
    }
};

UtlString SocketsTest::mLocalHost;

CPPUNIT_TEST_SUITE_REGISTRATION(SocketsTest);

