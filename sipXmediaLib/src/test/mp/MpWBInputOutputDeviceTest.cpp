//  
// Copyright (C) 2007-2010 SIPez LLC.   All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#include <os/OsIntTypes.h>
#include <sipxunittests.h>

#include <mp/MpAudioBuf.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#include <mp/MpInputDeviceManager.h>

#include <mp/MpFlowGraphBase.h>
#include <mp/MpOutputDeviceManager.h>
#include <mp/MprFromInputDevice.h>
#include <mp/MprFromFile.h>
#include <mp/MprMixer.h>
#include <mp/MprToOutputDevice.h>
#include <mp/MpodBufferRecorder.h>
#include <mp/MpMisc.h>
#include <mp/MpMediaTask.h>
#include <os/OsEvent.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlInt.h>
#include <os/OsMsgDispatcher.h>
#include <mp/MpResNotificationMsg.h>

// Include static wave data headers.
// These were generated from 16bit 48kHz mono raw sound files, using the following command:
// $ incbin.exe file.raw WBMixerTestTones.h -n=WBMixerTestTones -c=13 -d -h
#include "WBMixerTestTones.h"
#include "WBMixerTestReversedTones.h"

#define NUM_BUFFERS                   500
#define TEST_SAMPLE_DATA_MAGNITUDE    INT16_MAX
#define TEST_DRIVER_SINE_PERIOD       (1000000/60) //in microseconds 60 Hz
#define TEST_MAX_SAMPLE_RATE          48000
#define TEST_MAX_SAMPLES_PER_FRAME    TEST_MAX_SAMPLE_RATE/100
#define NUM_INPUT_DEV_BUFFERED_FRAMES 5  

#define TICKER_TEST_WRITE_RUNS_NUMBER 3
#define FREQ_TO_PERIODUS(X) 1000000/X

#undef USE_TEST_INPUT_DRIVER
#undef USE_TEST_OUTPUT_DRIVER

#ifdef USE_TEST_INPUT_DRIVER // USE_TEST_INPUT_DRIVER [
#  include <mp/MpSineWaveGeneratorDeviceDriver.h>
#  define INPUT_DRIVER MpSineWaveGeneratorDeviceDriver
#  define INPUT_DRIVER_CONSTRUCTOR_PARAMS(mgr) "default", mgr, TEST_SAMPLE_DATA_MAGNITUDE, TEST_DRIVER_SINE_PERIOD, 0
#elif defined(WIN32) // USE_TEST_INPUT_DRIVER ][ WIN32
#  include <mp/MpidWinMM.h>
#  define INPUT_DRIVER MpidWinMM
#  define INPUT_DRIVER_CONSTRUCTOR_PARAMS(mgr) MpidWinMM::getDefaultDeviceName(), mgr
#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#  if __APPLE__
#     include <mp/MpidCoreAudio.h>
#     define INPUT_DRIVER MpidCoreAudio
#     define INPUT_DRIVER_CONSTRUCTOR_PARAMS(mgr) "[default]", mgr
#  else
#     include <mp/MpidOss.h>
#     define INPUT_DRIVER MpidOss
#     define INPUT_DRIVER_CONSTRUCTOR_PARAMS(mgr) "/dev/dsp", mgr
#  endif
#else // __pingtel_on_posix__ ]
#  error Unknown platform!
#endif

#ifdef USE_TEST_OUTPUT_DRIVER // USE_TEST_OUTPUT_DRIVER [
#  include <mp/MpodBufferRecorder.h>
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
#endif
#else // __pingtel_on_posix__ ]
#  error Unknown platform!
#endif

/**
 * Unittest for Wide band support in input and output device driver
 */
class WBInputOutputDeviceTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(WBInputOutputDeviceTest);
   CPPUNIT_TEST(testMixerWB);
   CPPUNIT_TEST(testInputOutputWB);
   CPPUNIT_TEST_SUITE_END();


