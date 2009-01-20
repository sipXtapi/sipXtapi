// 
// Copyright (C) 2008-2009 SIPez LLC
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2008-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>
#ifdef HAVE_SSL

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include "mi/CpMediaInterfaceFactory.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"
#include "CpTopologyGraphInterface.h"
#include "mi/CpMediaInterface.h"
#include "mi/MiNotification.h"
#include "mi/MiDtmfNotf.h"
#include <os/OsTask.h>
#include <utl/UtlSList.h>
#include <utl/UtlInt.h>
#include <os/OsMsgDispatcher.h>
#include <utl/UtlCryptoKeySym.h>
#include <os/OsSocketCrypto.h>
#include <os/OsSocket.h>

#include <os/OsTask.h>

// Defaults for Media Interface Factory and Media Interface initialization
// Zeros here indicate to use defaults.
#define FRAME_SIZE_MS       0
#define MAX_SAMPLE_RATE     0 
#define DEFAULT_SAMPLE_RATE 0

//#define DISABLE_RECORDING
#define EMBED_PROMPTS
#ifdef EMBED_PROMPTS
#  define PROMPTBUF_SAMPLERATE 8000
#endif

#ifdef RTL_ENABLED
#  include <rtl_macro.h>
   RTL_DECLARE
#else
#  define RTL_START(x)
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#  define RTL_WRITE(x)
#  define RTL_STOP
#endif

#define MAX_RTP_PORTS 1000

extern unsigned char sine_330hz_16b_8k_signed[160116];

class CpCryptoTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(CpCryptoTest);
   CPPUNIT_TEST(testCryptoOuroboros);
   CPPUNIT_TEST_SUITE_END();

