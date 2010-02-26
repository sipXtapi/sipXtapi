//  
// Copyright (C) 2008-2010 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MpMisc.h>
#include <mp/MprBridge.h>
#include <mp/MpBufferMsg.h>
#include <mp/MpAudioBuf.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MpOutputDeviceManager.h>
#include <mp/MprFromInputDevice.h>
#include <mp/MprFromFile.h>
#include <mp/MprMixer.h>
#include <mp/MprToOutputDevice.h>
#include <mp/MpodBufferRecorder.h>
#include <mp/MpMisc.h>
#include <mp/MpMediaTask.h>
#include <os/OsEvent.h>
#include <os/OsDateTime.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlInt.h>
#include <os/OsMsgDispatcher.h>
#include <mp/MpResNotificationMsg.h>

#include "mp/MpGenericResourceTest.h"

#ifdef RTL_ENABLED // [
#  include "rtl_macro.h"
#else  // RTL_ENABLED ][
#  define RTL_WRITE(x)
#  define RTL_BLOCK(x)
#  define RTL_START(x)
#  define RTL_STOP
#endif // RTL_ENABLED ]


#define TEST_MAX_SAMPLE_RATE     48000
#define FREQ_TO_PERIODUS(X)      1000000/X

//#undef USE_TEST_OUTPUT_DRIVER
#define USE_TEST_OUTPUT_DRIVER

#ifdef USE_TEST_OUTPUT_DRIVER // USE_TEST_OUTPUT_DRIVER [
#  include <mp/MpodBufferRecorder.h>
#define TEST_SAMPLE_DATA_SIZE sizeof (MpAudioSample)
#define TEST_SAMPLES_PER_SECOND TEST_MAX_SAMPLE_RATE
#  define OUTPUT_DRIVER MpodBufferRecorder
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "default", TEST_SAMPLE_DATA_SIZE*1000/TEST_SAMPLES_PER_SECOND
#elif defined(WIN32) // USE_TEST_OUTPUT_DRIVER ][ WIN32
#  include <mp/MpodWinMM.h>
#  define OUTPUT_DRIVER MpodWinMM
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS MpodWinMM::getDefaultDeviceName()
#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#  if __APPLE__
#     include <mp/MpodCoreAudio.h>
#     define OUTPUT_DRIVER MpodCoreAudio
#     define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "[default]"
#  else
#     include <mp/MpodOss.h>
#     define OUTPUT_DRIVER MpodOss
#     define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "/dev/dsp"
# endif
#else // __pingtel_on_posix__ ]
#  error Unknown platform!
#endif

///  Unit test for MprBridgeWB
class MprBridgeTestWB : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUB_SUITE(MprBridgeTestWB, MpGenericResourceTest);
    CPPUNIT_TEST(bridgingTwoFreqsWB);
    CPPUNIT_TEST_SUITE_END();