public:
   void printCountdownDelay(size_t nSecs)
   {
      assert(nSecs < 10);  // this only works for values < 10.
      printf("  seconds.");
      size_t i;
      for(i = nSecs; i > 0; i--)
      {
         printf("\b\b\b\b\b\b\b\b\b\b%d seconds.", (int)i);
         fflush(stdout);
         OsTask::delay(1000);
      }
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\bNOW.         \n");
      fflush(stdout);

   }

   // Caller owns pFlowgraph.
   void setupWBMediaTask(unsigned sampleRate, unsigned samplesPerFrame,
                         MpInputDeviceManager*& pInMgr,
                         MpOutputDeviceManager*& pOutMgr,
                         MpFlowGraphBase*& pFlowgraph, MpMediaTask*& pMediaTask)
   {
      MpFrameTime outMgrDefaultMixerBufLen = 30;

      // Both pointers passed in should be NULL, as we allocate it here.
      assert(pFlowgraph == NULL);
      assert(pMediaTask == NULL);

      unsigned mpTaskNBuffers = 6*10; // # taken from MpFlowGraphTest

      // Setup codec paths..
      UtlString codecPaths[] = {
#ifdef WIN32
         "bin",
         "..\\bin",
#elif __pingtel_on_posix__
         "../../../../bin",
         "../../../bin",
#else
#        error "Unknown platform"
#endif
         "."
      };
      int numCodecPaths = sizeof(codecPaths)/sizeof(codecPaths[0]);

      // Setup media task
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         mpStartUp(sampleRate, samplesPerFrame, mpTaskNBuffers, NULL,
         numCodecPaths, codecPaths));

      pFlowgraph = new MpFlowGraphBase(samplesPerFrame, sampleRate);

      // Create MediaTask
      pMediaTask = MpMediaTask::createMediaTask(10);
      
      // Turn on notifications
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowgraph->setNotificationsEnabled(true));

      // Create the input and output managers -- those are common to both the 
      // 'record' and 'play' legs of the flowgraph.
      pInMgr = new MpInputDeviceManager(samplesPerFrame, sampleRate, 
                                        NUM_INPUT_DEV_BUFFERED_FRAMES, *MpMisc.RawAudioPool);
      pOutMgr = new MpOutputDeviceManager(samplesPerFrame, sampleRate, 
                                          outMgrDefaultMixerBufLen);

   }

   void tearDownWBMediaTask(MpInputDeviceManager& inMgr, UtlSList& inDevHandles, 
                            MpOutputDeviceManager& outMgr, UtlSList& outDevHandles,
                            MpFlowGraphBase* pFlowgraph, MpMediaTask*& pMediaTask)
   {
      // Clear the flowgraph ticker so it stops ticking..
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         outMgr.setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE, NULL));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->stopFlowGraph(*pFlowgraph));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->unmanageFlowGraph(*pFlowgraph));
      // Let media task to process unmanage message (we have stopped ticker, so
      // have to manually feed media task with ticks).
      MpMediaTask::signalFrameStart();
      while (pMediaTask->isManagedFlowGraph(pFlowgraph))
      {
         OsTask::delay(20);
      }

      // Disable all of the devices..
      UtlInt* pCurHnd;
      UtlSListIterator outHndIter(outDevHandles);
      while((pCurHnd = (UtlInt*)outHndIter()) != NULL)
      {
         if (outMgr.isDeviceEnabled(pCurHnd->getValue()))
         {
            // Disable the output devices.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, outMgr.disableDevice(pCurHnd->getValue()));
         }
      }
      UtlSListIterator inHndIter(inDevHandles);
      while((pCurHnd = (UtlInt*)inHndIter()) != NULL)
      {
         if (inMgr.isDeviceEnabled(pCurHnd->getValue()))
         {
            // Disable the input devices.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inMgr.disableDevice(pCurHnd->getValue()));
         }
      }

      // Stop the flow graph, process next frame so the stop actually happens.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowgraph->stop());
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowgraph->processNextFrame());

      // Remove all drivers from managers..
      outHndIter.reset(); // reset the iterator positions to the beginning of the lists.
      inHndIter.reset();
      while((pCurHnd = (UtlInt*)inHndIter()) != NULL)
      {
         // Remove the input devices.
         inMgr.removeDevice(pCurHnd->getValue());
      }
      while((pCurHnd = (UtlInt*)outHndIter()) != NULL)
      {
         // Remove the output devices.
         outMgr.removeDevice(pCurHnd->getValue());
      }

      // Clear all media processing data and delete MpMediaTask instance.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpShutdown());

      // Set the media task to null, since (I think) mpShutdown deletes it.
      pMediaTask = NULL;
   }

   // This is a CPPUNIT compliant method that actually does the calling of the wideband
   // test, giving various values for sample rates.
   void testInputOutputWB()
   {
      printf("For this test, please plug in a male minijack-to-male minijack "
             "cable from your default sound card's speaker output to it's mic "
             "input.\n");
      printf("The test will begin in ");
      printCountdownDelay(5); // 5 seconds.
      int i;
      for(i = 0; i < sNumRates; i++)
      {
         printf("Testing rate %d\n", sSampleRates[i]);
         testIOFreqRange(sSampleRates[i], sSampleRates[i]/100);

         // If there's going to be another run, pause a bit.
         if(i < sNumRates-1)
         {
            printf("Please plug the minijack-to-minijack cable back into your "
                   "mic and speaker jacks now.\n"
                   "We will proceed with testing the next sample rate in ");
            printCountdownDelay(3);
         }
      }
   }

   void testMixerWB()
   {
      int i;
      for(i = 0; i < sNumRates; i++)
      {
         printf("Testing rate %d\n", sSampleRates[i]);

         testMixerWBHelper(sSampleRates[i], sSampleRates[i]/100);

         // If there's going to be another run, pause a bit.
         if(i < sNumRates-1)
         {
            OsTask::delay(500);
         }
      }
   }

   /**
   *  Test wideband support in flowgraph, from input device, through resources,
   *  to output device.
   *
   *  This test does the following:
   *  Sets up one flowgraph with two legs.  
   *     * One leg of the flowgraph, the 'record' leg, which just listens to the 
   *       microphone, recording to a buffer output device driver.
   *     * The second leg, the 'play' leg uses a sine wave generator device
   *       driver to generate tones, and plays them out to the speaker.
   *  
   *  The tester is expected to connect the speaker output to the microphone 
   *  input via a male minijack-to-male minijack cable, thereby having the 
   *  'record' leg record the speaker output generated by the 'play' leg.
   *
   *  Once the tones are recorded, the test moves onto a new phase.
   *  The tester then needs to disconnect the minijack-to-minijack connection from
   *  the speaker output, instead playing to an actual speaker that the tester
   *  can hear from.
   *  Next, the recorded tones recorded in the last phase by the buffer recorder 
   *  device driver are played back by the 'play' leg.
   *  And that concludes the test.
   *  
   *  Additional todo: Write the recorded audio to a file for further testing.
   */
   void testIOFreqRange(unsigned sampleRate, unsigned samplesPerFrame)
   {
      unsigned sineMagnitude = TEST_SAMPLE_DATA_MAGNITUDE;
      int frequencies[] = {1000, 2000, 4000, 8000, 16000, 32000, 24000, 48000};
      int numFreqs = sizeof(frequencies)/sizeof(int);
      int toneDurationMS = 1000; // duration of playing a tone.
      MpFrameTime bufRecDevBufferLen = toneDurationMS*numFreqs;

      MpMediaTask* pMediaTask = NULL;
      OsNotification *pTicker = NULL;
      MpFlowGraphBase* pFlowgraph = NULL;

      // Things we need for the general flowgraph
      MpInputDeviceManager* pInMgr = NULL;
      MpOutputDeviceManager* pOutMgr = NULL;

      // Specific devices needed for the microphone 'record' leg of the flowgraph.
      INPUT_DRIVER* pMicInDrv = NULL;
      MpInputDeviceHandle micDevHnd = 0;
      MprFromInputDevice* pRecRFromInDev = NULL;
      MprToOutputDevice* pRecRToOutputDev = NULL;
      MpodBufferRecorder* pRecBufOutDrv = NULL;
      MpOutputDeviceHandle recBufDevHnd = 0;

      // Things we need for the tone and speaker 'play' leg of the flowgraph
      MpSineWaveGeneratorDeviceDriver* pPlaySineInDrv = NULL;
      MpInputDeviceHandle playSineDevHnd = 0;
      MprFromInputDevice* pPlayRFromInDev = NULL;
      MprFromFile* pPlayRFromFile = NULL;
      MprMixer* pPlayRMixer = NULL;
      MprToOutputDevice* pPlayRToOutputDev = NULL;
      OUTPUT_DRIVER* pSpkrOutDrv = NULL;
      MpOutputDeviceHandle spkrDevHnd = 0;

      // Two lists of the device handles that are added to the device managers,
      // so they can properly be cleaned up from their managers.
      UtlSList inDevHandles;
      UtlSList outDevHandles;

      // Setup the initial bits of the flowgraph, allocating the flowgraph and 
      // media task (thereby filling in the passed in pointers, which should
      // always be null when passed in).
      setupWBMediaTask(sampleRate, samplesPerFrame,
                       pInMgr, pOutMgr,
                       pFlowgraph, pMediaTask);

      // Get media task ticker.
      pTicker = pMediaTask->getTickerNotification();

      // Create the 'record' leg of the flowgraph.
      // Create the input and output devices, and add the devices for the 'record' leg.
      pMicInDrv = new INPUT_DRIVER(INPUT_DRIVER_CONSTRUCTOR_PARAMS(*pInMgr));
      micDevHnd = pInMgr->addDevice(*pMicInDrv);
      CPPUNIT_ASSERT(micDevHnd > 0);
      inDevHandles.append(new UtlInt(micDevHnd));
      pRecBufOutDrv = new MpodBufferRecorder("MpodRecorder", bufRecDevBufferLen);
      recBufDevHnd = pOutMgr->addDevice(pRecBufOutDrv);
      CPPUNIT_ASSERT(recBufDevHnd > 0);
      outDevHandles.append(new UtlInt(recBufDevHnd));

      // Create the resources for the 'record' leg.
      pRecRFromInDev = 
         new MprFromInputDevice("FromInput-RecordMic", pInMgr, micDevHnd);
      pRecRToOutputDev =
         new MprToOutputDevice("ToOutput-RecordBuffer", pOutMgr, recBufDevHnd);


      // Create the 'play' leg of the flowgraph.
      // Create the input and output devices, and add the devices for the 'play' leg.
      pPlaySineInDrv = 
         new MpSineWaveGeneratorDeviceDriver("SineGeneratorDriver", *pInMgr, 
                                             sineMagnitude, 
                                             FREQ_TO_PERIODUS(frequencies[0]), 0);
      playSineDevHnd = pInMgr->addDevice(*pPlaySineInDrv);
      CPPUNIT_ASSERT(playSineDevHnd > 0);
      inDevHandles.append(new UtlInt(playSineDevHnd));
      pSpkrOutDrv = new OUTPUT_DRIVER(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      spkrDevHnd = pOutMgr->addDevice(pSpkrOutDrv);
      CPPUNIT_ASSERT(spkrDevHnd > 0);
      outDevHandles.append(new UtlInt(spkrDevHnd));

      // Create the resources for the 'play' leg.
      pPlayRFromInDev = 
         new MprFromInputDevice("FromInput-PlaySine", pInMgr, playSineDevHnd);
      pPlayRFromFile = new MprFromFile("FromFile");
      pPlayRMixer = new MprMixer("Play-Mixer", 2);
      pPlayRToOutputDev =
         new MprToOutputDevice("ToOutput-PlaySpkr", pOutMgr, spkrDevHnd);



      // Ok, now that everything is created, the flowgraph needs to be wired up
      // with those resources.
      // Add and connect up the 'record' leg of the flowgraph.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pRecRFromInDev));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pRecRToOutputDev));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pFlowgraph->addLink(*pRecRFromInDev, 0, 
                                               *pRecRToOutputDev, 0));

      // Add and connect up the 'play' leg of the flowgraph.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pPlayRFromInDev));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pPlayRFromFile));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pPlayRMixer));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         pFlowgraph->addResource(*pPlayRToOutputDev));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pFlowgraph->addLink(*pPlayRFromInDev, 0, 
                                               *pPlayRMixer, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pFlowgraph->addLink(*pPlayRFromFile, 0, 
                                               *pPlayRMixer, 1));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pFlowgraph->addLink(*pPlayRMixer, 0, 
                                               *pPlayRToOutputDev, 0));

      // Make sure that the mixer weights are set, and set equally.
      pPlayRMixer->setWeight(1, 0);
      pPlayRMixer->setWeight(1, 1);

      // Enable all of the devices..
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pInMgr->enableDevice(micDevHnd));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pOutMgr->enableDevice(recBufDevHnd));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pInMgr->enableDevice(playSineDevHnd));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pOutMgr->enableDevice(spkrDevHnd));

      // Set the flowgraph ticker source..
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pOutMgr->setFlowgraphTickerSource(spkrDevHnd, pTicker));

      // Now we enable the flowgraph..  Which should enable resources.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowgraph->enable());

      // Manage the flow graph so it flows, and finally start it flowing, 
      // and bring it into focus
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->manageFlowGraph(*pFlowgraph));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->startFlowGraph(*pFlowgraph));

      // Let the tone play for a few seconds..
      printf("Playing frequency: %d, ", frequencies[0]);
      fflush(stdout);
      int i;
      for (i = 1; i < numFreqs; i++)
      {
         OsTask::delay(toneDurationMS);
         printf("%d, ", frequencies[i]);
         fflush(stdout);
         pPlaySineInDrv->setTone(FREQ_TO_PERIODUS(frequencies[i]));
      }
      // Let the last tone play for it's duration.
      OsTask::delay(toneDurationMS);
      printf("\b\b  \nDone playing tones\n");

      // We should ensure that the entire capacity of the buffer provided in 
      // the buffer recorder has been filled.
      CPPUNIT_ASSERT_EQUAL(pRecBufOutDrv->getBufferLength(), pRecBufOutDrv->getBufferEnd());

      // Disable the sine wave generator device so that we don't keep hearing
      // it's generated tone, and instead hear the recorded buffer played back.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           MpResource::disable("FromInput-PlaySine", *pFlowgraph->getMsgQ()));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pInMgr->disableDevice(playSineDevHnd));



      //------------------------------------------------------------------------
      // Phase 2 of the test:  Play back the result we just recorded via the mic.

      printf("Please unplug the minijack-to-minijack cable from your speaker input now.\n"
             "We will proceed with the playback portion of the test in ");
      printCountdownDelay(3);

      const char* pRecBuf = (const char*)pRecBufOutDrv->getBufferData();
      unsigned pRecBufLen = pRecBufOutDrv->getBufferEnd() * sizeof(MpAudioSample);

      OsMsgDispatcher notfDisp;
      pFlowgraph->setNotificationDispatcher(&notfDisp);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           MprFromFile::playBuffer("FromFile", *pFlowgraph->getMsgQ(), 
                                                   pRecBuf, pRecBufLen, 
                                                   pFlowgraph->getSamplesPerSec(),
                                                   pFlowgraph->getSamplesPerSec(),
                                                   0, FALSE, NULL));

      int maxPlayLoops = (toneDurationMS * numFreqs)/100 + 50;
      for(i = 0; i < maxPlayLoops && notfDisp.numMsgs() < 2; i++)
      {
         OsTask::delay(100);
      }
      
      OsMsg* pMsg;
      CPPUNIT_ASSERT_EQUAL(2, notfDisp.numMsgs());
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*&)pMsg, OsTime(10)));
      CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_FROMFILE_STARTED, 
         (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());
      pMsg->releaseMsg();
      pMsg = NULL;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*&)pMsg, OsTime(10)));
      CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_FROMFILE_FINISHED, 
         (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());
      pMsg->releaseMsg();
      pMsg = NULL;

      // Shutdown the test.

      tearDownWBMediaTask(*pInMgr, inDevHandles, *pOutMgr, outDevHandles,
                          pFlowgraph, pMediaTask);

      inDevHandles.destroyAll();
      outDevHandles.destroyAll();

      // Delete all our data.
      // No need to delete resources in the flowgraph explicitly, as the 
      // flowgraph deletes any it has.
      delete pFlowgraph;
      delete pInMgr;
      delete pOutMgr;
      delete pRecBufOutDrv;
      delete pMicInDrv;
      delete pSpkrOutDrv;
      delete pPlaySineInDrv;
   }

   void testMixerWBHelper(unsigned sampleRate, unsigned samplesPerFrame)
   {
      size_t mixer_buffers_lenms = 8000; // 8000 millisecond buffer play length hardcoded.

      MpMediaTask* pMediaTask = NULL;
      OsNotification *pTicker = NULL;
      MpFlowGraphBase* pFlowgraph = NULL;

      // Things we need for the general flowgraph
      MpInputDeviceManager* pInMgr = NULL;
      MpOutputDeviceManager* pOutMgr = NULL;

      // create empty lists for input and output device handles since we had none.
      UtlSList inDevHandles, outDevHandles;

      // Declare resources, drivers, and driver handles.
      MprFromFile* pPlayRFromFile = NULL;
      MprFromFile* pPlayRFromFile2 = NULL;
      MprMixer* pPlayRMixer = NULL;
      MprToOutputDevice* pPlayRToOutputDev = NULL;
      OUTPUT_DRIVER* pSpkrOutDrv = NULL;
      MpOutputDeviceHandle spkrDevHnd = 0;

      // Setup the initial bits of the flowgraph, allocating the flowgraph and 
      // media task (thereby filling in the passed in pointers, which should
      // always be null when passed in).
      setupWBMediaTask(sampleRate, samplesPerFrame, 
                       pInMgr, pOutMgr, pFlowgraph, pMediaTask);

      // Get media task ticker.
      pTicker = pMediaTask->getTickerNotification();

#ifdef USE_CCPUNIT_EXCEPTIONS
      try {
#endif
         pSpkrOutDrv = new OUTPUT_DRIVER(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
         spkrDevHnd = pOutMgr->addDevice(pSpkrOutDrv);
         CPPUNIT_ASSERT(spkrDevHnd > 0);
         outDevHandles.append(new UtlInt(spkrDevHnd));

         pPlayRFromFile = new MprFromFile("FromFile-1");
         pPlayRFromFile2 = new MprFromFile("FromFile-2");
         pPlayRMixer = new MprMixer("Play-Mixer", 2);
         pPlayRToOutputDev =
            new MprToOutputDevice("ToOutput-PlaySpkr", 
                                  pOutMgr, spkrDevHnd);


         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                              pFlowgraph->addResource(*pPlayRFromFile));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                              pFlowgraph->addResource(*pPlayRFromFile2));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                              pFlowgraph->addResource(*pPlayRMixer));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                              pFlowgraph->addResource(*pPlayRToOutputDev));

         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              pFlowgraph->addLink(*pPlayRFromFile, 0, 
                                                  *pPlayRMixer, 0));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              pFlowgraph->addLink(*pPlayRFromFile2, 0, 
                                                  *pPlayRMixer, 1));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              pFlowgraph->addLink(*pPlayRMixer, 0, 
                                                  *pPlayRToOutputDev, 0));

         // Make sure that the mixer weights are set, and set equally.
         pPlayRMixer->setWeight(1, 0);
         pPlayRMixer->setWeight(1, 1);

         // Enable the speaker device.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pOutMgr->enableDevice(spkrDevHnd));

         // Set the flowgraph ticker source..
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                              pOutMgr->setFlowgraphTickerSource(spkrDevHnd, pTicker));

         // Now we enable the flowgraph..  Which should enable resources.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowgraph->enable());

         // Manage the flow graph so it flows, and finally start it flowing, 
         // and bring it into focus
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->manageFlowGraph(*pFlowgraph));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->startFlowGraph(*pFlowgraph));

         // Provide a notification dispatcher so we can get notifications.
         OsMsgDispatcher notfDisp;
         pFlowgraph->setNotificationDispatcher(&notfDisp);

         // Now we load up the buffers.
         MprFromFile::playBuffer("FromFile-1", *pFlowgraph->getMsgQ(),
                                 (const char*)WBMixerTestTones,
                                 WBMixerTestTones_in_bytes,
                                 48000, pFlowgraph->getSamplesPerSec(),
                                 0, FALSE, NULL);

         MprFromFile::playBuffer("FromFile-2", *pFlowgraph->getMsgQ(),
                                 (const char*)WBMixerTestReversedTones,
                                 WBMixerTestReversedTones_in_bytes,
                                 48000, pFlowgraph->getSamplesPerSec(),
                                 0, FALSE, NULL);

         // Delay until tones are finished playing, adding in an extra five
         // seconds if the notifications are late.
         int notfsToExpect = 4;
         size_t maxPlayLoops = mixer_buffers_lenms/100 + 50;
         size_t i;
         for(i = 0; i < maxPlayLoops && notfDisp.numMsgs() < notfsToExpect; i++)
         {
            OsTask::delay(100);
         }

         // Now grab the actual messages out of the queue.
         OsMsg* pMsg;
         CPPUNIT_ASSERT_EQUAL(notfsToExpect, notfDisp.numMsgs());
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*&)pMsg, OsTime(10)));
         CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_FROMFILE_STARTED, 
                              (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*&)pMsg, OsTime(10)));
         CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_FROMFILE_STARTED, 
                              (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*&)pMsg, OsTime(10)));
         CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_FROMFILE_FINISHED, 
                              (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*&)pMsg, OsTime(10)));
         CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_FROMFILE_FINISHED, 
                              (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());
