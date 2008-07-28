// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
// 
// Copyright (C) 2005-2008 SIPez LLC
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2005-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

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
#include <os/OsMediaContact.h>
#include <sipxunit/TestUtilities.h>
#include "sine_330hz_16b_8k_signed.h"
#include "sine_530hz_16b_8k_signed.h"

// Defaults for Media Interface Factory and Media Interface initialization
// Zeros here indicate to use defaults.
#define FRAME_SIZE_MS       0
#define MAX_SAMPLE_RATE     0 
#define DEFAULT_SAMPLE_RATE 0

//#define DISABLE_RECORDING
#define EMBED_PROMPTS
#ifdef EMBED_PROMPTS
#  include "playback_prompt.h"
#  include "record_prompt.h"
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
    CPPUNIT_TEST(testSetCodecPath);
    CPPUNIT_TEST(testProperties);
    CPPUNIT_TEST(testTones);
    CPPUNIT_TEST(testOuroboros);
    CPPUNIT_TEST(testTwoTones);
    CPPUNIT_TEST(testPlayPauseResumeStop);
    CPPUNIT_TEST(testRecordPlayback);
    CPPUNIT_TEST(testConnectionNotifications);
    CPPUNIT_TEST(testThreeGraphs);
    CPPUNIT_TEST_SUITE_END();

    public:

    CpMediaInterfaceFactory* mpMediaFactory;

    CpPhoneMediaInterfaceTest()
    {
    };

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
                                   MAX_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    } 

    virtual void tearDown()
    {
        sipxDestroyMediaFactoryFactory();
        CpMediaInterfaceFactory::clearCodecPaths();
        mpMediaFactory = NULL;
        RTL_STOP
    }

    void printMediaInterfaceType()
    {
        CPPUNIT_ASSERT(mpMediaFactory);

        ProxyDescriptor stunServer;
        ProxyDescriptor turnServer;
        ProxyDescriptor arsProxy;
        ProxyDescriptor arsHttpProxy;
        CpMediaInterface* mediaInterface = 
            mpMediaFactory->createMediaInterface(
            NULL, // publicAddress
            NULL, // localAddress
            0, // num Codecs
            NULL,    // sdpCodecArrayf
            NULL,  // locale
            0, // expeditedIpTos
            stunServer,
            turnServer,
            arsProxy,
            arsHttpProxy,
            NULL,   // callback
            false,  // enable ice
            8000);  // samps per sec

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

    void testSetCodecPath()
    {
       // Test storing codec paths for loading.
       UtlString testBadCodecPath1 = "|****|****|";
       KNOWN_BUG("addCodec paths does not check for syntactic validity", "???");
       CPPUNIT_ASSERT_EQUAL(OS_FAILED,
                            CpMediaInterfaceFactory::addCodecPaths(1, &testBadCodecPath1));
    }

    OsStatus waitForNotf(OsMsgDispatcher& notfDispatcher,
                         MiNotification::NotfType notfType, 
                         unsigned maxTotalDelayTime,
                         MiNotification** ppNotf = NULL)
    {
       // keep count of the milliseconds we're gone
       unsigned delayPeriod = 10; // Milliseconds in each delay
       unsigned curMsecsDelayed = 0;
       for(curMsecsDelayed = 0; 
          notfDispatcher.isEmpty() && curMsecsDelayed < maxTotalDelayTime;
          curMsecsDelayed += delayPeriod)
       {
          // Delay just a bit
          OsTask::delay(delayPeriod);
       }

       if(curMsecsDelayed >= maxTotalDelayTime)
       {
          return OS_WAIT_TIMEOUT;
       }

       // Assert that there is a notification available now.
       CPPUNIT_ASSERT_EQUAL(FALSE, notfDispatcher.isEmpty());

       // Grab the message with a short timeout, since we know it's there.
       OsMsg* pMsg = NULL;
       MiNotification* pNotfMsg = NULL;
       notfDispatcher.receive(pMsg, OsTime(delayPeriod));
       CPPUNIT_ASSERT(pMsg != NULL);
       CPPUNIT_ASSERT_EQUAL(OsMsg::MI_NOTF_MSG, 
                            (OsMsg::MsgTypes)pMsg->getMsgType());
       pNotfMsg = (MiNotification*)pMsg;
       CPPUNIT_ASSERT_EQUAL(notfType, 
                            (MiNotification::NotfType)pNotfMsg->getType());

       if(ppNotf != NULL)
       {
          *ppNotf = pNotfMsg;
       }
       else
       {
          // We're not holding onto the message, so release it.
          // If we don't do that, there will be a memory leak.
          pNotfMsg->releaseMsg();
       }

       return OS_SUCCESS;
    }

    void testProperties()
    {
        CPPUNIT_ASSERT(mpMediaFactory);

        SdpCodecList* codecFactory = new SdpCodecList();
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

        ProxyDescriptor stunServer;
        ProxyDescriptor turnServer;
        ProxyDescriptor arsProxy;
        ProxyDescriptor arsHttpProxy;

        CpMediaInterface* mediaInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);


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
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mediaInterface->createConnection(connectionId, true, NULL));
        CPPUNIT_ASSERT(connectionId > 0);

