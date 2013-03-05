//  
// Copyright (C) 2007-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <os/OsIntTypes.h>

#include <sipxunittests.h>
#include "mp/MpFlowGraphBase.h"
#include "rtcp/RTCPSession.h"


/**
 * Unittest for MpAudioBuf
 */
class RtcpParserTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(RtcpParserTest);

    // Register methods to be called for testing here
    CPPUNIT_TEST(testBadPaddedPacket);

    CPPUNIT_TEST_SUITE_END();


public:

#if 0
    // This method gets called before running each test method
    // Implement this method if you have common setup for each test method
    void setUp()
    {
    }
#endif

#if 0
    // This method gets called after running each test method
    // Implement this method if you have common teardown/clean to do after each test method
    void tearDown()
    {
    }
#endif

    void testBadPaddedPacket()
    {
        int ret;

        // Do stuff
        // int foo = 0;

        // Check expected results
        // CPPUNIT_ASSERT(foo==0);
        // CPPUNIT_ASSERT_EQUAL(foo, 0);
        // CPPUNIT_ASSERT_EQUAL_MESSAGE("This test failed testing the blabla", foo, 0);

        // Do more stuff and check results ...

        {
            // This is based on a non-compliant packet we get several times per call from the
            //  LifeSize box.  The packet it sends is 63 bytes long (not a multiple of 4 as
            //  required by RFC-3550.
            unsigned char bogusLifeSizePacket[] = {
               0x80,0xc9,0x00,0x01,0x1f,0xcd,0x53,0xe2,  // RR chunk
               0x81,0xca,0x00,0x06,0x1f,0xcd,0x53,0xe2,0x01,0x11,0x4c,0x69,0x66,0x65,0x53,
               0x69,0x7a,0x65,0x20,0x45,0x6e,0x64,0x70,0x6f,0x69,0x6e,0x74,0x00, // end of SDES chunk
               0x80,0xcc,0x00,0x06,0x1f,0xcd,0x53,0xe2,0x4c,0x53,0x00,0x00,0x28,0x00,0x00,
               0x04,0x06,0xfa,0x00,0x00,0x00,0x00,0x00,0x03,0x01,0x1d,0x0a, // end of odd length APP chunk
               // That's the 63 bytes
               // Here are a few more to use in test variations.
               0xff,0xbf,0xcc,0x00,0x40,0x1f,0xcd,0x53,0xe2,0x01,0x11,0x4c,0x69,0x8f,0xcc,0x00,
               0x40,0x1f,0xcd,0x53,0xe2,0x01,0x11,0x4c,0x69
            };
            ret = CRTCPHeader::VetPacket(bogusLifeSizePacket, 63);  // the original packet
            CPPUNIT_ASSERT_EQUAL(ret, 64);
            ret = CRTCPHeader::VetPacket(bogusLifeSizePacket, 64);  // All is good
            CPPUNIT_ASSERT_EQUAL(ret, 64);
            ret = CRTCPHeader::VetPacket(bogusLifeSizePacket, 76);  // trailing garbage
            CPPUNIT_ASSERT_EQUAL(ret, 64);
            ret = CRTCPHeader::VetPacket(bogusLifeSizePacket, 67);  // trailing garbage, odd
            CPPUNIT_ASSERT_EQUAL(ret, 64);
            ret = CRTCPHeader::VetPacket(bogusLifeSizePacket, 55);  // APP too short
            CPPUNIT_ASSERT_EQUAL(ret, 36);
            ret = CRTCPHeader::VetPacket(bogusLifeSizePacket, 7);   // too short
            CPPUNIT_ASSERT_EQUAL(ret, 0);
            ret = CRTCPHeader::VetPacket(bogusLifeSizePacket, 0);   // too short
            CPPUNIT_ASSERT_EQUAL(ret, 0);
        }
        {
            // This is based on a non-compliant packet we get several times per call from the
            //  LifeSize box.  The packet it sends is 63 bytes long (not a multiple of 4 as
            //  required by RFC-3550.
            unsigned char bogusLifeSizePacket[] = {
              // First RR.  OK
                0x80, 0xc9, 0x00, 0x01, 0xe4, 0x47, 0xf9, 0x28,

              // SDES, OK
                0x81, 0xca, 0x00, 0x06, 0xe4, 0x47, 0xf9, 0x28, 0x01, 0x11,
                0x4c, 0x69, 0x66, 0x65, 0x53, 0x69, 0x7a, 0x65, 0x20, 0x45,
                0x6e, 0x64, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x00,

              // APP, invalid length, only 27 bytes remain, not a multiple of 4
                0x80, 0xcc, 0x00, 0x06, 0xe4, 0x47, 0xf9, 0x28, 0x4c, 0x53,
                0x00, 0x00, 0x28, 0x00, 0x00, 0x04, 0x08, 0x81, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x03, 0x01, 0x1d, 0x0a,

              // The particular garbage that followed the packet in our
              // read buffer, and appears to have put our parser into an
              // infinite loop (r12079 code, should be fixed in r12083)
                0x69, 0x43, 0xc9, 0xd0, 0xcb, 0xfc, 0x3d, 0x1b, 0xa9, 0xd1,
                0xf6, 0xde, 0x5b, 0x68, 0x8d, 0x83, 0x52, 0x47, 0x9a, 0x1f,
                0xe0, 0xa7, 0x2b, 0x0b, 0x0f, 0x32, 0x49, 0x12, 0x67, 0xd4,
                0x98, 0x79, 0xf3, 0xd3, 0x25, 0x0f, 0xbf

            };
            ret = CRTCPHeader::VetPacket(bogusLifeSizePacket, 63);  // the original packet
            CPPUNIT_ASSERT_EQUAL(ret, 64);
            ret = CRTCPHeader::VetPacket(bogusLifeSizePacket, 64);  // All is good
            CPPUNIT_ASSERT_EQUAL(ret, 64);
            ret = CRTCPHeader::VetPacket(bogusLifeSizePacket, 76);  // trailing garbage
            CPPUNIT_ASSERT_EQUAL(ret, 64);
        }
    }