#ifdef USE_CCPUNIT_EXCEPTIONS
      } catch(CppUnit::Exception& e) {
         // Shutdown the test.
         tearDownWBMediaTask(*pInMgr, inDevHandles, *pOutMgr, outDevHandles,
                             pFlowgraph, pMediaTask);

         // Delete all our data.
         // No need to delete resources in the flowgraph explicitly, as the 
         // flowgraph deletes any it has.
         delete pFlowgraph;
         delete pInMgr;
         delete pOutMgr;
         delete pSpkrOutDrv;

         throw(e);
      }
#endif

      // Shutdown the test.
      tearDownWBMediaTask(*pInMgr, inDevHandles, *pOutMgr, outDevHandles,
                          pFlowgraph, pMediaTask);

      // Delete all our data.
      // No need to delete resources in the flowgraph explicitly, as the 
      // flowgraph deletes any it has.
      delete pFlowgraph;
      delete pInMgr;
      delete pOutMgr;
      delete pSpkrOutDrv;
   }

protected:

   const static int sSampleRates[];
   const static int sNumRates;

};

CPPUNIT_TEST_SUITE_REGISTRATION(WBInputOutputDeviceTest);

const int WBInputOutputDeviceTest::sSampleRates[] = {8000, 16000, 32000, 48000};
const int WBInputOutputDeviceTest::sNumRates = sizeof(WBInputOutputDeviceTest::sSampleRates)/sizeof(int);
