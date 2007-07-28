// 
// Copyright (C) 2005-2007 SIPez LLC
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2005-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <mi/CpMediaInterfaceFactory.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>
#include <mi/CpMediaInterface.h>
#include <os/OsTask.h>
#include <utl/UtlSList.h>
#include <utl/UtlInt.h>
//#define DISABLE_RECORDING
#define EMBED_PROMPTS
#ifdef EMBED_PROMPTS
#  include "playback_prompt.h"
#  include "record_prompt.h"
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

class StoreSignalNotification : public OsNotification
{
public:
   StoreSignalNotification() {}
   virtual ~StoreSignalNotification() {}

   OsStatus signal(const intptr_t eventData) 
   { 
      UtlInt* pED = new UtlInt(eventData);
      return (mEDataList.insert(pED) == pED) ?
         OS_SUCCESS : OS_FAILED;
   }
   OsStatus popLastEvent(int& evtData) 
   {
      OsStatus stat = OS_NOT_FOUND;
      UtlInt* lastEData = (UtlInt*)mEDataList.get();
      if(lastEData != NULL)
      {
         evtData = lastEData->getValue();
         delete lastEData;
         stat = OS_SUCCESS;
      }
      return stat;
   }

   // Data (public now)
   UtlSList mEDataList;
private:
};

// Unittest for CpPhoneMediaInterface

class CpPhoneMediaInterfaceTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(CpPhoneMediaInterfaceTest);
    CPPUNIT_TEST(printMediaInterfaceType); // Just prints the media interface type.
#ifndef SANDBOX
    CPPUNIT_TEST(testProperties);
    CPPUNIT_TEST(testTones);
    CPPUNIT_TEST(testTwoTones);