// ***  // This routine is needed for testing pre-r12083 code that did not contain
// ***  //  the new VetPacket routine.
// ***  // It also demonstrates how to set up the RTCP object cluster to the point
// ***  //  where those objects can be used to process packets.
// ***
// ***  // NOTE: some of these tests will put the pre-r12079 code into an infinite
// ***  //  loop, which was the original reason for adding VetPacket.
// ***
// ***
// ***    void testBadPaddedPacketOld()
// ***    {
// ***        int ret;
// ***
// ***        // Do stuff
// ***        // int foo = 0;
// ***
// ***        // Check expected results
// ***        // CPPUNIT_ASSERT(foo==0);
// ***        // CPPUNIT_ASSERT_EQUAL(foo, 0);
// ***        // CPPUNIT_ASSERT_EQUAL_MESSAGE("This test failed testing the blabla", foo, 0);
// ***
// ***        // Do more stuff and check results ...
// ***      /////////////////////////////////////////////////////////////////////////////////
// ***      // For future reference...
// ***      // The following sequence will create enough RTCP "stuff" to call PrcoessPacket
// ***      /////////////////////////////////////////////////////////////////////////////////
// ***      // Lifted from MpFlowGraphBase constructor:
// ***        IRTCPSession* mpiRTCPSession;
// ***        IRTCPConnection* mpiRTCPConnection;
// ***    
// ***        IRTCPControl *piRTCPControl = CRTCManager::getRTCPControl();
// ***        CPPUNIT_ASSERT(piRTCPControl);
// ***    
// ***        // Create an RTCP Session for this Flow Graph.  Pass the SSRC ID to be
// ***        // used to identify our audio source uniquely within this RTP/RTCP Session.
// ***        mpiRTCPSession = piRTCPControl->CreateSession(0x12345678);
// ***    
// ***        // Subscribe for Events associated with this Session
// ***        piRTCPControl->Advise((IRTCPNotify *)this);
// ***    
// ***        // Release Reference to RTCP Control Interface
// ***        piRTCPControl->Release();
// ***      /////////////////////////////////////////////////////////////////////////////////
// ***    
// ***        MpFlowGraphBase* pFlowGraph = new MpFlowGraphBase();
// ***      // Lifted from MpFlowGraphBase constructor:
// ***        mpiRTCPConnection = pFlowGraph->getRTCPConnectionPtr('c', 'A', 's');
// ***        // Let's use the Connection interface to acquire the constituent interfaces
// ***        // required for dispatching RTP and RTCP packets received from the network as
// ***        // well as the statistics interface tabulating RTP packets going to the network.
// ***        INetDispatch         *piRTCPDispatch = NULL;
// ***        IRTPDispatch         *piRTPDispatch = NULL;
// ***        ISetSenderStatistics *piRTPAccumulator = NULL;
// ***    
// ***        if(mpiRTCPConnection)
// ***        {
// ***            mpiRTCPConnection->GetDispatchInterfaces(&piRTCPDispatch, &piRTPDispatch, &piRTPAccumulator);
// ***        }
// ***      /////////////////////////////////////////////////////////////////////////////////
// ***
// ***
// ***        {
// ***            // This is based on a non-compliant packet we get several times per call from the
// ***            //  LifeSize box.  The packet it sends is 63 bytes long (not a multiple of 4 as
// ***            //  required by RFC-3550.
// ***            unsigned char bogusLifeSizePacket[] = {
// ***               0x80,0xc9,0x00,0x01,0x1f,0xcd,0x53,0xe2,  // RR chunk
// ***               0x81,0xca,0x00,0x06,0x1f,0xcd,0x53,0xe2,0x01,0x11,0x4c,0x69,0x66,0x65,0x53,
// ***               0x69,0x7a,0x65,0x20,0x45,0x6e,0x64,0x70,0x6f,0x69,0x6e,0x74,0x00, // end of SDES chunk
// ***               0x80,0xcc,0x00,0x06,0x1f,0xcd,0x53,0xe2,0x4c,0x53,0x00,0x00,0x28,0x00,0x00,
// ***               0x04,0x06,0xfa,0x00,0x00,0x00,0x00,0x00,0x03,0x01,0x1d,0x0a, // end of odd length APP chunk
// ***               // That's the 63 bytes
// ***               // Here are a few more to use in test variations.
// ***               0xff,0xbf,0xcc,0x00,0x40,0x1f,0xcd,0x53,0xe2,0x01,0x11,0x4c,0x69,0x8f,0xcc,0x00,
// ***               0x40,0x1f,0xcd,0x53,0xe2,0x01,0x11,0x4c,0x69
// ***            };
// ***
// ***            ret = 1;
// ***            piRTCPDispatch->ProcessPacket(bogusLifeSizePacket, 63);  // the original packet
// ***            CPPUNIT_ASSERT_EQUAL(ret, 1);
// ***            piRTCPDispatch->ProcessPacket(bogusLifeSizePacket, 64);  // All is good
// ***            CPPUNIT_ASSERT_EQUAL(ret, 1);
// ***            piRTCPDispatch->ProcessPacket(bogusLifeSizePacket, 76);  // trailing garbage
// ***            CPPUNIT_ASSERT_EQUAL(ret, 1);
// ***            piRTCPDispatch->ProcessPacket(bogusLifeSizePacket, 67);  // trailing garbage, odd
// ***            CPPUNIT_ASSERT_EQUAL(ret, 1);
// ***            piRTCPDispatch->ProcessPacket(bogusLifeSizePacket, 55);  // APP too short
// ***            CPPUNIT_ASSERT_EQUAL(ret, 1);
// ***            piRTCPDispatch->ProcessPacket(bogusLifeSizePacket, 7);   // too short
// ***            CPPUNIT_ASSERT_EQUAL(ret, 1);
// ***            piRTCPDispatch->ProcessPacket(bogusLifeSizePacket, 0);   // too short
// ***            CPPUNIT_ASSERT_EQUAL(ret, 1);
// ***        }
// ***        {
// ***            // This is based on a non-compliant packet we get several times per call from the
// ***            //  LifeSize box.  The packet it sends is 63 bytes long (not a multiple of 4 as
// ***            //  required by RFC-3550.
// ***            unsigned char bogusLifeSizePacket[] = {
// ***              // First RR.  OK
// ***                0x80, 0xc9, 0x00, 0x01, 0xe4, 0x47, 0xf9, 0x28,
// ***
// ***              // SDES, OK
// ***                0x81, 0xca, 0x00, 0x06, 0xe4, 0x47, 0xf9, 0x28, 0x01, 0x11,
// ***                0x4c, 0x69, 0x66, 0x65, 0x53, 0x69, 0x7a, 0x65, 0x20, 0x45,
// ***                0x6e, 0x64, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x00,
// ***
// ***              // APP, invalid length, only 27 bytes remain, not a multiple of 4
// ***                0x80, 0xcc, 0x00, 0x06, 0xe4, 0x47, 0xf9, 0x28, 0x4c, 0x53,
// ***                0x00, 0x00, 0x28, 0x00, 0x00, 0x04, 0x08, 0x81, 0x00, 0x00,
// ***                0x00, 0x00, 0x00, 0x03, 0x01, 0x1d, 0x0a,
// ***
// ***              // The particular garbage that followed the packet in our
// ***              // read buffer, and appears to have put our parser into an
// ***              // infinite loop (r12079 code, should be fixed in r12083)
// ***                0x69, 0x43, 0xc9, 0xd0, 0xcb, 0xfc, 0x3d, 0x1b, 0xa9, 0xd1,
// ***                0xf6, 0xde, 0x5b, 0x68, 0x8d, 0x83, 0x52, 0x47, 0x9a, 0x1f,
// ***                0xe0, 0xa7, 0x2b, 0x0b, 0x0f, 0x32, 0x49, 0x12, 0x67, 0xd4,
// ***                0x98, 0x79, 0xf3, 0xd3, 0x25, 0x0f, 0xbf
// ***
// ***            };
// ***            piRTCPDispatch->ProcessPacket(bogusLifeSizePacket, 63);  // the original packet
// ***            CPPUNIT_ASSERT_EQUAL(ret, 1);
// ***            piRTCPDispatch->ProcessPacket(bogusLifeSizePacket, 64);  // All is good
// ***            CPPUNIT_ASSERT_EQUAL(ret, 1);
// ***            piRTCPDispatch->ProcessPacket(bogusLifeSizePacket, 76);  // trailing garbage
// ***            CPPUNIT_ASSERT_EQUAL(ret, 1);
// ***        }
// ***    }




protected:
    // Add member data or utility methods used by test methods here
};

CPPUNIT_TEST_SUITE_REGISTRATION(RtcpParserTest);