public:

   CpMediaInterfaceFactory* mpMediaFactory;

   virtual void setUp()
   {
      enableConsoleOutput(0);

      // Add some codec paths.

      UtlString codecPaths[] = {
#ifdef WIN32
         "..\\sipXmediaLib\\bin",
         "..\\..\\sipXmediaLib\\bin",
#elif __pingtel_on_posix__
         "../../../../../sipXmediaLib/bin",
         "../../../../sipXmediaLib/bin",
#else
#                                  error "Unknown platform"
#endif
         "."
      };

      int codecPathsNum = sizeof(codecPaths)/sizeof(codecPaths[0]);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         CpMediaInterfaceFactory::addCodecPaths(codecPathsNum, codecPaths));

      // Initialize the factory factory
      mpMediaFactory = 
         sipXmediaFactoryFactory(NULL, FRAME_SIZE_MS, 
         MAX_SAMPLE_RATE, DEFAULT_SAMPLE_RATE, TRUE);
   } 

   virtual void tearDown()
   {
      sipxDestroyMediaFactoryFactory();
      CpMediaInterfaceFactory::clearCodecPaths();
      mpMediaFactory = NULL;
      RTL_STOP
   }

   OsStatus createRtpSocketPairCrypto(UtlString localAddress,
                                       int localPort,
                                       OsSocket* &rtpSocket,
                                       OsSocket* &rtcpSocket,
                                       const char* pEncBinData,
                                       int encBinLength,
                                       const char* pDecBinData,
                                       int decBinLength)
   {
      int firstRtpPort;
      bool localPortGiven = (localPort != 0); // Does user specified the local port?
      UtlBoolean isMulticast = OsSocket::isMcastAddr(localAddress);

      firstRtpPort = localPort;
      if (!localPortGiven)
      {
         return OS_FAILED;
      }

      if (isMulticast)
      {
         rtpSocket = new OsMulticastSocketCrypto(localPort, localAddress,
                                                 localPort, localAddress,
                                                 pEncBinData, encBinLength,
                                                 pDecBinData, decBinLength);
         rtcpSocket = new OsMulticastSocket(
            localPort == 0 ? 0 : localPort + 1, localAddress,
            localPort == 0 ? 0 : localPort + 1, localAddress);
      }
      else
      {
         rtpSocket = new OsNatDatagramSocketCrypto(localPort, localAddress,
                                                   localPort, localAddress,
                                                   NULL,
                                                   pEncBinData, encBinLength,
                                                   pDecBinData, decBinLength);
         ((OsNatDatagramSocket*)rtpSocket)->enableTransparentReads(false);

         rtcpSocket = new OsNatDatagramSocket(0, NULL,localPort == 0 ? 0 : localPort+1,
            localAddress, NULL);
         ((OsNatDatagramSocket*)rtcpSocket)->enableTransparentReads(false);
      }

      // Validate local port is not auto-selecting.
      if (localPort != 0 && !localPortGiven)
      {
         // If either of the sockets are bad (e.g. already in use) or
         // if either have stuff on them to read (e.g. someone is
         // sending junk to the ports, look for another port pair
         while(!rtpSocket->isOk() ||
               !rtcpSocket->isOk() ||
               rtcpSocket->isReadyToRead() ||
               rtpSocket->isReadyToRead(60))
         {
            localPort +=2;
            // This should use mLastRtpPort instead of some
            // hardcoded MAX, but I do not think mLastRtpPort
            // is set correctly in all of the products.
            if(localPort > firstRtpPort + MAX_RTP_PORTS) 
            {
               OsSysLog::add(FAC_CP, PRI_ERR, 
                             "No available ports for RTP and RTCP in range %d - %d",
                             firstRtpPort, firstRtpPort + MAX_RTP_PORTS);
               break;  // time to give up
            }

            delete rtpSocket;
            delete rtcpSocket;
            if (isMulticast)
            {
               rtpSocket = new OsMulticastSocketCrypto(localPort, localAddress,
                                                       localPort, localAddress,
                                                       pEncBinData, encBinLength,
                                                       pDecBinData, decBinLength);
               rtcpSocket = new OsMulticastSocket(
                                localPort == 0 ? 0 : localPort + 1, localAddress,
                                localPort == 0 ? 0 : localPort + 1, localAddress);
            }
            else
            {
               rtpSocket = new OsNatDatagramSocketCrypto(localPort, localAddress,
                                                         localPort, localAddress,
                                                         NULL,
                                                         pEncBinData, encBinLength,
                                                         pDecBinData, decBinLength);
               ((OsNatDatagramSocket*)rtpSocket)->enableTransparentReads(false);

               rtcpSocket = new OsNatDatagramSocket(0, NULL,localPort == 0 ? 0 : localPort+1,
                  localAddress, NULL);
               ((OsNatDatagramSocket*)rtcpSocket)->enableTransparentReads(false);
            }
         }
      }

      // Did our sockets get created OK?
      if (!rtpSocket->isOk() || !rtcpSocket->isOk())
      {
         delete rtpSocket;
         delete rtcpSocket;
         return OS_NETWORK_UNAVAILABLE;
      }

      if (isMulticast)
      {
         // Set multicast options
         const unsigned char MC_HOP_COUNT = 8;
         ((OsMulticastSocket*)rtpSocket)->setHopCount(MC_HOP_COUNT);
         ((OsMulticastSocket*)rtcpSocket)->setHopCount(MC_HOP_COUNT);
         ((OsMulticastSocket*)rtpSocket)->setLoopback(false);
         ((OsMulticastSocket*)rtcpSocket)->setLoopback(false);
      }

      // Set a maximum on the buffers for the sockets so
      // that the network stack does not get swamped by early media
      // from the other side;
      {
         int sRtp, sRtcp, oRtp, oRtcp, optlen;

         sRtp = rtpSocket->getSocketDescriptor();
         sRtcp = rtcpSocket->getSocketDescriptor();

         optlen = sizeof(int);
         oRtp = 20000;
         setsockopt(sRtp, SOL_SOCKET, SO_RCVBUF, (char *) (&oRtp), optlen);
         oRtcp = 500;
         setsockopt(sRtcp, SOL_SOCKET, SO_RCVBUF, (char *) (&oRtcp), optlen);
      }

      return OS_SUCCESS;
   }

    void testCryptoOuroboros()
    {
        RTL_START(2400000);

        CPPUNIT_ASSERT(mpMediaFactory);

        UtlCryptoKeySym symKey;
        symKey.generateKey();

        char tempMemory[1000];
        symKey.getBinaryKey(tempMemory, 1000);

        printf("KeyDump:\n%s\n", symKey.output().data());

        // If we wanted to supply a different set of codecs than the
        // defaults, then we would do the below, and supply 
        // numCodecs and codecArray when creating a mediaInterface.
        // SdpCodecList* codecFactory = new SdpCodecList();
        // CPPUNIT_ASSERT(codecFactory);
        // int numCodecs;
        // SdpCodec** codecArray = NULL;
        // codecFactory->getCodecs(numCodecs, codecArray);

        UtlString localRtpInterfaceAddress("127.0.0.1");
        OsSocket::getHostIp(&localRtpInterfaceAddress);
        UtlString locale;
        int tosOptions = 0;
        UtlString stunServerAddress;
        int stunOptions = 0;
        int stunKeepAlivePeriodSecs = 25;
        UtlString turnServerAddress;
        int turnPort = 0 ;
        UtlString turnUser;
        UtlString turnPassword;
        int turnKeepAlivePeriodSecs = 25;
        bool enableIce = false ;

        // Create a flowgraph to receive and mix 2 sources
        CpMediaInterface* mixedInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 0, NULL, // use default codecs
                                                 locale,
                                                 tosOptions,
                                                 stunServerAddress, 
                                                 stunOptions, 
                                                 stunKeepAlivePeriodSecs,
                                                 turnServerAddress,
                                                 turnPort,
                                                 turnUser,
                                                 turnPassword,
                                                 turnKeepAlivePeriodSecs,
                                                 enableIce);
        
        UtlString miType = mixedInterface->getType();
        if(miType == "CpPhoneMediaInterface")
        {
           printf("Phone media interface enabled! Test canceled.\n");
           return;
        }
        else if(miType != "CpTopologyGraphInterface")
        {
           CPPUNIT_FAIL("ERROR: Unknown type of media interface!");
        }

        CpTopologyGraphInterface* ti = (CpTopologyGraphInterface*)mixedInterface;
        // Create connection
        int connectionId = -1;

        OsSocket* srtp = NULL;
        OsSocket* srctp = NULL;

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
           createRtpSocketPairCrypto("127.0.0.1", 6000, srtp, srctp, tempMemory, 1000, tempMemory, 1000));

        CPPUNIT_ASSERT_EQUAL(TRUE, srtp->isOk());
        CPPUNIT_ASSERT_EQUAL(TRUE, srctp->isOk());

        //Should be topology graph interface
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, ti->createConnection(connectionId, srtp, srctp, FALSE));
        
        CPPUNIT_ASSERT(connectionId > 0);
        
        // Get the address of the connections so we can send RTP to them
        // capabilities of first connection on mixed(sink) flowgraph
        const int maxAddresses = 1;
        UtlString rtpHostAddresses1[maxAddresses];
        int rtpAudioPorts1[maxAddresses];
        int rtcpAudioPorts1[maxAddresses];
        int rtpVideoPorts1[maxAddresses];
        int rtcpVideoPorts1[maxAddresses];
        RTP_TRANSPORT transportTypes1[maxAddresses];
        int numActualAddresses1;
        SdpCodecList supportedCodecs1;
        SdpSrtpParameters srtpParameters1;
        int bandWidth1 = 0;
        int videoBandwidth1;
        int videoFramerate1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->getCapabilitiesEx(connectionId, 
                                                               maxAddresses,
                                                               rtpHostAddresses1, 
                                                               rtpAudioPorts1,
                                                               rtcpAudioPorts1,
                                                               rtpVideoPorts1,
                                                               rtcpVideoPorts1,
                                                               transportTypes1,
                                                               numActualAddresses1,
                                                               supportedCodecs1,
                                                               srtpParameters1,
                                                               bandWidth1,
                                                               videoBandwidth1,
                                                               videoFramerate1));


        // Prep the connection's sink to receive RTP
        int numCodecsFactory;
        SdpCodec** codecArray = NULL;
        supportedCodecs1.getCodecs(numCodecsFactory, codecArray);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->startRtpReceive(connectionId,
                                                             numCodecsFactory,
                                                             codecArray));

        // Want to hear what is on the mixed flowgraph
        mixedInterface->giveFocus();

        // Set the destination for sending RTP from source 1 to connection 1 on
        // the mix flowgraph
        printf("rtpHostAddresses1: \"%s\"\nrtpAudioPorts1: %d\nrtcpAudioPorts1: %d\nrtpVideoPorts1: %d\nrtcpVideoPorts1: %d\n",
               rtpHostAddresses1->data(), 
               *rtpAudioPorts1,
               *rtcpAudioPorts1,
               *rtpVideoPorts1,
               *rtcpVideoPorts1);

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->setConnectionDestination(connectionId,
                                                                      rtpHostAddresses1->data(), 
                                                                      *rtpAudioPorts1,
                                                                      *rtcpAudioPorts1,
                                                                      *rtpVideoPorts1,
                                                                      *rtcpVideoPorts1));

        RTL_EVENT("Prompt", 0);

        // Record the entire "call" - all connections.
        mixedInterface->recordChannelAudio(-1, "testOuroboros_call_recording.wav");

        // Start sending RTP from source 2 to the mix flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->startRtpSend(connectionId, 
                                                          numCodecsFactory,
                                                          codecArray));

        RTL_EVENT("Prompt", 1);
        int waveHeaderSize = 116;
        CPPUNIT_ASSERT_EQUAL((unsigned)160116, sizeof(sine_330hz_16b_8k_signed));
        mixedInterface->playBuffer((char*)&sine_330hz_16b_8k_signed[waveHeaderSize], 
                                   sizeof(sine_330hz_16b_8k_signed) - waveHeaderSize, 
                                   PROMPTBUF_SAMPLERATE,
                                   0, //type, 
                                   FALSE, // repeat,
                                   FALSE, // local, 
                                   TRUE //remote,
                                   //OsProtectedEvent* event = NULL,
                                   //UtlBoolean mixWithMic = false,
                                   //int downScaling = 100
                                   );

        OsTask::delay(10000);

        mixedInterface->stopAudio();

        OsTask::delay(500);

        // Stop recording the "call" -- all connections.
        mixedInterface->stopRecordChannelAudio(-1);

        // Delete connections
        mixedInterface->deleteConnection(connectionId);

        // delete interfaces
        mixedInterface->release();

        OsTask::delay(500) ;

        RTL_WRITE("testCryptoOuroboros.rtl");
        RTL_STOP;

        // delete codecs set
        for ( numCodecsFactory--; numCodecsFactory>=0; numCodecsFactory--)
        {
           delete codecArray[numCodecsFactory];
        }
        delete[] codecArray;
    };
};


CPPUNIT_TEST_SUITE_REGISTRATION(CpCryptoTest);

#endif