#endif
    CPPUNIT_TEST(testRecordPlayback);
    CPPUNIT_TEST_SUITE_END();

    public:

    CpMediaInterfaceFactory* mpMediaFactory;

    CpPhoneMediaInterfaceTest()
    {
    };

    virtual void setUp()
    {
        enableConsoleOutput(0);

        mpMediaFactory = sipXmediaFactoryFactory(NULL);
    } 

    virtual void tearDown()
    {
        sipxDestroyMediaFactoryFactory();
        mpMediaFactory = NULL;
    }

    void printMediaInterfaceType()
    {
        CPPUNIT_ASSERT(mpMediaFactory);
        CpMediaInterface* mediaInterface = 
            mpMediaFactory->createMediaInterface(NULL, "", 0, NULL, 
                                                 "", 0, "", 0, 0, "",
                                                 0, "", "", 0, false);
        UtlString miType = mediaInterface->getType();
        if(miType == "CpPhoneMediaInterface")
        {
            printf("Phone media interface enabled\n");
        }
        else if(miType == "CpTopologyGraphInterface")
        {
            printf("Topology flowgraph interface enabled\n");
        }
        else
        {
            CPPUNIT_FAIL("ERROR: Unknown type of media interface!");
        }
    }

    void testProperties()
    {
        CPPUNIT_ASSERT(mpMediaFactory);

        SdpCodecFactory* codecFactory = new SdpCodecFactory();
        CPPUNIT_ASSERT(codecFactory);
        int numCodecs;
        SdpCodec** codecArray = NULL;
        codecFactory->getCodecs(numCodecs, codecArray);
        printf("CpPhoneMediaInterfaceTest::testProperties numCodec: %d\n", numCodecs);

        UtlString localRtpInterfaceAddress("127.0.0.1");
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


        CpMediaInterface* mediaInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
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

        UtlString propertyName("foo");
        UtlString setPropertyValue("bar");
        mediaInterface->setMediaProperty(propertyName, setPropertyValue);
        UtlString getPropertyValue;
        mediaInterface->getMediaProperty(propertyName, getPropertyValue);
        UtlBoolean diffValue = setPropertyValue.compareTo(getPropertyValue);
        if(diffValue)
        {
            printf("set value: \"%s\" get value\"%s\" not equal\n", 
                setPropertyValue.data(), getPropertyValue.data());
        }
        CPPUNIT_ASSERT(!diffValue);
        
        // Unset property
        CPPUNIT_ASSERT(mediaInterface->getMediaProperty("splat", getPropertyValue) != OS_SUCCESS);
        CPPUNIT_ASSERT(getPropertyValue.isNull());

        // Properties specific to a connection
        int connectionId = -1;
        CPPUNIT_ASSERT(mediaInterface->createConnection(connectionId, NULL) == OS_SUCCESS);
        CPPUNIT_ASSERT(connectionId > 0);

        propertyName = "connectionLabel";
        setPropertyValue = "connection1";
        mediaInterface->setMediaProperty(connectionId, propertyName, setPropertyValue);
        mediaInterface->getMediaProperty(connectionId, propertyName, getPropertyValue);
        diffValue = setPropertyValue.compareTo(getPropertyValue);
        if(diffValue)
        {
            printf("set value: \"%s\" get value\"%s\" not equal\n", 
                setPropertyValue.data(), getPropertyValue.data());
        }
        CPPUNIT_ASSERT(!diffValue);

        // Unset property
        CPPUNIT_ASSERT(mediaInterface->getMediaProperty(connectionId, "splat", getPropertyValue) != OS_SUCCESS);
        CPPUNIT_ASSERT(getPropertyValue.isNull());

        getPropertyValue = "f";
        // Invalid connectionId
        CPPUNIT_ASSERT(mediaInterface->getMediaProperty(6, "splat", getPropertyValue) != OS_SUCCESS);
        CPPUNIT_ASSERT(getPropertyValue.isNull());

        mediaInterface->deleteConnection(connectionId) ;

        // delete interface
        mediaInterface->release(); 

        // delete codecs set
        for ( numCodecs--; numCodecs>=0; numCodecs--)
        {
           delete codecArray[numCodecs];
        }
        delete[] codecArray;

        // delete mpMediaFactory ;
        delete codecFactory ;
    }

    void testRecordPlayback()
    {
        RTL_START(4500000);

        CPPUNIT_ASSERT(mpMediaFactory);

        SdpCodecFactory* codecFactory = new SdpCodecFactory();
        CPPUNIT_ASSERT(codecFactory);
        int numCodecs;
        SdpCodec** codecArray = NULL;
        codecFactory->getCodecs(numCodecs, codecArray);

        UtlString localRtpInterfaceAddress("127.0.0.1");
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

        //enableConsoleOutput(1);

        CpMediaInterface* mediaInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
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

        // Properties specific to a connection
        int connectionId = -1;
        CPPUNIT_ASSERT(mediaInterface->createConnection(connectionId, NULL) == OS_SUCCESS);
        CPPUNIT_ASSERT(connectionId > 0);

        mediaInterface->giveFocus() ;

        int taskId;
        OsTask::getCurrentTaskId(taskId);

        // Record the entire "call" - all connections.
        mediaInterface->recordChannelAudio(-1, "testRecordPlayback_call_recording.wav");
     
        StoreSignalNotification playAudNote;
#ifdef EMBED_PROMPTS
        printf("Playing record_prompt from RAM bytes: %d samples: %d frames: %d\n",
                sizeof(record_prompt),
                sizeof(record_prompt) / 2,
                sizeof(record_prompt) / 2 / 80);
        mediaInterface->playBuffer((char*)record_prompt, sizeof(record_prompt), 
                                   0, // type (does not need conversion to raw)
                                   false, //repeat
                                   true, // local
                                   false) ; //remote
#else   
        printf("Play record_prompt.wav taskId: %d\n",taskId);
        mediaInterface->playAudio("record_prompt.wav", 
                                  false, //repeat
                                  true, // local
                                  false, //remote
                                  false,
                                  100,
                                  &playAudNote);
#endif
        OsTask::delay(3500);
        //enableConsoleOutput(0);

        // Check via old OsNotification mechanism if the file finished playing.
        printf("%d event(s) on play event queue:  ", playAudNote.mEDataList.entries());
        int evtData = -1;
        while((evtData = playAudNote.popLastEvent(evtData)) != OS_NOT_FOUND)
        {
           printf("%d ", evtData);
        }
        printf("\n");

        mediaInterface->stopAudio() ;
        
        mediaInterface->startTone(0, true, false) ;
        OsTask::delay(100) ;
        mediaInterface->stopTone() ;
        OsTask::delay(100) ;

#ifdef DISABLE_RECORDING
        printf("recording disabled\n");
#else
        printf("Record to 10sec buffer\n");

        // Create a buffer to record to.
        // HACK: assume 8000 samples per second and 16 bit audio
        int bytesPerSec = 8000*2;
        int nSecsToRecord = 10;
        UtlString audioBuffer;
        audioBuffer.resize(nSecsToRecord * bytesPerSec);

        mediaInterface->recordMic(&audioBuffer);
        OsTask::delay(nSecsToRecord * 1000);
#endif
        OsTask::delay(100) ;
        mediaInterface->startTone(0, true, false) ;
        OsTask::delay(100) ;
        mediaInterface->stopTone() ;

#ifdef EMBED_PROMPTS
        printf("Playing playback_prompt from RAM bytes: %d samples: %d frames: %d\n",
                sizeof(playback_prompt),
                sizeof(playback_prompt) / 2,
                sizeof(playback_prompt) / 2 / 80);
        mediaInterface->playBuffer((char*)playback_prompt, sizeof(playback_prompt), 
                                   0, // type (does not need conversion to raw)
                                   false, //repeat
                                   true, // local
                                   false) ; //remote
#else   
        printf("Play playback_prompt.wav\n");
        mediaInterface->playAudio("playback_prompt.wav", false, true, false) ;
#endif
        OsTask::delay(2500) ;
        mediaInterface->stopAudio() ;
#ifdef DISABLE_RECORDING
        printf("record disabled so no play back of recorded message\n");
#else
        printf("Play record buffer\n");
        mediaInterface->playBuffer((char*)audioBuffer.data(), 
                                   audioBuffer.length(), 
                                   0, // type (does not need conversion to raw)
                                   false,  // repeat
                                   true,   // local
                                   false); // remote
        OsTask::delay(10000) ;
        mediaInterface->stopAudio() ;
#endif

        mediaInterface->startTone(0, true, false) ;
        OsTask::delay(100) ;
        mediaInterface->stopTone() ;

        printf("Play all done\n");
        OsTask::delay(500) ;

        RTL_WRITE("testRecordPlayback.rtl");
        RTL_STOP;

        // Stop recording the "call" -- all connections.
        mediaInterface->stopRecordChannelAudio(-1);

        mediaInterface->deleteConnection(connectionId) ;

        // delete codecs set
        for ( numCodecs--; numCodecs>=0; numCodecs--)
        {
           delete codecArray[numCodecs];
        }
        delete[] codecArray;

        delete codecFactory ;
        // delete interface
        mediaInterface->release(); 
    }

    void testTones()
    {
        RTL_START(1600000);

        CPPUNIT_ASSERT(mpMediaFactory);

        SdpCodecFactory* codecFactory = new SdpCodecFactory();
        CPPUNIT_ASSERT(codecFactory);
        int numCodecs;
        SdpCodec** codecArray = NULL;
        codecFactory->getCodecs(numCodecs, codecArray);

        UtlString localRtpInterfaceAddress("127.0.0.1");
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


        CpMediaInterface* mediaInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
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


        // Record the entire "call" - all connections.
        mediaInterface->recordChannelAudio(-1, "testTones_call_recording.wav");

        mediaInterface->giveFocus() ;

        RTL_EVENT("Tone set", 0);
        printf("first tone set\n");
        RTL_EVENT("Tone set", 1);
        mediaInterface->startTone(6, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        RTL_EVENT("Tone set", 2);
        mediaInterface->startTone(8, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        RTL_EVENT("Tone set", 3);
        mediaInterface->startTone(4, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        RTL_EVENT("Tone set", 0);
        printf("second tone set\n");        
        OsTask::delay(500) ;
        RTL_EVENT("Tone set", 1);
        mediaInterface->startTone(6, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        RTL_EVENT("Tone set", 2);
        mediaInterface->startTone(8, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        RTL_EVENT("Tone set", 3);
        mediaInterface->startTone(4, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        RTL_EVENT("Tone set", 0);
        printf("third tone set\n");        
        OsTask::delay(500) ;
        RTL_EVENT("Tone set", 1);
        mediaInterface->startTone(9, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        mediaInterface->startTone(5, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        mediaInterface->startTone(5, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        mediaInterface->startTone(4, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        RTL_EVENT("Tone set", 0);
        printf("fourth tone set\n");        
        OsTask::delay(500) ;
        RTL_EVENT("Tone set", 1);
        mediaInterface->startTone(9, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        mediaInterface->startTone(5, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        mediaInterface->startTone(5, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        mediaInterface->startTone(4, true, false) ;OsTask::delay(250) ;mediaInterface->stopTone() ;OsTask::delay(250) ;
        RTL_EVENT("Tone set", 0);
        printf("tone set done\n");        
        OsTask::delay(1000) ;

        // Stop recording the "call" -- all connections.
        mediaInterface->stopRecordChannelAudio(-1);

        RTL_WRITE("testTones.rtl");
        RTL_STOP;

        // delete codecs set
        for ( numCodecs--; numCodecs>=0; numCodecs--)
        {
           delete codecArray[numCodecs];
        }
        delete[] codecArray;

        // delete interface
        mediaInterface->release(); 

        OsTask::delay(500) ;
        delete codecFactory ;
    };

    void testTwoTones()
    {
        RTL_START(2400000);

        // This test creates three flowgraphs.  It streams RTP with tones
        // from the 2nd and 3rd to be received and mixed in the first flowgraph
        // So we test RTP and we test that we can generate 2 different tones in
        // to different flowgraphs to ensure that the ToneGen has no global
        // interactions or dependencies.
        CPPUNIT_ASSERT(mpMediaFactory);

        SdpCodecFactory* codecFactory = new SdpCodecFactory();
        CPPUNIT_ASSERT(codecFactory);
        int numCodecs;
        SdpCodec** codecArray = NULL;
        codecFactory->getCodecs(numCodecs, codecArray);

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

        // Create a flowgraph (sink) to receive and mix 2 sources
        CpMediaInterface* mixedInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
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

        // Create connections for mixed(sink) flowgraph
        int mixedConnection1Id = -1;
        CPPUNIT_ASSERT(mixedInterface->createConnection(mixedConnection1Id, NULL) == OS_SUCCESS);
        CPPUNIT_ASSERT(mixedConnection1Id > 0);
        int mixedConnection2Id = -1;
        CPPUNIT_ASSERT(mixedInterface->createConnection(mixedConnection2Id, NULL) == OS_SUCCESS);
        CPPUNIT_ASSERT(mixedConnection2Id > 0);
        
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
        SdpCodecFactory supportedCodecs1;
        SdpSrtpParameters srtpParameters1;
        int bandWidth1 = 0;
        int videoBandwidth1;
        int videoFramerate1;
        CPPUNIT_ASSERT_EQUAL(
            mixedInterface->getCapabilitiesEx(mixedConnection1Id, 
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
                                             videoFramerate1), 

             OS_SUCCESS);

        // capabilities of second connection on mixed(sink) flowgraph
        UtlString rtpHostAddresses2[maxAddresses];
        int rtpAudioPorts2[maxAddresses];
        int rtcpAudioPorts2[maxAddresses];
        int rtpVideoPorts2[maxAddresses];
        int rtcpVideoPorts2[maxAddresses];
        RTP_TRANSPORT transportTypes2[maxAddresses];
        int numActualAddresses2;
        SdpCodecFactory supportedCodecs2;
        SdpSrtpParameters srtpParameters2;
        int bandWidth2 = 0;
        int videoBandwidth2;
        int videoFramerate2;
        CPPUNIT_ASSERT_EQUAL(
            mixedInterface->getCapabilitiesEx(mixedConnection2Id, 
                                             maxAddresses,
                                             rtpHostAddresses2, 
                                             rtpAudioPorts2,
                                             rtcpAudioPorts2,
                                             rtpVideoPorts2,
                                             rtcpVideoPorts2,
                                             transportTypes2,
                                             numActualAddresses2,
                                             supportedCodecs2,
                                             srtpParameters2,
                                             bandWidth2,
                                             videoBandwidth2,
                                             videoFramerate2), 

             OS_SUCCESS);

        // Prep the sink connections to receive RTP
        int numCodecsFactory1;
        SdpCodec** codecArray1 = NULL;
        supportedCodecs1.getCodecs(numCodecsFactory1, codecArray1);
        CPPUNIT_ASSERT_EQUAL(
            mixedInterface->startRtpReceive(mixedConnection1Id,
                                            numCodecsFactory1,
                                            codecArray1),
            OS_SUCCESS);

        int numCodecsFactory2;
        SdpCodec** codecArray2 = NULL;
        supportedCodecs2.getCodecs(numCodecsFactory2, codecArray2);
        CPPUNIT_ASSERT_EQUAL(
            mixedInterface->startRtpReceive(mixedConnection2Id,
                                            numCodecsFactory2,
                                            codecArray2),
            OS_SUCCESS);

        // Second flowgraph to be one of two sources
        CpMediaInterface* source1Interface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
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

        // Create connection for source 1 flowgraph
        int source1ConnectionId = -1;
        CPPUNIT_ASSERT(source1Interface->createConnection(source1ConnectionId, NULL) == OS_SUCCESS);
        CPPUNIT_ASSERT(source1ConnectionId > 0);

        // Set the destination for sending RTP from source 1 to connection 1 on
        // the mix flowgraph
        printf("rtpHostAddresses1: \"%s\"\nrtpAudioPorts1: %d\nrtcpAudioPorts1: %d\nrtpVideoPorts1: %d\nrtcpVideoPorts1: %d\n",
            rtpHostAddresses1->data(), 
            *rtpAudioPorts1,
            *rtcpAudioPorts1,
            *rtpVideoPorts1,
            *rtcpVideoPorts1);

        CPPUNIT_ASSERT_EQUAL(
            source1Interface->setConnectionDestination(source1ConnectionId,
                                                       rtpHostAddresses1->data(), 
                                                       *rtpAudioPorts1,
                                                       *rtcpAudioPorts1,
                                                       *rtpVideoPorts1,
                                                       *rtcpVideoPorts1),
            OS_SUCCESS);

        // Start sending RTP from source 1 to the mix flowgraph
        CPPUNIT_ASSERT_EQUAL(
            source1Interface->startRtpSend(source1ConnectionId, 
                                           numCodecsFactory1,
                                           codecArray1),
            OS_SUCCESS);


        // Second flowgraph to be one of two sources
        CpMediaInterface* source2Interface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
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

        // Create connection for source 2 flowgraph
        int source2ConnectionId = -1;
        CPPUNIT_ASSERT(source2Interface->createConnection(source2ConnectionId, NULL) == OS_SUCCESS);
        CPPUNIT_ASSERT(source2ConnectionId > 0);

        // Set the destination for sending RTP from source 2 to connection 2 on
        // the mix flowgraph
        CPPUNIT_ASSERT_EQUAL(
            source2Interface->setConnectionDestination(source2ConnectionId,
                                                       *rtpHostAddresses2, 
                                                       *rtpAudioPorts2,
                                                       *rtcpAudioPorts2,
                                                       *rtpVideoPorts2,
                                                       *rtcpVideoPorts2),
            OS_SUCCESS);

        RTL_EVENT("Tone count", 0);

        // Record the entire "call" - all connections.
        mixedInterface->recordChannelAudio(-1, "testTwoTones_call_recording.wav");

        // Start sending RTP from source 2 to the mix flowgraph
        CPPUNIT_ASSERT_EQUAL(
            source2Interface->startRtpSend(source2ConnectionId, 
                                           numCodecsFactory2,
                                           codecArray2),
            OS_SUCCESS);

        // Want to hear what is on the mixed flowgraph
        mixedInterface->giveFocus();

        RTL_EVENT("Tone count", 1);
        printf("generate tones in source 1\n");
        source1Interface->startTone(1, true, true);

        OsTask::delay(1000);

        RTL_EVENT("Tone count", 2);
        printf("generate tones in source 2 as well\n");
        source2Interface->startTone(2, true, true);

        OsTask::delay(1000);

        RTL_EVENT("Tone count", 1);
        printf("stop tones in source 1\n");
        source1Interface->stopTone();


        OsTask::delay(1000);

        RTL_EVENT("Tone count", 0);
        printf("stop tone in source 2\n");
        source2Interface->stopTone();

        OsTask::delay(1000);
        printf("two tones done\n");        

        // Stop recording the "call" -- all connections.
        mixedInterface->stopRecordChannelAudio(-1);

        // Delete connections
        mixedInterface->deleteConnection(mixedConnection1Id);
        mixedInterface->deleteConnection(mixedConnection2Id);
        source1Interface->deleteConnection(source1ConnectionId);
        source2Interface->deleteConnection(source2ConnectionId);

        // delete interfaces
        mixedInterface->release();
        source1Interface->release();
        source2Interface->release();

        OsTask::delay(500) ;

        RTL_WRITE("testTwoTones.rtl");
        RTL_STOP;

        // delete codecs set
        for ( numCodecs--; numCodecs>=0; numCodecs--)
        {
           delete codecArray[numCodecs];
        }
        delete[] codecArray;
        for ( numCodecsFactory1--; numCodecsFactory1>=0; numCodecsFactory1--)
        {
           delete codecArray1[numCodecsFactory1];
        }
        delete[] codecArray1;
        for ( numCodecsFactory2--; numCodecsFactory2>=0; numCodecsFactory2--)
        {
           delete codecArray2[numCodecsFactory2];
        }
        delete[] codecArray2;

        delete codecFactory ;
    };
};

CPPUNIT_TEST_SUITE_REGISTRATION(CpPhoneMediaInterfaceTest);