#ifdef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
        // Test that we can get the bridge mixer port on which this new connection
        // is connected to
        int portOnBridge;
        ((CpTopologyGraphInterface*)mediaInterface)->getConnectionPortOnBridge(connectionId, 
                                                                               portOnBridge);
        CPPUNIT_ASSERT_EQUAL(3, portOnBridge);
#endif
 

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

        SdpCodecList* codecFactory = new SdpCodecList();
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
        ProxyDescriptor stunServer;
        ProxyDescriptor turnServer;
        ProxyDescriptor arsProxy;
        ProxyDescriptor arsHttpProxy;

        CpMediaInterface* mediaInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);


        // Properties specific to a connection
        int connectionId = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mediaInterface->createConnection(connectionId, true, NULL));
        CPPUNIT_ASSERT(connectionId > 0);

        // Create a Media notification dispatcher and give it to the media interface.
        OsMsgDispatcher notfDispatcher;
        mediaInterface->setNotificationDispatcher(&notfDispatcher);
        mediaInterface->setNotificationsEnabled(true);

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
                                   PROMPTBUF_SAMPLERATE,
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
        // Keep around a status object to query various statuses
        OsStatus stat = OS_SUCCESS;

        // Wait for a wee bit (100msecs) to receive a started playing message
        stat = waitForNotf(notfDispatcher, MiNotification::MI_NOTF_PLAY_STARTED, 100);
        CPPUNIT_ASSERT_MESSAGE("No play finished notification was sent while playing record prompt!",
                               stat == OS_SUCCESS);

        // Wait for a maximum of 5000 msecs to receive a finished playing message
        stat = waitForNotf(notfDispatcher, MiNotification::MI_NOTF_PLAY_FINISHED, 5000);
        CPPUNIT_ASSERT_MESSAGE("No play finished notification was sent while playing record prompt!",
                               stat == OS_SUCCESS);

        //enableConsoleOutput(0);

        // Check via old OsNotification mechanism if the file finished playing.
        printf("%d event(s) on play event queue:  ", playAudNote.mEDataList.entries());
        int evtData = -1;
        while((evtData = playAudNote.popLastEvent(evtData)) != OS_NOT_FOUND)
        {
           printf("%d ", evtData);
        }
        printf("\n");

        mediaInterface->startTone(0, true, false) ;
        OsTask::delay(100) ;
        mediaInterface->stopTone() ;
        OsTask::delay(100) ;

#ifdef DISABLE_RECORDING
        printf("recording disabled\n");
#else
        printf("Record to 10sec buffer\n");

        // Create a buffer to record to.
        // HACK! This assumes recording is done at 8kHz 16bit!
        const int nSecsToRecord = 10;
        UtlString audioBuffer;

        mediaInterface->recordMic(nSecsToRecord*1000, &audioBuffer);

        // Wait for a maximum of the size of the buffer (10secs), plus an additional
        // 10 seconds to receive a recording stopped message
        stat = waitForNotf(notfDispatcher, 
                           MiNotification::MI_NOTF_RECORD_FINISHED,
                           nSecsToRecord*1000 + 10000);
        CPPUNIT_ASSERT_MESSAGE("No record finished notification was sent while recording!",
                               stat == OS_SUCCESS);

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
                                   PROMPTBUF_SAMPLERATE,
                                   0, // type (does not need conversion to raw)
                                   false, //repeat
                                   true, // local
                                   false) ; //remote
#else   
        printf("Play playback_prompt.wav\n");
        mediaInterface->playAudio("playback_prompt.wav", false, true, false) ;
#endif
        // Wait for a wee bit (100msecs) to receive a started playing message
        stat = waitForNotf(notfDispatcher, MiNotification::MI_NOTF_PLAY_STARTED, 100);
        CPPUNIT_ASSERT_MESSAGE("No play finished notification was sent while playing playback prompt!",
                               stat == OS_SUCCESS);
        // Wait for maximum of 3.5 seconds for the play to finish.
        stat = waitForNotf(notfDispatcher, MiNotification::MI_NOTF_PLAY_FINISHED, 3500);
        CPPUNIT_ASSERT_MESSAGE("No play finished notification was sent while playing playback prompt!",
                               stat == OS_SUCCESS);


#ifdef DISABLE_RECORDING
        printf("record disabled so no play back of recorded message\n");
#else
        printf("Play record buffer\n");
        mediaInterface->playBuffer((char*)audioBuffer.data(), 
                                   audioBuffer.length(), 
                                   mediaInterface->getSamplesPerSec(), 
                                   0, // type (does not need conversion to raw)
                                   false,  // repeat
                                   true,   // local
                                   false); // remote

        // Wait for a wee bit (100msecs) to receive a started playing message
        stat = waitForNotf(notfDispatcher, MiNotification::MI_NOTF_PLAY_STARTED, 100);
        CPPUNIT_ASSERT_MESSAGE("No play finished notification was sent while playing back recording!",
                               stat == OS_SUCCESS);
        // Wait for maximum of 15 seconds for the play to finish.
        stat = waitForNotf(notfDispatcher, MiNotification::MI_NOTF_PLAY_FINISHED, 15000);
        CPPUNIT_ASSERT_MESSAGE("No play finished notification was sent while playing back recording!",
                               stat == OS_SUCCESS);
