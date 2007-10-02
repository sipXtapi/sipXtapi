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
#include <mp/MprFromFile.h>

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
      //printf("Hit event %d\n", eventData);
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

class PlayAudioOldNotificationTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PlayAudioOldNotificationTest);
    CPPUNIT_TEST(testRecordPlayback);
    CPPUNIT_TEST_SUITE_END();

    public:

    CpMediaInterfaceFactory* mpMediaFactory;

    PlayAudioOldNotificationTest()
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
        unsigned i;
        for (i=0; i< 100; i++)
        {
           printf("Play record_prompt.wav taskId: %d\n",taskId);
           mediaInterface->playAudio("short.wav", 
                                     false, //repeat
                                     true, // local
                                     false, //remote
                                     false,
                                     100,
                                     &playAudNote);
           OsTask::delay(250);
           //enableConsoleOutput(0);

           // Check via old OsNotification mechanism if the file finished playing.
           // We should have 3 notifications:
           // PLAYING(2), PLAY_STOPPED(1), PLAY_FINISHED(0)
           // Check this.
           printf("loopcnt %d: %d event(s) on play event queue:  ", i, playAudNote.mEDataList.entries());
           CPPUNIT_ASSERT_EQUAL(size_t(3), playAudNote.mEDataList.entries());
           int evtData = -1;
           CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, playAudNote.popLastEvent(evtData));
           CPPUNIT_ASSERT_EQUAL(int(MprFromFile::PLAYING), evtData);
           printf("PLAYING, ");
           evtData = -1;
           CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, playAudNote.popLastEvent(evtData));
           CPPUNIT_ASSERT_EQUAL(int(MprFromFile::PLAY_STOPPED), evtData);
           printf("PLAY_STOPPED, ");
           evtData = -1;
           CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, playAudNote.popLastEvent(evtData));
           CPPUNIT_ASSERT_EQUAL(int(MprFromFile::PLAY_FINISHED), evtData);
           printf("PLAY_FINISHED\n");

           mediaInterface->stopAudio() ;
        }
        
        mediaInterface->startTone(0, true, false) ;
        OsTask::delay(100) ;
        mediaInterface->stopTone() ;
        OsTask::delay(100) ;

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
};

CPPUNIT_TEST_SUITE_REGISTRATION(PlayAudioOldNotificationTest);