public:

   void bridgingTwoFreqs()
   {
      const int duration = 3000;

      const int freq1 = 1200;
      const int sineMagnitude1 = 10000;
      const int freq2 = 3200;
      const int sineMagnitude2 = 10000;

      const int  numParticipants = 2;
      MprBridge* pBridge    = NULL;

      pBridge = new MprBridge("MprBridge", numParticipants+1);
      CPPUNIT_ASSERT(pBridge != NULL);
      
      // Create MediaTask
      MpMediaTask *pMediaTask = MpMediaTask::getMediaTask();
      if(pMediaTask == NULL)
      {
          MpMediaTask::createMediaTask(10);
      }
      // Get media task ticker.
      OsNotification *pTicker = pMediaTask->getTickerNotification();

      //setupFramework(pBridge);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->setNotificationsEnabled(true));

      // Things we need for the general flowgraph
      MpInputDeviceManager* pInMgr = NULL;
      MpOutputDeviceManager* pOutMgr = NULL;
    
      pInMgr = new MpInputDeviceManager(getSamplesPerFrame(), getSamplesPerSec(), 
                                        5, *MpMisc.RawAudioPool);
      pOutMgr = new MpOutputDeviceManager(getSamplesPerFrame(), getSamplesPerSec(), 
                                          30);

      MpSineWaveGeneratorDeviceDriver* pPlaySineInDrv1 = NULL;
      MpInputDeviceHandle playSineDevHnd1 = 0;
      MprFromInputDevice* pFromInDev1 = NULL;
      MpSineWaveGeneratorDeviceDriver* pPlaySineInDrv2 = NULL;
      MpInputDeviceHandle playSineDevHnd2 = 0;
      MprFromInputDevice* pFromInDev2 = NULL;

      OUTPUT_DRIVER* pSpkrOutDrv = NULL;
      MpOutputDeviceHandle spkrDevHnd = 0;
      MprToOutputDevice* pToOutputDev1 = NULL;
      MpodBufferRecorder* pSpkrOutDrv2 = NULL;
      MpOutputDeviceHandle spkrDevHnd2 = 0;
      MprToOutputDevice* pToOutputDev2 = NULL;

      pPlaySineInDrv1 =
         new MpSineWaveGeneratorDeviceDriver("SineGeneratorDriver1", *pInMgr,
                                             sineMagnitude1,
                                             FREQ_TO_PERIODUS(freq1), 0);
      playSineDevHnd1 = pInMgr->addDevice(*pPlaySineInDrv1);
      CPPUNIT_ASSERT(playSineDevHnd1 > 0);

      pPlaySineInDrv2 =
         new MpSineWaveGeneratorDeviceDriver("SineGeneratorDriver2", *pInMgr,
                                             sineMagnitude2,
                                             FREQ_TO_PERIODUS(freq2), 0);
      playSineDevHnd2 = pInMgr->addDevice(*pPlaySineInDrv2);
      CPPUNIT_ASSERT(playSineDevHnd2 > 0);

      pSpkrOutDrv = new OUTPUT_DRIVER(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      spkrDevHnd = pOutMgr->addDevice(pSpkrOutDrv);
      CPPUNIT_ASSERT(spkrDevHnd > 0);

      pSpkrOutDrv2 = new MpodBufferRecorder("MpodRecorder2", duration);
      spkrDevHnd2 = pOutMgr->addDevice(pSpkrOutDrv2);
      CPPUNIT_ASSERT(spkrDevHnd > 0);

      // Create the resources for the 'play' leg.
      pFromInDev1 = 
         new MprFromInputDevice("FromInput-PlaySine1", pInMgr, playSineDevHnd1);
      pFromInDev2 = 
         new MprFromInputDevice("FromInput-PlaySine2", pInMgr, playSineDevHnd2);

      pToOutputDev1 =
         new MprToOutputDevice("ToOutput-PlaySpkr1", pOutMgr, spkrDevHnd);
      pToOutputDev2 =
         new MprToOutputDevice("ToOutput-PlaySpkr2", pOutMgr, spkrDevHnd2);


      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         mpFlowGraph->addResource(*pBridge));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         mpFlowGraph->addResource(*pFromInDev1));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         mpFlowGraph->addResource(*pFromInDev2));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         mpFlowGraph->addResource(*pToOutputDev1));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         mpFlowGraph->addResource(*pToOutputDev2));


      // Connect resources to bridge
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         mpFlowGraph->addLink(*pFromInDev1, 0, 
                              *pBridge, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         mpFlowGraph->addLink(*pFromInDev2, 0, 
                              *pBridge, 1));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         mpFlowGraph->addLink(*pBridge, 0, 
                              *pToOutputDev1, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         mpFlowGraph->addLink(*pBridge, 1, 
                              *pToOutputDev2, 0));


      OsMsgQ* flowgraphQueue = mpFlowGraph->getMsgQ();
      CPPUNIT_ASSERT(flowgraphQueue != NULL);

      MpBridgeGain gainsOut[numParticipants] =
         {MP_BRIDGE_GAIN_PASSTHROUGH, MP_BRIDGE_GAIN_PASSTHROUGH};
      
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         MprBridge::setMixWeightsForOutput("MprBridge",
            *flowgraphQueue, 0, numParticipants, gainsOut));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
         MprBridge::setMixWeightsForOutput("MprBridge",
            *flowgraphQueue, 1, numParticipants, gainsOut));


      // Enable all of the devices..
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pInMgr->enableDevice(playSineDevHnd1));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pOutMgr->enableDevice(spkrDevHnd));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pInMgr->enableDevice(playSineDevHnd2));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pOutMgr->enableDevice(spkrDevHnd2));

      // Set the flowgraph ticker source..
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         pOutMgr->setFlowgraphTickerSource(spkrDevHnd, pTicker));


      // Now we enable the flowgraph..  Which should enable resources.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->enable());


      // Manage the flow graph so it flows, and finally start it flowing, 
      // and bring it into focus
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->manageFlowGraph(*mpFlowGraph));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->startFlowGraph(*mpFlowGraph));

      pPlaySineInDrv1->setMagnitude(0);
      OsTask::delay(duration/3);
      pPlaySineInDrv1->setMagnitude(sineMagnitude1);
      OsTask::delay(duration/3);
      pPlaySineInDrv2->setMagnitude(0);
      OsTask::delay(duration/3);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         MpResource::disable("FromInput-PlaySine1", *mpFlowGraph->getMsgQ()));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         MpResource::disable("FromInput-PlaySine2", *mpFlowGraph->getMsgQ()));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pInMgr->disableDevice(playSineDevHnd1));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pInMgr->disableDevice(playSineDevHnd2));


      // Clear the flowgraph ticker so it stops ticking..
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         pOutMgr->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE, NULL));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->stopFlowGraph(*mpFlowGraph));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->unmanageFlowGraph(*mpFlowGraph));
      // Let media task to process unmanage message (we have stopped ticker, so
      // have to manually feed media task with ticks).
      MpMediaTask::signalFrameStart();
      OsTask::delay(20);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pOutMgr->disableDevice(spkrDevHnd));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pOutMgr->disableDevice(spkrDevHnd2));

      // Stop the flow graph, process next frame so the stop actually happens.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->stop());
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->processNextFrame());


      pInMgr->removeDevice(playSineDevHnd1);
      pInMgr->removeDevice(playSineDevHnd2);
      pOutMgr->removeDevice(spkrDevHnd);
      pOutMgr->removeDevice(spkrDevHnd2);

      delete pInMgr;
      delete pOutMgr;

      delete pPlaySineInDrv1;
      delete pPlaySineInDrv2;
      delete pSpkrOutDrv;
      delete pSpkrOutDrv2;
   }

   void bridgingTwoFreqsWB()
   {
      printf("Playing 3200 kz, 1200 kz + 3200 hz, 1200 hz tones with different flowgraph samplerate...\n");
      size_t     i;	 
      for(i = 0; i < sNumRates; i++)	 
      {	 
         printf("Test sample rate %d\n", sSampleRates[i]);	 
         // For this test, we want to modify the sample rate and samples per frame	 
         // so we need to de-inititialize what has already been initialized for us	 
         // by cppunit, or by a previous loop.	 
         tearDown();	 

         // Set the sample rates	 
         setSamplesPerSec(sSampleRates[i]);	 
         setSamplesPerFrame(sSampleRates[i]/100);	 
         setUp();

         bridgingTwoFreqs();
      }
   }

};


CPPUNIT_TEST_SUITE_REGISTRATION(MprBridgeTestWB);