#endif

        mediaInterface->startTone(0, true, false) ;
        OsTask::delay(100) ;
        mediaInterface->stopTone() ;

        printf("Play all done\n");
        OsTask::delay(500) ;

        RTL_WRITE("testRecordPlayback.rtl");

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

    void testPlayPauseResumeStop()
    {
        RTL_START(4500000);

        CPPUNIT_ASSERT(mpMediaFactory);

        SdpCodecList* codecFactory = new SdpCodecList();
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
        ProxyDescriptor stunServer;
        ProxyDescriptor turnServer;
        ProxyDescriptor arsProxy;
        ProxyDescriptor arsHttpProxy;

        CpMediaInterface* mediaInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);

        // Properties specific to a connection
        int connectionId = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mediaInterface->createConnection(connectionId, true, NULL));
        CPPUNIT_ASSERT(connectionId > 0);

        // Create a Media notification dispatcher and give it to the media interface.
        OsMsgDispatcher notfDispatcher;
        mediaInterface->setNotificationDispatcher(&notfDispatcher);
        mediaInterface->setNotificationsEnabled(true);

        mediaInterface->giveFocus() ;

        int taskId;
        OsTask::getCurrentTaskId(taskId);

        // Record the entire "call" - all connections.
        mediaInterface->recordChannelAudio(-1, "testPlayPauseResumeStop_call_recording.wav");
        
#ifdef EMBED_PROMPTS
        printf("Playing record_prompt from RAM bytes: %d samples: %d frames: %d\n",
                sizeof(record_prompt),
                sizeof(record_prompt) / 2,
                sizeof(record_prompt) / 2 / 80);
        mediaInterface->playBuffer((char*)record_prompt, sizeof(record_prompt), 
                                   PROMPTBUF_SAMPLERATE,
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
                                  100);
#endif
        // Keep around a status object to query various statuses
        OsStatus stat = OS_SUCCESS;

        // play for a second, pause, wait for a second, resume.
        OsTask::delay(1000);
        mediaInterface->pauseAudio();
        OsTask::delay(1000);
        mediaInterface->resumeAudio();
        OsTask::delay(500);
        mediaInterface->stopAudio();
        stat = waitForNotf(notfDispatcher, MiNotification::MI_NOTF_PLAY_STARTED, 100);
        CPPUNIT_ASSERT_MESSAGE("No play started notification was sent after starting to play record prompt!",
                               stat == OS_SUCCESS);
        stat = waitForNotf(notfDispatcher, MiNotification::MI_NOTF_PLAY_PAUSED, 100);
        CPPUNIT_ASSERT_MESSAGE("No play paused notification was sent after pausing record prompt!",
                               stat == OS_SUCCESS);
        stat = waitForNotf(notfDispatcher, MiNotification::MI_NOTF_PLAY_RESUMED, 100);
        CPPUNIT_ASSERT_MESSAGE("No play resumed notification was sent after resuming record prompt!",
                               stat == OS_SUCCESS);
        stat = waitForNotf(notfDispatcher, MiNotification::MI_NOTF_PLAY_STOPPED, 100);
        CPPUNIT_ASSERT_MESSAGE("No play stopped notification was sent after stopping record prompt!",
                               stat == OS_SUCCESS);

        RTL_WRITE("testPlayPauseResumeStop.rtl");
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
        RTL_START(3000000);

        CPPUNIT_ASSERT(mpMediaFactory);

        SdpCodecList* codecFactory = new SdpCodecList();
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

        ProxyDescriptor stunServer;
        ProxyDescriptor turnServer;
        ProxyDescriptor arsProxy;
        ProxyDescriptor arsHttpProxy;

        CpMediaInterface* mediaInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);


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

        ProxyDescriptor stunServer;
        ProxyDescriptor turnServer;
        ProxyDescriptor arsProxy;
        ProxyDescriptor arsHttpProxy;
        SdpCodecList supportedCodecs;
        int numCodecs;
        SdpCodec** codecArray = NULL;
        supportedCodecs.getCodecs(numCodecs, codecArray);

        // Create a flowgraph (sink) to receive and mix 2 sources
        CpMediaInterface* mixedInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);

        // Create connections for mixed(sink) flowgraph
        int mixedConnection1Id = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->createConnection(mixedConnection1Id, true, NULL));
        CPPUNIT_ASSERT(mixedConnection1Id > 0);
        int mixedConnection2Id = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->createConnection(mixedConnection2Id, true, NULL));
        CPPUNIT_ASSERT(mixedConnection2Id > 0);
        
        // Get the address of the connections so we can send RTP to them
        // capabilities of first connection on mixed(sink) flowgraph
        SdpCodecList supportedCodecs1;
        SdpSrtpParameters srtpParameters1;
        int bandWidth1 = 0;
        int videoBandwidth1;
        int videoFramerate1;
/*
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
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
                                                               videoFramerate1));
*/
        UtlSList audioContacts1;
        UtlSList videoContacts1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->getCapabilities(mixedConnection1Id, 
                                &audioContacts1,
                                &videoContacts1,
                                supportedCodecs1,
                                srtpParameters1,
                                0,
                                videoBandwidth1,
                                videoFramerate1)
                             );

        // capabilities of second connection on mixed(sink) flowgraph
        UtlSList audioContacts2;
        UtlSList videoContacts2;
        SdpCodecList supportedCodecs2;
        SdpSrtpParameters srtpParameters2;
        int bandWidth2 = 0;
        int videoBandwidth2;
        int videoFramerate2;
        /*
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
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
                                                               videoFramerate2));

                                                               */
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->getCapabilities(mixedConnection2Id, 
                                &audioContacts2,
                                &videoContacts2,
                                supportedCodecs2,
                                srtpParameters2,
                                0,
                                videoBandwidth2,
                                videoFramerate2)
                             );

        // Prep the sink connections to receive RTP
        int numCodecsFactory1;
        SdpCodec** codecArray1 = NULL;
        supportedCodecs1.getCodecs(numCodecsFactory1, codecArray1);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->startRtpReceive(mixedConnection1Id,
                                                             numCodecsFactory1,
                                                             codecArray1));

        // Want to hear what is on the mixed flowgraph
        mixedInterface->giveFocus();

        int numCodecsFactory2;
        SdpCodec** codecArray2 = NULL;
        supportedCodecs2.getCodecs(numCodecsFactory2, codecArray2);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->startRtpReceive(mixedConnection2Id,
                                                             numCodecsFactory2,
                                                             codecArray2));

        // Second flowgraph to be one of two sources
        CpMediaInterface* source1Interface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecsFactory2, 
                                                 codecArray2, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);

        // Create connection for source 1 flowgraph
        int source1ConnectionId = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source1Interface->createConnection(source1ConnectionId, true, NULL));
        CPPUNIT_ASSERT(source1ConnectionId > 0);

        // Set the destination for sending RTP from source 1 to connection 1 on
        // the mix flowgraph
        UtlString audioHostAddr2;
        UtlString videoHostAddr2;
        UtlString hostAddr;
        UtlString videoHostAddr;
        int audioRtpPort = 0;
        int audioRtcpPort = 0;
        int videoRtpPort = 0;
        int videoRtcpPort = 0;
        if (audioContacts2.at(0) != NULL)
        {
            ((OsMediaContact*)audioContacts2.at(0))->getAddress(hostAddr);
            audioRtpPort = ((OsMediaContact*)audioContacts2.at(0))->getPort();
            audioRtcpPort = ((OsMediaContact*)audioContacts2.at(0))->getRtcpPort();
        }
        if (videoContacts2.at(0) != NULL)
        {
            ((OsMediaContact*)videoContacts2.at(0))->getAddress(videoHostAddr);
            videoRtpPort = ((OsMediaContact*)videoContacts2.at(0))->getPort();
            videoRtcpPort = ((OsMediaContact*)videoContacts2.at(0))->getRtcpPort();
        }

        printf("rtpHostAddresses1: \"%s\"\nrtpAudioPorts1: %d\nrtcpAudioPorts1: %d\nrtpVideoPorts1: %d\nrtcpVideoPorts1: %d\n",
               hostAddr.data(),
               audioRtpPort,
               audioRtcpPort,
               videoRtpPort,
               videoRtcpPort);

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source1Interface->setConnectionDestination(source1ConnectionId,
                                                                      hostAddr.data(), 
                                                                      audioRtpPort,
                                                                      audioRtcpPort,
                                                                      0,
                                                                      videoHostAddr.data(), 
                                                                      videoRtpPort,
                                                                      videoRtcpPort,
                                                                      0));

        // Start sending RTP from source 1 to the mix flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source1Interface->startRtpSend(source1ConnectionId, 
                                                            numCodecsFactory1,
                                                            codecArray1));

        // Second flowgraph to be one of two sources
        CpMediaInterface* source2Interface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);

        // Create connection for source 2 flowgraph
        int source2ConnectionId = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source2Interface->createConnection(source2ConnectionId, true, NULL));
        CPPUNIT_ASSERT(source2ConnectionId > 0);

        // Set the destination for sending RTP from source 2 to connection 2 on
        // the mix flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source2Interface->setConnectionDestination(source2ConnectionId,
                                                                      hostAddr.data(), 
                                                                      audioRtpPort,
                                                                      audioRtcpPort,
                                                                      0,
                                                                      videoHostAddr.data(), 
                                                                      videoRtpPort,
                                                                      videoRtcpPort,
                                                                      0));



        RTL_EVENT("Tone count", 0);

        // Record the entire "call" - all connections.
        mixedInterface->recordChannelAudio(-1, "testTwoTones_call_recording.wav");

        // Start sending RTP from source 2 to the mix flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source2Interface->startRtpSend(source2ConnectionId, 
                                                            numCodecsFactory2,
                                                            codecArray2));

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

        // delete codecs set
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
    };

    void testConnectionNotifications()
    {
        RTL_START(2400000);

        // This test creates two flowgraphs.  It streams RTP with tones
        // from the second flowgraph to be received in the first flowgraph
        // So we test RTP-response generated notifications to tones and other
        // related behaviors to ensure that they are properly generated.
        CPPUNIT_ASSERT(mpMediaFactory);

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

        ProxyDescriptor stunServer;
        ProxyDescriptor turnServer;
        ProxyDescriptor arsProxy;
        ProxyDescriptor arsHttpProxy;
        SdpCodecList supportedCodecs;
        int numCodecs;
        SdpCodec** codecArray = NULL;
        supportedCodecs.getCodecs(numCodecs, codecArray);


        // Create a flowgraph (sink) to receive another flowgraph (source)
        CpMediaInterface* sinkInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);

        // Create a notification dispatcher and supply it to the queue.
        OsMsgDispatcher sinkNotfDispatcher;
        sinkInterface->setNotificationDispatcher(&sinkNotfDispatcher);
        sinkInterface->setNotificationsEnabled(true);

        // BUG: We shouldn't have to delay to make notifications enabled.
        // This really should be a synchronous operation.
        printf("\n**BUG**: Notifications sent shortly after setting notifications"
               " enabled results in no notifications sent.\n");
        printf("WORKAROUND: Delay to be able to test rest of DTMF notification framework.\n\n");
        OsTask::delay(100);

        // Create connections for sink flowgraph
        int sinkConnectionId = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             sinkInterface->createConnection(sinkConnectionId, true, NULL));
        CPPUNIT_ASSERT(sinkConnectionId > 0);
        
        // Get the address of the connections so we can send RTP to them
        // capabilities of a connection on the sink flowgraph
        SdpCodecList supportedSinkCodecs;
        SdpSrtpParameters srtpParameters;
        int bandWidth = 0;
        int videoBandwidth;
        int videoFramerate;
        UtlSList sinkAudioContacts;
        UtlSList sinkVideoContacts;

        /*
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             sinkInterface->getCapabilitiesEx(sinkConnectionId, 
                                                              maxAddresses,
                                                              rtpHostAddresses, 
                                                              rtpAudioPorts,
                                                              rtcpAudioPorts,
                                                              rtpVideoPorts,
                                                              rtcpVideoPorts,
                                                              transportTypes,
                                                              numActualAddresses,
                                                              supportedSinkCodecs,
                                                              srtpParameters,
                                                              bandWidth,
                                                              videoBandwidth,
                                                              videoFramerate));
        */
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             sinkInterface->getCapabilities(sinkConnectionId, 
                                &sinkAudioContacts,
                                &sinkVideoContacts,
                                supportedSinkCodecs,
                                srtpParameters,
                                0,
                                videoBandwidth,
                                videoFramerate)
                             );

        // Prep the sink connections to receive RTP
        int numSupportedSinkCodecs;
        SdpCodec** supportedSinkCodecArray = NULL;
        supportedSinkCodecs.getCodecs(numSupportedSinkCodecs, supportedSinkCodecArray);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             sinkInterface->startRtpReceive(sinkConnectionId,
                                                            numSupportedSinkCodecs,
                                                            supportedSinkCodecArray));

        // Want to hear what is on the mixed flowgraph
        sinkInterface->giveFocus();

        // Second flowgraph to be the source flowgraph
        CpMediaInterface* sourceInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);

        // Create connection for source flowgraph
        int sourceConnectionId = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             sourceInterface->createConnection(sourceConnectionId, true, NULL));
        CPPUNIT_ASSERT(sourceConnectionId > 0);

        // Set the destination for sending RTP from source to connection on
        // the sink flowgraph
        UtlString sinkAudioHostAddr;
        UtlString sinkVideoHostAddr;
        UtlString hostAddr;
        UtlString videoHostAddr;
        int audioRtpPort = 0;
        int audioRtcpPort = 0;
        int videoRtpPort = 0;
        int videoRtcpPort = 0;
        if (sinkAudioContacts.at(0) != NULL)
        {
            ((OsMediaContact*)sinkAudioContacts.at(0))->getAddress(hostAddr);
            audioRtpPort = ((OsMediaContact*)sinkAudioContacts.at(0))->getPort();
            audioRtcpPort = ((OsMediaContact*)sinkAudioContacts.at(0))->getRtcpPort();
        }
        if (sinkVideoContacts.at(0) != NULL)
        {
            ((OsMediaContact*)sinkVideoContacts.at(0))->getAddress(videoHostAddr);
            videoRtpPort = ((OsMediaContact*)sinkVideoContacts.at(0))->getPort();
            videoRtcpPort = ((OsMediaContact*)sinkVideoContacts.at(0))->getRtcpPort();
        }

        printf("rtpHostAddresses1: \"%s\"\nrtpAudioPorts1: %d\nrtcpAudioPorts1: %d\nrtpVideoPorts1: %d\nrtcpVideoPorts1: %d\n",
               hostAddr.data(),
               audioRtpPort,
               audioRtcpPort,
               videoRtpPort,
               videoRtcpPort);

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             sourceInterface->setConnectionDestination(sourceConnectionId,
                                                                      hostAddr.data(), 
                                                                      audioRtpPort,
                                                                      audioRtcpPort,
                                                                      0,
                                                                      videoHostAddr.data(), 
                                                                      videoRtpPort,
                                                                      videoRtcpPort,
                                                                      0));

        // Start sending RTP from source to the sink flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             sourceInterface->startRtpSend(sourceConnectionId, 
                                                           numSupportedSinkCodecs,
                                                           supportedSinkCodecArray));


        RTL_EVENT("Tone count", 0);

        // Record the entire "call" - all connections.
        sinkInterface->recordChannelAudio(-1, "testConnectionNotifications_callrec.wav");

        RTL_EVENT("Tone count", 1);
        printf("generate tone in source\n");
        sourceInterface->startTone(1, true, true);
        MiNotification* pNotf;
        OsStatus stat = waitForNotf(sinkNotfDispatcher, 
                                    MiNotification::MI_NOTF_DTMF_RECEIVED, 50, &pNotf);
        KNOWN_BUG("DTMF is not working now.", "???");
        CPPUNIT_ASSERT_MESSAGE("Didn't receive DTMF key down message after startTone", stat == OS_SUCCESS);
        MiDtmfNotf* pDtmfNotf = (MiDtmfNotf*)pNotf;
        CPPUNIT_ASSERT_EQUAL(MiDtmfNotf::DTMF_1, pDtmfNotf->getKeyCode());
        CPPUNIT_ASSERT_EQUAL(MiDtmfNotf::KEY_DOWN, pDtmfNotf->getKeyPressState());
        CPPUNIT_ASSERT_EQUAL(MiDtmfNotf::DURATION_NOT_APPLICABLE, pDtmfNotf->getDuration());
        pDtmfNotf = NULL;
        pNotf->releaseMsg();
        pNotf = NULL;

        OsTask::delay(1000);

        RTL_EVENT("Tone count", 0);
        printf("stop tone in source\n");
        sourceInterface->stopTone();
        stat = waitForNotf(sinkNotfDispatcher, 
                           MiNotification::MI_NOTF_DTMF_RECEIVED, 50, &pNotf);
        pDtmfNotf = (MiDtmfNotf*)pNotf;
        CPPUNIT_ASSERT_MESSAGE("Didn't receive DTMF key up message after startTone", stat == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(MiDtmfNotf::DTMF_1, pDtmfNotf->getKeyCode());
        CPPUNIT_ASSERT_EQUAL(MiDtmfNotf::KEY_UP, pDtmfNotf->getKeyPressState());
        CPPUNIT_ASSERT(pDtmfNotf->getDuration() > 0);
        pDtmfNotf = NULL;
        pNotf->releaseMsg();
        pNotf = NULL;

        OsTask::delay(200);
        printf("tone testing done\n");

        printf("%d media notifications are awaiting retrieval\n", sinkNotfDispatcher.numMsgs());

        // Stop recording the "call" -- all connections.
        sinkInterface->stopRecordChannelAudio(-1);

        // Delete connections
        sinkInterface->deleteConnection(sinkConnectionId);
        sourceInterface->deleteConnection(sourceConnectionId);

        // delete interfaces
        sinkInterface->release();
        sourceInterface->release();

        OsTask::delay(500) ;

        RTL_WRITE("testTwoTones.rtl");
        RTL_STOP;

        for ( numSupportedSinkCodecs--; numSupportedSinkCodecs >= 0; numSupportedSinkCodecs--)
        {
           delete supportedSinkCodecArray[numSupportedSinkCodecs];
        }
        delete[] supportedSinkCodecArray;
    };


    void testThreeGraphs()
    {
        RTL_START(2400000);

        // This test creates three flowgraphs.  It streams RTP with prompts
        // from the 2nd and 3rd to be received and mixed in the first flowgraph
        // So we test RTP and we test that we can play from 2 different files in
        // to different flowgraphs.
        CPPUNIT_ASSERT(mpMediaFactory);

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

        ProxyDescriptor stunServer;
        ProxyDescriptor turnServer;
        ProxyDescriptor arsProxy;
        ProxyDescriptor arsHttpProxy;
        SdpCodecList supportedCodecs;
        int numCodecs;
        SdpCodec** codecArray = NULL;
        supportedCodecs.getCodecs(numCodecs, codecArray);


        // Create a flowgraph (sink) to receive and mix 2 sources
        CpMediaInterface* mixedInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);

        // Create connections for mixed(sink) flowgraph
        int mixedConnection1Id = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->createConnection(mixedConnection1Id, true, NULL));
        CPPUNIT_ASSERT(mixedConnection1Id > 0);
        int mixedConnection2Id = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->createConnection(mixedConnection2Id, true, NULL));
        CPPUNIT_ASSERT(mixedConnection2Id > 0);
        
        // Get the address of the connections so we can send RTP to them
        // capabilities of first connection on mixed(sink) flowgraph
        SdpCodecList supportedCodecs3;
        SdpSrtpParameters srtpParameters3;
        int bandWidth3 = 0;
        int videoBandwidth3;
        int videoFramerate3;
        UtlSList audioContacts3;
        UtlSList videoContacts3;
        /*
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
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
                                                               videoFramerate1));

                                                               */
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->getCapabilities(mixedConnection1Id, 
                                &audioContacts3,
                                &videoContacts3,
                                supportedCodecs3,
                                srtpParameters3,
                                0,
                                videoBandwidth3,
                                videoFramerate3)
                             );

        // capabilities of second connection on mixed(sink) flowgraph
        SdpCodecList supportedCodecs2;
        SdpSrtpParameters srtpParameters2;
        int bandWidth2 = 0;
        int videoBandwidth2;
        int videoFramerate2;
        UtlSList audioContacts2;
        UtlSList videoContacts2;
        /*
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
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
                                                               videoFramerate2));
        */
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->getCapabilities(mixedConnection2Id, 
                                &audioContacts2,
                                &videoContacts2,
                                supportedCodecs2,
                                srtpParameters2,
                                0,
                                videoBandwidth2,
                                videoFramerate2)
                             );

        // Prep the sink connections to receive RTP
        int numCodecsFactory1;
        SdpCodec** codecArray1 = NULL;
        supportedCodecs2.getCodecs(numCodecsFactory1, codecArray1);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->startRtpReceive(mixedConnection1Id,
                                                             numCodecsFactory1,
                                                             codecArray1));

        // Want to hear what is on the mixed flowgraph
        mixedInterface->giveFocus();

        int numCodecsFactory2;
        SdpCodec** codecArray2 = NULL;
        supportedCodecs2.getCodecs(numCodecsFactory2, codecArray2);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->startRtpReceive(mixedConnection2Id,
                                                             numCodecsFactory2,
                                                             codecArray2));

        // Second flowgraph to be one of two sources
        CpMediaInterface* source1Interface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);

        // Create connection for source 1 flowgraph
        int source1ConnectionId = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source1Interface->createConnection(source1ConnectionId, true, NULL));
        CPPUNIT_ASSERT(source1ConnectionId > 0);

        // Set the destination for sending RTP from source 1 to connection 1 on
        // the mix flowgraph
        UtlString audioHostAddr2;
        UtlString videoHostAddr2;
        UtlString hostAddr;
        UtlString videoHostAddr;
        int audioRtpPort = 0;
        int audioRtcpPort = 0;
        int videoRtpPort = 0;
        int videoRtcpPort = 0;
        if (audioContacts2.at(0) != NULL)
        {
            ((OsMediaContact*)audioContacts2.at(0))->getAddress(hostAddr);
            audioRtpPort = ((OsMediaContact*)audioContacts2.at(0))->getPort();
            audioRtcpPort = ((OsMediaContact*)audioContacts2.at(0))->getRtcpPort();
        }
        if (videoContacts2.at(0) != NULL)
        {
            ((OsMediaContact*)videoContacts2.at(0))->getAddress(videoHostAddr);
            videoRtpPort = ((OsMediaContact*)videoContacts2.at(0))->getPort();
            videoRtcpPort = ((OsMediaContact*)videoContacts2.at(0))->getRtcpPort();
        }

        printf("rtpHostAddresses1: \"%s\"\nrtpAudioPorts1: %d\nrtcpAudioPorts1: %d\nrtpVideoPorts1: %d\nrtcpVideoPorts1: %d\n",
               hostAddr.data(),
               audioRtpPort,
               audioRtcpPort,
               videoRtpPort,
               videoRtcpPort);

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source1Interface->setConnectionDestination(source1ConnectionId,
                                                                      hostAddr.data(), 
                                                                      audioRtpPort,
                                                                      audioRtcpPort,
                                                                      0,
                                                                      videoHostAddr.data(), 
                                                                      videoRtpPort,
                                                                      videoRtcpPort,
                                                                      0));

        // Start sending RTP from source 1 to the mix flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source1Interface->startRtpSend(source1ConnectionId, 
                                                            numCodecsFactory1,
                                                            codecArray1));

        // Second flowgraph to be one of two sources
        CpMediaInterface* source2Interface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);

        // Create connection for source 2 flowgraph
        int source2ConnectionId = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source2Interface->createConnection(source2ConnectionId, true, NULL));
        CPPUNIT_ASSERT(source2ConnectionId > 0);

        // Set the destination for sending RTP from source 2 to connection 2 on
        // the mix flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source2Interface->setConnectionDestination(source2ConnectionId,
                                                                      hostAddr.data(), 
                                                                      audioRtpPort,
                                                                      audioRtcpPort,
                                                                      0,
                                                                      videoHostAddr.data(), 
                                                                      videoRtpPort,
                                                                      videoRtcpPort,
                                                                      0));

        RTL_EVENT("Prompt", 0);

        // Record the entire "call" - all connections.
        mixedInterface->recordChannelAudio(-1, "testThreeGraphs_call_recording.wav");

        // Start sending RTP from source 2 to the mix flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             source2Interface->startRtpSend(source2ConnectionId, 
                                                            numCodecsFactory2,
                                                            codecArray2));

        RTL_EVENT("Prompt", 1);
        printf("playing prompt in source 1\n");
        int waveHeaderSize = 116;
        CPPUNIT_ASSERT_EQUAL((unsigned)160116, sizeof(sine_330hz_16b_8k_signed));
        source1Interface->playBuffer((char*)&sine_330hz_16b_8k_signed[waveHeaderSize], 
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

        OsTask::delay(1000);

        RTL_EVENT("Prompt", 2);
        printf("playing prompt in source 2 as well\n");
        //source2Interface->startTone(2, true, true);
        source2Interface->playBuffer((char*)&sine_530hz_16b_8k_signed[waveHeaderSize], 
                                     sizeof(sine_530hz_16b_8k_signed) - waveHeaderSize, 
                                     PROMPTBUF_SAMPLERATE,
                                     0, //type, 
                                     FALSE, // repeat,
                                     FALSE, // local, 
                                     TRUE //remote,
                                     //OsProtectedEvent* event = NULL,
                                     //UtlBoolean mixWithMic = false,
                                     //int downScaling = 100
                                     );

        OsTask::delay(1000);

        RTL_EVENT("Prompt", 1);
        printf("stopping prompt in source 1\n");
        source1Interface->stopAudio();


        OsTask::delay(1000);

        RTL_EVENT("Prompt", 0);
        printf("stopping prompt in source 2\n");
        source2Interface->stopAudio();

        OsTask::delay(1000);
        printf("three graphs done\n");        

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

        RTL_WRITE("testThreeGraphs.rtl");
        RTL_STOP;

        // delete codecs set
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
    };

    /// This test creates one flowgraph and stream its output to its own
    /// input. It try to simulate usual call with only two parties.
    void testOuroboros()
    {
        RTL_START(2400000);

        CPPUNIT_ASSERT(mpMediaFactory);

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

        ProxyDescriptor stunServer;
        ProxyDescriptor turnServer;
        ProxyDescriptor arsProxy;
        ProxyDescriptor arsHttpProxy;
        SdpCodecList supportedCodecs;
        int numCodecs;
        SdpCodec** codecArray = NULL;
        supportedCodecs.getCodecs(numCodecs, codecArray);


        // Create a flowgraph to receive and mix 2 sources
        CpMediaInterface* mixedInterface = 
            mpMediaFactory->createMediaInterface(NULL, // public mapped RTP IP address
                                                 localRtpInterfaceAddress, 
                                                 numCodecs, 
                                                 codecArray, 
                                                 locale,
                                                 tosOptions,
                                                 stunServer,
                                                 turnServer,
                                                 arsProxy,
                                                 arsHttpProxy,
                                                 NULL,
                                                 enableIce,
                                                 8000);

        // Create connection
        int connectionId = -1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->createConnection(connectionId, true, NULL));
        CPPUNIT_ASSERT(connectionId > 0);
        
        // Get the address of the connections so we can send RTP to them
        // capabilities of first connection on mixed(sink) flowgraph
        SdpCodecList supportedCodecs1;
        SdpSrtpParameters srtpParameters1;
        int bandWidth1 = 0;
        int videoBandwidth1;
        int videoFramerate1;
        UtlSList audioContacts1;
        UtlSList videoContacts1;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->getCapabilities(connectionId, 
                                &audioContacts1,
                                &videoContacts1,
                                supportedCodecs1,
                                srtpParameters1,
                                0,
                                videoBandwidth1,
                                videoFramerate1)
                             );

        // Prep the connection's sink to receive RTP
        int numCodecsFactory;
        supportedCodecs1.getCodecs(numCodecsFactory, codecArray);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->startRtpReceive(connectionId,
                                                             numCodecsFactory,
                                                             codecArray));

        // Want to hear what is on the mixed flowgraph
        mixedInterface->giveFocus();

        // Set the destination for sending RTP from source 1 to connection 1 on
        // the mix flowgraph
        UtlString hostAddr;
        UtlString videoHostAddr;
        int audioRtpPort = 0;
        int audioRtcpPort = 0;
        int videoRtpPort = 0;
        int videoRtcpPort = 0;
        if (audioContacts1.at(0) != NULL)
        {
            ((OsMediaContact*)audioContacts1.at(0))->getAddress(hostAddr);
            audioRtpPort = ((OsMediaContact*)audioContacts1.at(0))->getPort();
            audioRtcpPort = ((OsMediaContact*)audioContacts1.at(0))->getRtcpPort();
        }
        if (videoContacts1.at(0) != NULL)
        {
            ((OsMediaContact*)videoContacts1.at(0))->getAddress(videoHostAddr);
            videoRtpPort = ((OsMediaContact*)videoContacts1.at(0))->getPort();
            videoRtcpPort = ((OsMediaContact*)videoContacts1.at(0))->getRtcpPort();
        }

        printf("rtpHostAddresses1: \"%s\"\nrtpAudioPorts1: %d\nrtcpAudioPorts1: %d\nrtpVideoPorts1: %d\nrtcpVideoPorts1: %d\n",
               hostAddr.data(),
               audioRtpPort,
               audioRtcpPort,
               videoRtpPort,
               videoRtcpPort);

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             mixedInterface->setConnectionDestination(connectionId,
                                                                      hostAddr.data(), 
                                                                      audioRtpPort,
                                                                      audioRtcpPort,
                                                                      0,
                                                                      videoHostAddr.data(), 
                                                                      videoRtpPort,
                                                                      videoRtcpPort,
                                                                      0));

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

        OsTask::delay(3000);

        mixedInterface->stopAudio();

        OsTask::delay(500);

        // Stop recording the "call" -- all connections.
        mixedInterface->stopRecordChannelAudio(-1);

        // Delete connections
        mixedInterface->deleteConnection(connectionId);

        // delete interfaces
        mixedInterface->release();

        OsTask::delay(500) ;

        RTL_WRITE("testOuroboros.rtl");
        RTL_STOP;

        // delete codecs set
        for ( numCodecsFactory--; numCodecsFactory>=0; numCodecsFactory--)
        {
           delete codecArray[numCodecsFactory];
        }
        delete[] codecArray;
    };

};

CPPUNIT_TEST_SUITE_REGISTRATION(CpPhoneMediaInterfaceTest);
